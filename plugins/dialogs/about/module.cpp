#include <math.h>
#include <qaccel.h>
#include <qpntarry.h>
#include <qdir.h>
#include <stdio.h>
#include <stdlib.h>
#include <qpushbutton.h>
#include <qkeycode.h>
#include "module.h"
#include <kapp.h>

const char *version = "1.0";
const char *author = "Martin Wilz";
const char *name = "about";
//**********************************************************
Dialog *getDialog (DialogOperation *operation) {
    return new AboutDialog(operation->getGlobals(), operation->isModal());
}
//**********************************************************
char about_text[] = "\nKwave Version "VERSION"\n\
"VERSION_DATE"\n\
(c) 1998-99 by Martin Wilz (mwilz@ernie.mi.uni - koeln.de)\n\
\nFFT - Code by GNU gsl - Project, library version 0.3 beta\n\
(GSL - Library may be retrieved from ftp://alpha.gnu.org/gnu/)\n\n\
Memory Mapping routines by Juhana Kouhia\n\
Some Gui modifications and additional functionality by Gerhard Zintel\n\
Complete revision by Thomas Eschenbacher (1999/2000)\n\
\n\
Thanks go to: \n\
\
Carsten Jacobi\n\
Frank Christian Stoffel\n\
Achim Dahlhaus\n\
Klaus Hendrik Lorenz\n\n\
People, who provided valuable feedback (in no particular order)\n\
\
Gerhard Zintel\n\
Gael Duval\n\
Aaron Johnson\n\
Uwe Steinmann\n\
Juhana Kouhia\n\
Dave Phillips\n\
Martin Petriska\n\
Winfried Truemper\n\
Bruce Garlock\n\
Christoph Raab\n\
tOpHEr lAfaTA\n\
Nemosoft\n\
Guido\n\
Eero\n\
\
This program is free software; you can redistribute it and / or\n\
modify it under the terms of the GNU General Public License\n\
as published by the Free Software Foundation; either version 2\n\
of the License, or (at your option) any later version.\n\n\
This program is distributed in the hope that it will be useful, \n\
but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the\n\
GNU General Public License for more details.\n\n\
You should have received a copy of the GNU General Public License\n\
along with this program; if not, write to the Free Software\n\
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.\n\
";
//**********************************************************
AboutDialog::AboutDialog (const Global *globals,bool modal): Dialog(modal)
{
    resize      (600,400);
    setCaption  ("About KWave");

    abouttext=new QMultiLineEdit (this);
    ok          =new QPushButton ("Ok",this);

    logo=new LogoWidget (this, KApplication::getKApplication());

    abouttext->setText(about_text);
    abouttext->setReadOnly(TRUE);
    abouttext->setCursorPosition (0,0);

    int bsize=ok->sizeHint().height();
    setMinimumSize (480,bsize*6);
    setMaximumSize (640,400);

    ok->setAccel        (Key_Return);
    ok->setFocus        ();
    connect     (ok     ,SIGNAL(clicked()),SLOT (accept()));
}
//**********************************************************
void AboutDialog::resizeEvent (QResizeEvent *)
{
    int bsize=ok->sizeHint().height();

    abouttext->setGeometry(width()/6,0,width()*5/6,height()-bsize*3/2);
    logo->setGeometry   (0,0,width()/6,height()-bsize*3/2);
    ok->setGeometry     (width()/20,height()-bsize*5/4,width()*18/20,bsize);
}
//**********************************************************
const char *AboutDialog::getCommand ()
{
    return 0; //about does nothing
}
//**********************************************************
AboutDialog::~AboutDialog ()
{
}
//**********************************************************
LogoWidget::LogoWidget (QWidget *parent,KApplication *app): QWidget (parent)
{
    for (int i=0;i<MAXSIN;deg[i++]=0);
    height=-1;
    buffer=0;
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

    timer->start(40,false);  //gives 50 Hz refresh... qt-lib gives interlace-feeling ;-)...
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
    repaint(false);
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
	if (buffer) delete buffer;

	pixmap=new QPixmap( size() );
	buffer=new QPixmap( size() );
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

    if (pixmap && buffer)
    {
	buffer->fill(black);
// 	bitBlt (this,-ampx+(int) (sin(deg[0])*ampx),-ampy+(int) (sin(deg[1])*ampy),
// 		img,0,0,img->width(),img->height(),CopyROP);
// 	bitBlt (this,0,0,pixmap,0,0,width,height,XorROP);
	
	bitBlt(buffer,-ampx+(int) (sin(deg[0])*ampx),-ampy+(int) (sin(deg[1])*ampy),
		img,0,0,img->width(),img->height(),CopyROP);
	bitBlt(buffer,0,0,pixmap,0,0,width,height,XorROP);
	
	bitBlt(this,0,0,buffer,0,0,width,height,CopyROP);
    }
}

