#ifndef _DIALOGS_AVERAGE_H_
#define _DIALOGS_AVERAGE_H 1

#include <kintegerline.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include "../../../libgui/Dialog.h"
#include <libkwave/DialogOperation.h>
//*****************************************************************************
class AverageDialog : public Dialog {
    Q_OBJECT

public:
    AverageDialog (bool = false);
    ~AverageDialog ();

    const char*getCommand ();

protected:

    void resizeEvent (QResizeEvent *);

private:

    KIntegerLine *taps;
    QLabel *taplabel;
    QLabel *typelabel;
    QComboBox *type;
    QPushButton *ok, *cancel;
    char *comstr;
};
#endif

