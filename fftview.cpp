#include "fftview.h"
#include <qpainter.h>
#include <math.h>
#include <limits.h>

__inline void  getMaxMinImag (complex *sample,int len,double &max,double &min)
{
  double c;
  min=INT_MAX;
  max=INT_MIN;
  for (int i=0;i<len;i++)
    {
      c=sample[i].imag;
      if (c>max) max=c;
      if (c<min) min=c;
    }
}
//****************************************************************************
__inline void  getMaxMinReal (complex *sample,int len,double &max,double &min)
{
  double c;
  min=INT_MAX;
  max=INT_MIN;
  for (int i=0;i<len;i++)
    {
      c=sample[i].real;
      if (c>max) max=c;
      if (c<min) min=c;
    }
}
//****************************************************************************
FFTWidget::FFTWidget (QWidget *parent,const char *name) : QWidget
(parent,name)
{
  data=0;
  fftsize=0;
  height=-1;
  pixmap=0;
}
//****************************************************************************
FFTWidget::~FFTWidget (QWidget *parent,const char *name)
{
	if (pixmap==0)	delete pixmap;
	if (data!=0)	delete data;
}
//****************************************************************************
void FFTWidget::setSignal  (complex *data,int size, int rate)
{
	this->data=data;
	this->fftsize=size;
	this->rate=rate;
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
	  if (y<height/2)
	  emit ampInfo ((height/2-y)*100/(height/2)+100/height,(int) floor(100/(double)height+.5));
	  else
	  emit phaseInfo ((y-height/2)*360/(height/2)+360/height,(int) floor(360/(double)height+.5));
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
  printf ("overview\n");
  if (fftsize)
    {
      int step;
      double max=0,min=0;

      p.setPen (yellow);
      for (int i=0;i<width;i++)
	{
	  step=((int) zoom*i);
	  getMaxMinImag (&data[step],(int)((zoom)+1),max,min);
	  max=(max)*height/2;
	  min=(min)*height/2;
	  p.drawLine (i,(int)max,i,(int)min);
	}
      p.setPen (white);
      for (int i=0;i<width;i++)
	{
	  step=((int) zoom*i);
	  getMaxMinReal (&data[step],(int)((zoom)+1),max,min);
	  max=(max)*height/2;
	  min=(min)*height/2;
	  p.drawLine (i,-(int)max,i,-(int)min);
	}
    }
}
//****************************************************************************
void FFTWidget::drawInterpolatedFFT ()
{
  printf ("interpolated zoom=%e\n",zoom);

  if (fftsize)
    {
      int      lx,ly,y,x;

      lx=0;
      ly=0;
      for (int i=0;i<fftsize/2;i++)
	{
	  x=(int) (i/zoom);	
	  y=(int)((data[i].real)*height/2);

	  p.drawLine (lx,-ly,x,-y);
	  lx=x;
	  ly=y;
	}
      p.setPen (yellow);

      lx=0;
      ly=0;
      for (int i=0;i<fftsize/2;i++)
	{
	  x=(int) (i/zoom);	
	  y=(int)((data[i].imag)*height/2);

	  p.drawLine (lx,ly,x,y);
	  lx=x;
	  ly=y;
	}
    }
}
//****************************************************************************
void FFTWidget::paintEvent  (QPaintEvent *)
{

  ///if pixmap has to be resized ...
  if ((rect().height()!=height)||(rect().width()!=width))
    {
      
      height=rect().height();
      width=rect().width();

      if (pixmap) delete pixmap;
      pixmap=new QPixmap (size());

      p.begin (pixmap);
      p.translate (0,height/2);
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
  KMenuBar *bar=	new KMenuBar (this,"Frequency Menu"); 


  view->insertItem	(klocale->translate("Linear"),	this,SLOT(fadeInlOp()));
  view->insertItem	(klocale->translate("Logarithmic"),	this,SLOT(fadeInLogOp()));

  bar->insertItem	(klocale->translate("View"),view);

  status=new KStatusBar (this,"Frequencies Status Bar");
  status->insertItem ("Frequency:          0 Hz     ",1);
  status->insertItem ("Amplitude:    0 %      ",2);
  status->insertItem ("Phase:    0        ",3);  

  fftview=new FFTWidget (this); fftview->setBackgroundColor (QColor(black) );

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
void FFTWindow::setSignal (complex *data,int size, int rate)
{
  fftview->setSignal (data,size,rate);
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




