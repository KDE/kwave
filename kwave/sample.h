#ifndef _SAMPLE_H_
#define _SAMPLE_H_ 1

#include <stdlib.h>
#include <qapp.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qscrbar.h>
#include <qbutton.h>
#include <qcombo.h>
#include <qdialog.h>
#include <qbttngrp.h>
#include <qradiobt.h> 

#include "gsl_fft.h"
#include <kapp.h>
#include "sampleop.h"

#define	PROGRESS_SIZE	65536

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
	MSignal		(QWidget *parent,QString *filename);
 	MSignal		(QWidget *parent,int size,int rate);
 	~MSignal	();
 void	newSignal	();
 void	play8		();
 void	loop8		();
 void	play16		();
 void	loop16		();
 void	stopplay	();
 int 	getRate		();
 int 	*getSample	();
 int 	getLength	();
 int 	getLMarker	();
 int 	getRMarker	();
 int 	getPlayPosition	();
 void	doRangeOp	(int);
 void	save16Bit	(QString *filename);

 void	deleteRange	();
 void	flipRange	();
 void	zeroRange	();
 void	reverseRange	();
 void	cutRange	();
 void	cropRange	();
 void	copyRange	();
 void	pasteRange	();
 void	center		();
 void	fadeInLinear	();
 void	fadeOutLinear	();
 void	fadeInLogarithmic();
 void	fadeOutLogarithmic();
 void	amplifyMax	();
 void	delay           ();
 void	noise           ();
 void	fft             ();
 void	rateChange      ();
 void	playBackOptions ();
 void	delayRecursive  (int delay,int ampl,int startpos,int lastpos);
 void	delayOnce	(int delay,int ampl,int startpos,int lastpos);

 protected:

 void	findDatainFile	(QFile *sigin);
 void	load8Bit	(QFile *sigin,int offset,int interleave);
 void	load16Bit	(QFile *sigin,int offset,int interleave);

 signals:

 void	sampleChanged	();

 public slots:

 void setMarkers (int,int);

 private: 

 int		*sample;
 int		length;
 int		rate;
 int		rmarker,lmarker;
 int		*msg;
 int		memid;
 QString 	filename;		
 QWidget	*parent;		//used for displaying requesters
};
//*****************************************************************************
class NewSampleDialog : public QDialog

{
 Q_OBJECT

 public:

 	NewSampleDialog 	(QWidget *parent=NULL);
 	~NewSampleDialog 	();
 int	getLength();
 int	getRate	();

 public slots:

 void setLength (int);

 protected:

 void resizeEvent (QResizeEvent *);

 private:

 QScrollBar	*timeslider;
 QLabel		*timelabel;
 QLabel		*ratelabel;
 QComboBox	*ratefield;
 QPushButton	*ok,*cancel;
};
//*****************************************************************************

#include <qchkbox.h>
class DelayDialog : public QDialog
{
 Q_OBJECT

 public:

 	DelayDialog 	(QWidget *parent=NULL);
 	~DelayDialog 	();
 int	getDelay();
 int	getAmpl();
 int	isRecursive();

 public slots:

 void setDelay (int);
 void setAmpl (int);

 protected:

 void resizeEvent (QResizeEvent *);

 private:

 QScrollBar	*delayslider;
 QLabel		*delaylabel;
 QScrollBar	*amplslider;
 QLabel		*ampllabel;
 QCheckBox	*recursive;
 QPushButton	*ok,*cancel;
};
//****************************************************************************************
#include <kprogress.h>
class ProgressDialog : public QDialog
{
 Q_OBJECT

 public:

 	ProgressDialog 	(int max=100,char*caption="Progress");
 	~ProgressDialog 	();
 void	setProgress(int);

 public slots:

 protected:

 void resizeEvent (QResizeEvent *);

 private:

 KProgress	*progress;
};
//*****************************************************************************
class RateDialog : public QDialog
{
 Q_OBJECT

 public:

 	RateDialog 	(QWidget *parent=NULL);
 	~RateDialog 	();
 int	getRate	();

 public slots:

 protected:

 void resizeEvent (QResizeEvent *);

 private:

 QLabel		*ratelabel;
 QComboBox	*ratefield;
 QPushButton	*ok,*cancel;
};
//*****************************************************************************
class PlayBackDialog : public QDialog
{
 Q_OBJECT

 public:

 	PlayBackDialog 	(QWidget *parent=NULL,int play16bit=false);
 	~PlayBackDialog 	();
	int getResolution ();

 public slots:

 protected:

 void resizeEvent (QResizeEvent *);

 private:

 QLabel		*label1,*label2;
 QButtonGroup	*bg;
 QRadioButton	*b16,*b8;

 QPushButton	*ok,*cancel;
};
//*****************************************************************************

#endif  /* sample.h */   




