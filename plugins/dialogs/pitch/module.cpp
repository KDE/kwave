#include <stdio.h>
#include <stdlib.h>
#include <qpushbutton.h>
#include <qkeycode.h>
#include "module.h"
#include <kapp.h>

const char *version="1.0";
const char *author="Martin Wilz";
const char *name="pitch";
//**********************************************************
Dialog *getDialog (DialogOperation *operation)
{
  return new PitchDialog(operation->isModal(),operation->getLength());
}
//**********************************************************
PitchDialog::PitchDialog (bool modal,int time): Dialog(modal)
{
  comstr=0;
  setCaption	(i18n("Select frequency range :"));

  ok	 = new QPushButton (OK,this);
  cancel = new QPushButton (CANCEL,this);

  high=  new KIntegerLine (this);
  low =  new KIntegerLine (this);
  adjust=new KIntegerLine (this);
  highlabel = new QLabel (i18n("Highest Freq. in Hz"),this);
  lowlabel  = new QLabel (i18n("Lowest Freq. in Hz"),this);
  octave    = new QCheckBox  (i18n("avoid octave jumps with twiddle factor"),this);
  octave->setChecked (true);

  low->setValue    (200);
  high->setValue   (2000);
  adjust->setValue (50);

  int bsize=ok->sizeHint().height();

  setMinimumSize (320,bsize*6);
  resize	 (320,bsize*6);

  ok->setAccel	(Key_Return);
  cancel->setAccel(Key_Escape);
  ok->setFocus	();

  connect 	(ok	,SIGNAL(clicked()),SLOT (accept()));
  connect 	(cancel	,SIGNAL(clicked()),SLOT (reject()));
}
//**********************************************************
const char *PitchDialog::getCommand ()
{
  deleteString (comstr);

  int i=adjust->value();
  if (i<0) i=0;
  char buf[128];
  sprintf (buf,"%f",1+(((double)i)/1000));

  comstr=catString ("pitch (",
		    high->text(),
		    ",",
		    low->text(),
		    ",",
		    octave->isChecked()?"true":"false",
		    ",",
		    buf);
  char *tmpstr=comstr;
  comstr=catString (comstr,")");
  deleteString (tmpstr);
  return comstr;
}
//**********************************************************
void PitchDialog::resizeEvent (QResizeEvent *)
{
 int bsize=ok->sizeHint().height();

 lowlabel->setGeometry	(8,8,width()/2-8,bsize);
 low->setGeometry       (width()/2,8,width()/2-8,bsize);  
 highlabel->setGeometry	(8,16+bsize,width()/2-8,bsize);
 high->setGeometry      (width()/2,16+bsize,width()/2-8,bsize);  
 octave->setGeometry	(8,24+bsize*2,width()*3/4-8,bsize);
 adjust->setGeometry    (width()*3/4,24+bsize*2,width()/4-8,bsize);  
 ok->setGeometry	(width()/10,height()-bsize*3/2,width()*3/10,bsize);  
 cancel->setGeometry	(width()*6/10,height()-bsize*3/2,width()*3/10,bsize);  
}
//**********************************************************
PitchDialog::~PitchDialog ()
{
  deleteString (comstr);
}










