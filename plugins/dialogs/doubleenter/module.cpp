#include <stdio.h>
#include <stdlib.h>
#include <qkeycode.h>
#include <qpushbutton.h>
#include "module.h"
#include <kapp.h>

const char *version = "1.0";
const char *author = "Martin Wilz";
const char *name = "doubleenter";

const char *allow_double = "-0123456789.";
//**********************************************************
Dialog *getDialog (DialogOperation *operation) {
    return new DoubleEnterDialog(operation->getName(), operation->isModal());
}
//**********************************************************
DoubleEnterDialog::DoubleEnterDialog (const char *name, bool modal):
Dialog(name, modal) {
    setCaption (i18n("Enter a value"));

    val = new KRestrictedLine (this);
    val->setValidChars (allow_double);
    val->setText ("0");

    ok = new QPushButton (i18n("&Ok"), this);
    cancel = new QPushButton (i18n("&Cancel"), this);

    int bsize = ok->sizeHint().height();
    int lsize = val->sizeHint().height();
    setMinimumSize (320, lsize*2 + bsize);
    resize (320, lsize*2 + bsize);

    ok->setAccel (Key_Return);
    cancel->setAccel(Key_Escape);
    val->setFocus ();
    connect (ok , SIGNAL(clicked()), SLOT (accept()));
    connect (cancel , SIGNAL(clicked()), SLOT (reject()));
}
//**********************************************************
const char * DoubleEnterDialog::getCommand () {
    return val->text();
}
//**********************************************************
void DoubleEnterDialog::resizeEvent (QResizeEvent *) {
    int bsize = ok->sizeHint().height();
    int lsize = val->sizeHint().height();

    val->setGeometry (width() / 20, 0, width()*18 / 20, lsize);
    ok->setGeometry (width() / 10, height() - bsize*3 / 2, width()*3 / 10, bsize);
    cancel->setGeometry (width()*6 / 10, height() - bsize*3 / 2, width()*3 / 10, bsize);
}
//**********************************************************
DoubleEnterDialog::~DoubleEnterDialog () {}
//**********************************************************




