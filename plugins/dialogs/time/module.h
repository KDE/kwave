#ifndef _DIALOGS_TIME_H
#define _DIALOGS_TIME_H 1
#include "../../../libgui/Dialog.h"
#include <libkwave/DialogOperation.h>
#include <qlabel.h>
#include "../../../libgui/TimeLine.h"
//*****************************************************************************
class TimeDialog : public Dialog {
    Q_OBJECT

public:

    TimeDialog (bool, int rate);
    ~TimeDialog ();
    const char *getCommand ();

public slots:

    void setLength (int);

protected:

    void resizeEvent (QResizeEvent *);

private:

    TimeLine *time;
    QLabel *timelabel;
    QPushButton *ok, *cancel;
    char *comstr;
};
#endif
