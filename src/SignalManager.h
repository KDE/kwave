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
class SignalManager
{
 public:
	SignalManager	(KwaveSignal *);
	SignalManager	(QWidget *parent,const char *filename,int type=0);
	SignalManager	(QWidget *parent,int length,int rate,int channel=1);
  	~SignalManager	();

 int    doCommand       (const char *);
 void   info ();

 int    setSoundParams  (int audio,int bitspersample,int channels,int rate,int bufbase);

 void	play8		(bool);
 void	play16		(bool);

 void   getMaxMin       (int channel,int& max,int& min,int begin,int len);
 int    getBitsPerSample();

 inline int	getRate	        () {return rate;};
 inline int	getChannelCount	() {return channels;};
 inline int	getLength	()
   {return signal[0] ? signal[0]->getLength() : 0;};
 inline int	getLMarker	() {return lmarker;};
 inline int	getRMarker	() {return rmarker;}; 
 inline int	getPlayPosition () {return msg[samplepointer];};
 inline KwaveSignal *getSignal	() {return signal[0];};
 inline KwaveSignal *getSignal	(int channel) {return signal[channel];};
 inline int    getSingleSample  (int channel,int offset)
   {return signal[channel] ? signal[channel]->getSingleSample(offset) : 0;};
 inline void   setParent        (QWidget *par)
   {parent=par;};

 void  command          (const char *);
 const char *getName	();


 void	setOp      	(int);  //triggers function via given id

 void	save	        (const char *filename,int bits,bool selection=false);


 void   appendChannel        (KwaveSignal *);
 void   setRange             (int,int);
 bool   promoteCommand       (const char *command);

 inline void toggleChannel   (int c) {selected[c]=!selected[c];};

 private:
 void   initialize      ();
 void   checkRange      ();
 void	deleteChannel	(int);
 int    newChannel      (int);
 void   rateChange      ();
 void   addChannel      ();
 void   appendChanneltoThis   (KwaveSignal *);

 void	play		(bool);
 void	stopplay	();

 int	findDatainFile	(FILE *);    //searches for data chunk in wav file
                                     //returns 0 if failed, else position
 void	loadAscii	();
 void	loadWav	        ();
 void	loadWavChunk	(FILE *sigin,int length,int channels,int bits);

 void	exportAscii	();
 void	writeWavChunk	(FILE *sigout,int begin,int length,int bits);

 private: 

 char           *name;
 QWidget        *parent;
 KwaveSignal    *signal[MAXCHANNELS];
 bool           selected[MAXCHANNELS];
 int            lmarker,rmarker;
 int            channels;
 int		rate;                  //sampling rate being used
 int            msg[4];
};
#endif  /* signalmanager.h */   

