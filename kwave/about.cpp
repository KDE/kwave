#include "about.h"
#include <qaccel.h>
#include <qpntarry.h>
#include <math.h>

char *about_text="Programming by Martin Wilz\n(mwilz@ernie.mi.uni-koeln.de)\
\n\nAlpha-Testing by Carsten Jacobi\n\
(carsten@jakes.kawo1.rwth-aachen.de)\n\
\nFFT-Code by GNU gsl-Project\n\
GSL-Library may be retrieved from ftp://alpha.gnu.org/gnu/ !\n\
";

AboutDialog::AboutDialog (QWidget *par=NULL): QDialog(par, "Choose Length and Rate",true)
{
  resize 		(320,200);
  setCaption	("About KWave");

  abouttext=new QMultiLineEdit (this);
  ok		=new QPushButton ("Ok",this);

  logo=new LogoWidget (this);

  abouttext->setText(about_text);
  abouttext->setReadOnly(TRUE);

  int bsize=ok->sizeHint().height();
  setMinimumSize (320,bsize*6);
  setMaximumSize (640,400);

  ok->setAccel	(Key_Return);
  ok->setFocus	();
  connect 	(ok	,SIGNAL(clicked()),SLOT (accept()));
}
//**********************************************************
void AboutDialog::resizeEvent (QResizeEvent *)
{
  int bsize=ok->sizeHint().height();

  abouttext->setGeometry(width()/6,0,width()*5/6,height()-bsize*3/2);  
  logo->setGeometry	(0,0,width()/6,height()-bsize*3/2);  
  ok->setGeometry	(width()/20,height()-bsize*5/4,width()*18/20,bsize);  
}
//**********************************************************
AboutDialog::~AboutDialog ()
{
}
//**********************************************************
LogoWidget::LogoWidget (QWidget *parent): QWidget (parent)
{
  for (int i=0;i<MAXSIN;deg[i++]=0);
  height=-1;
  pixmap=0;

  img=new QPixmap ();
  img->convertFromImage (QImage ("logo.xpm"));
  this->setBackgroundColor (black);

  timer=new QTimer (this);

  connect (timer,SIGNAL(timeout()),this,SLOT(doAnim()));

  timer->start (40,false);  //gives 50 Hz refresh... qt-lib gives interlace-feeling ;-)...
}
//**********************************************************
void LogoWidget::doAnim ()
{
  double mul=0.04131211+deg[MAXSIN-1]/75;

  for (int i=0;i<MAXSIN;i++)
    {
      deg[i]+=mul;
      if (deg[i]>2*PI) deg[i]=0;
      mul=((mul*521)/437);
      mul-=floor(mul);  // gives again a number between 0 and 1
      mul/=17;
      mul+=deg[i]/100; //so that chaos may be ...
    }

  rpaint=true;
  repaint ();
}
//**********************************************************
LogoWidget::~LogoWidget ()
{
  if (img) delete img;
  timer->stop();
}
//**********************************************************
void LogoWidget::paintEvent  (QPaintEvent *)
{

  ///if pixmap has to be resized ...
  if ((rect().height()!=height)||(rect().width()!=width))
    {
      height=rect().height();
      width=rect().width();

      if (pixmap) delete pixmap;
      pixmap=new QPixmap (size());
      rpaint=true;
    }

  if ((rpaint)&&(pixmap))
    {
      QPointArray si(20+3);

      p.begin (pixmap);
      p.setPen (white);
      p.setBrush (white);
      p.drawRect (0,0,width,height);
      p.setRasterOp (XorROP);
      
      double amp=sin (deg[MAXSIN-1]*3);
      for (int j=0;j<MAXSIN;j++)
	{
	  for (int i=0;i<21;i++)
	    si.setPoint (i,
			 (j*width/MAXSIN)+(int) (amp*sin(PI*i/10+deg[j])*width/2),height*i/20);


	  si.setPoint (21,width/2,height);
	  si.setPoint (22,width/2,0);	

	  p.drawPolygon (si);
	  amp=sin (deg[j]*3);
	}

      p.end();
      rpaint=false;
    }

  //blit pixmap to window in every case
  int ampx=(img->width()-width)/2;
  int ampy=(img->height()-height)/2;

  if (pixmap)
    {
      bitBlt (this,-ampx+(int) (sin(deg[0])*ampx),-ampy+(int) (sin(deg[1])*ampy),
	      img,0,0,img->width(),img->height(),CopyROP);    
      bitBlt (this,0,0,pixmap,0,0,width,height,XorROP);
    }
}
