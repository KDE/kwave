#ifndef _PLAYBACK_DIALOG_H_
#define _PLAYBACK_DIALOG_H_ 1

#include <qlabel.h>
#include <qframe.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>

#include <libkwave/DialogOperation.h>

#include <libgui/Slider.h>
#include <libgui/Dialog.h>
#include "../../../src/SignalManager.h"
#include "../../../src/KwaveApp.h"

//*****************************************************************************
class PlayBackDialog : public Dialog {
    Q_OBJECT

public:

    PlayBackDialog(bool);
    bool isOK();
    ~PlayBackDialog ();
    const char *getCommand ();

private slots:

    void setBufferSize(int);

    void selectPlaybackDevice();

private:
    playback_param_t playback_params;

    QLabel *bufferlabel;
    Slider *buffersize;
    QLabel *devicelabel;
    QComboBox *devicebox;
    QCheckBox *stereo;
    QButtonGroup *bg;
    QRadioButton *b24, *b16, *b8;
    QFrame *separator;
    QPushButton *select_device, *test, *ok, *cancel;
    char *comstr;
};

//*****************************************************************************
#endif /* _PLAYBACK_DIALOGS_H */
