#include <stdio.h>
#include <stdlib.h>
#include <qpushbutton.h>
#include <qkeycode.h>
#include <qcombobox.h>
#include <qtooltip.h>
#include "module.h"
#include <libkwave/kwavestring.h>
#include <libkwave/windowfunction.h>
#include <kapp.h>

const char *version="1.0";
const char *author="Martin Wilz";
const char *name="averagefft";
//**********************************************************
KwaveDialog *getDialog (DialogOperation *operation)
{
  return new AverageFFTDialog(operation->getRate(),operation->isModal());
}
//**********************************************************
AverageFFTDialog::AverageFFTDialog (int rate,bool modal):
  KwaveDialog(modal)
{
  WindowFunction w(0);
  this->rate=rate;
  resize 	(320,200);
  setCaption	(klocale->translate("Choose window type and length :"));

  pointlabel	=new QLabel	(klocale->translate("Length of window :"),this);
  windowlength	=new TimeLine (this);
  windowlength->setMs (100);

  windowtypebox	=new QComboBox (true,this);
  windowtypebox->insertStrList (w.getTypes(),w.getCount());
  QToolTip::add(windowtypebox,klocale->translate("Choose windowing function here."));
  
  windowtypelabel=new QLabel	(klocale->translate("Window Function :"),this);

  ok		=new QPushButton (OK,this);
  cancel       	=new QPushButton (CANCEL,this);

  int bsize=ok->sizeHint().height();

  setMinimumSize (320,bsize*8);
  resize (320,bsize*3);

  ok->setAccel	(Key_Return);
  cancel->setAccel(Key_Escape);
  ok->setFocus	();
  connect 	(ok	,SIGNAL(clicked()),SLOT (accept()));
  connect 	(cancel	,SIGNAL(clicked()),SLOT (reject()));
}
//**********************************************************
const char *AverageFFTDialog::getCommand ()
{
  char buf[512];
  deleteString (comstr);
  sprintf (buf,"%f",windowlength->getMs());

  comstr=catString ("averagefft (",
		    buf,
		    ",",
		    windowtypebox->currentText(),		       
		    ")");
  return comstr;
}
//**********************************************************
void AverageFFTDialog::resizeEvent (QResizeEvent *)
{
  int bsize=ok->sizeHint().height();

  pointlabel->setGeometry  (8,	        bsize/2,width()/2-8,bsize);  
  windowlength->setGeometry(width()/2,	bsize/2,width()*3/10,bsize);  

  windowtypelabel->setGeometry(8,	bsize*3/2+8,width()/2-8,bsize);  
  windowtypebox->setGeometry (width()/2,bsize*3/2+8,width()/2-8,bsize);  

  ok->setGeometry	(width()/10,height()-bsize*3/2,width()*3/10,bsize);  
  cancel->setGeometry	(width()*6/10,height()-bsize*3/2,width()*3/10,bsize);  
}
//**********************************************************
AverageFFTDialog::~AverageFFTDialog ()
{
  deleteString (comstr);
}
//**********************************************************











