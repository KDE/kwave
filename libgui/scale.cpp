
#include <stdio.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qdir.h>
#include <kapp.h>
#include "scale.h"

#define FONTSIZE 6
QPixmap *scalefont=0;
int scalefontusage=0;
extern KApplication *app;

//**********************************************************
int getlessten (int val)
// returns the next smaller power of 10
{
  int x=1;
  while (val>x*10) x*=10;
  return x;
}
//**********************************************************
ScaleWidget::ScaleWidget (QWidget *parent,int low,int high,char *text): QWidget (parent)
{
  this->low=low;
  this->high=high;
  this->unittext=text;

  logmode=false;

  if (!scalefont)
    {
      scalefont=new QPixmap ();

      const QString dirname=app->kde_datadir ();
      QDir dir (dirname.data());
      dir.cd ("kwave");
      dir.cd ("pics");
      
      QImage  *img=new QImage(dir.filePath("font.xpm").data());

      if (scalefont&&img)
	scalefont->convertFromImage(*img);
    }
  scalefontusage++;
}
//**********************************************************
ScaleWidget::~ScaleWidget ()
{
  scalefontusage--;
  if ((scalefontusage==0)&&scalefont)
    {
      delete scalefont;
      scalefont=0;
    }
}
//**********************************************************
void ScaleWidget::setUnit (char *text)
{
  this->unittext=text;
  repaint ();
}
//**********************************************************
void ScaleWidget::setLogMode (bool a)
{
  if (logmode!=a)
    {
      logmode=a;
      repaint ();
    }
}
//**********************************************************
void ScaleWidget::setMaxMin (int b,int a)
{
  low=a;
  high=b;
  repaint (true);
}
//**********************************************************
void ScaleWidget::paintText  (QPainter *p,int x, int y,int ofs,int dir,char *text)
  //painting routine for own small font with fixed size
  //There are Problems with smaller displays using QFont, Sizes are not correct
{
  if (scalefont)
    {
      int len=strlen(text);

      for (int i=0;i<len;i++)
	{
	  int c=text[i];
	  if ((c>64)&&(c<91)) //letter
	    p->drawPixmap (x,y,*scalefont,(c-65)*FONTSIZE,0,FONTSIZE,FONTSIZE);
	  if ((c>96)&&(c<123)) //letter
	    p->drawPixmap (x,y,*scalefont,(c-97)*FONTSIZE,0,FONTSIZE,FONTSIZE);

	  if ((c>47)&&(c<58)) //number
	    p->drawPixmap (x,y,*scalefont,(c-48+26)*FONTSIZE,0,FONTSIZE,FONTSIZE);
	  if (c=='°')  p->drawPixmap (x,y,*scalefont,(38)*FONTSIZE,0,FONTSIZE,FONTSIZE);
	  if (c=='.')  p->drawPixmap (x,y,*scalefont,(39)*FONTSIZE,0,FONTSIZE,FONTSIZE);
	  if (c=='-')  p->drawPixmap (x,y,*scalefont,(42)*FONTSIZE,0,FONTSIZE,FONTSIZE);
	  if (c=='%')
	    {
	      p->drawPixmap (x,y,*scalefont,(40)*FONTSIZE,0,FONTSIZE,FONTSIZE);
	      p->drawPixmap (x+ofs,y,*scalefont,(41)*FONTSIZE,0,FONTSIZE,FONTSIZE);
	      if (dir) x+=ofs;
	    }



	  if (dir) x+=ofs;
	  else y+=ofs;
	}
    }
}
//**********************************************************
void ScaleWidget::drawLog  (QPainter *p, int w,int h)
{
  if (w>h) //horizontal orientation
    {
      p->setPen (colorGroup().dark());
      p->drawLine (0,h-1,w,h-1);
      p->drawLine (w-1,0,w-1,h-1);

      p->setPen (colorGroup().text());
    }
  else
    {
      p->drawLine (0,0,0,h);
      p->setPen (colorGroup().dark());
      p->drawLine (w-1,0,w-1,h-1);
      p->setPen (colorGroup().text());
    }
}
//**********************************************************
void ScaleWidget::drawLinear  (QPainter *p, int w,int h)
{
  if (w>h) //horizontal orientation
    {
      p->setPen (colorGroup().dark());
      p->drawLine (0,h-1,w,h-1);
      p->drawLine (w-1,0,w-1,h-1);

      p->setPen (colorGroup().text());

      int a,x;
      double ofs;
      double t=w-1;
      int h2=h;
      
      while ((t/10>1)&&(h2>0))
	{
	  for (ofs=0;ofs<w-1;ofs+=t)
	    {
	      for (a=0;a<6;a++)
		{
		  x=(int)(ofs+(t*a/5));
		  p->drawLine (x,1,x,h2-2);
		}
	    }

	  h2=h2>>1;
	  t/=5;
	}

      for (a=0;a<5;a++)
	{
	  char buf[16];
	  sprintf (buf,"%d %s",low+((high-low)*a/5),unittext);
	  x=(w-1)*a/5;
	  paintText (p,x+2,h-FONTSIZE-2,FONTSIZE,true,buf);
	}
    }
  else
    {
      p->drawLine (0,0,0,h);
      p->setPen (colorGroup().dark());
      p->drawLine (w-1,0,w-1,h-1);
      p->setPen (colorGroup().text());

      int a,y;
      double ofs;
      double t=h-1;
      int h2=w-1;
      
      while ((t/10>1)&&(h2>0))
	{
	  for (ofs=0;ofs<h-1;ofs+=t)
	    {
	      for (a=0;a<6;a++)
		{
		  y=(int)(ofs+(t*a/5));
		  p->drawLine (w-h2,y,w-1,y);
		}
	    }
	  h2=h2>>1;
	  t/=5;
	}

      for (a=0;a<5;a++)
	{
	  char buf[16];
	  sprintf (buf,"%d%s",low+((high-low)*a/5),unittext);
	  y=(h-1)*a/5+1;
	  paintText (p,2,y,FONTSIZE,false,buf);
	}
    }
}
//**********************************************************
void ScaleWidget::paintEvent  (QPaintEvent *)
{
  int h=height();
  int w=width();
  QPainter p;
  p.begin (this);

  p.setPen (colorGroup().light());
  p.drawLine (0,0,w,0);
  
  if (logmode) drawLog (&p,w,h);
  else drawLinear (&p,w,h);

  p.end ();
}
//**********************************************************
CornerPatchWidget::CornerPatchWidget (QWidget *parent,int pos): QWidget (parent)
{
  this->pos=pos;
}
//**********************************************************
CornerPatchWidget::~CornerPatchWidget ()
{
}
//**********************************************************
void CornerPatchWidget::paintEvent  (QPaintEvent *)
{
  int h=height();
  int w=width();
  QPainter p;
  p.begin (this);

  p.setPen (colorGroup().light());
  p.drawLine (0,0,0,h-1);
  p.setPen (colorGroup().dark());
  p.drawLine (0,h-1,w,h-1);
  p.end ();
}







