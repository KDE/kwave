#include <stdio.h>
#include <stdlib.h>

#include <qpushbutton.h>
#include <qkeycode.h>
#include <qlabel.h>
#include <qlineedit.h>

#include <kintegerline.h>
#include <ktreelist.h>
#include <kapp.h>

#include "libkwave/DialogOperation.h"
#include "libkwave/Global.h"

#include "module.h"

const char *version = "1.0";
const char *author = "Martin Wilz";
const char *name = "menus";
//**********************************************************
Dialog *getDialog (DialogOperation *operation) {
    return new MenuDialog(operation->getGlobals(), operation->isModal());
}
//**********************************************************
MenuDialog::MenuDialog (Global *globals, bool modal): Dialog(modal) {
    this->globals = globals;
    setCaption (i18n ("Choose menu layout :"));

    source = new KTreeList (this);
    dest = new KTreeList (this);
    ok = new QPushButton (i18n("&Ok"), this);
    cancel = new QPushButton ("&Cancel", this);

    int bsize = ok->sizeHint().height();

    setMinimumSize (320, bsize*9 / 2);
    resize (320, bsize*9 / 2);

    ok->setAccel (Key_Return);
    cancel->setAccel(Key_Escape);
    ok->setFocus ();
    connect (ok , SIGNAL(clicked()), SLOT (accept()));
    connect (cancel , SIGNAL(clicked()), SLOT (reject()));
}
//**********************************************************
const char *MenuDialog::getCommand () {
    return "readmenus()";
}
//**********************************************************
void MenuDialog::resizeEvent (QResizeEvent *) {
    int bsize = ok->sizeHint().height();

    source->setGeometry (8, 8, width()*5 / 10-8, bsize*4);
    dest->setGeometry (width() / 2, 8, width()*5 / 10-8, bsize*4);

    ok->setGeometry (width() / 10, height() - bsize*3 / 2, width()*3 / 10, bsize);
    cancel->setGeometry (width()*6 / 10, height() - bsize*3 / 2, width()*3 / 10, bsize);
}
//**********************************************************
MenuDialog::~MenuDialog () {}









