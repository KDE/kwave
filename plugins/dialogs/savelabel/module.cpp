#include <stdio.h>
#include <stdlib.h>
#include <qpushbutton.h>
#include <qkeycode.h>
#include "module.h"
#include <libkwave/markers.h>
#include <kapp.h>

const char *version="1.0";
const char *author="Martin Wilz";
const char *name="savelabel";
//**********************************************************
KwaveDialog *getDialog (DialogOperation *operation)
{
  return new MarkSaveDialog(operation->getGlobals(),operation->isModal());
}
//**********************************************************
MarkSaveDialog::MarkSaveDialog (Global *globals,bool modal): KwaveDialog(modal)
  //dialog of markertypes to be selected...
{
  selectall=false;
  comstr=0;
  setCaption	(klocale->translate("Select label types to be saved :"));

  this->globals=globals;
  save=new QListBox (this);

  MarkerType *act;
  maxcnt=0;
  //traverse all markertypes, add them to widget, selection state set to  false
  for (act=globals->markertypes.first();act;act=globals->markertypes.next())
    {
      save->insertItem (act->name);
      act->selected=false;
      maxcnt++;
    }

  save->setMultiSelection (true);
  
  ok		=new QPushButton (OK,this);
  cancel       	=new QPushButton (CANCEL,this);
  all       	=new QPushButton ("&All",this);

  int bsize=ok->sizeHint().height();

  setMinimumSize (320,bsize*6);

  ok->setAccel	(Key_Return);
  all->setAccel	(Key_A);
  cancel->setAccel(Key_Escape);
  ok->setFocus	();
  connect 	(ok	,SIGNAL(clicked()),SLOT (accept()));
  connect 	(all	,SIGNAL(clicked()),this,SLOT(selectAll()));
  connect 	(cancel	,SIGNAL(clicked()),SLOT (reject()));
}
//**********************************************************
void MarkSaveDialog::selectAll ()
{
  selectall=true;
  accept ();
}
//**********************************************************
const char *MarkSaveDialog::getCommand ()
{
  char *tmpstr=0;
  deleteString (comstr);  

  comstr=duplicateString ("savelabel (");

  for (int cnt=0;cnt<maxcnt;cnt++)
    {
      if (save->isSelected(cnt)||selectall)
	{
	  tmpstr=comstr;
	  comstr=catString (comstr,save->text(cnt),",");
	  deleteString (tmpstr);
	}
    }
  if (comstr[strlen(comstr)-1]==',') comstr[strlen(comstr)-1]=0;
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

 ok->setGeometry	(width()/10,height()-bsize*3/2,width()*2/10,bsize);  
 all->setGeometry	(width()*4/10,height()-bsize*3/2,width()*2/10,bsize);  
 cancel->setGeometry	(width()*7/10,height()-bsize*3/2,width()*2/10,bsize);  
}
//**********************************************************
MarkSaveDialog::~MarkSaveDialog ()
{
  deleteString (comstr);
}














