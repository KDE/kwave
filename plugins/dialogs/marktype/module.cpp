#include <stdio.h>
#include <stdlib.h>
#include <qpushbutton.h>
#include <qkeycode.h>
#include "module.h"
#include <kapp.h>

const char *version="1.0";
const char *author="Martin Wilz";
const char *name="labeltype";
//**********************************************************
KwaveDialog *getDialog (DialogOperation *operation)
{
  return new MarkerTypeDialog(operation->isModal());
}
//**********************************************************
MarkerTypeDialog::MarkerTypeDialog (bool modal): KwaveDialog(modal)
{
  comstr=0;
  setCaption (klocale->translate ("Choose new label type"));
  namelabel	=new QLabel	(klocale->translate("Name :"),this);
  this->name    =new QLineEdit	(this);
  color         =new KColorCombo(this);
  col.setRgb (255,255,0);
  color->setColor (col);

  individual   =new QCheckBox  (klocale->translate("individual name for each label"),this);   

  ok		=new QPushButton (OK,this);
  cancel       	=new QPushButton (CANCEL,this);

  int bsize=ok->sizeHint().height();

  setMinimumSize (320,bsize*5);
  resize (320,bsize*5);

  this->name->setFocus();

  ok->setAccel	(Key_Return);
  cancel->setAccel(Key_Escape);
  connect 	(ok	,SIGNAL(clicked()),SLOT (accept()));
  connect 	(cancel	,SIGNAL(clicked()),SLOT (reject()));
  connect 	(color	,SIGNAL(activated(const QColor &))
		 ,SLOT (setColor(const QColor &)));
}
//**********************************************************
const char *MarkerTypeDialog::getCommand ()
{
  if (comstr) free (comstr);


  comstr=catString ("newlabeltype (",
		    name->text(),
		    ",",
		    individual->isChecked()?"true":"false",
		    ",",
  		    col.getCommand(),
		    ")"
		    );

  return comstr;
}
//**********************************************************
void MarkerTypeDialog::setColor (const Color &col)
{
  this->col=col;
}
//**********************************************************
void MarkerTypeDialog::resizeEvent (QResizeEvent *)
{
 int bsize=ok->sizeHint().height();
 int offset=bsize/2;

 namelabel->setGeometry	(width()/20,offset,width()*3/20,bsize);  
 name->setGeometry	(width()*2/10,offset,width()*5/10,bsize);
 color->setGeometry	(width()*15/20,offset,width()*2/10,bsize);
 offset+=bsize*3/2;
 individual->setGeometry(width()/20,offset,width()*18/20,bsize);

 ok->setGeometry	(width()/10,height()-bsize*3/2,width()*3/10,bsize);  
 cancel->setGeometry	(width()*6/10,height()-bsize*3/2,width()*3/10,bsize);  
}
//**********************************************************
MarkerTypeDialog::~MarkerTypeDialog ()
{
  if (comstr) free (comstr);
}
//**********************************************************













