#ifndef _DIALOGS_PERCENT_H_
#define _DIALOGS_PERCENT_H 1

#include <qcheckbox.h>
#include <qlabel.h>
#include "../../../libgui/Dialog.h"
#include <libkwave/DialogOperation.h>
#include "../../../libgui/Slider.h"
#include <kintegerline.h>

//*****************************************************************************
class PercentDialog : public Dialog {
    Q_OBJECT

public:
    PercentDialog (bool modal, const char *name);
    ~PercentDialog ();
    const char *getCommand ();

public slots:

    void setValue (int);

protected:

    void resizeEvent (QResizeEvent *);

private:
    Slider *slider;
    QLabel *label;
    QPushButton *ok, *cancel;
    char *comstr;
};
#endif
