#include <stdio.h>
#include <stdlib.h>
#include <qpushbutton.h>
#include <qkeycode.h>
#include "module.h"
#include <libkwave/kwavestring.h>
#include <kapp.h>

const char *version="1.0";
const char *author="Martin Wilz";
const char *name="amplifyfree";
//**********************************************************
KwaveDialog *getDialog (DialogOperation *operation)
{
  return new AmplifyCurveDialog(operation->getLength(),operation->isModal());
}
//**********************************************************
AmplifyCurveDialog::AmplifyCurveDialog (int time,bool modal):
KwaveDialog(modal)
{
  comstr=0;
  setCaption	(klocale->translate("Choose Amplification Curve :"));

  ok	 = new QPushButton (OK,this);
  cancel = new QPushButton (CANCEL,this);

  xscale=new ScaleWidget (this,0,time,"ms");
  yscale=new ScaleWidget (this,100,0,"%");
  corner=new CornerPatchWidget (this);

  curve= new CurveWidget (this);

  int bsize=ok->sizeHint().height();

  setMinimumSize (320,bsize*8);
  resize	 (320,bsize*8);

  ok->setAccel	(Key_Return);
  cancel->setAccel(Key_Escape);
  ok->setFocus	();
  connect 	(ok	,SIGNAL(clicked()),SLOT (accept()));
  connect 	(cancel	,SIGNAL(clicked()),SLOT (reject()));
}
//**********************************************************
const char*AmplifyCurveDialog::getCommand ()
{
  char buf[4096];
  deleteString (comstr);
  sprintf (buf,"amplifyfree (%s)",curve->getCommand());

  comstr=duplicateString (buf);
  return comstr;
}
//**********************************************************
void AmplifyCurveDialog::resizeEvent (QResizeEvent *)
{
 int bsize=ok->sizeHint().height();

 curve->setGeometry	(8+bsize,0,width()-bsize-16,height()-bsize*3);  
 xscale->setGeometry	(8+bsize,height()-bsize*3,width()-bsize-16,bsize);  
 yscale->setGeometry	(8,0,bsize,height()-bsize*3);
 corner->setGeometry	(8,height()-bsize*3,bsize,bsize);

 ok->setGeometry	(width()/10,height()-bsize*3/2,width()*3/10,bsize);  
 cancel->setGeometry	(width()*6/10,height()-bsize*3/2,width()*3/10,bsize);  
}
//**********************************************************
AmplifyCurveDialog::~AmplifyCurveDialog ()
{
  deleteString (comstr);
  delete curve ;
}











