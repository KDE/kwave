#include <stdio.h>
#include <stdlib.h>
#include <qpushbutton.h>
#include <qkeycode.h>
#include "module.h"
#include <kapp.h>

const char *version = "1.0";
const char *author = "Martin Wilz";
const char *name = "percent";

//**********************************************************
Dialog *getDialog (DialogOperation *operation) 
{
    return new PercentDialog(operation->isModal(), operation->getName());
}
//**********************************************************
PercentDialog::PercentDialog(bool modal, const char *title)
    :Dialog(modal) 
{
    comstr = 0;
    setCaption (title);

    slider = new Slider (1, 1000, 1, 500, Slider::Horizontal, this);
    label = new QLabel ("", this);
    setValue (500);

    ok = new QPushButton (OK, this);
    cancel = new QPushButton (CANCEL, this);

    int bsize = ok->sizeHint().height();

    setMinimumSize (320, bsize*4);
    resize (320, bsize*4);

    ok->setAccel (Key_Return);
    cancel->setAccel(Key_Escape);
    ok->setFocus ();
    connect (ok , SIGNAL(clicked()), SLOT (accept()));
    connect (cancel , SIGNAL(clicked()), SLOT (reject()));
    connect (slider , SIGNAL(valueChanged(int)), SLOT (setValue(int)));
}

//**********************************************************
void PercentDialog::setValue(int num) 
{
    char buf[64];
    snprintf(buf, sizeof(buf), "%d.%d %% :", num / 10, num % 10);
    label->setText (buf);
}

//**********************************************************
const char* PercentDialog::getCommand() 
{
    char buf[512];
    deleteString (comstr);

    snprintf(buf, sizeof(buf), "%f", ((double)slider->value() / 1000));

    comstr = catString ("percent (",
			buf,
			")");
    return comstr;
}

//**********************************************************
void PercentDialog::resizeEvent(QResizeEvent *) 
{
    int bsize = ok->sizeHint().height();

    label->setGeometry (width() / 20, 0, width()*8 / 20, bsize);
    slider->setGeometry (width() / 2, 0, width()*9 / 20, bsize);
    ok->setGeometry (width() / 10, height() - bsize*3 / 2, width()*3 / 10, bsize);
    cancel->setGeometry (width()*6 / 10, height() - bsize*3 / 2, width()*3 / 10, bsize);
}

//**********************************************************
PercentDialog::~PercentDialog() 
{
    deleteString (comstr);
}
