#include <stdio.h>
#include <stdlib.h>
#include <qpushbutton.h>
#include <qkeycode.h>
#include "module.h"
#include <kapp.h>

const char *version="1.0";
const char *author="Martin Wilz";
const char *name="newsignal";
//**********************************************************
Dialog *getDialog (DialogOperation *operation)
{
  return new NewSampleDialog(operation->isModal());
}
//**********************************************************
const char *ratetext[]={"48000","44100","32000","22050","16000","12000","10000",0}; 
//**********************************************************
NewSampleDialog::NewSampleDialog (bool modal): Dialog(modal)
{
  comstr=0;
  setCaption	(klocale->translate("Choose Length and Rate :"));
  timelabel	=new QLabel   (klocale->translate("Time :"),this);
  time       	=new TimeLine (this,44100);
  time->setMs (1000);

  ratelabel	=new QLabel 	(klocale->translate("Rate in Hz :"),this);
  ratefield	=new QComboBox  (true,this);
  ratefield->insertStrList (ratetext,6);
  ratefield->setCurrentItem (1);

  ok		=new QPushButton (OK,this);
  cancel	=new QPushButton (CANCEL,this);

  int bsize=ok->sizeHint().height();

  setMinimumSize (320,bsize*5);
  resize (320,bsize*5);

  ok->setFocus	();
  ok->setAccel	(Key_Return);
  cancel->setAccel(Key_Escape);

  connect (ok	    ,SIGNAL(clicked()),SLOT (accept()));
  connect (cancel   ,SIGNAL(clicked()),SLOT (reject()));
  connect (ratefield,SIGNAL(activated (const char *)),SLOT(setRate(const char *)));
}
//**********************************************************
void NewSampleDialog::setRate (const char *res)
{
  time->setRate (atoi(res));
}
//**********************************************************
const char* NewSampleDialog::getCommand ()
{
  deleteString (comstr);
  
  comstr=catString ("newsignal (",
		    ratefield->currentText(),
		    ",",
		    time->getMsStr(),
		    ")");
  return comstr;
}
//**********************************************************
void NewSampleDialog::resizeEvent (QResizeEvent *)
{
 int bsize=ok->sizeHint().height();

 timelabel->setGeometry	(width()/10,	bsize/2,width()*3/10,bsize);  
 time->setGeometry      (width()*4/10,	bsize/2,width()*5/10,bsize);  

 ratelabel->setGeometry	(width()/10,	bsize*2,width()*3/10,bsize);  
 ratefield->setGeometry	(width()*4/10,	bsize*2,width()/2,bsize);  

 ok->setGeometry	(width()/10,height()-bsize*3/2,width()*3/10,bsize);  
 cancel->setGeometry	(width()*6/10,height()-bsize*3/2,width()*3/10,bsize);  
}
//**********************************************************
NewSampleDialog::~NewSampleDialog ()
{
  deleteString (comstr);
}














