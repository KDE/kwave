#include <math.h>
#include <limits.h>
#include "addsynth.h"
#include "fftview.h"
#include <qpainter.h>
#include <qkeycode.h>
#include <qlayout.h>
#include "functions.h"
#include "sample.h"

//****************************************************************************
AddSynthWidget::AddSynthWidget (QWidget *parent) : QWidget
(parent)
{
  this->setBackgroundColor (QColor(black));    
  count=0;
  func=0;
}
//****************************************************************************
void AddSynthWidget::setSines (int count,int *power,int *phase,int *mult)
{
  this->count=count;
  this->power=power;
  this->phase=phase;
  this->mult=mult;
  repaint ();
}
//****************************************************************************
void AddSynthWidget::setFunction (int func)
{
  this->func=func;
  repaint ();
}
//****************************************************************************
AddSynthWidget::~AddSynthWidget (QWidget *parent,const char *name)
{
}
//****************************************************************************
void AddSynthWidget::paintEvent  (QPaintEvent *)
{
  Functions foo;
  double (*func)(double)=(double (*)(double))foo.getFunc(this->func);

  int height=rect().height()/2;
  int width=rect().width();

  p.begin (this);
  p.setPen (white);
  if (count)
    {
      int i,ay,ly;
      double max=0,y=0,x;
      double *dphase=new double [count];

      if (dphase)
	{
	  for (int j=0;j<count;j++)
	    dphase[j]=((((double)phase[j])/180)*M_PI/mult[j]);

	  for (int j=0;j<count;j++) max+=(double) power[j];
	  max/=1000;

	  x=0;
	  for (int j=0;j<count;j++)
	    y+=(func(x*mult[j]+(((double)phase[j])/180)*M_PI)*power[j])/1000;

	  ly=(int)(y/max*height)+height;

	  for (i=1;i<width;i++)
	    {
	      x=((double)i)/width*2*M_PI;
	      y=0;
	      for (int j=0;j<count;j++)
		y+=(func(x*mult[j]+(((double)phase[j])/180)*M_PI)*power[j])/1000;

	      ay=(int)(y/max*height)+height;
	      p.drawLine (i-1,ly,i,ay);
	      ly=ay;
	    }
	  delete dphase;
	  
	  p.setPen (green);
	  p.drawLine (0,height,width,height);
	}
    }
  p.end();
}
//**********************************************************
AddSynthDialog::AddSynthDialog (QWidget *par,int rate,int time,char *name): QDialog(par,0,true)
{
  num=20;
  menu=new QPopupMenu ();
  QPopupMenu *multmenu=new QPopupMenu();
  tflag=false;


  times=0;

  menu->insertItem (klocale->translate("Multipliers"),multmenu);
  multmenu->insertItem (klocale->translate("all"));
  multmenu->insertItem (klocale->translate("odd"));
  menu->insertItem (klocale->translate("Reset all"));

  Functions func;
  freq=440;
  this->rate=rate;
  this->time=time;

  setCaption	(name);
  multlab       =new QLabel	(klocale->translate("Frequency Multiplier :"),this);
  phaselab	=new QLabel	(klocale->translate("Phase in degree:"),this);
  powerlab	=new QLabel	(klocale->translate("Power in %:"),this);

  aphase=0;
  apower=0;
  amult=0;

  powerlabel=new KIntegerLine *[num];
  phaselabel=new KIntegerLine *[num];
  multlabel =new KIntegerLine *[num];
  power     =new QSlider *[num];
  phase     =new QSlider *[num];
  mult      =new QSlider *[num];

  functype=new QComboBox  (false,this);
  functype->insertStrList (func.getTypes());

  /*  signaltype=new QComboBox  (false,this);
  signaltype->insertStrList (SignalTypes);
  */
  bg = new QButtonGroup( this);
  bg->setTitle(klocale->translate("Frequency"));  
  QVBoxLayout *vbox = new QVBoxLayout(bg, 10);

  vbox->addSpacing(bg->fontMetrics().height()/2);
  fixed = new QRadioButton(bg);
  fixed->setText(klocale->translate("Fixed"));
  vbox->addWidget (fixed);
  fixed->setChecked (true);
  fixed->setMinimumSize(fixed->sizeHint());

  sweep = new QRadioButton(bg);
  sweep->setText(klocale->translate("Sweep"));
  vbox->addWidget (sweep);
  sweep->setMinimumSize( sweep->sizeHint() );

  file = new QRadioButton (bg);
  file->setText(klocale->translate("File"));
  vbox->addWidget (file);
  file->setMinimumSize( file->sizeHint() );

  view=new AddSynthWidget (this);
  connect 	(functype,SIGNAL(activated(int)),view,SLOT (setFunction(int)));

  channel=new KIntegerLine (this);
  channel->setText ("20");
  channellabel=new QLabel (klocale->translate("# of :"),this);

  ok		=new QPushButton ("Ok",this);
  cancel	=new QPushButton ("Cancel",this);

  getNSlider (20,true);

  times=new QList<CPoint>;
  CPoint *newpoint=new CPoint;
  newpoint->x=100;
  newpoint->y=(double)rate/440;
  times->append (newpoint);

  cancel->setAccel(Key_Escape);
  ok->setFocus	();
  connect 	(ok	,SIGNAL(clicked()),SLOT (accept()));
  connect 	(cancel	,SIGNAL(clicked()),SLOT (reject()));
  connect 	(channel,SIGNAL(textChanged(const char *)),SLOT (setChannels(const char *)));
  connect 	(fixed,SIGNAL(pressed()),SLOT (fixedMode()));
  connect 	(sweep,SIGNAL(pressed()),SLOT (sweepMode()));
  connect 	(file,SIGNAL(pressed()), SLOT (fileMode()));
  updateView();
}
//**********************************************************
void AddSynthDialog::fixedMode ()
{
  CPoint *newpoint;

  fixed->setChecked (true);
  file->setChecked (false);
  sweep->setChecked (false);

  FixedFrequencyDialog dialog;
  if (dialog.exec())
    {
      if (times) delete times;

      times=new QList<CPoint>;
      newpoint=new CPoint;
	
      newpoint->x=(double)dialog.getTime();
      newpoint->y=(double)rate/dialog.getFrequency();
      times->append (newpoint);
    }
}
//**********************************************************
void AddSynthDialog::sweepMode ()
{
  sweep->setChecked (true);
  file->setChecked (false);
  fixed->setChecked (false);
  CPoint *newpoint;
  SweepDialog dialog(this,rate);
  if (dialog.exec())
    {
      double freq1=dialog.getFreq1();
      double freq2=dialog.getFreq2();
      double freq;
      Interpolation interpolation(dialog.getInterpolationType());

      interpolation.prepareInterpolation (dialog.getPoints());

      int time=dialog.getTime();
      int count=0;
            
      if (times) delete times;
      times=new QList<CPoint>;

      while (count<time)
	{
	  double dif=((double)count)/time;
	  dif=interpolation.getSingleInterpolation (dif);
	  freq=(freq2*dif+freq1*(1-dif));

	  newpoint=new CPoint;

	  newpoint->x=1;
	  newpoint->y=floor (rate/freq);
	  count+=(int)newpoint->y;
	  times->append (newpoint);
	}
    }
}
//**********************************************************
void AddSynthDialog::fileMode ()
{
  file->setChecked (true);
  fixed->setChecked (false);
  sweep->setChecked (false);
  CPoint *newpoint;
  QString name=QFileDialog::getOpenFileName (0,"*.dat",this);
  if (!name.isNull())
    {
      QFile in(name.data());
      if (in.exists())
	{
	  if (in.open(IO_ReadWrite))
	    {
	      CPoint *lastpoint=0;

	      char buf[80];
	      float time;
	      float freq;

	      if (times) delete times;

	      times=new QList<CPoint>;

	      while (in.readLine(&buf[0],80)>0)
		{

		  if ((buf[0]!='#')&&(buf[0]!='/'))    //check for commented lines
		    {
		      char *p;
		      sscanf (buf,"%e",&time);
		      p=strchr (buf,'\t');
		      if (p==0) p=strchr (buf,' ');

		      sscanf (p,"%e",&freq);

		      newpoint=new CPoint;

		      newpoint->x=(double)time;
		      newpoint->y=(double)freq;
			    
		      if (lastpoint)
			{
			  if (lastpoint->y>0)
			    {
			      lastpoint->y=floor(rate/lastpoint->y);
			      lastpoint->x=(newpoint->x-lastpoint->x)*rate/1000;
			      lastpoint->x=floor(lastpoint->x/lastpoint->y);
			      times->append (lastpoint);
			    }
			}
		      lastpoint=newpoint;
		    }
		}
	    }
	  else printf (klocale->translate("could not open file !\n"));
	}
      else printf (klocale->translate("file %s does not exist!\n"),name.data());
    }
}
 //**********************************************************
void AddSynthDialog::setChannels (const char *n)
{
  int x=1;
  if (n) x=strtol(n,0,0);
  if (x<1) x=1;
  if (x>100) x=100;         //feel free to change, if you have a big screen ...
  if (x!=num) getNSlider (x,false);
}
//**********************************************************
void AddSynthDialog::getNSlider (int n,int first)
{
  if (!first)
    {
      for (int i=0;i<num;i++)
	{
	  delete powerlabel[i];
	  delete phaselabel[i];
	  delete multlabel[i];
	  delete power[i];
	  delete phase[i];
	  delete mult[i];
	}
      delete powerlabel;
      delete phaselabel;
      delete multlabel;
      delete power;
      delete phase;
      delete mult;
    }

  num=n;
  powerlabel=new KIntegerLine *[num];
  phaselabel=new KIntegerLine *[num];
  multlabel =new KIntegerLine *[num];
  power     =new QSlider *[num];
  phase     =new QSlider *[num];
  mult      =new QSlider *[num];

  for (int i=0;i<num;i++)
    {
      char buf[4];
      powerlabel[i]=new KIntegerLine (this);
      powerlabel[i]->setText ("0");
      phaselabel[i]=new KIntegerLine (this);
      phaselabel[i]->setText ("0");
      multlabel[i] =new KIntegerLine (this);
      sprintf (buf,"%d",i);
      multlabel[i]->setText (buf);
    }
  for (int i=0;i<num;i++)
    {
      power[i]=new QSlider (0,1000,1,0   ,QSlider::Horizontal,this);
      phase[i]=new QSlider (-180,179,1,0,QSlider::Horizontal,this);
      mult[i ]=new QSlider (1,100,1,i+1  ,QSlider::Horizontal,this);
    }

  power[0]->setValue (1000);
  int bsize=ok->sizeHint().height();
  int lsize=powerlab->sizeHint().height();
  int nsize=powerlabel[0]->sizeHint().height();
  int toppart=lsize*12;

  resize (bsize*16,(nsize*(num+1)+bsize*2)+toppart);

  for (int i=0;i<num;i++)
  {
      power[i]->show();
      phase[i]->show();
      mult[i]->show();
      powerlabel[i]->show();
      phaselabel[i]->show();
      multlabel[i]->show();
      connect  (power[i],SIGNAL(valueChanged(int)),SLOT(showPower(int)));
      connect  (phase[i],SIGNAL(valueChanged(int)),SLOT(showPhase(int)));
      connect  (mult[i],SIGNAL (valueChanged(int)),SLOT(showMult(int)));
      connect  (powerlabel[i],SIGNAL(textChanged(const char *)),SLOT(showPower(const char *)));
      connect  (phaselabel[i],SIGNAL(textChanged(const char *)),SLOT(showPhase(const char *)));
      connect  (multlabel[i],SIGNAL (textChanged(const char *)),SLOT(showMult(const char *)));
  }
 setMinimumSize (bsize*16,(nsize*(num+1)+bsize*2)+toppart);
}
//**********************************************************
void AddSynthDialog::updateView ()
{
  int count=0;
  int x=0;

  for (int i=0;i<num;i++) if (power[i]->value()>0) count++; //count active elements

  if (apower) delete apower;
  if (amult) delete  amult;
  if (aphase) delete aphase;

  apower=new int[count]; 
  aphase=new int[count]; 
  amult =new int[count]; 

  for (int i=0;i<num;i++) 
    if (this->power[i]->value()>0)
    {
      apower[x]=power[i]->value();
      amult[x] =mult[i]->value();
      aphase[x]=phase[i]->value();
      x++;
    }
  view->setSines (count,apower,aphase,amult);
}
//**********************************************************
int AddSynthDialog::getCount ()
{
  int count=0;

  for (int i=0;i<num;i++) if (power[i]->value()>0) count++; //count active elements

  return count;
}
//**********************************************************
MSignal *AddSynthDialog::getSignal ()
{
  Functions foo;
  int count=getCount();
  int *phase=aphase;
  int *mult=amult;
  int *power=apower;

  if (times)
    {
      CPoint *t;
      int len=0;
      int dif;
      int lastz;

      for (t=times->first();t;t=times->next()) len+=int (t->x*t->y); //count number of samples

      MSignal *add=new MSignal ((QWidget *)parent(),len,rate);

      if (add&&add->getSample()&&len);
      {
	int *sample=add->getSample();

	double (*func)(double)=(double (*)(double))foo.getFunc(functype->currentItem());

	if (count)
	  {
	    int i;
	    double max=0,y=0,x;

	    for (int j=0;j<count;j++) max+=(double) power[j];
	    max/=1000;

	    int z=0;
	    for (t=times->first();t;t=times->next())
	      {
		lastz=z;
		for (i=0;i<(int)(t->y);i++)
		  {
		    x=(double(i))/t->y*2*M_PI;
		    y=0;
		    for (int j=0;j<count;j++)
		      y+=(func(x*mult[j]+(((double)phase[j])/180)*M_PI)*power[j])/1000;

		    sample[z++]=(int)(y/max*((1<<23)-1));
		  }
		dif=z-lastz;
		for (i=1;i<(int)t->x;i++) memcpy (&sample[lastz+i*dif],&sample[lastz],dif*sizeof (int));
		z+=(int)((t->x-1)*dif);
	      }
	    return add;
	  }
	if (add) delete add;
      }
    }

  return 0;
}
//**********************************************************
void AddSynthDialog::showPower (const char *str)
{
  for (int i=0;i<num;i++) 
    if (strcmp(powerlabel[i]->text(),str)==0)
      {
	tflag=true;
	power[i]->setValue (strtol(str,0,0)*10);
	tflag=false;
      }
  updateView ();
}
//**********************************************************
void AddSynthDialog::showMult (const char *str)
{
  for (int i=0;i<num;i++) 
    if (strcmp(multlabel[i]->text(),str)==0)
      {
	tflag=true;
	mult[i]->setValue (strtol(str,0,0)*10);
	tflag=false;
      }
  updateView ();
}
//**********************************************************
void AddSynthDialog::showPhase (const char *str)
{
  for (int i=0;i<num;i++) 
    if (strcmp(phaselabel[i]->text(),str)==0)
      {
      }
  updateView ();
}
//**********************************************************
void AddSynthDialog::showPower (int newvalue)
{
  for (int i=0;i<num;i++) 
    if (power[i]->value()==newvalue)
      if (!tflag)
      {
	char buf[8];
	sprintf (buf,"%d.%d",newvalue/10,newvalue%10);
	powerlabel[i]->setText (buf);
      }
  updateView ();
}
//**********************************************************
void AddSynthDialog::showPhase (int newvalue)
{
  for (int i=0;i<num;i++) 
    if (phase[i]->value()==newvalue)
      if (!tflag)
      {
	char buf[8];
	sprintf (buf,"%d",newvalue);
	phaselabel[i]->setText (buf);
      }
  updateView ();
}
//**********************************************************
void AddSynthDialog::showMult (int newvalue)
{
  for (int i=0;i<num;i++) 
    if (mult[i]->value()==newvalue)
      if (!tflag)
      {
	char buf[8];
	sprintf (buf,"%d",newvalue);
	multlabel[i]->setText (buf);
      }
  updateView ();
}
//**********************************************************
void AddSynthDialog::resizeEvent (QResizeEvent *)
{
  int bsize=ok->sizeHint().height();
  int lsize=powerlab->sizeHint().height();
  int width=this->width();
  int height=this->height();
  int toppart=lsize*12;
  int offset=0;

  view->setGeometry (0,0,width*2/3,toppart);
  channellabel->setGeometry   (width*2/3+lsize,lsize/2,width/3-bsize*2-lsize,lsize);
  channel->setGeometry  (width*2/3+lsize+width/3-bsize*2-lsize,0,bsize+lsize,bsize);
  offset+=bsize+4;
  functype->setGeometry      (width*2/3+lsize,offset,width/3-bsize,bsize);
  offset+=bsize+4;
  bg->setGeometry      (width*2/3+lsize,offset,width/3-bsize,bsize*4);

  width-=bsize; //create some border
  powerlab->setGeometry	(0        ,toppart,width/3,lsize);
  phaselab->setGeometry	(width*1/3,toppart,width/3,lsize);
  multlab->setGeometry	(width*2/3,toppart,width/3,lsize);

  int nsize=powerlabel[0]->sizeHint().height();

  for (int i=0;i<num;i++)
    {
      int yoffset=nsize*(i+1)+toppart;
      int textx=lsize*3;

      powerlabel[i]->setGeometry (lsize/2,yoffset,textx,nsize);
      phaselabel[i]->setGeometry (lsize/2+width/3,yoffset,textx,nsize);
      multlabel [i]->setGeometry (lsize/2+width*2/3,yoffset,textx,nsize);
      textx+=lsize;
      power[i]->setGeometry (textx,yoffset+2,width/3-textx,nsize-4);
      phase[i]->setGeometry (width/3+textx,yoffset+2,width/3-textx,nsize-4);
      mult[i]->setGeometry  (width*2/3+bsize+lsize,yoffset+2,width/3-bsize-lsize,nsize-4);
    }
  width=this->width();

  ok->setGeometry	(width/10,height-bsize*3/2,width*3/10,bsize);  
  cancel->setGeometry	(width*6/10,height-bsize*3/2,width*3/10,bsize);  
}
//**********************************************************
void AddSynthDialog::mousePressEvent( QMouseEvent *e)
{
  if (e->button()==RightButton)
    {
      QPoint popup=QCursor::pos();
      menu->popup(popup);
    }
}
//**********************************************************
AddSynthDialog::~AddSynthDialog ()
{
  if (powerlabel) delete powerlabel;
  if (multlabel)  delete multlabel;
  if (phaselabel) delete phaselabel;
  if (apower) delete apower;
  if (amult)  delete  amult;
  if (aphase) delete aphase;
}



































