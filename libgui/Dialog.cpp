
#include "config.h"
#include "Dialog.h"

//**********************************************************************
Dialog::Dialog(bool modal)
    :QDialog(0, 0, modal)
{
    this->modal = modal;
}

//**********************************************************************
Dialog::Dialog(const char *name, bool modal)
    :QDialog(0, name, modal)
{
    if (name) setCaption (name);
    this->modal = modal;
}

//**********************************************************************
void Dialog::accept()
{
    hide();

//    if (!modal) {
	//call virtual function getcommand()
	const char *c = getCommand();
	debug("Dialog::accept: command='%s'", c); // ###
	if (c) emit command(c);
//	delete this;
//    }

    setResult(true);
}

//**********************************************************************
void Dialog::reject()
{
    hide();
//    if (!modal) delete this;
    debug("Dialog::reject"); // ###
    setResult (false);
}

//**********************************************************************
Dialog::~Dialog()
{
}

//**********************************************************************
//**********************************************************************
