#include <qpushbutton.h>
#include <qkeycode.h>
#include <kapp.h>
#include "dialog_fixedfreq.h"

extern QString mstotime (int ms);  
extern const char *OK;
extern const char *CANCEL;
//**********************************************************
FixedFrequencyDialog::FixedFrequencyDialog (QWidget *parent,int rate,char *title): QWidget (parent, 0)
{
  setCaption	(title);
  this->rate=rate;
  frequencyslider=new KwaveSlider (1,(rate/2)*9/10,1,440   ,KwaveSlider::Horizontal,this);
  frequencylabel =new QLabel (klocale->translate("Base: 440 Hz"),this);

  timeslider=new KwaveSlider (1,10000,1,100,KwaveSlider::Horizontal,this);
  timelabel =new QLabel (klocale->translate("Time: 1 ms"),this);

  showTime (100);

  QPushButton button(this);
  int bsize=button.sizeHint().height();

  setMinimumSize (320,bsize*5);
  resize	 (320,bsize*5);

  connect 	(timeslider	,SIGNAL(valueChanged(int)),SLOT (showTime(int)));
  connect 	(frequencyslider,SIGNAL(valueChanged(int)),SLOT (showFrequency(int)));
}
//**********************************************************
void FixedFrequencyDialog::showFrequency (int newvalue)
{
  char buf[16];
  sprintf (buf,klocale->translate("Base: %d Hz"),newvalue);
  frequencylabel->setText (buf);
  showTime (timeslider->value());
}
//**********************************************************
int FixedFrequencyDialog::getTime (){return timeslider->value();}
int FixedFrequencyDialog::getFrequency () {return frequencyslider->value();}
//**********************************************************
void FixedFrequencyDialog::showTime (int newvalue)
{
  char buf[20];
  sprintf (buf,klocale->translate("Time: %s"),mstotime (10000*newvalue/frequencyslider->value()).data());
  timelabel->setText (buf);
}
//**********************************************************
void FixedFrequencyDialog::resizeEvent (QResizeEvent *)
{
  QPushButton button(this);
  int bsize=button.sizeHint().height();
  int offset=bsize/2;

  timelabel->setGeometry	(width()/20,offset,width()*8/20,bsize);  
  timeslider->setGeometry(width()/2,offset,width()*9/20,bsize);
  offset+=bsize*3/2;
  frequencylabel->setGeometry	(width()/20,offset,width()*8/20,bsize);  
  frequencyslider->setGeometry	(width()/2,offset,width()*9/20,bsize);  
}
//**********************************************************
FixedFrequencyDialog::~FixedFrequencyDialog ()
{
}

