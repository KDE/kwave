#include <stdio.h>
#include <stdlib.h>
#include <qpushbutton.h>
#include <qkeycode.h>
#include "module.h"
#include <libkwave/markers.h>
#include <kapp.h>

const char *version="1.0";
const char *author="Martin Wilz";
const char *name="marksave";
//**********************************************************
KwaveDialog *getDialog (DialogOperation *operation)
{
  return new MarkSaveDialog(operation->getGlobals(),operation->isModal());
}
//**********************************************************
MarkSaveDialog::MarkSaveDialog (Global *globals,bool modal): KwaveDialog(modal)
  //dialog of markertypes to be selected...
{
  comstr=0;
  setCaption	(klocale->translate("Select label types to be saved :"));

  this->globals=globals;
  save=new QListBox (this);

  MarkerType *act;
  //traverse all markertypes, add them to widget, selection state set to  false
  for (act=globals->markertypes.first();act;act=globals->markertypes.next())
    {
      save->insertItem (act->name);
      act->selected=false;
    }

  save->setMultiSelection (true);
  
  ok		=new QPushButton (OK,this);
  cancel       	=new QPushButton (CANCEL,this);

  int bsize=ok->sizeHint().height();

  setMinimumSize (320,bsize*6);

  ok->setAccel	(Key_Return);
  cancel->setAccel(Key_Escape);
  ok->setFocus	();
  connect 	(ok	,SIGNAL(clicked()),SLOT (accept()));
  connect 	(cancel	,SIGNAL(clicked()),SLOT (reject()));
}
//**********************************************************
const char *MarkSaveDialog::getCommand ()
{
  char *tmpstr;
  deleteString (comstr);  
  int cnt=0;
  MarkerType *act;

  comstr="marksave (";

  for (act=globals->markertypes.first();act;act=globals->markertypes.next())
    {
      tmpstr=comstr;
      comstr=catString (comstr,act->name,",");
      deleteString (tmpstr);
    }

  tmpstr=comstr;
  comstr=catString (comstr,")");
  deleteString (tmpstr);

  return comstr;
}
//**********************************************************
void MarkSaveDialog::resizeEvent (QResizeEvent *)
{
 int bsize=ok->sizeHint().height();

 save->setGeometry	(width()/20,0,width()*18/20,height()-bsize*2);

 ok->setGeometry	(width()/10,height()-bsize*3/2,width()*3/10,bsize);  
 cancel->setGeometry	(width()*6/10,height()-bsize*3/2,width()*3/10,bsize);  
}
//**********************************************************
MarkSaveDialog::~MarkSaveDialog ()
{
  deleteString (comstr);
}














