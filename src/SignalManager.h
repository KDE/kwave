#ifndef _SIGNAL_MANAGER_H_
#define _SIGNAL_MANAGER_H_ 1

#define MAXCHANNELS 64

#define processid	0
#define stopprocess	1
#define samplepointer	2

#include <stdio.h>

class Signal;
class KwaveSignal;
class QWidget;

//**********************************************************************
class SignalManager
{
 public:
	SignalManager	(Signal *);
	SignalManager	(QWidget *parent,const char *filename,int type=0);
	SignalManager	(QWidget *parent,int length,int rate,int channel=1);
  	~SignalManager	();

 int    doCommand       (const char *);
 void   refresh();

 int    setSoundParams  (int audio,int bitspersample,int channels,int rate,int bufbase);

 void	play8		(bool);
 void	play16		(bool);

 void   getMaxMin       (int channel,int& max,int& min,int begin,int len);
 int    getBitsPerSample();

 inline int	getRate	        () {return rate;};
 inline int	getChannelCount	() {return channels;};
        int	getLength	();
 inline int	getLMarker	() {return lmarker;};
 inline int	getRMarker	() {return rmarker;}; 
 inline int	getPlayPosition () {return msg[samplepointer];};
 inline Signal *getSignal	() {return signal[0];};
 inline Signal *getSignal	(int channel) {return signal[channel];};
        int     getSingleSample (int channel,int offset);
 inline void    setParent       (QWidget *par) {parent=par;};

 void  command          (const char *);
 const char *getName	();

 void	setOp      	(int);  //triggers function via given id

 void	save	        (const char *filename,int bits, bool selection);

 /**
  * Exports ascii file with one sample per line and only one channel.
  */
 void	exportAscii	(const char *name);

 void   appendChannel        (Signal *);
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
 void   appendChanneltoThis   (Signal *);

 void	play		(bool);
 void	stopplay	();

 int	findDatainFile	(FILE *);    //searches for data chunk in wav file
                                     //returns 0 if failed, else position
 /**
  * Imports ascii file with one sample per line and only one
  * channel. Everything that cannot be parsed by strod will be ignored.
  * @return 0 if succeeded or error number if failed
  */
 int loadAscii();

 /**
  * Loads a .wav-File.
  * @return 0 if succeeded or a negative error number if failed:
  *           -ENOENT if file does not exist,
  *           -ENODATA if the file has no data chunk or is zero-length,
  *           -EMEDIUMTYPE if the file has an invalid/unsupported format
  */
 int loadWav();

 private: 

 /**
  * Reads in the wav data chunk from a .wav-file. It creates
  * a new empty Signal for each channel and fills it with
  * data read from an opened file. The file's actual position
  * must already be set to the correct position.
  * @param sigin pointer to the already opened file
  * @param length number of samples to be read
  * @param channels number of channels [1..n]
  * @param number of bits per sample [8,16,24,...]
  * @return 0 if succeeded or error number if failed
  */
 int loadWavChunk(FILE *sigin,int length,int channels,int bits);

 /**
  * Writes the chunk with the signal to a .wav file (not including
  * the header!).
  * @param sigout pointer to the already opened file
  * @param length number of samples to be written
  * @param channels number of channels [1..n]
  * @param number of bits per sample [8,16,24,...]
  * @return 0 if succeeded or error number if failed
  */
 int writeWavChunk(FILE *sigout,int begin,int length,int bits);

 char          *name;
 QWidget       *parent;
 Signal        *signal[MAXCHANNELS];
 bool           selected[MAXCHANNELS];
 int            lmarker,rmarker;
 int            channels;
 int		rate;                  //sampling rate being used
 int            msg[4];
};

#endif  // _SIGNAL_MANAGER_H_
