#ifndef _DIALOGS_PITCH_H_
#define _DIALOGS_PITCH_H 1

#include <kintegerline.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include "libgui/Dialog.h"
#include "libkwave/DialogOperation.h"

//*****************************************************************************
class PitchDialog : public Dialog {
    Q_OBJECT

public:
    PitchDialog (bool modal, int time);
    ~PitchDialog ();
    const char *getCommand ();
protected:

    void resizeEvent (QResizeEvent *);

private:

    QLabel *lowlabel, *highlabel;
    KIntegerLine *low, *high, *adjust;
    QPushButton *ok, *cancel;
    QCheckBox *octave;
    char* comstr;
};
#endif
