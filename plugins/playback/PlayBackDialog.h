/***************************************************************************
       PlayBackDialog.h  -  dialog for configuring the playback
			     -------------------
    begin                : Sun May 13 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
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

#ifndef _PLAY_BACK_DIALOG_H_
#define _PLAY_BACK_DIALOG_H_

#include <kdialog.h>

#include "libkwave/PlayBackParam.h" // for playback_param_t

class QLabel;
class QFrame;
class QComboBox;
class QCheckBox;
class QRadioButton;
class QButtonGroup;

class KwavePlugin;
class Slider;

//*****************************************************************************
class PlayBackDialog : public KDialog
{
    Q_OBJECT

public:
    /** Constructor */
    PlayBackDialog(KwavePlugin &p, const playback_param_t &params);

    /** Destructor */
    virtual ~PlayBackDialog();

    /**
     * Fills the current parameters into a parameter list.
     * The list always is cleared before it gets filled.
     */
    void parameters(QStringList &list);

private slots:

    /** Selects a buffer size exponent */
    void setBufferSize(int exp);

    void selectPlaybackDevice();

private:
    /** all parameters needed for playback */
    playback_param_t m_playback_params;

    QLabel *m_buffer_label;
    Slider *m_buffer_size;
    QLabel *m_device_label;
    QComboBox *m_device_box;
    QCheckBox *m_stereo;
    QButtonGroup *m_bg;
    QRadioButton *m_b24, *m_b16, *m_b8;
    QFrame *m_separator;
    QPushButton *m_select_device, *m_test, *m_ok, *m_cancel;
};

//*****************************************************************************
#endif /* _PLAY_BACK_DIALOG_H */
