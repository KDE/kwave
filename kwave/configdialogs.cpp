#include <unistd.h>
#include <kmsgbox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qkeycode.h>
#include "configdialogs.h"

long mmap_threshold=50;
char *mmap_dir="/tmp";
extern const char *OK;
extern const char *CANCEL;
static const char *devicetext[]={"/dev/dsp","/dev/dio",0}; 
//**********************************************************
MemoryDialog::MemoryDialog (QWidget *par): QDialog(par,0,true)
{
  setCaption (klocale->translate("Memory Options :"));

  ok	 =new QPushButton (OK,this);
  cancel =new QPushButton (CANCEL,this);

  memlabel =new QLabel 	     (klocale->translate("Threshold for use of mapped memory (in Mb):"),this);  
  mem      =new KIntegerLine (this);
  mem->setValue (mmap_threshold);
  browse   =new QPushButton  (klocale->translate("Browse"),this);

  dirlabel =new QLabel    (klocale->translate("Directory for mapping:"),this);  
  dir      =new QLineEdit (this);  
  dir->setText (mmap_dir);

  int bsize=ok->sizeHint().height();

  setMinimumSize (bsize*20,bsize*9/2);
  resize	 (bsize*20,bsize*9/2);

  ok->setFocus	();
  connect 	(ok	,SIGNAL(clicked()),SLOT (accept()));
  connect 	(cancel	,SIGNAL(clicked()),SLOT (reject()));
}
//**********************************************************
char *MemoryDialog::getDir ()
{
  return (strdup (dir->text()));
}
//**********************************************************
int MemoryDialog::getThreshold()
{
  return mem->value();
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
}
//**********************************************************
PlayBackDialog::PlayBackDialog (QWidget *par,int play16bit,int bufbase): QDialog(par,0,true)
{
  setCaption	(klocale->translate("Playback Options :"));

  devicelabel	=new QLabel 	(klocale->translate("Playback Device :"),this);
  devicebox	=new QComboBox  (true,this);
  devicebox->insertStrList (devicetext,-1);

  QVBoxLayout *vbox;
  bg = new QButtonGroup( this);
  bg->setTitle(klocale->translate("Resolution"));  
  vbox = new QVBoxLayout(bg, 10);

  vbox->addSpacing( bg->fontMetrics().height() );
  b16 = new QRadioButton( bg );
  b16->setText( "16 Bit");
  vbox->addWidget(b16);
  b16->setMinimumSize(b16->sizeHint());
  QToolTip::add( b16, klocale->translate("Set Playback-Mode to 16 Bit\nNote that not all sound hardware supports this mode !"));
  b16->setMinimumSize( b16->sizeHint());

  b8 = new QRadioButton( bg );
  b8->setText( "8 Bit" );
  vbox->addWidget(b8);
  b8->setMinimumSize( b8->sizeHint() );
 
  buffersize=new QSlider (4,16,1,bufbase,QSlider::Horizontal,this);
  bufferlabel=new QLabel ("",this); 
  setBufferSize (bufbase);
  QToolTip::add( buffersize, klocale->translate("This is the size of the buffer used for communication with the sound driver\nIf your computer is rather slow select a big buffer"));

  stereo=new QCheckBox (klocale->translate("enable stereo playback"),this);
  stereo->setEnabled (false);

  if (play16bit) b16->setChecked( TRUE );
  else b8->setChecked( TRUE );

  ok	 =new QPushButton (OK,this);
  cancel =new QPushButton (CANCEL,this);

  int bsize=ok->sizeHint().height();
  int lsize=ok->sizeHint().width();

  setMinimumSize (lsize*8,bsize*12);
  resize	 (lsize*8,bsize*12);

  ok->setFocus	();
  connect 	(ok	,SIGNAL(clicked()),SLOT (accept()));
  connect 	(cancel	,SIGNAL(clicked()),SLOT (reject()));
  connect (buffersize,SIGNAL(valueChanged(int)),SLOT(setBufferSize(int)));
}
//**********************************************************
void  PlayBackDialog::setBufferSize (int exp)
{
  char buf [64];
  int val=1<<exp;

  sprintf (buf,klocale->translate("Buffer Size : %d Samples"),val);
  bufferlabel->setText (buf);
}
//**********************************************************
int  PlayBackDialog::getResolution ()
{
  return b16->isChecked();
}
//**********************************************************
int  PlayBackDialog::getBufferSize ()
{
  return buffersize->value();
}
//**********************************************************
void PlayBackDialog::resizeEvent (QResizeEvent *)
{
 int bsize=ok->sizeHint().height();
 int offset=0;

 bg->setGeometry	        (width()/20,bsize/2,width()*18/20,bsize*4);  

 offset+=bsize*5;
 devicelabel->setGeometry	(width()*1/10,offset,width()*7/20,bsize);  
 devicebox->setGeometry	        (width()*1/2,offset,width()*8/20,bsize);  
 offset+=bsize*3/2;

 bufferlabel->setGeometry	(width()/10,offset,width()*4/10,bsize);  
 buffersize->setGeometry	(width()*6/10,offset,width()*3/10,bsize);  
 offset+=bsize*3/2;

 stereo->setGeometry	(width()/10,offset,width()*8/10,bsize);  

 ok->setGeometry	(width()/10,height()-bsize*3/2,width()*3/10,bsize);  
 cancel->setGeometry	(width()*6/10,height()-bsize*3/2,width()*3/10,bsize);  
}
//**********************************************************
PlayBackDialog::~PlayBackDialog ()
{
}
//**********************************************************





