#ifndef _DIALOGS_DELAY_H_
#define _DIALOGS_DELAY_H_ 1

#include <qlabel.h>
#include <qcheckbox.h>
#include "libgui/Slider.h"
#include "libgui/TimeLine.h"
#include "libgui/Dialog.h"
#include "libkwave/DialogOperation.h"

//****************************************************************************
class DelayDialog : public Dialog {
    Q_OBJECT

public:

    DelayDialog (int rate, bool modal);
    ~DelayDialog ();

    const char *getCommand();

public slots:

    void setAmpl (int);

protected:

    void resizeEvent (QResizeEvent *);

private:

    TimeLine *delay;
    QLabel *delaylabel;
    Slider *amplslider;
    QLabel *ampllabel;
    QCheckBox *recursive;
    QPushButton *ok, *cancel;
    char *comstr;
};
//****************************************************************************
#endif
