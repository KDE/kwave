
#include "libgui/Dialog.h"
#include "libgui/FaderWidget.h"
#include "libgui/ScaleWidget.h"
#include "libgui/CornerPatchWidget.h"
#include "libgui/Slider.h"
#include "libkwave/DialogOperation.h"

class FadeDialog : public Dialog {
    Q_OBJECT

public:
    FadeDialog (bool modal, int ms);
    ~FadeDialog ();
    const char *getCommand();

protected:

    void resizeEvent (QResizeEvent *);

private:

    ScaleWidget *x, *y;
    CornerPatchWidget *corner;
    QPushButton *ok, *cancel;
    Slider *slider;
    FaderWidget *fade;
    char *comstr;
};
