#include <stdio.h>
#include <qpushbutton.h>
#include <qkeycode.h>
#include "module.h"
#include <kapp.h>

const char *version = "1.0";
const char *author = "Martin Wilz";
const char *name = "stringenter";
//**********************************************************
Dialog *getDialog (DialogOperation *operation) {
    return new StringEnterDialog(operation->getName(), operation->isModal());
}
//**********************************************************
StringEnterDialog::StringEnterDialog (const char *text, bool modal): Dialog(modal) {
    if (name) setCaption (i18n(text));
    setCaption (i18n(text));

    name = new QLineEdit (this);

    ok = new QPushButton (OK, this);
    cancel = new QPushButton (CANCEL, this);

    int bsize = ok->sizeHint().height();
    int lsize = name->sizeHint().height();
    setMinimumSize (320, lsize*2 + bsize);
    resize (320, lsize*2 + bsize);

    ok->setAccel (Key_Return);
    cancel->setAccel(Key_Escape);
    name->setFocus ();
    connect (ok , SIGNAL(clicked()), SLOT (accept()));
    connect (cancel , SIGNAL(clicked()), SLOT (reject()));
}
//**********************************************************
const char *StringEnterDialog::getCommand () {
    printf ("module:%s\n", name->text());
    return name->text();
}
//**********************************************************
void StringEnterDialog::resizeEvent (QResizeEvent *) {
    int bsize = ok->sizeHint().height();
    int lsize = name->sizeHint().height();

    name->setGeometry (width() / 20, 0, width()*18 / 20, lsize);
    ok->setGeometry (width() / 10, height() - bsize*3 / 2, width()*3 / 10, bsize);
    cancel->setGeometry (width()*6 / 10, height() - bsize*3 / 2, width()*3 / 10, bsize);
}
//**********************************************************
StringEnterDialog::~StringEnterDialog () {}










