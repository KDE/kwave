#include <stdio.h>
#include <stdlib.h>
#include "module.h"
#include <qpushbutton.h>
#include <qkeycode.h>
#include <qcombobox.h>
#include <kapp.h>

const char *version = "1.0";
const char *author = "Martin Wilz";
const char *name = "rate";
//**********************************************************
Dialog *getDialog (DialogOperation *operation) {
    return new RateDialog(operation->isModal());
}
//**********************************************************
const char *ratetext[] = {"48000", "44100", "32000", "22050", "16000", "12000", "10000", 0
			 };
//**********************************************************
RateDialog::RateDialog (bool modal): Dialog(modal) {
    comstr = 0;
    setCaption (i18n("Choose New Rate :"));

    ratelabel = new QLabel (i18n("Rate in Hz :"), this);
    ratefield = new QComboBox (true, this);
    ratefield->insertStrList (ratetext, -1);

    ok = new QPushButton (OK, this);
    cancel = new QPushButton (CANCEL, this);

    int bsize = ok->sizeHint().height();

    setMinimumSize (320, bsize*7 / 2);
    resize (320, bsize*7 / 2);

    ok->setAccel (Key_Return);
    cancel->setAccel(Key_Escape);
    ok->setFocus ();
    connect (ok , SIGNAL(clicked()), SLOT (accept()));
    connect (cancel , SIGNAL(clicked()), SLOT (reject()));
}
//**********************************************************
const char *RateDialog::getCommand () {
    if (comstr) delete comstr;
    const char *buf = ratefield->currentText();
    comstr = catString ("rate (", buf, ")");
    return comstr;
}
//**********************************************************
void RateDialog::resizeEvent (QResizeEvent *) {
    int bsize = ok->sizeHint().height();

    ratelabel->setGeometry (width() / 10, bsize / 2, width()*3 / 10, bsize);
    ratefield->setGeometry (width()*4 / 10, bsize / 2, width() / 2, bsize);

    ok->setGeometry (width() / 10, height() - bsize*3 / 2, width()*3 / 10, bsize);
    cancel->setGeometry (width()*6 / 10, height() - bsize*3 / 2, width()*3 / 10, bsize);
}
//**********************************************************
RateDialog::~RateDialog () {
    if (comstr) delete comstr;
}













