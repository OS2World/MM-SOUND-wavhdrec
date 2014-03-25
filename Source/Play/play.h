#define FALSE 0
#define TRUE 1

#define NUM_BUFFERS	8

typedef struct {
	int bits;
	int rate;
	int format;
	int numchan;
} REC_STRUCT;

extern int OpenDevice();
extern void CloseDevice();
extern int StartPlayback();

extern char *filename;
extern USHORT CurPosX, CurPosY;
extern int ulBuffersize;
extern  REC_STRUCT RecSettings;
extern USHORT device;
