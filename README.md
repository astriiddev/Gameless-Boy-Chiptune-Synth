# ArduiNES-Synth
MIDI Controlled wavetable synth based around NES wavetables for Arduino UNO

  Inspired by trash80's mGB for arduinoboy and using Adventure Kid's NES wavetables, I decided to create a standalone
chiptune synthesizer with an Arduino UNO. When I set out to begin this project, I had neither a gameboy nor an everdrive
to run LSDJ or mGB so I wanted to build this to supplant my lack of gear. Now, over a year later, even though I now have
both an everdrive and quite a few gameboys, I still wanted a standalone chiptune synth that was lowcost and compact and
so I decided to finish the project I had started.

  As the ArduiNES is inspired by mGB, I decided to keep the MIDI channel layout the same. MIDI channels 1 and 2 are both 
pulse wave channels with varialbe pulse width. These pulse waves can be set to 12.5%, 25%, 50% (square wave--default on
boot up), and 75% duty cycles. CC knob 48 is the the default to change the pulse width for each channel. MIDI channel 3
is triangle wave while channel 4 is a noise channel. As it is on mGB, MIDI channel 5 allows you to play the triangle wave
and pulse waves together as three note polyphony.

  ArduiNES is complete with pitch vibrato and pitch bend. Vibrato intensity is controlled by the mod wheel while its speed
is controlled by CC knob 47 by default. Pitch bend is controlled by, obviously, the pitch bend wheel and have an octave up
and octave down range. When in polyphony mode, pitch bend will only bend the last note that was played. This is an issue
that I hope to rectify in later releases.

  I hope to eventually build this into a full enclosure, complete with push-buttons to change the duty cycles, knobs for
ADSR controls, and a preamp. But for now, the code is designed for simple MIDI-in and audio out circuitry.

# Installation
  This project is built on the Arduino MIDI and Mozzi libraries and they must be already added to your Arduino IDE library. 
Find them here:
      https://github.com/FortySevenEffects/arduino_midi_library/releases/tag/5.0.2
      https://github.com/sensorium/Mozzi
  
  It also uses custom wavetable files for Mozzi that I made using Adventure Kid's NES waveforms. Download them from the
tables folder in this project and place them in the Mozzi tables folder (should be Arduino\libraries\Mozzi-master\tables
on your computer).

# Special Thanks
Special thanks to trash80 for their inspiring mGB and ArduinoBoy projects and to Adventure Kid for their NES waveforms.

Also special thanks to jidagraphy for their Mozzi Poly Synth code which helped me create the three note polyphony channel 
within the context of the Mozzi library. Without them, this project could not have been realized.
