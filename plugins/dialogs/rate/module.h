#ifndef _DIALOGS_NEWSAMPLE_H_
#define _DIALOGS_NEWSAMPLE_H 1

#include <kintegerline.h>
#include <qlabel.h>
#include "../../../libgui/Dialog.h"
#include <libkwave/DialogOperation.h>
//*****************************************************************************
class RateDialog : public Dialog {
    Q_OBJECT

public:

    RateDialog (bool);
    ~RateDialog ();
    const char *getCommand ();

protected:

    void resizeEvent (QResizeEvent *);

private:

    QLabel *ratelabel;
    QComboBox *ratefield;
    QPushButton *ok, *cancel;
    const char *comstr;
};
#endif
