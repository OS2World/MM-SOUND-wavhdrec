#define INCL_KBD
#define INCL_DOSFILEMGR
#define INCL_DOSDEVICES
#define INCL_DOSEXCEPTIONS   /* Exception values */
#define INCL_DOSDATETIME
#define INCL_DOSPROCESS
#define INCL_BASE
#define INCL_OS2MM
#include <os2.h>
#include <os2me.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <process.h>
#include <iostream.h>
#include "record.h"

char syntax[] = "Incorrect command line syntax. Usage: RECORD [/options] filename\n \
        /8     - 8 bit recording\n\
	/f:xxxxx 	- frequency (2000hz up to 48000 hz)\n\
	/b:xxxxx 	- buffersize in bytes\n\
		  /u			  - uLaw (8 bits)\n\
		  /a			  - aLaw (8 bits)\n\
		  /d			  - IMA ADPCM hardware compression (16 bits; 4:1)\n\
		  /m			  - mono\n\
		  /h			  - displays this text\n\
        /i:x        - device number, default=0\n\
		  filename	  - output filename \n\
Defaults: 44100 hz PCM (uncompressed)\n\
			 16 bits stereo recording\n\
			 8192 bytes buffer\n\
			 Record from Line In (change with UltiMIX)\n";

char wrongpar[] = "Unknown parameter.\n";

char *filename 	 = "record.wav";
int ulBuffersize	 = 8192;
USHORT CurPosX, CurPosY;
REC_STRUCT RecSettings;
ULONG input;
USHORT device;

/*************************************************************************/
/*************************************************************************/
int main(int argc, char *argv[])
{
  KBDKEYINFO key;
  int i;

  RecSettings.bits	 = 16;
  RecSettings.rate	 = 44100;
  RecSettings.format  = MCI_WAVE_FORMAT_PCM;
  RecSettings.numchan = 2;
  device 				 = 0;

  for(i=1;i<argc;i++) {
			 if(argv[i][0] == '/' || argv[i][0] == '-') {
					 switch(argv[i][1]) {
		  case 'B':
		  case 'b':
								ulBuffersize = atoi(&argv[i][3]);
			if(ulBuffersize < 4096 || ulBuffersize > 64*1024)
				ulBuffersize = 8192;
			break;
		  case 'F':
		  case 'f':
								RecSettings.rate = atoi(&argv[i][3]);
			if(RecSettings.rate < 2000 || RecSettings.rate > 48000) {
				RecSettings.rate = 44100;
			}
			break;
		  case 'h':
		  case 'H':
			cout << syntax;
			return(FALSE);
		  case 'u':
		  case 'U':
			RecSettings.format = MCI_WAVE_FORMAT_IBM_MULAW;
			break;
		  case 'a':
		  case 'A':
				  RecSettings.format = MCI_WAVE_FORMAT_IBM_ALAW;
			break;
		  case 'd':
		  case 'D':
				  RecSettings.format = MCI_WAVE_FORMAT_AVC_ADPCM;
			break;
        case '8':
              RecSettings.bits = 8;
			break;
		  case 'm':
		  case 'M':
								RecSettings.numchan = 1;
			break;
		  case 'i':
		  case 'I':
								device = atoi(&argv[i][3]);
			break;

		default:
								cout << wrongpar;
			return(FALSE);
			 }
	}
	else {
		filename = &argv[i][0];
	}
  }
  //force correct bits per sample
  switch(RecSettings.format) {
	case MCI_WAVE_FORMAT_IBM_MULAW:
	case MCI_WAVE_FORMAT_IBM_ALAW:
		RecSettings.bits = 8;
		break;
	case MCI_WAVE_FORMAT_AVC_ADPCM:
		RecSettings.bits = 16;
		break;
  }

  if(!OpenDevice())	return(TRUE);
  cout << "DART Record applet for the Manley Gravis UltraSound MMPM/2 Drivers" << endl;
  cout << "Copyright (1996) Sander van Leeuwen" << endl;
  cout << "Modifications: Copyright (1998) Samuel Audet"  << endl;
  cout << "Press Escape to exit" << endl << endl;
  cout << "Recording to file: " << filename << endl;
  VioGetCurPos(&CurPosX, &CurPosY, 0);

  StartRecording();
  while(TRUE) {
			 key.chChar = 0;
			 DosSleep(50); 		 //1 seconde delay
			 KbdCharIn(&key, 1, 0); 		 //don't wait for key

	  if(key.chChar == 27)	break;
  }
  CloseDevice();
  return(TRUE);
}
