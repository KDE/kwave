#ifndef _DIALOGS_MARKTYPE_H_
#define _DIALOGS_MARKTYPE_H 1

#include <qlabel.h>
#include <qcheckbox.h>
#include <kintegerline.h>
#include <kcolordlg.h>
#include "../../../libgui/Dialog.h"
#include <libkwave/DialogOperation.h>
#include <libkwave/Color.h>

//*****************************************************************************
class MarkerTypeDialog : public Dialog {
    Q_OBJECT

public:
    MarkerTypeDialog (bool modal);
    ~MarkerTypeDialog ();
    const char *getCommand();

public slots:

    void setColor(const Color &col);

protected:

    void resizeEvent (QResizeEvent *);

private:

    KColorCombo *color;
    QCheckBox *individual;
    QLabel *namelabel;
    QLineEdit *name;
    QPushButton *ok, *cancel;
    Color col;
    char *comstr;
};
#endif
