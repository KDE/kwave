#ifndef _DIALOGS_H_
#define _DIALOGS_H_ 1

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
#include <qlined.h> 
#include <qcolor.h> 
#include <qfiledlg.h> 
#include <qlistbox.h>
#include <qtabdialog.h>
#include <qwidgetstack.h>

#include "addsynth.h"
#include "faderwidget.h"
#include "curvewidget.h"
#include "functions.h"
#include "fftview.h"
#include "scale.h"

#include <kapp.h>
#include <kcolordlg.h>
#include <kintegerline.h>

class TimeLine : public KRestrictedLine
{
 Q_OBJECT
   public:
 TimeLine (QWidget* parent,int rate=48000);
 ~TimeLine ();

 void setSamples    (int);
 void setMs         (int);
 int  getValue      ();

 public slots:
 void setSampleMode ();
 void setMsMode     ();
 void setKbMode     ();
 void setSMode      ();
 void setValue      (const char *);
 void setRate       (int);

 protected:
 void mousePressEvent( QMouseEvent *);

 int value;
 int mode;     //flag for display and enter mode...
 int rate;
 QPopupMenu *menu;
};

class NewSampleDialog : public QDialog
{
 Q_OBJECT

 public:

 	NewSampleDialog 	(QWidget *parent=0);
 	~NewSampleDialog 	();
 int	getLength();
 int	getRate	();

 public slots:

 void setLength (int);
 void setRate   (const char *);

 protected:

 void resizeEvent (QResizeEvent *);

 private:

 TimeLine	*time;
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

 	DelayDialog 	(QWidget *parent=0,int rate=48000);
 	~DelayDialog 	();
 int	getDelay();
 int	getAmpl();
 int	isRecursive();

 public slots:

 void setAmpl (int);

 protected:

 void resizeEvent (QResizeEvent *);

 private:

 TimeLine       *delay;
 QLabel		*delaylabel;
 QScrollBar	*amplslider;
 QLabel		*ampllabel;
 QCheckBox	*recursive;
 QPushButton	*ok,*cancel;
};
//****************************************************************************
class ProgressDialog : public QDialog
{
 Q_OBJECT

 public:
	ProgressDialog 	(int max,int *counter,char*caption="Progress");
 	ProgressDialog 	(int max=100,char*caption="Progress");
 	~ProgressDialog ();
 void	setProgress(int);

 signals:

 void done ();

 public slots:

 void	timedProgress();

 protected:

 void paintEvent  (QPaintEvent *);

 private:

 int max;
 int act;
 int last;
 int lastx,lasty;
 int *counter;
 QTimer *timer;
};
//*****************************************************************************
class RateDialog : public QDialog
{
 Q_OBJECT

 public:

 	RateDialog 	(QWidget *parent=0);
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

 PlayBackDialog 	(QWidget *parent=0,int =false,int =5);
 ~PlayBackDialog 	();
 int getResolution ();
 int getBufferSize ();

 public slots:

   void setBufferSize (int);

 protected:

 void resizeEvent (QResizeEvent *);

 private:

 QLabel		*bufferlabel;
 QSlider        *buffersize;
 QLabel         *devicelabel;
 QComboBox      *devicebox;
 QCheckBox      *stereo;
 QLabel		*label1,*label2;
 QButtonGroup	*bg;
 QRadioButton	*b16,*b8;

 QPushButton	*ok,*cancel;
};
//*****************************************************************************
class FadeDialog : public QDialog
{
 Q_OBJECT

 public:
 	FadeDialog 	(QWidget *parent=0,int dir=1);
 	~FadeDialog 	();
 int getCurve ();

 public slots:

 protected:

 void resizeEvent (QResizeEvent *);

 private:

 QPushButton	*ok,*cancel;
 QScrollBar	*slider;
 FaderWidget	*fade;
};

//*****************************************************************************
class DistortDialog : public QDialog
{
  Q_OBJECT

    public:
  DistortDialog 	(QWidget *parent=0);
  ~DistortDialog 	();
  QList<CPoint> *getPoints ();
  int   getType ();

  public slots:

 protected:

  void resizeEvent (QResizeEvent *);

 private:

  ScaleWidget   *xscale,*yscale;
  CornerPatchWidget *corner;
  QPushButton	*ok,*cancel;
  CurveWidget	*curve;
};
//*****************************************************************************
class AmplifyCurveDialog : public QDialog
{
  Q_OBJECT

    public:
  AmplifyCurveDialog 	(QWidget *parent=0,int time=100);
  ~AmplifyCurveDialog 	();
  QList<CPoint> *getPoints ();
  int   getType ();

  public slots:

 protected:

  void resizeEvent (QResizeEvent *);

 private:

  CurveWidget	*curve;
  ScaleWidget   *xscale,*yscale;
  CornerPatchWidget *corner;
  QPushButton	*ok,*cancel;
};
//*****************************************************************************
class FrequencyMultDialog : public QDialog
{
  Q_OBJECT

    public:
  FrequencyMultDialog 	(QWidget *parent=0,int time=100);
  ~FrequencyMultDialog 	();
  QList<CPoint> *getPoints ();
  int   getType ();

  public slots:

 protected:

  void resizeEvent (QResizeEvent *);

 private:

  CurveWidget	*curve;
  ScaleWidget   *xscale,*yscale;
  CornerPatchWidget *corner;
  QLabel        *xlabel,*ylabel;
  KIntegerLine  *x,*y;
  QPushButton	*add;
  QPushButton	*ok,*cancel;
};
//*****************************************************************************
class PitchDialog : public QDialog
{
  Q_OBJECT

    public:
  PitchDialog 	(QWidget *parent=0,int time=100);
  ~PitchDialog 	();
  int getLow    ();
  int getHigh   ();
  int getOctave ();

  public slots:

 protected:

  void resizeEvent (QResizeEvent *);

 private:

  QLabel        *lowlabel,*highlabel;
  KIntegerLine  *low,*high;
  QPushButton	*ok,*cancel;
  QCheckBox     *octave;
};
//*****************************************************************************
class TimeDialog : public QDialog
{
 Q_OBJECT

 public:

 	TimeDialog 	(QWidget *parent=0,int rate=48000,char *name="Length :");
 	~TimeDialog 	();
 int	getLength();

 public slots:

 void setLength (int);

 protected:

 void resizeEvent (QResizeEvent *);

 private:

 TimeLine	*time;
 QLabel		*timelabel;
 QPushButton	*ok,*cancel;
};
//*****************************************************************************
class HullCurveDialog : public QDialog
{
 Q_OBJECT

 public:

 	HullCurveDialog 	(QWidget *parent=0,char *name="Hullcurve :");
 	~HullCurveDialog 	();
 int	getTime();
 int	getType();

 public slots:

 void setTime (int);

 protected:

 void resizeEvent (QResizeEvent *);

 private:

 QScrollBar	*timeslider;
 QLabel		*timelabel;
 QLabel		*typelabel;
 QComboBox	*typebox;
 QPushButton	*ok,*cancel;
};
//*****************************************************************************
class AverageDialog : public QDialog
{
 Q_OBJECT

 public:

 	AverageDialog 	(QWidget *parent=0,char *name="");
 	~AverageDialog 	();
 int	getTaps();

 public slots:
   void setTaps (int);

  protected:

 void resizeEvent (QResizeEvent *);

 private:

 QScrollBar	*tapslider;
 QLabel		*taplabel;
 QPushButton	*ok,*cancel;
};
//*****************************************************************************
class MarkerTypeDialog : public QDialog
{
 Q_OBJECT

 public:
 	MarkerTypeDialog 	(QWidget *parent,char *name="Create new  Type :");
 	~MarkerTypeDialog 	();
	const char *getName();
	QColor getColor();
	int  getIndividual();

 public slots:

   void setColor(const QColor &col);

 protected:

 void resizeEvent (QResizeEvent *);

 private:

 KColorCombo    *color;
 QCheckBox      *individual;
 QLabel		*namelabel;
 QLineEdit	*name;
 QPushButton	*ok,*cancel;
 QColor         col;
};
//*****************************************************************************
class SonagramDialog : public QDialog
{
 Q_OBJECT

 public:

 	SonagramDialog 	(QWidget *parent,int len,int=1024,char *name="Select # of Points");
 	~SonagramDialog 	();
 int	getPoints();

 public slots:
   void setPoints (int);
   void setBoxPoints (int);
 protected:

 void resizeEvent (QResizeEvent *);

 private:

 QScrollBar	*pointslider;
 QLabel		*pointlabel;
 QLabel		*windowlabel;
 QLabel		*bitmaplabel;
 QComboBox	*pointbox;
 QPushButton	*ok,*cancel;
 int            length,rate;
};
//*****************************************************************************
class StringEnterDialog : public QDialog
{
 Q_OBJECT

 public:

 	StringEnterDialog 	(QWidget *parent=0,char *name="Please Enter Name :",char * init=0);
 	~StringEnterDialog 	();
 const char   *getString();
 public slots:

  protected:

 void resizeEvent (QResizeEvent *);

 private:

 QLineEdit	*name;
 QPushButton	*ok,*cancel;
};
//*****************************************************************************
class PercentDialog : public QDialog
{
 Q_OBJECT

 public:
 	PercentDialog 	(QWidget *parent=0,char *name="Choose Amount:");
 	~PercentDialog 	();
 int    getPerMille();
 public slots:

   void setValue (int);

  protected:

 void resizeEvent (QResizeEvent *);

 private:
 QSlider     *slider;
 QLabel      *label; 
 QPushButton *ok,*cancel;
};
//*****************************************************************************
class SweepDialog : public QWidget
{
 Q_OBJECT

 public:
 	SweepDialog 	(QWidget *parent=0,int rate=48000,char *name="Choose Parameters :");
 	~SweepDialog 	();
 int    getTime();
 double getLowFreq();
 double getHighFreq();
 int    getInterpolationType();
 QList<CPoint> *getPoints ();
 void   convert (QList<CPoint>*);

 public slots:

 void import       ();
 void showTime     (int);
 void showLowFreq  (int);
 void showHighFreq (int);

  protected:

 void resizeEvent (QResizeEvent *);

 private:
 TimeLine       *time;
 QLabel         *timelabel; 
 QComboBox      *note1,*note2;
 QLabel         *notelabel1,*notelabel2; 
 KIntegerLine   *lowfreq,*highfreq;
 CurveWidget	*curve;
 QPushButton	*load;
 int rate;
};
//*****************************************************************************
class FixedFrequencyDialog : public QWidget
{
 Q_OBJECT

 public:
 	FixedFrequencyDialog 	(QWidget *parent=0,int rate=48000,char *name="Choose Parameters :");
 	~FixedFrequencyDialog 	();
 int    getFrequency();
 int    getTime();

 public slots:

  void showFrequency (int);
  void showTime  (int);

  protected:

 void resizeEvent (QResizeEvent *);

 private:
 QSlider     *timeslider;
 QLabel      *timelabel; 
 QSlider     *frequencyslider;
 QLabel      *frequencylabel; 
 int rate;
};
//*****************************************************************************
class FrequencyDialog : public QTabDialog
{
 Q_OBJECT

 public:
        FrequencyDialog 	(QWidget *parent=0,int rate=48000,char *name="Choose Parameters :");
 	~FrequencyDialog 	();
	QList<CPoint> *FrequencyDialog::getFrequency ();

 public slots:

 void setSelected  (const char *);

 protected:

 void resizeEvent (QResizeEvent *);

 private:

 FixedFrequencyDialog *fixed;
 SweepDialog          *sweep;
 const char           *type;
 int                  rate;
};
//*****************************************************************************
class QuantiseDialog : public QDialog
{
 Q_OBJECT

 public:

 	QuantiseDialog 	(QWidget *parent=0,char *name="Choose new virtual resolution");
 	~QuantiseDialog 	();

	int getBits();

 public slots:

 protected:

 void resizeEvent (QResizeEvent *);

 private:

 KIntegerLine	*bits;
 QCheckBox      *linear;
 QLabel		*bitlabel;
 QPushButton	*ok,*cancel;
};
//*****************************************************************************
class StutterDialog : public QDialog
{
 Q_OBJECT

 public:

 	StutterDialog 	(QWidget *parent=0,int rate=48000,char *name="Choose Parameters");
 	~StutterDialog 	();

	int getLen1();
	int getLen2();

 public slots:

 protected:

 void resizeEvent (QResizeEvent *);

 private:

 TimeLine	*len1,*len2;
 QCheckBox      *insert;
 QLabel		*label1,*label2;
 QPushButton	*ok,*cancel;
};
//*****************************************************************************
class MarkSaveDialog : public QDialog
{
 Q_OBJECT

 public:

 	MarkSaveDialog 	(QWidget *parent=0,char *name="Select label types to be saved :",int multi=true);
 	~MarkSaveDialog ();

	void getSelection();

 public slots:

 protected:

 void resizeEvent (QResizeEvent *);

 private:

 QListBox	*save;
 QPushButton	*ok,*cancel;
};
//*****************************************************************************
class MarkSignalDialog : public QDialog
{
 Q_OBJECT

 public:

 	MarkSignalDialog 	(QWidget *parent=0,int=48000,char *name="Choose Labeling Criteria :");
 	~MarkSignalDialog ();
 int   getLevel ();
 int   getTime  ();
 int   getType1  ();
 int   getType2  ();

 public slots:

 void setAmpl (int);
 void setAmpl (const char *);

 protected:

 void resizeEvent (QResizeEvent *);

 private:

 QLabel         *timelabel,*ampllabel;
 KIntegerLine   *ampl;
 TimeLine	*time;
 QSlider        *amplslider;
 QLabel         *mark1,*mark2;
 QComboBox	*marktype1,*marktype2;
 QPushButton	*ok,*cancel;
 int            tflag;
};
//*****************************************************************************
class SaveBlockDialog : public QDialog
{
 Q_OBJECT

 public:

 	SaveBlockDialog 	(QWidget *parent=0,char *name="Choose Labels to be used for dividing signal:");
 	~SaveBlockDialog ();
 const char  *getName ();
 QDir  *getDir ();
 int   getType1  ();
 int   getType2  ();

 public slots:

 void check ();

 protected:

 void resizeEvent (QResizeEvent *);

 private:

 QLineEdit      *name,*dirname;
 QLabel         *mark1,*mark2;
 QLabel         *dirlabel,*namelabel;
 QDir           *dir;
 QComboBox	*marktype1,*marktype2;
 QPushButton	*ok,*cancel;
};
//*****************************************************************************
#endif  /* dialogs.h */   





