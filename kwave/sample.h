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

class MSignal : public QObject
{
 Q_OBJECT
 public:
	MSignal		(QWidget *parent,QString *filename,int channels=1);
 	MSignal		(QWidget *parent,int size,int rate,int channels=1);
 	~MSignal	();
	MSignal *getNext	();  //returns pointer to next channel...


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
 void   sonagramChannel (int);
 void   filterCreate    ();
 void   filterChannel   (Filter *filter);
 void   stutter         ();
 void   replaceStutterChannel  (int,int);
 void   insertStutterChannel  (int,int);
 void   movingFilter    (int);
 void   movingFilterChannel (Filter*,int,double *);

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

 void setMarkers (int,int);

 private: 

 int		*sample;               //samples, linear in memory
 int            channels;              //number of channels attached
 int            selected;              //flag if this channel is selected
 int		length;                //number of samples
 int		rate;                  //sampling rate
 int		rmarker,lmarker;       //selection markers
 int		*msg;
 int            speaker;                //code for playback speaker
 MSignal        *next;                  //other channels linked similar together 
 QString 	filename;		//filename of used Signal
 QWidget	*parent;		//used for displaying requesters

};
#endif  /* sample.h */   







