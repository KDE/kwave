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

#include "config.h"

#include "PlayBackParam.h"
#include "PlayBackDlg.uih.h"
#include "PlayBackTypesMap.h"

class KwavePlugin;

//*****************************************************************************
class PlayBackDialog : public PlayBackDlg
{
    Q_OBJECT

public:
    /** Constructor */
    PlayBackDialog(KwavePlugin &p, const PlayBackParam &params);

    /** Destructor */
    virtual ~PlayBackDialog();

    /** Returns the current set of parameters */
    const PlayBackParam &params();

    /**
     * Selects a new playback method.
     */
    void setMethod(playback_method_t method);

    /**
     * Sets the list of supported devices, just entries
     * for the device selection combo box.
     */
    void setSupportedDevices(QStringList devices);

    /** return the file filter used for the "Select..." dialog */
    inline QString fileFilter() {
	return m_file_filter;
    }

    /** Sets the list of supported bits per sample */
    void setSupportedBits(const QValueList<unsigned int> &bits);

    /**
     * Sets the lowest and highest number of playback channels
     * @param min lowest supported number of channels
     * @param max highest supported number of channels
     */
    void setSupportedChannels(unsigned int min, unsigned int max);

signals:

    /** emits changes in the currently selected playback method */
    void sigMethodChanged(playback_method_t method);

    /** emits changes in the currently selected device name */
    void sigDeviceChanged(const QString &device);

public slots:

    /** set the file filter used for the "Select..." dialog */
    void setFileFilter(const QString &filter);

    /** set a new device name */
    void setDevice(const QString &device);

    /** sets the new value for bits per sample */
    void setBitsPerSample(unsigned int bits);

    /** Sets the number of channels */
    void setChannels(int channels);

private slots:

    /**
     * called when a new playback method has been selected
     * from the combo box
     * @param index the position within the combo box
     */
    void methodSelected(int index);

    /** Selects a buffer size exponent */
    void setBufferSize(int exp);

    /** Select a playback device through a File/Open dialog */
    void selectPlaybackDevice();

    /** selection in the bits per sample combo box has changed */
    void bitsPerSampleSelected(const QString &text);

private:

    /** all parameters needed for playback */
    PlayBackParam m_playback_params;

    /** map of playback methods/types */
    PlayBackTypesMap m_methods_map;

    /** file filter for the "Select..." dialog (optional) */
    QString m_file_filter;

};

//*****************************************************************************
#endif /* _PLAY_BACK_DIALOG_H */
