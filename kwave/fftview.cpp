#include <qdir.h>
#include "fftview.h"
#include <qpainter.h>
#include <math.h>
#include <limits.h>
#include <qcursor.h>
#include "dialogs.h"
#include "sample.h"
#include "main.h"

extern KApplication *app;
//****************************************************************************
__inline void  getMaxMinReal (complex *sample,int len,double &max,double &min)
{
  double rea,ima;
  double c;
  min=INT_MAX;
  max=INT_MIN;
  for (int i=0;i<len;i++)
    {
      rea=sample[i].real;
      ima=sample[i].imag;
      c=sqrt(rea*rea+ima*ima);
      if (c>max) max=c;
      if (c<min) min=c;
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
FFTWidget::FFTWidget (QWidget *parent,const char *name)
 : QWidget (parent,name)
{
  data=0;
  autodelete=true;
  fftsize=0;
  height=-1;
  pixmap=0;
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
void FFTWidget::setPhase  (complex *data,double max,int size, int rate)
{
  phase=true;
  this->max=max;
  this->data=data;
  this->fftsize=size;
  this->rate=rate;
}
//****************************************************************************
void FFTWidget::setSignal  (complex *data,double max,int size, int rate)
{
  phase=false;
  this->max=max;
  this->data=data;
  this->fftsize=size;
  this->rate=rate;
}
//****************************************************************************
void FFTWidget::refresh()
{
  redraw=true;
  repaint ();
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

	  redraw=true;
	  repaint ();
	}  

    }
}
//****************************************************************************
void FFTWidget::amplify()
{
  AmplifyCurveDialog *dialog =new AmplifyCurveDialog (this);
  if (dialog->exec())
    {
      QList<CPoint> *points=dialog->getPoints ();

      Interpolation interpolation (dialog->getType());

      double *y=interpolation.getInterpolation (points,fftsize/2);

      for (int i=0;i<fftsize/2;i++)
	{
	  data[i].real*=sqrt(2*y[i]);
	  data[i].imag*=sqrt(2*y[i]);
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
      if ((y>=0)&&(y<height))
	{
	  emit ampInfo ((height-y)*100/(height),(int) floor(100/(double)height+.5));
	}
      /*
	if (zoom<1)
	{
	emit phaseInfo((int) (data[(int)(x*zoom)].imag*360),0);
	emit ampInfo  ((int) (data[(int)(x*zoom)].real*100),0);
	}
	else
	{
	double min,max;
	getMaxMinImag (&data[(int)(x*zoom)],int (zoom)+1,min,max);
	emit phaseInfo((int) ((max+min)*180),int ((max-min)*180));
	getMaxMinReal (&data[(int)(x*zoom)],int (zoom)+1,min,max);
	emit ampInfo((int) ((max+min)*180),int ((max-min)*180));
	}
	*/
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


      if (phase)
	for (int i=0;i<width;i++)
	  {
	    step=((int) zoom*i);
	    getMaxMinPhase (&data[step],(int)((zoom)+1),max,min);
	    max=(max)*height;
	    min=(min)*height;
	    p.drawLine (i,-(int)max,i,-(int)min);
	  }
      else
	for (int i=0;i<width;i++)
	  {
	    step=((int) zoom*i);
	    getMaxMinReal (&data[step],(int)((zoom)+1),max,min);
	    max=(max)*height/this->max;
	    min=(min)*height/this->max;
	    p.drawLine (i,-(int)max,i,-(int)min);
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

      if (phase)
	  {
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
	else
	  {
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

      double ticks=width/5;
      if (ticks>16)
	{
	  p.setPen (gray);
	  double subticks=ticks/4;

	  for (int i=1;i<5*4;i++)
	    p.drawLine ((int) (i*subticks),0,(int) (i*subticks),-4);


	  for (int i=1;i<5;i++)
	    p.drawLine ((int) (i*ticks),0,(int) (i*ticks),-height);
	}

      ticks=height/5;
      if (ticks>16)
	{
	  p.setPen (gray);
	  double subticks=ticks/4;

	  for (int i=1;i<5*4;i++)
	    p.drawLine (0,-(int) (i*subticks),4,-(int) (i*subticks));


	  for (int i=1;i<5;i++)
	    p.drawLine (0,-(int) (i*ticks),width,-(int) (i*ticks));
	}

      p.setPen (white);
      if (fftsize!=0)
	{
	  zoom=((double)fftsize)/(width*2);

	  if (zoom>1)
	    drawOverviewFFT();
	  else drawInterpolatedFFT();
	}
      p.end();
    }

  //blit pixmap to window
  if (pixmap) bitBlt (this,0,0,pixmap);
}
//****************************************************************************
FFTWindow::FFTWindow (const char *name) : KTopLevelWidget (name)
{
  QPopupMenu *view=	new QPopupMenu ();
  QPopupMenu *edit=	new QPopupMenu ();
  KMenuBar *bar=	new KMenuBar (this); 

  bar->insertItem	(klocale->translate("Spectral Data"),view);
  bar->insertItem	(klocale->translate("Edit"),edit);

  status=new KStatusBar (this,"Frequencies Status Bar");
  status->insertItem ("Frequency:          0 Hz     ",1);
  status->insertItem ("Amplitude:    0 %      ",2);
  status->insertItem ("Phase:    0        ",3);  

  fftview=new FFTWidget (this);
  edit->insertItem	(klocale->translate("Multiply"),fftview,SLOT(amplify()));
  edit->insertItem	(klocale->translate("Smooth"),fftview,SLOT(smooth()));
  edit->insertSeparator	();
  edit->insertItem	(klocale->translate("Kill phase"),fftview,SLOT(killPhase()));

  view->insertItem	(klocale->translate("Inverse FFT"),fftview,SLOT(iFFT()));

  connect (fftview,SIGNAL(freqInfo(int,int)),this,SLOT(setFreqInfo(int,int)));
  connect (fftview,SIGNAL(ampInfo(int,int)),this,SLOT(setAmpInfo(int,int)));
  connect (fftview,SIGNAL(phaseInfo(int,int)),this,SLOT(setPhaseInfo(int,int)));

  setView (fftview);
  setStatusBar (status);
  setMenu (bar);
  
  setCaption ("Frequencies :"); 
  resize (320,200);
}
//****************************************************************************
void FFTWindow::setSignal (complex *data,double max,int size, int rate)
{
  fftview->setSignal (data,max,size,rate);
}
//****************************************************************************
FFTWindow::~FFTWindow (QWidget *parent,const char *name)
{
  if (fftview)	delete fftview;
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
void FFTWindow::setPhaseInfo  (int ph,int err)
{
  char buf[64];

  sprintf (buf,"Phase: %d +/- %d",ph,err);
  status->changeItem (buf,3);
}




