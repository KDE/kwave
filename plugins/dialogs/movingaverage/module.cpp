#include <stdio.h>
#include <stdlib.h>
#include <qpushbutton.h>
#include <qkeycode.h>
#include <qcombobox.h>
#include <kapp.h>
#include "module.h"

const char *version="1.0";
const char *author="Martin Wilz";
const char *name="movingaverage";

//**********************************************************
Dialog *getDialog (DialogOperation *operation)
{
  return new AverageDialog (operation->isModal());
}
//**********************************************************
AverageDialog::AverageDialog (bool modal): Dialog(modal)
{
  comstr=0;
  resize  (320,200);
  setCaption	(i18n ("Choose Parameters :"));
  taplabel	=new QLabel	 (i18n("# of Filter Taps :"),this);
  typelabel	=new QLabel	 (i18n("Filter Type :"),this);
  taps       	=new KIntegerLine(this);     

  taps->setValue (3);
  type	=new QComboBox  (false,this);
  type->insertItem(i18n("Lowpass filter"));
  type->insertItem(i18n("Highpass filter"));

  ok		=new QPushButton (i18n("&Ok"),this);
  cancel       	=new QPushButton (i18n("&Cancel"),this);

  int bsize=ok->sizeHint().height();

  setMinimumSize (320,bsize*9/2);
  resize (320,bsize*9/2);

  ok->setAccel	(Key_Return);
  cancel->setAccel(Key_Escape);
  ok->setFocus	();
  connect 	(ok	,SIGNAL(clicked()),SLOT (accept()));
  connect 	(cancel	,SIGNAL(clicked()),SLOT (reject()));
}
//**********************************************************
const char *AverageDialog::getCommand ()
{
  deleteString (comstr);

  char buf[512];
  sprintf (buf,i18n("movingaverage %d %s"),taps->value(),type->currentText());

  comstr=duplicateString (buf);
  return comstr;
}
//**********************************************************
void AverageDialog::resizeEvent (QResizeEvent *)
{
  int bsize=ok->sizeHint().height();

  taplabel->setGeometry	(8,8,width()*5/10-8,bsize);  
  taps->setGeometry     (width()/2,8,width()*5/10-8,bsize);  
  typelabel->setGeometry(8,bsize*3/2,width()*5/10-8,bsize);  
  type->setGeometry     (width()/2,bsize*3/2,width()*5/10-8,bsize);  

  ok->setGeometry	(width()/10,height()-bsize*3/2,width()*3/10,bsize);  
  cancel->setGeometry	(width()*6/10,height()-bsize*3/2,width()*3/10,bsize);  
}
//**********************************************************
AverageDialog::~AverageDialog ()
{
  deleteString (comstr);
}














