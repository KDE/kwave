
#include "config.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <kapp.h>
#include <libkwave/String.h>
#include "TimeLine.h"

extern QString mstotime (int ms);
extern char* mstotimec (int ms);

//**********************************************************
TimeLine::TimeLine(QWidget *parent, int rate)
    :KRestrictedLine(parent)
{
    comstr = 0;
    this->rate = rate;
    menu = new QPopupMenu();
    mode = 1;
    rate = 0;
    value = 1;

    ASSERT(menu);
    if (!menu) return;

    menu->insertItem(i18n("as number of samples"), this,
                     SLOT(setSampleMode()));
    menu->insertItem(i18n("in ms"), this, SLOT(setMsMode()));
    menu->insertItem(i18n("in s"), this, SLOT(setSMode()));
    menu->insertItem(i18n("in kb"), this, SLOT(setKbMode()));

    menu->setCheckable (true);

    menu->setItemChecked(menu->idAt(0), false);
    menu->setItemChecked (menu->idAt(1), true);
    menu->setItemChecked (menu->idAt(2), false);
    menu->setItemChecked (menu->idAt(3), false);

    connect(this, SIGNAL(textChanged(const char *)),
            SLOT(setValue(const char *)) );
};

//**********************************************************
int TimeLine::getValue() 
{
    return value;
}

//**********************************************************
double TimeLine::getMs() 
{
    ASSERT(rate);
    if (!rate) return 0.0;
    return ((double(value))*1000 / rate);
}

//**********************************************************
const char *TimeLine::getMsStr() 
{
    ASSERT(rate);
    if (!rate) return 0;

    char buf[128];
    snprintf(buf, sizeof(buf), "%f", ((double(value))*1000 / rate));
    if (comstr) delete[] comstr;
    comstr = duplicateString (buf);
    return comstr;
}

//**********************************************************
void TimeLine::setRate(int newrate) 
{
    rate = newrate;
    setSamples (value);
}

//**********************************************************
void TimeLine::setValue(const char *newvalue) 
{
    switch (mode) {
    case 0:
	value = strtol(newvalue, 0, 0);
	    break;
    case 1:
	value = (int)((double)(rate * strtod (newvalue, 0) / 1000) + .5);
	break;
    case 2:
	value = (int)((double)(rate * strtod (newvalue, 0)) + .5);
	break;
    case 3:
	value = (int)((double)(strtod(newvalue, 0)*1024) / sizeof(int) - .5);
	break;
    }
}

//**********************************************************
void TimeLine::setSampleMode() 
{
    ASSERT(menu);
    if (!menu) return;

    menu->setItemChecked(menu->idAt(0), true);
    menu->setItemChecked(menu->idAt(1), false);
    menu->setItemChecked(menu->idAt(2), false);
    menu->setItemChecked(menu->idAt(3), false);
    setValidChars("0123456789");
    mode = 0;
    setSamples(value);
}

//**********************************************************
void TimeLine::setMsMode() 
{
    ASSERT(menu);
    if (!menu) return;

    menu->setItemChecked(menu->idAt(0), false);
    menu->setItemChecked(menu->idAt(1), true);
    menu->setItemChecked(menu->idAt(2), false);
    menu->setItemChecked(menu->idAt(3), false);
    setValidChars("0123456789.");
    mode = 1;
    setSamples(value);
}

//**********************************************************
void TimeLine::setSMode() 
{
    ASSERT(menu);
    if (!menu) return;

    menu->setItemChecked(menu->idAt(0), false);
    menu->setItemChecked(menu->idAt(1), false);
    menu->setItemChecked(menu->idAt(2), true);
    menu->setItemChecked(menu->idAt(3), false);
    setValidChars ("0123456789.");
    mode = 2;
    setSamples (value);
}

//**********************************************************
void TimeLine::setKbMode() 
{
    ASSERT(menu);
    if (!menu) return;

    menu->setItemChecked (menu->idAt(0), false);
    menu->setItemChecked (menu->idAt(1), false);
    menu->setItemChecked (menu->idAt(2), false);
    menu->setItemChecked (menu->idAt(3), true);
    setValidChars ("0123456789.");
    mode = 3;
    setSamples (value);
}

//**********************************************************
void TimeLine::setSamples(int samples) 
{
    ASSERT(rate);
    if (!rate) return;

    char buf[64];
    double pr;
    value = samples;

    switch (mode) {
	case 0:
	    snprintf(buf, sizeof(buf), "%d samples", value);
	    this->setText (buf);
	    break;
	case 1:
	    pr = ((double)value) * 1000 / rate;
	    snprintf(buf, sizeof(buf), "%.03f ms", pr);
	    this->setText (buf);
	    break;
	case 2:
	    pr = ((double)value) / rate;
	    snprintf(buf, sizeof(buf), "%.3f s", pr);
	    this->setText (buf);
	    break;
	case 3:
	    pr = ((double)(value)) * sizeof(int) / 1024;
	    snprintf(buf, sizeof(buf), "%.3f kb", pr);
	    this->setText (buf);
	    break;
    }

}

//**********************************************************
void TimeLine::setMs(int ms) 
{
    char buf[16];

    value = (int) ((double)(rate * ms / 1000) + .5);
    if (mode == 0) {
	snprintf(buf, sizeof(buf), "%d samples", value);
	this->setText (buf);
    } else {
	snprintf(buf, sizeof(buf), "%d.%d ms", ms, 0);
	this->setText (buf);
    }
}

//**********************************************************
void TimeLine::mousePressEvent( QMouseEvent *e)
{
    ASSERT(e);
    ASSERT(menu);
    if (!e) return;
    if (!menu) return;

    if (e->button() == RightButton) {
	QPoint popup = QCursor::pos();
	menu->popup(popup);
    }
}

//**********************************************************
TimeLine::~TimeLine()
{
    if (comstr) delete[] comstr;
    if (menu) delete menu;
};

//**********************************************************
//**********************************************************
