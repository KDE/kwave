#ifndef _DIALOGS_NEWSAMPLE_H_
#define _DIALOGS_NEWSAMPLE_H 1
#include <stdlib.h>
#include <qcombobox.h>
#include <qlabel.h>
#include "libgui/Dialog.h"
#include "libkwave/DialogOperation.h"
#include "libgui/TimeLine.h"
#include <kintegerline.h>
//*****************************************************************************
class NewSampleDialog : public Dialog {
    Q_OBJECT

public:

    NewSampleDialog (bool modal);
    ~NewSampleDialog ();
    const char *getCommand ();

public slots:

    void setRate (const char *);

protected:

    void resizeEvent (QResizeEvent *);

private:

    TimeLine *time;
    QLabel *timelabel;
    QLabel *ratelabel;
    QComboBox *ratefield;
    QPushButton *ok, *cancel;
    char * comstr;
};
#endif
