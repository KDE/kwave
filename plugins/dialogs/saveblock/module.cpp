#include <stdio.h>
#include <stdlib.h>
#include <qpushbutton.h>
#include <qkeycode.h>
#include <qcombobox.h>
#include <libkwave/markers.h>
#include <libkwave/globals.h>
#include "module.h"
#include <kapp.h>
#include <kmsgbox.h>

const char *version="1.0";
const char *author="Martin Wilz";
const char *name="saveblock";
//**********************************************************
KwaveDialog *getDialog (DialogOperation *operation)
{
  return new SaveBlockDialog(operation->getGlobals(),operation->isModal());
}
//**********************************************************
SaveBlockDialog::SaveBlockDialog (Global *globals,bool modal): KwaveDialog(modal)
{
  comstr=0;
  setCaption	(klocale->translate("Choose Labels to be used for dividing signal:"));


  dir=new QDir ("./");

  ok		=new QPushButton (OK,this);
  cancel       	=new QPushButton (CANCEL,this);

  name=new QLineEdit (this);
  name->setText ("Unnamed");

  namelabel=new QLabel (klocale->translate("Base name:"),this);
  dirlabel=new QLabel (klocale->translate("Directory:"),this);
  dirname=new QLineEdit (this);
  dirname->setText (dir->absPath());

  mark1=new QLabel (klocale->translate("Start label:"),this);
  mark2=new QLabel (klocale->translate("Stop label:"),this);

  marktype1=new QComboBox (false,this);
  marktype2=new QComboBox (false,this);

  MarkerType *act;
  int cnt=0;

  for (act=globals->markertypes.first();act;act=globals->markertypes.next())
    {
      marktype1->insertItem (act->name);
      marktype2->insertItem (act->name);
      if (strcasecmp("start",act->name)==0) marktype1->setCurrentItem (cnt);
      if (strcasecmp("stop",act->name)==0) marktype2->setCurrentItem (cnt);
      cnt++;
    }
  int bsize=ok->sizeHint().height();

  setMinimumSize (bsize*8,bsize*9);

  ok->setAccel	(Key_Return);
  cancel->setAccel(Key_Escape);
  ok->setFocus	();
  connect (ok	,SIGNAL(clicked()),SLOT (check()));
  connect (cancel,SIGNAL(clicked()),SLOT (reject()));
}
//**********************************************************
const char *SaveBlockDialog::getCommand ()
{
  deleteString (comstr);

  comstr=catString ("saveblock (",
		    name->text(),
		    ",",
		    dir->absPath().data(),
		    ",",
		    marktype1->currentText(),
		    ",",
		    marktype2->currentText());
  if (comstr)
    {
      char *tmpstr=comstr;
      comstr=catString (comstr,")");
      deleteString (tmpstr);
    }
  return comstr;
}
//**********************************************************
void SaveBlockDialog::check ()
{
  if (dir) delete dir;
  dir =new QDir (dirname->text());
  if (dir->exists()) accept ();
  else
    {
      QString nix="The directory "+dir->absPath()+"does not exist\n What shall I do ?";
      int res=KMsgBox::yesNoCancel(this,"Attention",nix.data(),KMsgBox::QUESTION,"Create","Back","Cancel");

      switch (res)
	{
	case 1:
	  {
	    QString nox=dir->absPath(); //make copy of selected directory name
	    nix        =dir->absPath(); //make copy of selected directory name

	    while (!(QDir(nix.data()).exists()))
	      {
		nix.truncate (nix.findRev('/'));
	      }

	    int err=false;

	    while ((nox!=nix)&&(err==false))
	      {
		QString newdir;
		int ofs=nox.find ('/',nix.length()+1);

		if (ofs>0) newdir=nox.mid(nix.length()+1,ofs-nix.length()-1);
		else newdir=nox.right (nox.length()-nix.length()-1);

		if (!newdir.isEmpty())
		  {
		    if ((QDir (nix)).mkdir (newdir.data())) nix=nix+'/'+newdir;
		    else
		      {
			nix="Could not create "+newdir+" in "+nix;
			KMsgBox::message (this,"Info",nix,2);
			err=true;
		      }
		  }
		else
		  err=true;
	      }
	    if (err==false) accept();

	  }
	  break;
	case 3:
	  reject ();
	  break;
	}
    }
}
//**********************************************************
void SaveBlockDialog::resizeEvent (QResizeEvent *)
{
 int bsize=ok->sizeHint().height();

 namelabel->setGeometry (width()/10  ,bsize/2,width()*4/10,bsize);
 name->setGeometry      (width()/2   ,bsize/2,width()*4/10,bsize);
 dirlabel->setGeometry  (width()/10  ,bsize*2,width()*4/10,bsize);
 dirname->setGeometry   (width()/2   ,bsize*2,width()*4/10,bsize);

 mark1->setGeometry      (width()/10   ,bsize*7/2,width()*4/10,bsize);  
 mark2->setGeometry      (width()/10   ,bsize*5,width()*4/10,bsize);  
 marktype1->setGeometry  (width()/2    ,bsize*7/2,width()*4/10,bsize);  
 marktype2->setGeometry  (width()/2    ,bsize*5,width()*4/10,bsize);  

 ok->setGeometry	(width()/10,height()-bsize*3/2,width()*3/10,bsize);  
 cancel->setGeometry	(width()*6/10,height()-bsize*3/2,width()*3/10,bsize);  
}
//**********************************************************
SaveBlockDialog::~SaveBlockDialog ()
{
  if (dir) delete dir;
  deleteString (comstr);
}
//**********************************************************













