#include <stdio.h>
#include <stdlib.h>
#include <qpushbutton.h>
#include <qkeycode.h>

#include "module.h"
#include <kapp.h>

const char *version="1.0";
const char *author="Martin Wilz";
const char *name="quantize";
//**********************************************************
Dialog *getDialog (DialogOperation *operation)
{
  return new QuantiseDialog (operation->isModal());
}
//**********************************************************
QuantiseDialog::QuantiseDialog (bool modal): Dialog(modal)
{
  comstr=0;
  resize  (320,200);
  setCaption	(klocale->translate("Choose new virtual resolution"));

  bitlabel=new QLabel (klocale->translate("Number of quantisation steps"),this);
  bits=new KIntegerLine (this);
  bits->setText ("4");

  ok		=new QPushButton (klocale->translate("&Ok"),this);
  cancel       	=new QPushButton (klocale->translate("&Cancel"),this);

  int bsize=ok->sizeHint().height();

  setMinimumSize (320,bsize*4);

  ok->setAccel	(Key_Return);
  cancel->setAccel(Key_Escape);
  ok->setFocus	();
  connect 	(ok	,SIGNAL(clicked()),SLOT (accept()));
  connect 	(cancel	,SIGNAL(clicked()),SLOT (reject()));
}
//**********************************************************
const char *QuantiseDialog::getCommand ()
{
  deleteString (comstr);
  char buf[512];

  sprintf (buf,"quantize (%s)",bits->text());

  comstr=duplicateString (buf);

  return comstr;
}
//**********************************************************
void QuantiseDialog::resizeEvent (QResizeEvent *)
{
 int bsize=ok->sizeHint().height();

 bitlabel->setGeometry	(width()/20,bsize/2,width()*5/10,bsize);  
 bits->setGeometry	(width()*6/10,bsize/2,width()*3/10,bsize);  

 ok->setGeometry	(width()/10,height()-bsize*3/2,width()*3/10,bsize);  
 cancel->setGeometry	(width()*6/10,height()-bsize*3/2,width()*3/10,bsize);  
}
//**********************************************************
QuantiseDialog::~QuantiseDialog ()
{
  deleteString (comstr);
}
//**********************************************************












