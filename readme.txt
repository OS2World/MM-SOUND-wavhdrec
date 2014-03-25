                            July 8, 1998

I have modified the code, fixed a couple of minor bugs, changed the default
output to 16 bits, made the output RIFF WAVE compatible and added the
ability to choose the audio device (0=default, 1=first, 2=second).  As
usual, use /h for help on the applet.

These applet work on any audio card.

Samuel Audet <guardia@cam.org>

                            October 13th 1996

Play.exe & record.exe are the two apps I used to test the recording
capabilities of the current GUS drivers as well as Alaw, Ulaw & ADPCM
support.
Since the standard OS/2 digital audio app is quite limited I thought
you might like those two little apps to test the limits of your
GUS Max, PnP or Interwave card.

Both are very basic. Record saves the recording settings right in the
beginning of the data file and saves every buffer it gets from DART
to this file.
Play reads the recording settings and plays the following audio data.

The source of the two apps is included in the Source directory.

Enjoy,


Sander
PS: To use play & record you'll need the DART dlls. (if you're not using
    OS/2 Warp 4)
    They can be downloaded from the Ultimate SoundPage:
	http://wvnvaxa.wvnet.edu/~hrieke/us.html


******************** Command line syntax of record *****************************
Incorrect command line syntax. Usage: RECORD [/options] filename
        /8     - 8 bit recording
	/f:xxxxx    - frequency (2000hz up to 48000 hz)
	/b:xxxxx    - buffersize in bytes
        /u          - uLaw (8 bits)
        /a          - aLaw (8 bits)
        /d          - IMA ADPCM hardware compression (16 bits; 4:1)
        /m          - mono
        /h          - displays this text
    /i:x        - device number, default=0
        filename    - output filename 
Defaults: 44100 hz PCM (uncompressed)
          16 bits stereo recording
          8192 bytes buffer
          Record from Line In (change with UltiMIX)
********************** Command line syntax of play *****************************
Incorrect command line syntax. Usage: PLAY [/options] filename
        /h          - displays this text
	/b:xxxxx    - buffersize in bytes (default 8192 bytes)
        /i:x        - device number, default=0
