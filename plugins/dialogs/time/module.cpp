#include <stdio.h>
#include <stdlib.h>
#include <qpushbutton.h>
#include <qkeycode.h>
#include "module.h"
#include <kapp.h>

const char *version="1.0";
const char *author="Martin Wilz";
const char *name="time";
//**********************************************************
Dialog *getDialog (DialogOperation *operation)
{
  return new TimeDialog(operation->isModal(),operation->getRate());
}
//**********************************************************
TimeDialog::TimeDialog (bool modal,int rate): Dialog(modal)
{
  comstr=0;
  resize 	(320,200);
  setCaption	(klocale->translate("Enter length :"));
  timelabel	=new QLabel	(klocale->translate("Time :"),this);
  time	        =new TimeLine   (this,rate);     
  time->setMs   (1000);

  ok		=new QPushButton (OK,this);
  cancel	=new QPushButton (CANCEL,this);

  int bsize=ok->sizeHint().height();

  setMinimumSize (320,bsize*4);
  resize (320,bsize*4);

  ok->setAccel	(Key_Return);
  cancel->setAccel(Key_Escape);
  ok->setFocus	();

  connect 	(ok	,SIGNAL(clicked()),SLOT (accept()));
  connect 	(cancel	,SIGNAL(clicked()),SLOT (reject()));
}
//**********************************************************
const char *TimeDialog::getCommand ()
{
  deleteString (comstr);

  comstr=catString ("time (",
		    time->getMsStr(),
		    ")");

  return comstr;
}
//**********************************************************
void TimeDialog::setLength (int sam)
{
  time->setSamples (sam);
}
//**********************************************************
void TimeDialog::resizeEvent (QResizeEvent *)
{
 int bsize=ok->sizeHint().height();

 timelabel->setGeometry	(width()/10,	bsize/2,width()*3/10,bsize);  
 time->setGeometry(width()*5/10,	bsize/2,width()*4/10,bsize);  

 ok->setGeometry	(width()/10,height()-bsize*3/2,width()*3/10,bsize);  
 cancel->setGeometry	(width()*6/10,height()-bsize*3/2,width()*3/10,bsize);  
}
//**********************************************************
TimeDialog::~TimeDialog ()
{
  deleteString (comstr);
}














