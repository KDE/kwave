#ifndef _DIALOGS_AMPLIFY_H_
#define _DIALOGS_AMPLIFY_H 1

#include <qcheckbox.h>
#include <qlabel.h>
#include "libgui/CurveWidget.h"
#include "libgui/ScaleWidget.h"
#include "libgui/CornerPatchWidget.h"
#include "libgui/Dialog.h"
#include "libkwave/DialogOperation.h"
#include <kintegerline.h>
//*****************************************************************************
class AmplifyCurveDialog : public Dialog {
    Q_OBJECT

public:
    AmplifyCurveDialog (int time, bool modal);
    ~AmplifyCurveDialog ();
    const char *getCommand ();

protected:

    void resizeEvent (QResizeEvent *);

private:

    CurveWidget *curve;
    ScaleWidget *xscale, *yscale;
    CornerPatchWidget *corner;
    QPushButton *ok, *cancel;
    char *comstr;
};
#endif



