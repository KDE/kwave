#include "about.h"
#include <qaccel.h>
#include <qpntarry.h>
#include <qdir.h>
#include <math.h>
#include <kapp.h>

extern KApplication *app;
char about_text[]="Kwave Version 0.28alpha, first release\n\
(c) 1998 by Martin Wilz\n\
(mwilz@ernie.mi.uni-koeln.de)\n\n\
Alpha-Testing by Carsten Jacobi\n\
(carsten@jakes.kawo1.rwth-aachen.de)\n\
\nFFT-Code by GNU gsl-Project, library version 0.3 beta\n\
(GSL-Library may be retrieved from ftp://alpha.gnu.org/gnu/)\n\n\
Thanks go to:\n\
Frank Christian Stoffel\n\
Achim Dahlhaus\n\n\
This program is free software; you can redistribute it and/or\n\
modify it under the terms of the GNU General Public License\n\
as published by the Free Software Foundation; either version 2\n\
of the License, or (at your option) any later version.\n\n\
This program is distributed in the hope that it will be useful,\n\
but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n\
GNU General Public License for more details.\n\n\
You should have received a copy of the GNU General Public License\n\
along with this program; if not, write to the Free Software\n\
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.\n";

AboutDialog::AboutDialog (QWidget *par=NULL): QDialog(par, "Choose Length and Rate",true)
{
  resize 		(480,200);
  setCaption	("About KWave");

  abouttext=new QMultiLineEdit (this);
  ok		=new QPushButton ("Ok",this);

  logo=new LogoWidget (this);

  abouttext->setText(about_text);
  abouttext->setReadOnly(TRUE);

  int bsize=ok->sizeHint().height();
  setMinimumSize (480,bsize*6);
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

  QString dirname=app->kde_datadir ();
  QDir dir (dirname.data());
  dir.cd ("kwave");
  dir.cd ("pics");

  img->convertFromImage (QImage(dir.filePath("logo.xpm").data()));
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
      if (deg[i]>2*M_PI) deg[i]=0;
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
			 (j*width/MAXSIN)+(int)(amp*sin(M_PI*i/10+deg[j])*width/2),height*i/20);


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
