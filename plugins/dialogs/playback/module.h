/***************************************************************************
		  module.h  -  header file for the playback dialog plugin
			     -------------------
    begin                : Fri Aug 04 2000
    copyright            : (C) 2000 by Thomas Eschenbacher
    email                : Thomas.Eschenbacher@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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
#endif /* _PLAYBACK_DIALOG_H */
