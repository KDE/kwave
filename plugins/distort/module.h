#ifndef _DIALOGS_DISTORT_H_
#define _DIALOGS_DISTORT_H 1
#include <qlist.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include "../../../libgui/Dialog.h"
#include <libkwave/DialogOperation.h>
#include "../../../libgui/CurveWidget.h"
#include "../../../libgui/ScaleWidget.h"
#include "../../../libgui/CornerPatchWidget.h"
//*****************************************************************************
class QComboBox;

class DistortDialog : public Dialog {
    Q_OBJECT

public:
    DistortDialog (bool);
    ~DistortDialog ();

    const char *getCommand ();

protected:

    void resizeEvent (QResizeEvent *);

private:

    ScaleWidget *xscale, *yscale;
    CornerPatchWidget *corner;
    QComboBox *sym;
    QPushButton *ok, *cancel;
    CurveWidget *curve;
    char *comstr;
};
#endif
