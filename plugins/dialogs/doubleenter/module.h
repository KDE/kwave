#ifndef _DIALOGS_DENTER_H
#define _DIALOGS_DENTER_H 1

#include <kintegerline.h>
#include "../../../libgui/Dialog.h"
#include <libkwave/DialogOperation.h>

//*****************************************************************************
class DoubleEnterDialog : public Dialog {
    Q_OBJECT

public:
    DoubleEnterDialog (const char *name, bool = false);
    ~DoubleEnterDialog ();
    double value();

    const char *getCommand ();

protected:

    void resizeEvent (QResizeEvent *);

private:

    KRestrictedLine *val;
    QPushButton *ok, *cancel;
};
//*****************************************************************************
#endif  /* dialogs.h */





