#include <stdlib.h>
#include <stdio.h>
#include <qpushbutton.h>
#include <qkeycode.h>
#include <qcombobox.h>
#include "module.h"
#include <libkwave/String.h>
#include <kapp.h>

const char *version="1.0";
const char *author="Martin Wilz";
const char *name="distort";
//**********************************************************
Dialog *getDialog (DialogOperation *operation)
{
  return new DistortDialog(operation->isModal());
}
//**********************************************************
static const char *symtext[]={"symmetric","upper","lower",0}; 
//**********************************************************
DistortDialog::DistortDialog (bool modal): Dialog(modal)
{
  comstr=0;
  setCaption	(klocale->translate("Choose Line of distortion :"));

  ok	 = new QPushButton (OK,this);
  cancel = new QPushButton (CANCEL,this);

  curve= new CurveWidget (this);
  curve->setBackgroundColor (QColor(black) );

  sym=new QComboBox (this);
  sym->insertStrList (symtext,-1);

  xscale=new ScaleWidget (this);
  yscale=new ScaleWidget (this,100,0,"%");
  corner=new CornerPatchWidget (this);

  int bsize=ok->sizeHint().height();

  setMinimumSize (320,bsize*9);
  resize	 (320,bsize*9);

  ok->setAccel	(Key_Return);
  cancel->setAccel(Key_Escape);
  connect 	(ok	,SIGNAL(clicked()),SLOT (accept()));
  connect 	(cancel	,SIGNAL(clicked()),SLOT (reject()));
}
//**********************************************************
const char * DistortDialog::getCommand ()
{
  deleteString (comstr);
  
  comstr=catString ("distort (",
		    sym->currentText(),
		    ",",
		    curve->getCommand (),
		    ")"
		    );
  return comstr;
}
//**********************************************************
void DistortDialog::resizeEvent (QResizeEvent *)
{
 int bsize=ok->sizeHint().height();
 int h=height()-bsize*4;

 curve->setGeometry	(8+bsize,0,width()-16-bsize,h);
 xscale->setGeometry	(8+bsize,h,width()-16-bsize,bsize);  
 corner->setGeometry	(8,h,bsize,bsize);   
 yscale->setGeometry	(8,0,bsize,h);
 sym->setGeometry	(8,height()-bsize*3,width()-16,bsize);
 ok->setGeometry	(width()/10,height()-bsize*3/2,width()*3/10,bsize);
 cancel->setGeometry	(width()*6/10,height()-bsize*3/2,width()*3/10,bsize);  
}
//**********************************************************
DistortDialog::~DistortDialog ()
{
  delete curve ;
  deleteString (comstr);
}














