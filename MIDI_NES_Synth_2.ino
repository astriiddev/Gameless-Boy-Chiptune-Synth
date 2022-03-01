/*   MIDI Controlled wavetable synth based around NES wavetables for Arduino UNO
 *   
 *   MIDI channels 1 and 2 are pulse wave channels with four duty cycles
 *   MIDI CC 47 changes pulse wave duty cycle between 12.5%, 25%, 50% (square), and 75%
 *   MIDI channel 3 is triangle wave while MIDI channel 4 is noise
 *   MIDI channel 5 uses the two pulse wave and one triangle wav to create 3 note polyphony for chords
 *   
 *   MIDI CC 48 changes rate of vibrato LFO
 *   Mod wheel changes vibrato LFO intensity
 *   
 *   Audio output on Arduino UNO Pin 9 
 *   
 *   Special thanks to Adventure Kid for NES wavetables and jidagraphy for polyphony within Mozzi library
 *   
 *   Arduino MIDI Library:
 *      https://github.com/FortySevenEffects/arduino_midi_library/releases/tag/5.0.2
 *      
 *   Arduino Mozzi Library:
 *      https://github.com/sensorium/Mozzi
 *      
 *   MIDI input circuit:      
 *      https://hackaday.com/tag/6n137/
 * 
 */
#include <MozziGuts.h>
#include <mozzi_midi.h>
#include <ADSR.h>
#include <MIDI.h>
#include <Oscil.h>
#include <tables/square_nes_int8.h>                                     //NES pulse wavetable at 50% duty cycle (square wave)
#include <tables/pulse_12_nes_int8.h>                                   //NES pulse wavetable at 12.5% duty cycle
#include <tables/pulse_25_nes_int8.h>                                   //NES pulse wavetable at 25% duty cycle
#include <tables/pulse_75_nes_int8.h>                                   //NES pulse wavetable at 75% duty cycle
#include <tables/triangle_nes_int8.h>                                   //NES triangle wavetable
#include <tables/noise_nes_int8.h>                                      //NES noise wavetable
#include <tables/sin2048_int8.h>                                        //sine wavetable

#define CONTROL_RATE 256                                                //increase for faster MIDI reading, decrease for better audio stability (only use powers of 2)
#define OSC_NUM_CELLS 512                                               //sets pulse wavetables and triangle wavetable to 512 samples
#define LED 13                                                          //uses LED on Arduino to verify if MIDI signal is being received
#define MAX_POLY 3                                                      //sets max polyphony to three voices

MIDI_CREATE_DEFAULT_INSTANCE();

byte intensity, nt, vibrato, ch, poly, pu_width_1=2, pu_width_2=2;
float freq, bend=0;

Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aVibe(SIN2048_DATA);              //generates sine wave for vibrato
Oscil <NOISE_NES_NUM_CELLS, AUDIO_RATE> aNoise(NOISE_NES_DATA);         //noise generator
ADSR<CONTROL_RATE, AUDIO_RATE> envNoise;                                //envelope generator for noise

struct Voice{                                                           //generates oscillators for pulse waves and triangle wave
  Oscil<OSC_NUM_CELLS, AUDIO_RATE> osc;                                 // audio oscillator
  ADSR<CONTROL_RATE, AUDIO_RATE> env;                                   // envelope generator
  byte note = 0;
  byte velocity = 0;
};

Voice voices[MAX_POLY];

void HandleNoteOn(byte channel, byte note, byte velocity) {
  nt=note;
  freq=mtof(float(note-12));                                            //tunes MIDI note to frequency
  ch=channel;
  if(ch==1){                                                            //channel 1 pulse wave MIDI read
    voices[0].osc.setFreq(freq);
    voices[0].env.noteOn();
    voices[0].note = note;
    voices[0].velocity = velocity;}
  if(ch==2){                                                            //channel 2 pulse wave MIDI read
    voices[1].osc.setFreq(freq);
    voices[1].env.noteOn();
    voices[1].note = note;
    voices[1].velocity = velocity;}
  if(ch==3){                                                            //channel 3 triangle wave MIDI read
    voices[2].osc.setFreq(freq);
    voices[2].env.noteOn();
    voices[2].note = note;
    voices[2].velocity = velocity;}
  if(ch==4){                                                            //channel 4 noise MIDI read
    aNoise.setFreq(freq/64);
    envNoise.noteOn();}
    
  if(ch==5){                                                            //channel 5 polyphony
  if (velocity > 0) {

    int activeVoice = 0;
    int voiceToSteal = 0;
    int lowestVelocity = 128;

    for (unsigned int i = 0 ; i < MAX_POLY; i++) {
      if(!voices[i].env.playing()){                                     //checks if oscillator voice is currently playing
        voices[i].env.noteOff();
        voices[i].osc.setFreq(freq);                                    //sets oscillator voice to note frequency
        voices[i].env.noteOn();
        voices[i].note = note;
        voices[i].velocity = velocity;
        break;
      }else{
        activeVoice++;                                                  //increments to next voice
        if(lowestVelocity >= voices[i].velocity){
          lowestVelocity = voices[i].velocity;
          voiceToSteal = i;
        }
      }
    }

    if(activeVoice == MAX_POLY){                                        //checks if 3 note polyphony has been reached
        voices[voiceToSteal].env.noteOff();
        voices[voiceToSteal].osc.setFreq(freq);                         //sets frequency of free voice or first voice if no voice is free
        voices[voiceToSteal].env.noteOn();
        voices[voiceToSteal].note = note;
        voices[voiceToSteal].velocity = velocity;
    }
  }
  }
  digitalWrite(LED, HIGH);                                              //turns on LED if MIDI Note On is read
}

void HandleNoteOff(byte channel, byte note, byte velocity) {
  byte handsOffChecker = 0;
  if (ch==4){                                                           //note off for noise channel
    if (note==nt){
  envNoise.noteOff();}
  }
  for (unsigned int i = 0; i < MAX_POLY; i++) {                         //note off for melodic channels
    if (note == voices[i].note) {
      voices[i].env.noteOff();
      voices[i].note = 0;
      voices[i].velocity = 0;
    }
    handsOffChecker += voices[i].note;
  }

  if (handsOffChecker == 0) {                                           //turns off LED if no Note On signals are read
    digitalWrite(LED, LOW);
  }
}

void HandleControlChange(byte command, byte data1, byte data2){
  int rate = 7.f;                                                       //default frequency of vibrato LFO
  if(data1==48){                                                        //sets LFO rate to MIDI CC 48
  rate = map(data2, 0, 127, 1, 20);}                                    //maps LFO frequency range to MIDI CC values
  aVibe.setFreq(rate);
  if(data1 == 01){                                                      //maps mod wheel to vibrato intensity
    intensity = 2*data2;
  }
  if (data1==47){                                                       //sets pulse width change to MIDI CC 47
    if (data2<31){                                                      //-v-maps CC value ranges to pulse wavetables-v-
      if(command==1){
      pu_width_1=0;}
      if(command==2){
      pu_width_2=0;}
    }
    if (data2>31 && data2<63){
      if(command==1){
      pu_width_1=1;}
      if(command==2){
      pu_width_2=1;}
    }
    if (data2>63 && data2<95){
      if(command==1){
      pu_width_1=2;}
      if(command==2){
      pu_width_2=2;}
    }
    if (data2>95){
      if(command==1){
      pu_width_1=3;}
      if(command==2){
      pu_width_2=3;}
    }
  }                                                                   //-^-maps CC value ranges to pulse wavetables-^-
}

void HandlePitchBend(byte channel, int value){
  if(value>65){                                                             //maps pitch bend up
    bend = map(value, 0, 8191, 0, freq);}
  if(value<63){                                                             //maps pitch bend down
    bend = map(value, -8192, 0, -(freq/2), 0);}
  if(value==64){                                                            //maps pitch bend center
    bend = 0;}
  for (unsigned int i = 0; i < MAX_POLY; i++) {
    if (nt == voices[i].note) {                                             //applies pitch bend to last note played
      voices[i].osc.setFreq(freq+bend);}
  }
}

void setup() {
  pinMode(LED, OUTPUT);

  MIDI.setHandleNoteOn(HandleNoteOn);
  MIDI.setHandleNoteOff(HandleNoteOff);
  MIDI.setHandleControlChange(HandleControlChange);
  MIDI.setHandlePitchBend(HandlePitchBend);
  MIDI.begin(MIDI_CHANNEL_OMNI);

  for(unsigned int i = 0; i < MAX_POLY; i++){
    voices[i].env.setTimes(22,8000,8000,300);                             //sets ADSR levels for melodic voices
    voices[i].env.setADLevels(140,42);}                                   //sets volume levels for melodic voices
  envNoise.setTimes(22,8000,8000,300);                                    //sets ADSR levels for noise
  envNoise.setADLevels(70,42);                                            //sets volume levels for noise

  startMozzi(CONTROL_RATE);
}

void updateControl(){
  MIDI.read();
  for(unsigned int i = 0; i < MAX_POLY; i++){                             //updates envelope generators
    voices[i].env.update();}
  envNoise.update();
  if (pu_width_1==0){                                                     //-v-changes pulse wave wavetables for first two voices-v-
  voices[0].osc.setTable(PULSE_12_NES_DATA);}
  if (pu_width_1==1){
  voices[0].osc.setTable(PULSE_25_NES_DATA);}
  if (pu_width_1==2){
  voices[0].osc.setTable(SQUARE_NES_DATA);}
  if (pu_width_1==3){
  voices[0].osc.setTable(PULSE_75_NES_DATA);}
  if (pu_width_2==0){
  voices[1].osc.setTable(PULSE_12_NES_DATA);}
  if (pu_width_2==1){
  voices[1].osc.setTable(PULSE_25_NES_DATA);}
  if (pu_width_2==2){
  voices[1].osc.setTable(SQUARE_NES_DATA);}
  if (pu_width_2==3){
  voices[1].osc.setTable(PULSE_75_NES_DATA);}                              //-^-changes pulse wave wavetables for first two voices-^-
  voices[2].osc.setTable(TRIANGLE_NES_DATA); 
}

int updateAudio(){
  int currentSample=0;
  Q15n16 vibrato = (Q15n16) intensity * aVibe.next();                     //~vibrato math~
  for(unsigned int i = 0; i < MAX_POLY; i++){
    currentSample += ((voices[i].env.next() * voices[i].osc.phMod(vibrato)));     //applies vibrato to voices
  }
  return (int) (currentSample) + (envNoise.next()*aNoise.next())>>8;      //output audio for voices and noise
}


void loop() {
  audioHook();                                                            //required
}
