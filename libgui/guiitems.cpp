#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <kapp.h>
#include <libkwave/kwavestring.h>
#include "guiitems.h"

extern QString mstotime (int ms);
extern char*   mstotimec (int ms);
//**********************************************************
FloatLine::FloatLine (QWidget *parent,double value):KRestrictedLine (parent)
{
  setValue (value);
  setValidChars ("-0123456789.E");
  digits=1;
}
//**********************************************************
void FloatLine::setValue (double value)
{
  char buf[64];
  char conv[32];
  
  sprintf (conv,"%%.%df",digits);
  sprintf (buf,conv,value);
  setText (buf);
}
//**********************************************************
double FloatLine::value ()
{
  return strtod (text(),0); 
}
//**********************************************************
FloatLine::~FloatLine ()
{
}
//**********************************************************
TimeLine::TimeLine (QWidget *parent,int rate):KRestrictedLine (parent)
{
  comstr=0;
  this->rate=rate;
  mode=1;
  menu=new QPopupMenu ();
  value=1;
  
  menu->insertItem	(klocale->translate("as number of samples"),this,SLOT(setSampleMode()));
  menu->insertItem	(klocale->translate("in ms"),this,SLOT(setMsMode()));
  menu->insertItem	(klocale->translate("in s"), this,SLOT(setSMode()));
  menu->insertItem	(klocale->translate("in kb"),this,SLOT(setKbMode()));

  menu->setCheckable (true);

  menu->setItemChecked (menu->idAt(0),false);
  menu->setItemChecked (menu->idAt(1),true);
  menu->setItemChecked (menu->idAt(2),false);
  menu->setItemChecked (menu->idAt(3),false);

  connect( this, SIGNAL(textChanged(const char *)),SLOT(setValue(const char *)) ); 
};
//**********************************************************
int TimeLine::getValue ()
{
  return value;
}
//**********************************************************
double TimeLine::getMs ()
{
  return ((double(value))*1000/rate);
}
//**********************************************************
const char *TimeLine::getMsStr ()
{
  char buf[128];
  sprintf (buf,"%f",((double(value))*1000/rate));
  deleteString (comstr);
  comstr=duplicateString (buf);
  return comstr;
}
//**********************************************************
void TimeLine::setRate (int newrate)
{
  rate=newrate;
  setSamples (value);
}
//**********************************************************
void TimeLine::setValue (const char *newvalue)
{
  switch (mode)
    {
    case 0:
      value=strtol (newvalue,0,0); 
      break;
    case 1:
      value=(int) ((double)(rate*strtod (newvalue,0)/1000)+.5);
      break;
    case 2:
      value=(int) ((double)(rate*strtod (newvalue,0))+.5);
      break;
    case 3:
      value=(int) ((double)(strtod (newvalue,0)*1024)/sizeof(int)-.5);
      break;
    }
}
//**********************************************************
void TimeLine::setSampleMode ()
{
  menu->setItemChecked (menu->idAt(0),true);
  menu->setItemChecked (menu->idAt(1),false);
  menu->setItemChecked (menu->idAt(2),false);
  menu->setItemChecked (menu->idAt(3),false);
  setValidChars ("0123456789");
  mode=0;
  setSamples (value);
}
//**********************************************************
void TimeLine::setMsMode ()
{
  menu->setItemChecked (menu->idAt(0),false);
  menu->setItemChecked (menu->idAt(1),true);
  menu->setItemChecked (menu->idAt(2),false);
  menu->setItemChecked (menu->idAt(3),false);
  setValidChars ("0123456789.");
  mode=1;
  setSamples (value);
}
//**********************************************************
void TimeLine::setSMode ()
{
  menu->setItemChecked (menu->idAt(0),false);
  menu->setItemChecked (menu->idAt(1),false);
  menu->setItemChecked (menu->idAt(2),true);
  menu->setItemChecked (menu->idAt(3),false);
  setValidChars ("0123456789.");
  mode=2;
  setSamples (value);
}
//**********************************************************
void TimeLine::setKbMode ()
{
  menu->setItemChecked (menu->idAt(0),false);
  menu->setItemChecked (menu->idAt(1),false);
  menu->setItemChecked (menu->idAt(2),false);
  menu->setItemChecked (menu->idAt(3),true);
  setValidChars ("0123456789.");
  mode=3;
  setSamples (value);
}
//**********************************************************
void TimeLine::setSamples (int samples)
{
  char buf[64];

  value=samples;

  switch (mode)
    {
    case 0:
      sprintf (buf,"%d samples",value);
      this->setText (buf);
      break;
    case 1:
      {
	double pr=((double)value)*1000/rate;
	sprintf (buf,"%.03f ms",pr);
	this->setText (buf);
      }
      break;
    case 2:
      {
	double pr=((double)value)/rate;
	sprintf (buf,"%.3f s",pr);
	this->setText (buf);
      }
      break;
    case 3:
      {
	double pr=((double)(value))*sizeof(int)/1024;
	sprintf (buf,"%.3f kb",pr);
	this->setText (buf);
      }
      break;
    }
}
//**********************************************************
void TimeLine::setMs (int ms)
{
  char buf[16];

  value=(int) ((double)(rate*ms/1000)+.5);
  if (mode==0)
    {
      sprintf (buf,"%d samples",value);
      this->setText (buf);
    }
  else
    {
      sprintf (buf,"%d.%d ms",ms,0);
      this->setText (buf);
    }
}
//**********************************************************
void TimeLine::mousePressEvent( QMouseEvent *e)
{
  if (e->button()==RightButton)
    {
      QPoint popup=QCursor::pos();
      menu->popup(popup);
    }            
}
//**********************************************************
TimeLine::~TimeLine ()
{
  deleteString (comstr);
};




