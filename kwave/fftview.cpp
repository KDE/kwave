#include <qdir.h>
#include "fftview.h"
#include <qpainter.h>
#include <math.h>
#include <limits.h>
#include <qcursor.h>
#include "dialogs.h"
#include "formantwidget.h"
#include "sample.h"
#include "main.h"

extern KApplication *app;
//****************************************************************************
__inline void  getMaxMinPower (complex *sample,int len,double *max,double *min)
{
  double rea,ima;
  double c;
  *min=INT_MAX;
  *max=INT_MIN;
  for (int i=0;i<len;i++)
    {
      rea=sample[i].real;
      ima=sample[i].imag;
      c=sqrt(rea*rea+ima*ima);
      if (c>*max) *max=c;
      if (c<*min) *min=c;
    }
}
//****************************************************************************
__inline void  getMaxMinPhase (complex *sample,int len,double &max,double &min)
{
  double rea,ima;
  double c;
  min=INT_MAX;
  max=INT_MIN;
  for (int i=0;i<len;i++)
    {
      rea=sample[i].real;
      ima=sample[i].imag;
      c=(atan(ima/rea)+M_PI/2)/M_PI;
      if (c>max) max=c;
      if (c<min) min=c;
    }
}
//****************************************************************************
FFTWidget::FFTWidget (QWidget *parent)
 : QWidget (parent)
{
  data=0;
  autodelete=true;
  fftsize=0;
  height=-1;
  pixmap=0;
  phaseview=false;
  db=false;
  setCursor (crossCursor);
  setBackgroundColor (QColor(black) );
}
//****************************************************************************
FFTWidget::~FFTWidget (QWidget *parent,const char *name)
{
  if (pixmap==0)	delete pixmap;
  if (data&&autodelete)	delete data;
}
//****************************************************************************
void FFTWidget::setAutoDelete  (int tr)
{
   autodelete=tr;
}
//****************************************************************************
void FFTWidget::phaseMode  ()
{
  redraw=true;
  phaseview=true;
  repaint ();
}
//****************************************************************************
void FFTWidget::dbMode  (int db)
{
  redraw=true;
  phaseview=false;
  this->db=db;
  repaint ();
}
//****************************************************************************
void FFTWidget::percentMode  ()
{
  redraw=true;
  phaseview=false;
  db=false;
  repaint ();
}
//****************************************************************************
void FFTWidget::setPhase  (complex *data,int size, int rate)
{
  phaseview=true;
  this->data=data;
  this->fftsize=size;
  this->rate=rate;
}
//****************************************************************************
void FFTWidget::getMaxMin ()
{
  getMaxMinPower (data,fftsize/2,&this->max,&this->min);
}
//****************************************************************************
void FFTWidget::setSignal  (complex *data,int size, int rate)
{
  phaseview=false;
  this->data=data;
  this->fftsize=size;
  this->rate=rate;
  getMaxMin ();
}
//****************************************************************************
void FFTWidget::refresh()
{
  getMaxMin ();
  redraw=true;
  repaint ();
}
//****************************************************************************
void FFTWidget::formant()
{
  FormantDialog *dialog =new FormantDialog (this,rate);
  if (dialog->exec())
    {
      int i;
      double mul=0;
      int size=5000*(fftsize)/(rate); //formant spectrum is limited to 5khz
      double *points=dialog->getPoints (size);

      if (points)
	{
	  for (i=0;i<size;i++) 
	    {
	      //reconvert db scale to linear multiplication factor 
	      mul=1/(pow (2,(points[i]/6)));

	      data[i].real*=mul;
	      data[i].imag*=mul;
	      data[fftsize-i].real*=mul;
	      data[fftsize-i].imag*=mul;
	    }
	  for (;i<fftsize/2;i++) 
	    {
	      data[i].real*=mul;
	      data[i].imag*=mul;
	      data[fftsize-i].real*=mul;
	      data[fftsize-i].imag*=mul;
	    }
	  delete points;
	}

      delete dialog;

      refresh ();
    }
}
//****************************************************************************
void FFTWidget::smooth()
{
  AverageDialog *dialog =new AverageDialog (this,"Moving Average:");
  if (dialog->exec())
    {
      int average=dialog->getTaps();
      int b=average/2;
      double rea,ima,abs,old;
      int i,j;

      complex *newdata=new complex [fftsize];

      if (newdata)
	{
	  for (i=0;i<fftsize;i++)
	    {
	      abs=0;

	      if ((i-b<0)||(i+b>=fftsize))
	      for (j=-b;j<b;j++)
		{
		  if ((i+b>=0)&&(i+j<fftsize))
		    {
		      rea=data[i+j].real;
		      ima=data[i+j].imag;
		      abs+=sqrt(rea*rea+ima*ima);
		    }
		}
	      else
	      for (j=-b;j<b;j++)
		{
		  rea=data[i+j].real;
		  ima=data[i+j].imag;
		  abs+=sqrt(rea*rea+ima*ima);
		}

	      abs/=average;
	      rea=data[i].real;
	      ima=data[i].imag;
	      old=sqrt (rea*rea+ima*ima);

	      abs=abs/old;

	      newdata[i].real=data[i].real*abs;
	      newdata[i].imag=data[i].imag*abs;
	    }
	  delete data;
	  data=newdata;

	  refresh ();
	}  
    }
}
//****************************************************************************
void FFTWidget::amplify()
{
  FrequencyMultDialog *dialog =new FrequencyMultDialog (this,rate);
  if (dialog->exec())
    {
      QList<CPoint> *points=dialog->getPoints ();

      Interpolation interpolation (dialog->getType());

      double *y=interpolation.getInterpolation (points,fftsize/2);

      for (int i=0;i<fftsize/2;i++)
	{
	  data[i].real*=2*y[i];
	  data[i].imag*=2*y[i];
	  data[fftsize-i].real*=2*y[i];
	  data[fftsize-i].imag*=2*y[i];
	}

      refresh ();

      delete dialog; 
    }
}
//****************************************************************************
void FFTWidget::killPhase()
{
  double rea,ima;
      for (int i=0;i<fftsize;i++)
	{
	  rea=data[i].real;
	  ima=data[i].imag;
	  data[i].real=sqrt(rea*rea+ima*ima);
	  data[i].imag=0;
	}

      redraw=true;
      repaint ();
}
//****************************************************************************
void FFTWidget::iFFT()
{
  complex *data=new complex [fftsize];

  if (data)
    {
      for (int i=0;i<fftsize;i++) data[i]=this->data[i];

      gsl_fft_complex_wavetable table;

      gsl_fft_complex_wavetable_alloc (fftsize,&table);

      gsl_fft_complex_init (fftsize,&table);

      gsl_fft_complex_inverse (data,fftsize,&table);
      gsl_fft_complex_wavetable_free (&table);

      TopWidget *win=new TopWidget (app);

      if (win)
	{
	  MSignal *newsig=new MSignal (win,fftsize,rate);

	  win->show();
	  if (newsig)
	    {
	      int *sam=newsig->getSample();
	      if (sam)
		{
		  for (int i=0;i<fftsize;i++)
		    sam[i]=(int)(data[i].real*((1<<23)-1));
		}
	      win->setSignal (newsig);
	    }
	  else delete win;
	}
      delete data;
    }
}
//****************************************************************************
void FFTWidget::mousePressEvent( QMouseEvent *)
{
}
//****************************************************************************
void FFTWidget::mouseReleaseEvent( QMouseEvent *)
{
}
//****************************************************************************
void FFTWidget::mouseMoveEvent( QMouseEvent *e )
{
  int x=e->pos().x();
  if ((x<width)&&(x>=0))
    {
      int y=e->pos().y();

      emit freqInfo (rate/2*x/width+(rate/4)/width,(rate/4)/width);

      if (db)
	  emit dbInfo ((height-y)*db/(height),0);
      else
      if ((y>=0)&&(y<height))
	  emit ampInfo ((height-y)*100/(height),(int) floor(100/(double)height+.5));
    }
}
//****************************************************************************
void FFTWidget::drawOverviewPhase ()
{	
  if (fftsize)
    {
      int step;
      double max=0,min=0;

      p.setPen (white);

	for (int i=0;i<width;i++)
	  {
	    step=((int) zoom*i);
	    getMaxMinPhase (&data[step],(int)((zoom)+1),max,min);
	    max=(max)*height;
	    min=(min)*height;
	    p.drawLine (i,-(int)max,i,-(int)min);
	  }
    }
}
//****************************************************************************
void FFTWidget::drawOverviewFFT ()
{	
  if (fftsize)
    {
      int step;
      double max=0,min=0;

      p.setPen (white);

      for (int i=0;i<width;i++)
	{
	  step=((int) zoom*i);
	  getMaxMinPower (&data[step],(int)((zoom)+1),&max,&min);
	  max=(max)*height/this->max;
	  min=(min)*height/this->max;
	  p.drawLine (i,-(int)max,i,-(int)min);
	}
    }
}
//****************************************************************************
void FFTWidget::drawInterpolatedDB ()
{
  if (fftsize)
    {
      int lx,ly,y,x;
      double rea,ima;

      lx=0;
      rea=data[0].real;
      ima=data[0].imag;
      rea=sqrt(rea*rea+ima*ima);

      rea/=max;
      if (rea!=0) rea=-6*(1/log10(2))*log10(rea)/db;
      ly=(int)(rea-1)*height;


      for (int i=0;i<fftsize/2;i++)
	{
	  x=(int) (i/zoom);	

	  rea=data[i].real;
	  ima=data[i].imag;
	  rea=sqrt(rea*rea+ima*ima);
	  rea/=max;
	  if (rea!=0) rea=-6*(1/log10(2))*log10(rea)/db;
	  else rea=2;
	  y=(int)(rea-1)*height;

	  p.drawLine (lx,ly,x,y);
	  lx=x;
	  ly=y;
	}
    }
}
//****************************************************************************
__inline double getDB (double max,double x,double db)
{
  x/=max;
  x=-6*(1/log10(2))*log10(x)/db;
  x--;
  return x;
}
//****************************************************************************
void FFTWidget::drawOverviewDB ()
{	
  if (fftsize)
    {
      int step;
      double stepmax=0,stepmin=0;

      p.setPen (white);

      for (int i=0;i<width;i++)
	{
	  step=((int) zoom*i);
	  getMaxMinPower (&data[step],(int)((zoom)+1),&stepmax,&stepmin);

	  if (stepmax!=0) stepmax=getDB (max,stepmax,db);
	  else stepmax=1;
	  if (stepmin!=0) stepmin=getDB (max,stepmin,db);
	  else stepmin=1;

	  stepmax=stepmax*height;
	  stepmin=stepmin*height;
	  p.drawLine (i,(int)stepmax,i,(int)stepmin);
	}
    }
}
//****************************************************************************
void FFTWidget::drawInterpolatedPhase ()
{
  if (fftsize)
    {
      int lx,ly,y,x;
      double rea,ima;

      lx=0;
      rea=data[0].real;
      ima=data[0].imag;
      rea=(atan(ima/rea)+M_PI/2)/M_PI;
      ly=(int)(rea*height);

      for (int i=0;i<fftsize/2;i++)
	{
	  x=(int) (i/zoom);	

	  rea=data[i].real;
	  ima=data[i].imag;
	  rea=(atan(ima/rea)+M_PI/2)/M_PI;
	  y=(int)(rea*height);

	  p.drawLine (lx,-ly,x,-y);
	  lx=x;
	  ly=y;
	}
    }
}
//****************************************************************************
void FFTWidget::drawInterpolatedFFT ()
{
  if (fftsize)
    {
      int      lx,ly,y,x;
      double rea,ima;

      lx=0;
      rea=data[0].real;
      ima=data[0].imag;
      rea=sqrt(rea*rea+ima*ima);
      ly=(int)((rea/this->max)*height);

      for (int i=0;i<fftsize/2;i++)
	{
	  x=(int) (i/zoom);	

	  rea=data[i].real;
	  ima=data[i].imag;
	  rea=sqrt(rea*rea+ima*ima);
	  y=(int)((rea/this->max)*height);

	  p.drawLine (lx,-ly,x,-y);
	  lx=x;
	  ly=y;
	}
    }
}
//****************************************************************************
void FFTWidget::paintEvent  (QPaintEvent *)
{

  ///if pixmap has to be resized ...
  if ((rect().height()!=height)||(rect().width()!=width)||redraw)
    {
      redraw=false;
      height=rect().height();
      width=rect().width();

      if (pixmap) delete pixmap;
      pixmap=new QPixmap (size());

      pixmap->fill (black);
      p.begin (pixmap);
      p.translate (0,height);

      p.setPen (white);
      if (fftsize!=0)
	{
	  zoom=((double)fftsize)/(width*2);

	  if (phaseview)
	    {
	      if (zoom>1) drawOverviewPhase();
	      else drawInterpolatedPhase();
	    }
	  else
	    {
	      if (db)
		{
		  if (zoom>1) drawOverviewDB();
		  else drawInterpolatedDB();
		}
	      else
		{
		  if (zoom>1) drawOverviewFFT();
		  else drawInterpolatedFFT();
		}
	    }
	}
      p.end();
    }

  //blit pixmap to window
  if (pixmap) bitBlt (this,0,0,pixmap);
}
//****************************************************************************
FFTContainer::FFTContainer (QWidget *parent): QWidget (parent)
{
  this->view=0;
}
//****************************************************************************
void FFTContainer::setObjects (FFTWidget *view,ScaleWidget *x,ScaleWidget *y,CornerPatchWidget *corner)
{
  this->view=view;
  this->xscale=x;
  this->yscale=y;
  this->corner=corner;
}
//****************************************************************************
FFTContainer::~FFTContainer ()
{
}
//****************************************************************************
void FFTContainer::resizeEvent	(QResizeEvent *)
{
  if (view)
    {
      int bsize=(QPushButton("test",this).sizeHint()).height();
      view->setGeometry	(bsize,0,width()-bsize,height()-bsize);  
      xscale->setGeometry	(bsize,height()-bsize,width()-bsize,bsize);  
      yscale->setGeometry	(0,0,bsize,height()-bsize);
      corner->setGeometry	(0,height()-bsize,bsize,bsize);
    }
}
//****************************************************************************
FFTWindow::FFTWindow (QString *name) : KTopLevelWidget (name->data())
{
  QPopupMenu *fft=	new QPopupMenu ();
  QPopupMenu *view=	new QPopupMenu ();
  QPopupMenu *edit=	new QPopupMenu ();
  QPopupMenu *dbmenu=	new QPopupMenu ();
  KMenuBar   *bar=	new KMenuBar (this); 

  bar->insertItem	(klocale->translate("&Spectral Data"),fft);
  bar->insertItem	(klocale->translate("&Edit"),edit);
  bar->insertItem	(klocale->translate("&View"),view);

  status=new KStatusBar (this,"Frequencies Status Bar");
  status->insertItem ("Frequency:          0 Hz     ",1);
  status->insertItem ("Amplitude:    0 %      ",2);
  status->insertItem ("Phase:    0        ",3);  

  mainwidget=new FFTContainer (this);

  fftview=new FFTWidget (mainwidget);
  xscale=new ScaleWidget (mainwidget,0,100,"Hz");
  yscale=new ScaleWidget (mainwidget,100,0,"%");
  corner=new CornerPatchWidget (mainwidget);

  mainwidget->setObjects (fftview,xscale,yscale,corner);

  edit->insertItem	(klocale->translate("Multiply with graph"),fftview,SLOT(amplify()));
  edit->insertItem	(klocale->translate("Multiply with formant pattern"),fftview,SLOT(formant()));
  edit->insertItem	(klocale->translate("Smooth"),fftview,SLOT(smooth()));
  edit->insertSeparator	();
  edit->insertItem	(klocale->translate("Kill phase"),fftview,SLOT(killPhase()));

  fft->insertItem	(klocale->translate("Inverse FFT"),fftview,SLOT(iFFT()));
  view->insertItem	(klocale->translate("Amplitude in %"),this,SLOT(percentMode()));
  view->insertItem	(klocale->translate("Amplitude in dB"),dbmenu);

  for (int i=0;i<110;i+=10)
    {
      char buf[10];
      sprintf (buf,"0-%d dB",i+50);
      dbmenu->insertItem (buf);
    }
  connect (dbmenu,SIGNAL (activated(int)),this,SLOT(dbMode(int)));
  
  view->insertItem (klocale->translate("Phase"),this,SLOT(phaseMode()));

  connect (fftview,SIGNAL(freqInfo(int,int)),this,SLOT(setFreqInfo(int,int)));
  connect (fftview,SIGNAL(ampInfo(int,int)),this,SLOT(setAmpInfo(int,int)));
  connect (fftview,SIGNAL(dbInfo(int,int)),this,SLOT(setDBInfo(int,int)));
  connect (fftview,SIGNAL(phaseInfo(int,int)),this,SLOT(setPhaseInfo(int,int)));
  setView (mainwidget);
  setStatusBar (status);
  setMenu (bar);
  
 QString *windowname=new QString (QString ("Frequencies of ")+QString(name->data()));
  setCaption (windowname->data()); 
  resize (480,300);
  setMinimumSize (480,300);
}
//****************************************************************************
void FFTWindow::phaseMode  ()
{
  fftview->phaseMode ();
  yscale->setMaxMin (180,-180);
  yscale->setUnit   ("°");
}
//****************************************************************************
void FFTWindow::percentMode  ()
{
  fftview->percentMode ();
  yscale->setMaxMin (0,100);
  yscale->setUnit   ("%");
}
//****************************************************************************
void FFTWindow::dbMode  (int mode)
{
  fftview->dbMode (50+mode*10);
  yscale->setMaxMin (-(50+mode*10),0);
  yscale->setUnit   ("db");
}
//****************************************************************************
void FFTWindow::setSignal (complex *data,double max,int size, int rate)
{
  fftview->setSignal (data,size,rate);
  xscale ->setMaxMin (rate/2,0); 
}
//****************************************************************************
FFTWindow::~FFTWindow ()
{
}
//****************************************************************************
void FFTWindow::setFreqInfo  (int hz,int err)
{
  char buf[64];

  sprintf (buf,"Frequency: %d +/-%d Hz",hz,err);
  status->changeItem (buf,1);
}
//****************************************************************************
void FFTWindow::setAmpInfo  (int amp,int err)
{
  char buf[64];
  
  sprintf (buf,"Amplitude: %d +/-%d %%",amp,err);
  status->changeItem (buf,2);
}
//****************************************************************************
void FFTWindow::setDBInfo  (int amp,int err)
{
  char buf[64];
  sprintf (buf,"Amplitude: %d dB",amp);
  status->changeItem (buf,2);
}
//****************************************************************************
void FFTWindow::setPhaseInfo  (int ph,int err)
{
  char buf[64];

  sprintf (buf,"Phase: %d +/- %d",ph,err);
  status->changeItem (buf,3);
}




