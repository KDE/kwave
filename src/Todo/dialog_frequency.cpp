#include <math.h>
#include <qpushbutton.h>
#include <qkeycode.h>
#include <kapp.h>
#include "dialog_frequency.h"
#include "dialog_sweep.h"
#include "dialog_fixedfreq.h"
#include <libkwave/interpolation.h>

extern const char *OK;
extern const char *CANCEL;

//**********************************************************
FrequencyDialog::FrequencyDialog (QWidget *parent,int rate,char *title): QTabDialog(parent, 0,true)
{
  setCaption	(title);
  this->rate=rate;

  fixed=new FixedFrequencyDialog (this,rate,"Choose Frequency");
  sweep=new SweepDialog (this,rate,"Choose Parameters");
  connect 	(this	,SIGNAL(selected(const char *)),SLOT (setSelected(const char *)));
  addTab (fixed,klocale->translate("Fixed"));
  addTab (sweep,klocale->translate("Sweep"));
}
//**********************************************************
void FrequencyDialog::setSelected (const char *type)
{
  this->type=type;
}
//**********************************************************
FrequencyDialog::~FrequencyDialog ()
{
}
//**********************************************************
Curve *FrequencyDialog::getFrequency ()
//gets frequency list based on the selected tab and gives it back to caller
{
  Curve *times=new Curve();

  if (times)
    {
      if (strcmp (type,klocale->translate("Fixed"))==0)
	times->append ((double) fixed->getTime(),(double)fixed->getFrequency());
      else
	if (strcmp (type,klocale->translate("Sweep"))==0)
	  {
	    double lowfreq=sweep->getLowFreq();
	    double highfreq=sweep->getHighFreq();
	    double freq;

	    Interpolation interpolation(0);
	    interpolation.prepareInterpolation (new Curve(sweep->getPoints()));

	    double time=sweep->getTime();
	    double count=0;
            
	    while (count<time)
	      {
		double dif=count/time;
		dif=interpolation.getSingleInterpolation (dif);
		freq=lowfreq+(highfreq-lowfreq)*dif;

		double y=floor (rate/freq);
		count+=y;
		times->append (1,y);
	      }
	  }
    }
  return times;
}
//**********************************************************
void FrequencyDialog::resizeEvent (QResizeEvent *)
{
}

