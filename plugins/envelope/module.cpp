#include <stdio.h>
#include <stdlib.h>
#include <qpushbutton.h>
#include <qkeycode.h>
#include <qcombobox.h>
#include "libkwave/Interpolation.h"
#include  "module.h"
#include <kapp.h>

const char *version = "1.0";
const char *author = "Martin Wilz";
const char *name = "envelope";

//**********************************************************
Dialog *getDialog (DialogOperation *operation) 
{
    return new EnvelopeDialog (operation->isModal());
}

//**********************************************************
EnvelopeDialog::EnvelopeDialog(bool modal)
    :Dialog(modal) 
{
    comstr = 0;
    Interpolation interpolation(0);

    setCaption (i18n("Choose Envelope Parameters"));
    timelabel = new QLabel (i18n("Points are taken 20 times/s"), this);
    timeslider = new Slider (1, 1000, 1, 10, Slider::Horizontal, this);
    typelabel = new QLabel (i18n("Type of Interpolation :"), this);
    typebox = new QComboBox (true, this);
    typebox->insertStrList (interpolation.getTypes(), -1);

    ok = new QPushButton (i18n("&Ok"), this);
    cancel = new QPushButton (i18n("&Cancel"), this);

    int bsize = ok->sizeHint().height();

    setMinimumSize (320, bsize*6);
    resize (320, bsize*6);

    ok->setAccel (Key_Return);
    cancel->setAccel(Key_Escape);
    ok->setFocus ();
    connect (ok , SIGNAL(clicked()), SLOT (accept()));
    connect (cancel , SIGNAL(clicked()), SLOT (reject()));
    connect (timeslider, SIGNAL(valueChanged(int)), SLOT(setTime(int)));
}

//**********************************************************
const char *EnvelopeDialog::getCommand() 
{
    char buf[64];
    snprintf(buf, sizeof(buf), "%d", timeslider->value());

    deleteString (comstr);
    comstr = catString ("envelope (",
			buf,
			",",
			typebox->currentText(),
			")");
    return comstr;
}

//**********************************************************
void EnvelopeDialog::setTime(int ms) 
{
    char buf[256];
    snprintf(buf, sizeof(buf), 
	i18n("Points are taken %d.%01d times/s"), ms / 10, ms % 10);
    timelabel->setText (buf);
}

//**********************************************************
void EnvelopeDialog::resizeEvent(QResizeEvent *) 
{
    int bsize = ok->sizeHint().height();

    timelabel->setGeometry (width() / 10, bsize / 2, width()*8 / 10, bsize);
    timeslider->setGeometry(width() / 10, bsize*3 / 2, width()*8 / 10, bsize);

    typelabel->setGeometry (width() / 10, bsize*3, width()*4 / 10, bsize);
    typebox->setGeometry (width() / 2, bsize*3, width()*4 / 10, bsize);

    ok->setGeometry(width() / 10, height() - bsize*3 / 2, 
                    width()*3 / 10, bsize);
    cancel->setGeometry(width()*6 / 10, height() - bsize*3 / 2, 
                        width()*3 / 10, bsize);
}

//**********************************************************
EnvelopeDialog::~EnvelopeDialog() 
{
    deleteString (comstr);
}
