#ifndef _DIALOGS_AFFT_H_
#define _DIALOGS_AFFT_H 1

#include <qcheckbox.h>
#include <qlabel.h>
#include "../../../libgui/TimeLine.h"
#include "../../../libgui/Dialog.h"
#include <libkwave/DialogOperation.h>
#include <kintegerline.h>

//*****************************************************************************
class AverageFFTDialog : public Dialog {
    Q_OBJECT

public:

    AverageFFTDialog (int rate, bool);
    ~AverageFFTDialog ();
    const char* getCommand ();

protected:

    void resizeEvent (QResizeEvent *);

private:

    QLabel *pointlabel;
    QLabel *windowtypelabel;
    TimeLine *windowlength;
    QComboBox *windowtypebox;
    QPushButton *ok, *cancel;
    int length, rate;
    char *comstr;
};
#endif
