#include <qfiledlg.h>
#include <qkeycode.h>
#include "filterwidget.h"
#include <math.h>

QDir *filterDir;
extern QStrList filterNameList;
const char *allow_double="-0123456789.";
//**********************************************************
MovingFilterDialog::MovingFilterDialog (QWidget *par=NULL,int num): QDialog(par, 0,true)
{
  this->num=num;
  setCaption	(klocale->translate("Choose Filter Movement :"));

  ok	 = new QPushButton (klocale->translate("Filter"),this);
  cancel = new QPushButton (klocale->translate("Cancel"),this);

  lowlabel= new QLabel (klocale->translate("Range goes from"),this);
  highlabel= new QLabel (klocale->translate("to"),this);

  tap=new KIntegerLine (this);
  tap->setText ("1");
  usecurve=new QCheckBox (klocale->translate("do filtering with changing coefficient"),this);

  low=new KRestrictedLine (this);
  low->setValidChars (allow_double);
  low->setText ("-100 %");
  high=new KRestrictedLine (this);
  high->setValidChars (allow_double);
  high->setText ("100 %");

  curve= new CurveWidget (this);
  curve->setBackgroundColor	(QColor(black) );

  high->setEnabled (false);
  low->setEnabled (false);
  tap->setEnabled (false);
  curve->setEnabled (false);

  int bsize=ok->sizeHint().height();

  setMinimumSize (320,bsize*9);
  resize	 (320,bsize*9);

  ok->setAccel	(Key_Return);
  cancel->setAccel(Key_Escape);
  ok->setFocus	();
  connect 	(ok	,SIGNAL(clicked()),SLOT (accept()));
  connect 	(cancel	,SIGNAL(clicked()),SLOT (reject()));
  connect 	(usecurve,SIGNAL(clicked()),SLOT (toggleState()));
}
//**********************************************************
void MovingFilterDialog::toggleState ()
{
    if  (usecurve->isChecked())
      {
	  high->setEnabled (true);
	  low->setEnabled (true);
	  tap->setEnabled (true);
	  curve->setEnabled (true);
      }
      else
	{
	  high->setEnabled (false);
	  low->setEnabled (false);
	  tap->setEnabled (false);
	  curve->setEnabled (false);
	}
}
//**********************************************************
void MovingFilterDialog::checkTap (const char *text)
{
  char buf[16];
  int i=strtol (text,0,0);
  if (i<0) i=0;
  if (i>num) i=num;

  sprintf (buf,"%d",num);
  tap->setText (buf);
}
//**********************************************************
int MovingFilterDialog::getTap ()
{
  return strtol (tap->text(),0,0);
}
//**********************************************************
int MovingFilterDialog::getLow ()
{
  return (int) (10*strtod (low->text(),0));
}
//**********************************************************
int MovingFilterDialog::getHigh ()
{
  return (int) (10*strtod (high->text(),0));
}
//**********************************************************
int MovingFilterDialog::getState ()
{
  return usecurve->isChecked();
}
//**********************************************************
const char *MovingFilterDialog::getCommand ()
{
  return curve->getCommand ();
}
//**********************************************************
void MovingFilterDialog::resizeEvent (QResizeEvent *)
{
 int bsize=ok->sizeHint().height();
 int offset=0;

 curve->setGeometry	(width()/20,0,width()*18/20,height()-bsize*5);  

 usecurve->setGeometry	(width()/20,height()-bsize*9/2,usecurve->sizeHint().width(),bsize);
 offset+=usecurve->sizeHint().width()+bsize/4;
 tap->setGeometry	(width()/20+offset,height()-bsize*9/2,width()*18/20-offset,bsize);  

 offset=0;
 lowlabel->setGeometry	(width()/20,height()-bsize*3,lowlabel->sizeHint().width(),bsize);  
 offset+=lowlabel->sizeHint().width()+bsize/4;
 low->setGeometry	(width()/20+offset,height()-bsize*3,bsize*2,bsize);  
 offset+=bsize*9/4;
 highlabel->setGeometry	(width()/20+offset,height()-bsize*3,highlabel->sizeHint().width(),bsize);  
 offset+=highlabel->sizeHint().width()+bsize/4;
 high->setGeometry	(width()/20+offset,height()-bsize*3,bsize*2,bsize);  
 offset+=highlabel->sizeHint().width();
 ok->setGeometry	(width()/10,height()-bsize*3/2,width()*3/10,bsize);  
 cancel->setGeometry	(width()*6/10,height()-bsize*3/2,width()*3/10,bsize);  
}
//**********************************************************
MovingFilterDialog::~MovingFilterDialog ()
{
  delete curve ;
}
//**********************************************************












