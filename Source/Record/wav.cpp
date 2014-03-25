/* Geez, why are WAV RIFF headers are so secret?  I got something together,
   but wow...  anyway, I hope someone will find this useful.
   - Samuel Audet <guardia@cam.org> */

/* RIFF stuff (C) 1998 Samuel Audet <guardia@cam.org> */

#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <sys\stat.h>
#include <sys\types.h>
#include <memory.h>

#include "wav.hpp"

WAV::WAV()
{
    hfile = 0;
    memset(&header,0,sizeof(header));
}

WAV::~WAV()
{
   if(hfile)
      close(hfile);
}

int WAV::openWAV(char *filename, MODE mode, int &samplerate,
              int &channels, int &bits, int &format)
{
   switch(mode)
   {
      case READ:
         hfile = open(filename,O_RDONLY | O_BINARY,S_IWRITE);
         if(!hfile) return errno;
         if(!readHeader()) return NO_WAV_FILE;
         bits = header.WAVE.fmt.BitsPerSample;
         channels = header.WAVE.fmt.Channels;
         samplerate = header.WAVE.fmt.SamplesPerSec;
         format = header.WAVE.fmt.FormatTag;
         break;

      case CREATE:
         hfile = open(filename,O_CREAT | O_TRUNC | O_RDWR | O_BINARY, S_IWRITE);
         if(!hfile) return errno;
         initHeader(samplerate, channels, bits, format);
         if(write(hfile, &header, sizeof(header)) != sizeof(header))
            return errno;
         break;

      case APPEND:
         struct stat fi = {0};
         hfile = open(filename,O_CREAT | O_RDWR | O_BINARY, S_IWRITE);
         if(!hfile) return errno;
         lseek(hfile,0,SEEK_SET);
         if(!readHeader()) return NO_WAV_FILE;
         fstat(hfile,&fi);
         header.WAVE.datalen = fi.st_size - sizeof(header);
         header.WAVElen = header.WAVE.datalen + sizeof(header.WAVE);
         if(write(hfile, &header, sizeof(header)) != sizeof(header))
            return errno;
         lseek(hfile,0,SEEK_END);
         break;
   }
   return 0;
}

int WAV::closeWAV()
{
   int temp = hfile;
   hfile = 0;
   return close(temp);
}

/* increments the datalen bytes too */

int WAV::writeData(char *buffer, int bytes)
{
   int temp;
   if(!hfile) return 0;

   temp = write(hfile, buffer, bytes);

   header.WAVE.datalen += temp;
   header.WAVElen += temp;

   lseek(hfile,0,SEEK_SET);
   write(hfile, &header, sizeof(header));

   lseek(hfile,0,SEEK_END);

   return temp;
}

int WAV::readData(char *buffer, int bytes)
{
   if(!hfile) return 0;
   return read(hfile, buffer, bytes);
}

int WAV::readHeader()
{
   if(!hfile) return 0;

   lseek(hfile, 0, SEEK_SET);
   read(hfile, &header.R1,4);
   if(header.R1 == 'R' && header.I2 == 'I' && header.F3 == 'F' && header.F4 == 'F')
   {
      read(hfile, &header.WAVElen,4);
      read(hfile, &header.WAVE.W1,4);
      if(header.WAVE.W1 == 'W' && header.WAVE.A2 == 'A' && header.WAVE.V3 == 'V' && header.WAVE.E4 == 'E')
      {
         int foundfmt = 0;
         do
         {
            if(!read(hfile, &header.WAVE.f5,4)) break;
            if(!read(hfile, &header.WAVE.fmtlen,4)) break;
            if(header.WAVE.f5 == 'f' && header.WAVE.m6 == 'm' && header.WAVE.t7 == 't' &&
               header.WAVE.fmtlen == sizeof(header.WAVE.fmt))
            {
               if(!read(hfile, &header.WAVE.fmt,sizeof(header.WAVE.fmt))) break;
               foundfmt = 1;
            }
            else
            {
              /* we skip things we don't know */
               if(lseek(hfile,header.WAVE.fmtlen,SEEK_CUR)) break;
            }
         }
         while(!foundfmt);

         if(foundfmt)
         {
            char buffer[4];
            unsigned long length;
            /* if this is not a standard PCM sample, abort */
            //if(header.WAVE.fmt.FormatTag != 1) return 0;

            do
            {
               if(!read(hfile, &buffer,4)) break;
               if(!read(hfile, &length,4)) break;

               if(buffer[0] == 'd' && buffer[1] == 'a' && buffer[2] == 't' && buffer[3] == 'a')
               {
                  header.WAVE.d8 = 'd';
                  header.WAVE.a9 = 'a';
                  header.WAVE.t10 = 't';
                  header.WAVE.a11 = 'a';
                  header.WAVE.datalen = length;
                  return _tell(hfile); /* tada */
               }
               else
                  /* we skip things we don't know */
                  if(lseek(hfile,length,SEEK_CUR)) break;
            }
            while(1);
         }
      }
   }
   return 0;
}

void WAV::initHeader(int samplerate, int channels, int bits, int format)
{
   header.R1 = 'R';
   header.I2 = 'I';
   header.F3 = 'F';
   header.F4 = 'F';
   header.WAVElen = sizeof(header.WAVE); /* + datalen */
   header.WAVE.W1 = 'W';
   header.WAVE.A2 = 'A';
   header.WAVE.V3 = 'V';
   header.WAVE.E4 = 'E';
   header.WAVE.f5 = 'f';
   header.WAVE.m6 = 'm';
   header.WAVE.t7 = 't';
   header.WAVE.space = ' ';
   header.WAVE.fmtlen = sizeof(header.WAVE.fmt);
   header.WAVE.fmt.FormatTag = format; 
   header.WAVE.d8 = 'd';
   header.WAVE.a9 = 'a';
   header.WAVE.t10 = 't';
   header.WAVE.a11 = 'a';
   header.WAVE.datalen = 0; /* we don't know yet */

   header.WAVE.fmt.BitsPerSample = bits;
   header.WAVE.fmt.Channels = channels;
   header.WAVE.fmt.SamplesPerSec = samplerate;

   header.WAVE.fmt.AvgBytesPerSec =
         header.WAVE.fmt.Channels *
         header.WAVE.fmt.SamplesPerSec *
         header.WAVE.fmt.BitsPerSample / 8;
   header.WAVE.fmt.BlockAlign =
         header.WAVE.fmt.Channels *
         header.WAVE.fmt.BitsPerSample / 8;
}
