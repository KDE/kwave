#include <stdio.h>
#include <stdlib.h>
#include <qpushbutton.h>
#include <qkeycode.h>
#include "module.h"
#include <kapp.h>

const char *version="1.0";
const char *author="Martin Wilz";
const char *name="memory";
//**********************************************************
KwaveDialog *getDialog (DialogOperation *operation)
{
  return new MemoryDialog(operation->isModal());
}
//**********************************************************
MemoryDialog::MemoryDialog (bool modal): KwaveDialog(modal)
{
  comstr=0;
  setCaption (klocale->translate("Memory Options :"));

  ok	 =new QPushButton (OK,this);
  cancel =new QPushButton (CANCEL,this);

  memlabel =new QLabel 	     (klocale->translate("Threshold for use of mapped memory (in Mb):"),this);  
  mem      =new KIntegerLine (this);
  //  mem->setValue (mmap_threshold);
  mem->setValue (50);
  browse   =new QPushButton  (klocale->translate("Browse"),this);

  dirlabel =new QLabel    (klocale->translate("Directory for mapping:"),this);  
  dir      =new QLineEdit (this);  
  dir->setText ("/tmp");

  int bsize=ok->sizeHint().height();

  setMinimumSize (bsize*20,bsize*9/2);
  resize	 (bsize*20,bsize*9/2);

  ok->setFocus	();
  connect 	(ok	,SIGNAL(clicked()),SLOT (accept()));
  connect 	(cancel	,SIGNAL(clicked()),SLOT (reject()));
}
//**********************************************************
const char *MemoryDialog::getCommand ()
{
  if (comstr) free (comstr);
  comstr =catString ("memory (",
		     mem->text(),
		     ",",
		     dir->text(),
		     ")");
  return comstr;
}
//**********************************************************
void MemoryDialog::resizeEvent (QResizeEvent *)
{
 int bsize=ok->sizeHint().height();

 memlabel->setGeometry	(8,8,width()*7/10-8,bsize);
 mem->setGeometry	(width()*7/10,8,width()*3/10-8,bsize);  
 dirlabel->setGeometry	(8,bsize*3/2,width()*3/10-8,bsize);  
 dir->setGeometry	(width()*3/10+2,bsize*3/2,width()*4/10-4,bsize);  
 browse->setGeometry	(width()*7/10,bsize*3/2,width()*3/10-8,bsize);  

 ok->setGeometry	(width()/10,height()-bsize*3/2,width()*3/10,bsize); 
 cancel->setGeometry	(width()*6/10,height()-bsize*3/2,width()*3/10,bsize);  
}
//**********************************************************
MemoryDialog::~MemoryDialog ()
{
  if (comstr) free (comstr);
}
//**********************************************************













