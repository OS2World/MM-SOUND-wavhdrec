#define INCL_DOS
#define INCL_VIO
#define INCL_OS2MM
#include <os2.h>
#include <os2me.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream.h>
#include "record.h"
#include "wav.hpp"

static LONG APIENTRY MixHandler(ULONG, PMCI_MIX_BUFFER, ULONG);

MCI_MIX_BUFFER     MixBuffers[NUM_BUFFERS]; /* Device buffers          */
MCI_MIXSETUP_PARMS MixSetupParms;       /* Mixer parameters        */
MCI_BUFFER_PARMS   BufferParms;               /* Device buffer parms     */
MCI_PLAY_PARMS PlayParams;
int ulNumBuffers, BufNr = 0;
USHORT DeviceId;
//HFILE modin = 0;
WAV wavfile;
int BytesRecorded = 0;

/******************************************************************************/
/******************************************************************************/
int OpenDevice()
{
 CHAR  achBuffer[CCHMAXPATH] = "";
 APIRET rc;
 MCI_AMP_OPEN_PARMS   AmpOpenParms;
 MCI_CONNECTOR_PARMS  ConnectorParms;
 MCI_AMP_SET_PARMS    AmpSetParms ;
 MCI_GENERIC_PARMS    GenericParms;
 int i;
 ULONG Action;
 ULONG Wrote;

   // Setup the open structure, pass the playlist and tell MCI_OPEN to use it
   memset(&AmpOpenParms,0,sizeof(AmpOpenParms));

   AmpOpenParms.usDeviceID = ( USHORT ) 0;
//   AmpOpenParms.pszDeviceType = ( PSZ ) MCI_DEVTYPE_AUDIO_AMPMIX;
   AmpOpenParms.pszDeviceType = (PSZ) MAKEULONG(MCI_DEVTYPE_AUDIO_AMPMIX, device);

   rc = mciSendCommand(0,
                       MCI_OPEN,
                       MCI_WAIT | MCI_OPEN_TYPE_ID | MCI_OPEN_SHAREABLE,
                       ( PVOID ) &AmpOpenParms,
                       0 );

   if (ULONG_LOWD(rc) != MCIERR_SUCCESS) {
        mciGetErrorString( rc, (PSZ)achBuffer,   sizeof( achBuffer));

   cout << "Error #" << ULONG_LOWD(rc) << ": " << achBuffer << endl;
	goto Init_Err;
   }
   DeviceId = AmpOpenParms.usDeviceID;

   //Grab exclusive rights to device instance (NOT entire device)
   GenericParms.hwndCallback = 0;	//Not needed, so set to 0
   rc = mciSendCommand(DeviceId, MCI_ACQUIREDEVICE, MCI_EXCLUSIVE_INSTANCE,
		       (PVOID)&GenericParms, 0);
   if (ULONG_LOWD(rc) != MCIERR_SUCCESS) {
        mciGetErrorString( rc, (PSZ)achBuffer,   sizeof( achBuffer));

   cout << "Error #" << ULONG_LOWD(rc) << ": " << achBuffer << endl;
   goto Init_Err;
   }

   /* Set the MixSetupParms data structure to match the loaded file.
    * This is a global that is used to setup the mixer.
    */
   memset( &MixSetupParms, 0, sizeof( MCI_MIXSETUP_PARMS ) );

   MixSetupParms.ulBitsPerSample = RecSettings.bits;
   MixSetupParms.ulSamplesPerSec = RecSettings.rate;
   MixSetupParms.ulFormatTag = RecSettings.format;
   MixSetupParms.ulChannels = RecSettings.numchan;

   /* Setup the mixer for playback of wave data
    */
   MixSetupParms.ulFormatMode = MCI_RECORD;
   MixSetupParms.ulDeviceType = MCI_DEVTYPE_WAVEFORM_AUDIO;
   MixSetupParms.pmixEvent    = MixHandler;

   rc = mciSendCommand( DeviceId,
                        MCI_MIXSETUP,
                        MCI_WAIT | MCI_MIXSETUP_INIT,
                        ( PVOID ) &MixSetupParms,
                        0 );

   if ( rc != MCIERR_SUCCESS ) {
        mciGetErrorString( rc, (PSZ)achBuffer,   sizeof( achBuffer));

   cout << "Error #" << hex << rc << ": " << achBuffer << endl;
   goto Init_Err;
   }

   /* Use the suggested buffer size provide by the mixer device
    * and the size of the audio file to calculate the required
    * number of Amp-Mixer buffers.
    */
   ulNumBuffers = NUM_BUFFERS;

   /*
    * Set up the BufferParms data structure and allocate
    * device buffers from the Amp-Mixer
    */
   MixSetupParms.ulBufferSize = ulBuffersize;
	
   BufferParms.ulNumBuffers = ulNumBuffers;
   BufferParms.ulBufferSize = MixSetupParms.ulBufferSize;
   BufferParms.pBufList = MixBuffers;


   for(i=0;i<NUM_BUFFERS;i++) {
	MixBuffers[i].ulUserParm = i;	//nr of buffer (used in mixer.c)
   }

   rc = mciSendCommand( DeviceId,
                        MCI_BUFFER,
                        MCI_WAIT | MCI_ALLOCATE_MEMORY,
                        ( PVOID ) &BufferParms,
                        0 );

   if ( ULONG_LOWD( rc) != MCIERR_SUCCESS ) {
	goto Init_Err;
   }

   rc = wavfile.openWAV(filename, CREATE, RecSettings.rate,
              RecSettings.numchan, RecSettings.bits, RecSettings.format);
   if(rc)     goto Init_Err;

#if 0
   rc = DosOpen(filename,       	/* file name from Open dialog */
  	      &modin,	      		/* file handle returned */
	      &Action,
	      0L,
	      FILE_NORMAL,
              OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_REPLACE_IF_EXISTS,
              OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE,
	      (PEAOP2)NULL);
   if(rc)     goto Init_Err;
   //save settings to file
   DosWrite(modin, (char *)&RecSettings, sizeof(RecSettings), &Wrote);
#endif
   
   /* Set the connector to 'line in'
    */
   memset( &ConnectorParms, '\0', sizeof( MCI_CONNECTOR_PARMS ) );
   ConnectorParms.ulConnectorType = MCI_LINE_IN_CONNECTOR;

   rc = mciSendCommand( DeviceId,
                       MCI_CONNECTOR,
                       MCI_WAIT |
                       MCI_ENABLE_CONNECTOR |
                       MCI_CONNECTOR_TYPE,
                       ( PVOID ) &ConnectorParms,
                       0 );

   /* Allow the user to hear what is being recorded
    * by turning the monitor on
    */
   memset( &AmpSetParms, '\0', sizeof( MCI_AMP_SET_PARMS ) );
   AmpSetParms.ulItem = MCI_AMP_SET_MONITOR;
   rc = mciSendCommand( DeviceId,
                       MCI_SET,
                       MCI_WAIT | MCI_SET_ON | MCI_SET_ITEM,
                       ( PVOID ) &AmpSetParms,
                       0 );

   return(TRUE);
Init_Err:
   return(FALSE);
}
/******************************************************************************/
/******************************************************************************/
void CloseDevice()
{
   MCI_GENERIC_PARMS    GenericParms;

   // Generic parameters
   GenericParms.hwndCallback = 0;	//hwndFrame

   // Stop the playback.
   mciSendCommand(DeviceId,MCI_STOP,MCI_WAIT,(PVOID)&GenericParms,0);

   mciSendCommand( DeviceId,
                   MCI_BUFFER,
                   MCI_WAIT | MCI_DEALLOCATE_MEMORY,
                   ( PVOID )&BufferParms,
                   0 );

   // Generic parameters
   GenericParms.hwndCallback = 0;	//hwndFrame

   // Close the device
   mciSendCommand(DeviceId,MCI_CLOSE,MCI_WAIT,(PVOID)&GenericParms,0);
   wavfile.closeWAV();
//   DosClose(modin);
}
//******************************************************************************
//******************************************************************************
int StartRecording()
{
  cout << "Record mode: " << RecSettings.rate << " hz "<< RecSettings.bits << " bits";
  if(RecSettings.numchan == 1) cout << " mono" << endl;
  else 			       cout << " stereo" << endl;
  cout << "Data Format: ";
  switch(RecSettings.format) {
  	case MCI_WAVE_FORMAT_IBM_MULAW:
		cout << "uLaw" << endl;
		break;
	case MCI_WAVE_FORMAT_IBM_ALAW:
		cout << "aLaw" << endl;
		break;
	case MCI_WAVE_FORMAT_AVC_ADPCM:
		cout << "IMA ADPCM" << endl;
		break;
	case MCI_WAVE_FORMAT_PCM:
		cout << "PCM" << endl;
		break;
  }
//  cout << "Recorded bytes: ";
  cout << "Recorded bytes: "<<endl;
  VioGetCurPos(&CurPosX, &CurPosY, 0);

  CurPosX--;
  CurPosY = 16;

  MixSetupParms.pmixRead( MixSetupParms.ulMixHandle,
                          &MixBuffers[0],
                          ulNumBuffers);
  return(TRUE);
}
/******************************************************************************/
/******************************************************************************/
#ifdef __WATCOMC__
 #pragma off (unreferenced)
#endif
static LONG APIENTRY MixHandler(ULONG ulStatus, PMCI_MIX_BUFFER  pBuffer,
                                      ULONG ulFlags)
#ifdef __WATCOMC__
 #pragma on (unreferenced)
#endif
{
ULONG Wrote;
char tmptxt[12];

      //Transfer buffer to DART
      if(pBuffer->pBuffer) {
     Wrote = wavfile.writeData((char*)pBuffer->pBuffer, pBuffer->ulBufferLength);
//     DosWrite(modin, pBuffer->pBuffer, pBuffer->ulBufferLength, &Wrote);

	  BytesRecorded += Wrote;
	  sprintf(tmptxt, "%9d ", BytesRecorded); 
	  VioWrtCharStr(&tmptxt[0], (USHORT)strlen(tmptxt), CurPosX, CurPosY, 0);
      }

      MixSetupParms.pmixRead( MixSetupParms.ulMixHandle,
                              &MixBuffers[BufNr],
                              1);
      if(BufNr == NUM_BUFFERS-1) BufNr = 0;
      else           	         BufNr++;
      return(TRUE);
}
/******************************************************************************/
/******************************************************************************/
