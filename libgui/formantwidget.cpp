#include <qpainter.h>
#include <qkeycode.h>
#include <math.h>
#include "formantwidget.h"
#include "fftview.h"

//Formant modell according to G. Fant, acoustic theory of speech production,
//published by Moutom & Co,Netherlands, 1960, p. 48ff

extern const char *OK;
extern const char *CANCEL;
//****************************************************************************
FormantWidget::FormantWidget (QWidget *parent,int rate) : QWidget
(parent)
{
  height=-1;
  this->setBackgroundColor (QColor(black));    
  this->rate=rate;
  num=0;
  pos=0;
  widths=0;
  points=0;
}
//****************************************************************************
FormantWidget::~FormantWidget (QWidget *parent,const char *name)
{
  if (pos) delete pos;
  if (widths) delete widths;
}
//****************************************************************************
double *FormantWidget::getPoints (int psize)
{
  double x=0;
  double freq; //input frequency
  double p1;
  double p2;
  double pos2;
  double q;
  int i;

  points=new double [psize];

  if (points)
    {
      for (i=0;i<psize;i++)
	{
	  freq=(double)(5000*i+.1)/psize;
	  x=freq/(330/4);      //carelessly assumed constant
	 
	  freq*=freq;
	  for (int j=0;j<num;j++) //add every formant
	    {
	      q=pos[j]/widths[j];
	      pos2=(pos[j]*pos[j]);

	      p1=1-(freq/pos2);
	      p1*=p1;
	      p2=freq/(pos2*q*q);
	      x+=10*log10(p1+p2);
	    }
	  points[i]=x/num;
	}
      //eliminate function error below 50 Hz
      int border=psize*50/5000;
      for (i=0;i<border;i++) points[i]=points[border];

    }
  return points;
}
//****************************************************************************
void FormantWidget::setFormants (int num, int *pos,int *widths)
{
  if (this->pos) delete this->pos;
  if (this->widths) delete this->widths;
  this->num=num;
  this->pos=pos;
  this->widths=widths;

  repaint ();
}
//****************************************************************************
void FormantWidget::paintEvent  (QPaintEvent *)
{
  QPainter p;
  double *points;

  height=rect().height();
  width=rect().width();

  p.begin (this);
  p.setPen (white);

  if (num)
    {
      int i;
      double max=0;
      double min=10000;

      points=getPoints (width); //get function points

      if (points) 
	{
	  for (i=0;i<width;i++) //find max
	    {
	      if (max<points[i]) max=points[i];
	      if (min>points[i]) min=points[i];
	    }

	  //scale for display
	  for (i=0;i<width;i++) points[i]=(points[i]-min)/(max-min);

	  //set display of scale-range
	  emit dbscale((int) min,(int) max); 


	  for (i=0;i<width;i++)
	    p.drawLine (i,(int)(points[i]*height),i+1,(int)(points[i+1]*height));
	  delete points;
	  points=0;
	}
    }
  p.end();
}
//**********************************************************
FormantDialog::FormantDialog (QWidget *par,int rate): QDialog(par,0,true)
{
  setCaption	(klocale->translate("Choose formant positions and widths :"));
  pos=0;
  oldnum=0;
  inwidget=false;
  this->rate=rate;

  ok	 =new QPushButton (OK,this);
  cancel =new QPushButton (CANCEL,this);
  formant=new FormantWidget (this,rate);
  num    =new KIntegerLine(this);

  x=new ScaleWidget (this,0,5000,"Hz");
  y=new ScaleWidget (this,100,0,"db");
  corner=new CornerPatchWidget (this);
 
  poslabel=new QLabel (klocale->translate("Formant center in Hz:"),this);
  numlabel=new QLabel (klocale->translate("Number of formants:"),this);
  num->setValue (4);
  getWidgets (4);

  ok->setAccel	(Key_Return);
  cancel->setAccel(Key_Escape);
  ok->setFocus	();
  connect 	(num	,SIGNAL(textChanged(const char *)),SLOT (numberChanged(const char *)));
  connect 	(ok	,SIGNAL(clicked()),SLOT (accept()));
  connect 	(cancel	,SIGNAL(clicked()),SLOT (reject()));
  connect 	(formant,SIGNAL(dbscale(int,int)),SLOT (setScale(int,int)));
}
//**********************************************************
void FormantDialog::posChanged  (const char *str)
{
  for (int i=0;i<oldnum;i++) 
    if (strcmp(pos[i]->text(),str)==0)
      {
	inwidget=true;
	posslider[i]->setValue (strtol(str,0,0));
	inwidget=false;
      }
  refresh ();
}
//****************************************************************************
void FormantDialog::widthChanged  (const char *str)
{
  for (int i=0;i<oldnum;i++) 
    if (strcmp(widths[i]->text(),str)==0)
      {
	inwidget=true;
	widthslider[i]->setValue (strtol(str,0,0));
	inwidget=false;
      }
  refresh ();
}
//****************************************************************************
void FormantDialog::numberChanged  (const char *newstr)
{
  int newnum=strtol (newstr,0,0);

  if (newnum>0) getWidgets (newnum);
}
//****************************************************************************
void FormantDialog::posChanged  (int newvalue)
{
  for (int i=0;i<oldnum;i++) 
    if (posslider[i]->value()==newvalue)
      if (!inwidget)
	{
	  char buf[16];
	  sprintf (buf,"%d Hz",newvalue);
	  pos[i]->setText (buf);
	}
  refresh ();
}
//****************************************************************************
void FormantDialog::widthChanged  (int newvalue)
{
  for (int i=0;i<oldnum;i++) 
    if (widthslider[i]->value()==newvalue)
      if (!inwidget)
	{
	  char buf[16];
	  sprintf (buf,"%d Hz",newvalue);
	  widths[i]->setText (buf);
	}
  refresh ();
}
//****************************************************************************
void FormantDialog::setScale (int min,int max)
{
  y->setMaxMin (min,max);
}
//****************************************************************************
void FormantDialog::getWidgets (int num)
{
  int i;
  
  KIntegerLine **newpos=    new KIntegerLine*[num];
  KwaveSlider  **newslider= new KwaveSlider *[num];
  KIntegerLine **newwidth=  new KIntegerLine*[num];
  KwaveSlider  **newwslider=new KwaveSlider *[num];

  for (i=0;i<num;i++)
    {
      newpos[i]=new KIntegerLine (this);
      newslider[i]=new KwaveSlider (100,5000,1,500+1000*i,KwaveSlider::Horizontal,this);
      newwidth [i]=new KIntegerLine (this);
      newwslider[i]=new KwaveSlider (20,500,1,80+10*i,KwaveSlider::Horizontal,this);
      connect  (newslider[i],SIGNAL(valueChanged(int)),SLOT(posChanged(int)));
      connect  (newpos[i],   SIGNAL(textChanged(const char *)),SLOT(posChanged(const char *)));
      connect  (newwslider[i],SIGNAL(valueChanged(int)),SLOT(widthChanged(int)));
      connect  (newwidth[i],   SIGNAL(textChanged(const char *)),SLOT(widthChanged(const char *)));
    }
  int min=oldnum;
  if (num<min) min=num;

  for (i=0;i<min;i++)
    {
      newpos[i]->setText (pos[i]->text());
      newslider[i]->setValue(posslider[i]->value());
      newwidth[i]->setText (widths[i]->text());
      newwslider[i]->setValue(widthslider[i]->value());
    }
  for (;i<num;i++)
    {
      char buf[16];
      sprintf (buf,"%d",newslider[i]->value());
      newpos[i]->setText (buf);
      sprintf (buf,"%d",newwslider[i]->value());
      newwidth[i]->setText (buf);
    }
  int bsize=ok->sizeHint().height();
  int half=bsize*12+num*bsize-bsize*2-(oldnum+2)*bsize;
  half+=bsize;
  for (int i=0;i<num;i++)
    {
      newpos[i]->setGeometry     (8,half,width()/6-8,bsize);
      newslider[i]->setGeometry  (8+width()/6,half,width()*2/6-8,bsize);
      newwidth[i]->setGeometry   (width()/2+8,half,width()/6-8,bsize);
      newwslider[i]->setGeometry (width()*4/6,half,width()*2/6-8,bsize);
      half+=bsize;
      newslider[i]->show();
      newpos[i]->show();
      newwslider[i]->show();
      newwidth[i]->show();
    }

  if (pos)
    {
      for (i=0;i<oldnum;i++) delete pos[i];
      for (i=0;i<oldnum;i++) delete posslider[i];
      for (i=0;i<oldnum;i++) delete widths[i];
      for (i=0;i<oldnum;i++) delete widthslider[i];

      delete pos;
      delete posslider;
      delete widths;
      delete widthslider;
    }
  pos=newpos;
  posslider=newslider;
  widths=newwidth;
  widthslider=newwslider;
  oldnum=num;

  setMinimumSize (480,bsize*16+num*bsize);
  resize	 (480,bsize*16+num*bsize);
  repaint ();
  refresh ();
}
//**********************************************************
double *FormantDialog::getPoints (int psize)
{
  return (formant->getPoints(psize));
}
//**********************************************************
void FormantDialog::refresh ()
{
  int *pos   =new int[oldnum];
  int *width =new int[oldnum];

  for (int i=0;i<oldnum;i++)
    {
      pos[i]=this->pos[i]->value();
      width[i]=this->widths[i]->value();
      if (width[i]<20) width[i]=20;
    }
  formant->setFormants (oldnum,pos,width);
}
//**********************************************************
void FormantDialog::resizeEvent (QResizeEvent *)
{
 int bsize=ok->sizeHint().height();
 int half=height()-bsize*2-(oldnum+2)*bsize;

 formant->setGeometry	(8+bsize,0,width()-16-bsize,half-bsize);
 y      ->setGeometry	(8,0,bsize,half-bsize);
 x      ->setGeometry	(8+bsize,half-bsize,width()-16-bsize,bsize);
 corner ->setGeometry   (8,half-bsize,bsize,bsize);

 numlabel->setGeometry	(8,half,width()/2-8,bsize);
 num     ->setGeometry	(width()*3/4,half,width()/4-8,bsize);
 half+=bsize;
 poslabel->setGeometry	(8,half,width()/2-8,bsize);
 half+=bsize;
 for (int i=0;i<oldnum;i++)
   {
     pos[i]->setGeometry       (8,half,width()/6-8,bsize);
     posslider[i]->setGeometry (8+width()/6,half,width()*2/6-8,bsize);
     widths[i]->setGeometry   (width()/2+8,half,width()/6-8,bsize);
     widthslider[i]->setGeometry (width()*4/6,half,width()*2/6-8,bsize);
     half+=bsize;
   }
 ok->setGeometry	(width()/10,height()-bsize*3/2,width()*3/10,bsize);  
 cancel->setGeometry	(width()*6/10,height()-bsize*3/2,width()*3/10,bsize);  
}
//**********************************************************
FormantDialog::~FormantDialog ()
{
}

