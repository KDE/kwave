#ifndef _DIALOGS_MIX_H_
#define _DIALOGS_MIX_H_ 1

#include <qwidget.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include "../../../libgui/Slider.h"
#include "../../../libgui/FloatLine.h"
#include "../../../libgui/TimeLine.h"
#include "../../../libgui/Dialog.h"
#include <libkwave/DialogOperation.h>

class ChannelMixDialog : public Dialog {
    Q_OBJECT

public:

    ChannelMixDialog (bool modal, int numberofchannels);
    ~ChannelMixDialog ();
    const char *getCommand ();

public slots:

    void setValue (int);
    void setValue (const char *);
    void setdBMode(bool);

protected:

    void resizeEvent (QResizeEvent *);

private:

    QLabel **channelname;
    FloatLine **valuebox;
    Slider **slider;
    QPushButton *ok, *cancel;
    QLabel *tochannellabel;
    QComboBox *tochannel;
    QLabel *usedblabel;
    QCheckBox *usedb;
    int channels;    //number of channels available
    double *value;
    bool tflag;
    bool dbmode;
    char *comstr;
};
//*****************************************************************************
#endif
