#ifndef _DIALOGS_FREQMULT_H_
#define _DIALOGS_FREQMULT_H 1

#include <kintegerline.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include "../../../libgui/CurveWidget.h"
#include "../../../libgui/ScaleWidget.h"
#include "../../../libgui/CornerPatchWidget.h"
#include "../../../libgui/Dialog.h"
#include <libkwave/DialogOperation.h>
//*****************************************************************************
class FrequencyMultDialog : public Dialog {
    Q_OBJECT

public:
    FrequencyMultDialog (int rate, bool);
    ~FrequencyMultDialog ();
    const char *getCommand();

public slots:
    void addPoint ();

protected:
    void resizeEvent (QResizeEvent *);

private:

    CurveWidget *curve;
    ScaleWidget *xscale, *yscale;
    CornerPatchWidget *corner;
    QLabel *xlabel, *ylabel;
    KIntegerLine *x, *y;
    QPushButton *add;
    QPushButton *ok, *cancel;
    int rate;
    char *comstr;
};
#endif
