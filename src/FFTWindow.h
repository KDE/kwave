#ifndef _FFTVIEW_H_
#define _FFTVIEW_H_ 1

#include <libkwave/gsl_fft.h>
#include <qstring.h>
#include <qwidget.h>
#include <qpixmap.h>
#include <qtimer.h>
#include "../libgui/FFTWidget.h"
#include "FFTContainer.h"
#include <kapp.h>
#include <kselect.h>
#include <ktopwidget.h>
#include <kmenubar.h>
#include <kbuttonbox.h>
#include <kstatusbar.h>  

//***********************************************************************
class FFTWindow : public KTopLevelWidget
{
 Q_OBJECT
 public:
	FFTWindow       (QString *name);
	~FFTWindow      ();
 void   setSignal       (complex *,double,int,int);     //reaches through to class FFTWidget...

 public slots:

 void   setFreqInfo  (int,int);
 void   setAmpInfo   (int,int);
 void   setDBInfo    (int,int);
 void   setPhaseInfo (int,int);
 void   setNoteInfo  (int,int);
 void   phaseMode    ();
 void   dbMode       (int);
 void   percentMode  ();
 void   askFreqRange ();
 void   findPeak     ();

 signals:

 protected:

 private:
 FFTContainer  *mainwidget;
 FFTWidget     *fftview;
 QPopupMenu    *cursor;
 ScaleWidget   *xscale,*yscale;
 CornerPatchWidget *corner;
 KStatusBar    *status;
 int           findPeakID;
};
#endif






