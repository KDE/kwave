#include <stdio.h>
#include <stdlib.h>
#include "module.h"
#include <qpushbutton.h>
#include <qkeycode.h>
#include <libkwave/String.h>
#include <kapp.h>

const char *version = "1.0";
const char *author = "Martin Wilz";
const char *name = "delay";

//**********************************************************
Dialog *getDialog (DialogOperation *operation) 
{
    return new DelayDialog(operation->getRate(), operation->isModal());
}

//**********************************************************
DelayDialog::DelayDialog(int rate, bool modal): Dialog(modal) 
{
    comstr = 0;
    resize (320, 200);
    setCaption (i18n("Choose Length and Rate :"));
    delaylabel = new QLabel (i18n("Delay :"), this);
    delay = new TimeLine (this, rate);
    delay->setMs (200);

    ampllabel = new QLabel (i18n("Amplitude of delayed signal :50 %"), this);
    amplslider = new Slider (1, 200, 1, 10, Slider::Horizontal, this);
    recursive = new QCheckBox (i18n("do recursive delaying"), this);

    ok = new QPushButton (OK, this);
    cancel = new QPushButton (CANCEL, this);

    int bsize = ok->sizeHint().height();
    setMinimumSize (320, bsize*7);

    ok->setAccel (Key_Return);
    cancel->setAccel(Key_Escape);
    ok->setFocus ();
    connect (ok, SIGNAL(clicked()), SLOT (accept()));
    connect (cancel, SIGNAL(clicked()), SLOT (reject()));
    connect (amplslider, SIGNAL(valueChanged(int)), SLOT(setAmpl(int)));
}

//**********************************************************
const char* DelayDialog::getCommand() 
{
    char buf[512];
    deleteString (comstr);
    snprintf(buf, sizeof(buf), "delay (%f)", delay->getMs());
    comstr = duplicateString (buf);
    return comstr;
}

//**********************************************************
void DelayDialog::setAmpl(int percent) 
{
    char buf[512];
    snprintf(buf, sizeof(buf), i18n("Amplitude of delayed signal :%d %%"), 
	percent);
    ampllabel->setText (buf);
}

//**********************************************************
void DelayDialog::resizeEvent(QResizeEvent *) 
{
    int bsize = ok->sizeHint().height();

    delaylabel->setGeometry(width() / 10, bsize / 2, width()*3 / 10, bsize);
    delay->setGeometry(width()*5 / 10, bsize / 2, width()*4 / 10, bsize);
    ampllabel->setGeometry (width() / 10, bsize*2, width()*8 / 10, bsize);
    amplslider->setGeometry (width() / 10, bsize*3, width()*8 / 10, bsize);

    recursive->setGeometry (width() / 10, bsize*9 / 2, width()*8 / 10, bsize);

    ok->setGeometry(width() / 10, height() - bsize*3 / 2, 
	            width()*3 / 10, bsize);
    cancel->setGeometry(width()*6 / 10, height() - bsize*3 / 2, 
                        width()*3 / 10, bsize);
}

//**********************************************************
DelayDialog::~DelayDialog() 
{
    deleteString (comstr);
}
