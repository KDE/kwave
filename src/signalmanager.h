#ifndef _SIGNALMANAGE_H_
#define _SIGNALMANAGE_H_ 1

#define MAXCHANNELS 64

#include "sampleop.h"
#include "addsynth.h"
#include <libkwave/filter.h>
#include <libkwave/kwavesignal.h>

#define processid	0
#define stopprocess	1
#define samplepointer	2

class KwaveSignal;
//**********************************************************************
class SignalManager : public QObject
{
 Q_OBJECT
 public:
	SignalManager	(KwaveSignal *);
	SignalManager	(QWidget *parent,QString *filename,int channel=1,int type=0);
	SignalManager	(QWidget *parent,int length,int rate,int channel=1);
  	~SignalManager	();

 int    doCommand       (const char *);
 void   info ();

 int    setSoundParams  (int audio,int bitspersample,int channels,int rate,int bufbase);

 void	play8		(bool);
 void	play16		(bool);

 void   getMaxMin       (int channel,int& max,int& min,int begin,int len);

 inline int	getRate	        () {return rate;};
 inline int	getChannels	() {return channels;};
 inline int	getLength	() {return length;};
 inline int	getLMarker	() {return lmarker;};
 inline int	getRMarker	() {return rmarker;}; 
 inline int	getPlayPosition () {return msg[samplepointer];};
 inline KwaveSignal *getSignal	() {return signal[0];};
 inline int    getSingleSample  (int channel,int offset)
   {return signal[channel]->getSingleSample(offset);};
 inline void   setParent        (QWidget *par)
   {parent=par;};

 void  command          (const char *);
 const char *getName	();

 void	setOp      	(int);  //triggers function via given id

 void	save	        (QString *filename,int,int=false);

 inline void toggleChannel   (int c) {selected[c]=!selected[c];};
 void   appendChannel   (KwaveSignal *);

 private:
 void   checkRange      ();
 void   prepareChannels ();

 void	deleteChannel	(int);
 int    newChannel      (int);
 void   rateChange      ();
 void   addChannel      ();
 void   appendChanneltoThis   (KwaveSignal *);

 void	play		(bool);
 void	stopplay	();

 void   getridof        (int *mem);
 int    *getNewMem      (int size);

 void	findDatainFile	(QFile *sigin);
 void	loadWav	        (QString *name,int);
 void	loadAscii	(QString *name,int);
 void	load8Bit	(QFile *sigin,int offset,int interleave);
 void	load16Bit	(QFile *sigin,int offset,int interleave);
 void	load24Bit	(QFile *sigin,int offset,int interleave);
 void	loadStereo16Bit	(QFile *sigin);

 void	exportAscii	();

 signals:

 void	sampleChanged	();
 void	channelReset	();
 void   signaldeleted   (int,int);   //these are the notification used for keeping track of labels
 void   signalinserted  (int,int);  //dito, only for peaces of signal, being inserted...
 void lengthInfo	(int);
 void rateInfo		(int);
 void timeInfo		(int);
 void selectedTimeInfo	(int);
 void channelInfo       (int);

 public slots:

 void   setMarkers      (int,int);
 bool   promoteCommand  (const char *command);

 private: 

 QString        *name;
 QWidget        *parent;
 KwaveSignal    *signal[MAXCHANNELS];
 bool           selected[MAXCHANNELS];
 int            lmarker,rmarker;
 int            channels;
 int		length;                //number of samples
 int            len,begin;
 int		rate;                  //sampling rate being used
 int            msg[4];
};
#endif  /* signalmanager.h */   







