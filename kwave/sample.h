#ifndef _SAMPLE_H_
#define _SAMPLE_H_ 1

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

#include "gsl_fft.h"
#include "sampleop.h"
#include "dialogs.h"
#include "filter.h"
#include "interpolation.h"

#define	PROGRESS_SIZE	2*3*4*128    //2*3*4 to insure buffer is an multible of these for loading routines....
//**********************************************************************
struct FXParams
{
  int  *source;
  int  *counter;
  int  len;
  int  *dest;      //in case a new sample was created...
  void *data[6];     //6 individual Parameters to use
};
//**********************************************************************
struct wavheader
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
	MSignal		(QWidget *parent,QString *filename,int channels=1);
 	MSignal		(QWidget *parent,int size,int rate,int channels=1);
 	~MSignal	();
	MSignal *getNext	();  //returns pointer to next channel...
	int getLockState	();  //returns lock-state of signal


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
 void   distortChannel  (Interpolation *);
 void   amplifyChannel  (double *);

 void	newSignal	();

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
 void	playBackOptions ();
 void	addSynth        ();
 void	pulse           ();
 void	addSynthChannel (int num,int **power,int **phase, int **freq);
 void	delayRecursive  (int delay,int ampl,int startpos,int lastpos);
 void	delayOnce	(int delay,int ampl,int startpos,int lastpos);
 void	movingAverage	();
 void   averageChannel  (int);
 void	fft             ();
 void	reQuantize      ();
 void	quantizeChannel (int);
 void   fftChannel      ();
 void	averagefft      ();
 void   averagefftChannel(int);
 void   sonagram        ();
 void   sonagramChannel      (int);
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

 void	findDatainFile	(QFile *sigin);
 void	load8Bit	(QFile *sigin,int offset,int interleave);
 void	load16Bit	(QFile *sigin,int offset,int interleave);
 void	load24Bit	(QFile *sigin,int offset,int interleave);
 void	loadStereo16Bit	(QFile *sigin);
 void   writeData24Bit  (QFile *sigout,int begin,int end,int **samples,int numsamples);
 void   writeData16Bit  (QFile *sigout,int begin,int end,int **samples,int numsamples);
 void   writeData8Bit   (QFile *sigout,int begin,int end,int **samples,int numsamples);

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
 char           selected;              //flag if this channel is selected
 char           locked;                //boolean if sample is locked (intertask-semaphore)
 char           speaker;               //code for playback speaker (e.g. left or right)
 unsigned char  channels;              //number of channels attached to this signal
};
#endif  /* sample.h */   







