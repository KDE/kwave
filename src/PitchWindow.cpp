
#include <kapp.h>
#include <kstatusbar.h>

#include "libgui/ScaleWidget.h"
#include "libgui/CornerPatchWidget.h"

#include "PitchContainer.h"
#include "PitchWidget.h"
#include "PitchWindow.h"

extern char *mstotimec (int ms);
//****************************************************************************
PitchWindow::PitchWindow (const char *name) : KTopLevelWidget ()
{
  QPopupMenu *pitch=	new QPopupMenu ();
  KMenuBar   *bar=	new KMenuBar (this); 

  bar->insertItem	(i18n("&Pitch"),pitch);

  status=new KStatusBar (this);
  status->insertItem    (i18n("Time:         0 ms      "),1);
  status->insertItem    (i18n("Frequency:          0 Hz"),2);

  mainwidget=new PitchContainer (this);
  view=  new PitchWidget (mainwidget);
  xscale=new ScaleWidget (mainwidget,0,100,"ms");
  yscale=new ScaleWidget (mainwidget,20000,0,"Hz");
  corner=new CornerPatchWidget (mainwidget);
  mainwidget->setObjects (view,xscale,yscale,corner);

  setView (mainwidget);
  setStatusBar (status);
  setMenu (bar);
  
  connect (view,SIGNAL(freqRange(float,float)),this,SLOT(freqRange(float,float)));
  connect (view,SIGNAL(pitch(float)),this,SLOT(showPitch(float)));
  connect (view,SIGNAL(timeSamples(float)),this,SLOT(showTime(float)));
 
  QString *windowname=new QString (QString (i18n("Pitch of "))+QString(name));
  setCaption (windowname->data()); 
  setMinimumSize (320,200);
}
//****************************************************************************
void PitchWindow::freqRange (float min,float max)
{
  yscale->setMaxMin ((int)min,(int)max);
}
//****************************************************************************
void PitchWindow::showPitch (float freq)
{
  char buf [32];
  sprintf (buf,i18n("Frequency : %.1f Hz\n"),freq);
  status->changeItem (buf,2);
}
//****************************************************************************
void PitchWindow::showTime (float time)
{
  char buf [32];
  sprintf (buf,i18n("Time : %s\n"),mstotimec((int)((time*1000/rate))));
  status->changeItem (buf,1);
}
//****************************************************************************
void PitchWindow::setSignal (float *data,int len,int rate)
 //reaches through to class PitchWidget, same Method, last int is ommited, since only used for scales, that are managed from this class...
{
  view->setSignal (data,len);
  xscale->setMaxMin ((int)(((double)len)*10000/rate),0);
  this->rate=rate;
}
//****************************************************************************
PitchWindow::~PitchWindow ()
{
}
//****************************************************************************
