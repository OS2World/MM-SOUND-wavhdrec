#define INCL_KBD
#define INCL_DOSFILEMGR
#define INCL_DOSDEVICES
#define INCL_DOSEXCEPTIONS   /* Exception values */
#define INCL_DOSDATETIME
#define INCL_DOSPROCESS
#define INCL_BASE
#include <os2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <process.h>
#include <iostream.h>
#include "play.h"

char syntax[] = "Incorrect command line syntax. Usage: PLAY [/options] filename\n\
		  /h			  - displays this text\n\
   /b:xxxxx    - buffersize in bytes (default 8192 bytes)\n\
        /i:x        - device number, default=0\n";

char wrongpar[] = "Unknown parameter.\n";

char *filename = "record.wav";
USHORT CurPosX, CurPosY;
int ulBuffersize	 = 8192;
REC_STRUCT RecSettings;
USHORT device;

/*************************************************************************/
/*************************************************************************/
int main(int argc, char *argv[])
{
  KBDKEYINFO key;
  int i;

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
		  case 'h':
		  case 'H':
			cout << syntax;
			return(FALSE);
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

  if(!OpenDevice())	return(FALSE);

  cout << "DART Playback applet for the Manley Gravis UltraSound MMPM/2 Drivers" << endl;
  cout << "Copyright (1996) Sander van Leeuwen" << endl;
  cout << "Modifications: Copyright (1998) Samuel Audet"  << endl;
  cout << "Press Escape to exit" << endl << endl;
  cout << "Playing file: " << filename << endl;

  StartPlayback();
  while(TRUE) {
			 key.chChar = 0;
			 DosSleep(50); 		 //1 seconde delay
			 KbdCharIn(&key, 1, 0); 		 //don't wait for key

	  if(key.chChar == 27)	break;
  }
  CloseDevice();
  return(TRUE);
}
