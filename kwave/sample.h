#ifndef _KWAVESAMPLE_H_
#define _KWAVESAMPLE_H_ 1

//comment the next line to build a non-threading version of kwave
#define  THREADS

#ifdef   THREADS
 #include <pthread.h>
#else
 #define pthread_t int
#endif

#include <stdlib.h>
#include <qapp.h>
#include <qdir.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qscrbar.h>
#include <qbutton.h>
#include <qcombo.h>
#include <qdialog.h>
#include <qbttngrp.h>
#include <qradiobt.h> 
#include <kapp.h>

#include "menumanager.h"
#include "gsl_fft.h"
#include "sampleop.h"
#include "dialogs.h"
#include "addsynth.h"
#include "filter.h"
#include "interpolation.h"

#define	PROGRESS_SIZE	2*3*4*128
//2*3*4 to insure buffer is an multible of 2,3,4 because the loading routines
//rely on fixed sizes for the buffers, will change with threaded loading
//**********************************************************************
struct FXParams
// structure used to be passed to threads
{
  int  *source;
  int  *counter;
  int  len;
  int  *dest;      //in case a new sample was created...
  void *data[6];   //6 individual Parameters to use
};
//**********************************************************************
struct wavheader
// header format for reading wav files
{
  char		riffid[4];
  long		filelength;
  char		wavid[4];
  char		fmtid[4];
  long		fmtlength;
  short int	mode;
  short int	channels;
  long		rate;
  long		AvgBytesPerSec;
  short int	BlockAlign;
  short int	bitspersample;
};
//**********************************************************************
class MSignal : public QObject
{
 Q_OBJECT
 public:
	MSignal		(QWidget *parent,MenuManager *manage,QString *filename,int channel=1,int type=0);
 	MSignal		(QWidget *parent,MenuManager *manage,int size,int rate,int channels=1);
 	MSignal		(QWidget *parent,int size,int rate,int channels=1);
 	~MSignal	();

 void   setMenuManager  (MenuManager *);
 void 	initSignal      ();
 MSignal *getNext();  //returns pointer to next channel...
 int    getLockState();  //returns lock-state of signal

 QString *getName ();
 void	detachChannels  ();
 void	setChannels	(int);
 void	setParent	(QWidget *);
 void	deleteChannel	(int,int);
 void   insertZero      (int);
 void   zeroChannelRange();
 void   flipChannelRange();
 void   centerChannel   ();
 void   reverseChannelRange();
 void   overwriteChannelRange (MSignal *);
 void	fadeChannelIn	(int);
 void	fadeChannelOut  (int);
 int    newChannel      (int,int);
 void   noiseInsert     (int);
 void   noiseRange      ();
 void   distortChannel  (Interpolation *,int type=0);
 void   amplifyChannel  (double *);

 int    setSoundParams  (int audio,int bitspersample,int channels,int rate,int bufbase);
 void	play8		(int);
 void	play16		(int);
 void	stopplay	();
 int 	getRate		();
 int 	getChannels	();
 int 	*getSample	();
 int 	getLength	();
 int 	getLMarker	();
 int 	getRMarker	();
 int 	getPlayPosition	();
 void	doRangeOp	(int);
 void	save	        (QString *filename,int,int=false);

 void   addChannel      ();
 void   appendChannel         (MSignal *);
 void   appendChanneltoThis   (MSignal *);
 int    isSelected ();
 void   toggleChannel (int,int);

 void	deleteRange	();
 void	deleteChannelRange    ();
 void	flipRange	();
 void	zeroRange	();
 void	reverseRange	();
 void	cutRange	();
 void	cropRange	();
 void	cropChannelRange();
 void	copyRange	();
 void	copyChannelRange();
 void	pasteRange	();
 void	pasteChannelRange     (MSignal *);
 void	mixpasteRange	();
 void	center		();
 void	fadeIn		();
 void	fadeOut		();
 void	fadeInLogarithmic();
 void	fadeOutLogarithmic();

 int    getChannelMaximum   ();
 int    getRangeMaximum     ();
 void   amplifyChannelMax(int);
 void	amplifyMax	();
 void	amplify	        ();
 void	distort	        ();
 void	amplifywithClip	();
 void	amplifyChannelwithClip	(MSignal *);
 void	delay           ();
 void	noise           ();
 void	hullCurve       ();
 void	hullCurveChannel(int,int);
 void	rateChange      ();
 void   changeChannelRate(int);
 void   resample        ();
 void   resampleChannel (int);
 void	addSynth        ();
 void	pulse           ();
 void	addSynthChannel (int num,int **power,int **phase, int **freq);
 void	delayRecursive  (int delay,int ampl,int startpos,int lastpos);
 void	delayOnce	(int delay,int ampl,int startpos,int lastpos);
 void	movingAverage	();
 void   averageChannel  (int,int);
 void	fft             ();
 void	reQuantize      ();
 void	quantizeChannel (int);
 void   fftChannel      (int);
 void	averagefft      ();
 void   averagefftChannel(int,int);
 void   sonagram        ();
 void   sonagramChannel      (int,int);
 void   filterCreate         ();
 void   lockRead             ();
 void   lockWrite            ();
 void   filterChannel        (Filter *filter);
 void   stutter              ();
 void   replaceStutterChannel(int,int);
 void   insertStutterChannel (int,int);
 void   movingFilter         (int);
 void   movingFilterChannel  (Filter*,int,double *);
 FXParams *getFXParams       (void *a=0,void *b=0,void *c=0,void *d=0,void *e=0,void *f=0);

 protected:

 void   getIDs          ();
 void   appendMenus     ();
 void   deleteMenus     ();
 void   getridof        (int *mem);
 int    *getNewMem      (int size);
 void	findDatainFile	(QFile *sigin);
 void	loadWav	        (QString *name,int);
 void	loadAscii	(QString *name,int);
 void	load8Bit	(QFile *sigin,int offset,int interleave);
 void	load16Bit	(QFile *sigin,int offset,int interleave);
 void	load24Bit	(QFile *sigin,int offset,int interleave);
 void	loadStereo16Bit	(QFile *sigin);

 signals:

 void	sampleChanged	();
 void	channelReset	();
 void   signaldeleted   (int,int);   //these are the notification used for keeping track of labels
 void   signalinserted  (int,int);  //dito, only for peaces of signal, being inserted...

 public slots:

 void   setMarkers (int,int);
 void   unlockRead           ();
 void   unlockWrite          ();

 private: 

 int		*sample;               //samples, linear in memory
 int		length;                //number of samples
 int		rate;                  //sampling rate being used
 int		rmarker,lmarker;       //selection markers
 int		*msg;                  //shared memory used for communication
 MSignal        *next;                 //next channel linked to this 
 QWidget	*parent;	       //pointer to connected widget, used for displaying requesters etc...
 QString        *name;                 //filename of the signal
 MenuManager    *manage;               //interface to dynamically add menuitems
 char           selected;              //flag if this channel is selected
 char           locked;                //boolean if sample is locked (interthread-semaphore)
 char           speaker;               //code for playback speaker (e.g. left or right)
 unsigned char  channels;              //number of channels attached to this signal
 char           mapped;                //flag if memory allocated is mmapped
};
#endif  /* sample.h */   







