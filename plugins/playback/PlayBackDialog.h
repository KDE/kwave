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

#ifndef PLAY_BACK_DIALOG_H
#define PLAY_BACK_DIALOG_H

#include "config.h"

#include <QMap>
#include <QString>

#include "libkwave/PlayBackParam.h"
#include "libkwave/PlayBackTypesMap.h"

#include "libgui/TreeWidgetWrapper.h"

#include "ui_PlayBackDlg.h"

class QTreeWidgetItem;

namespace Kwave
{

    class PlaybackController;
    class PlayBackDevice;
    class Plugin;

    class PlayBackDialog: public QDialog, public Ui::PlayBackDlg
    {
        Q_OBJECT

    public:
        /** Constructor */
        PlayBackDialog(Kwave::Plugin &p,
                       Kwave::PlaybackController &playback_controller,
                       const Kwave::PlayBackParam &params);

        /** Destructor */
        virtual ~PlayBackDialog();

        /** Returns the current set of parameters */
        const Kwave::PlayBackParam &params();

        /**
         * Selects a new playback method.
         */
        void setMethod(Kwave::playback_method_t method);

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
        void setSupportedBits(const QList<unsigned int> &bits);

        /**
         * Sets the lowest and highest number of playback channels
         * @param min lowest supported number of channels
         * @param max highest supported number of channels
         */
        void setSupportedChannels(unsigned int min, unsigned int max);

    signals:

        /** emits changes in the currently selected playback method */
        void sigMethodChanged(Kwave::playback_method_t method);

        /** emitted when the user clicked on the "Test..." button */
        void sigTestPlayback();

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

        /** selection in the device list view has changed */
        void listEntrySelected(QTreeWidgetItem *current,
                               QTreeWidgetItem *previous);

        /** selection in the device list view has changed */
        void listItemExpanded(QTreeWidgetItem *item);

        /**
         * updates/fixes the device selection when the tree view has
         * lost focus, to avoid that nothing is selected
         */
        void updateListSelection();

        /** selection in the bits per sample combo box has changed */
        void bitsPerSampleSelected(const QString &text);

        /** invoke the online help */
        void invokeHelp();

    private:

        /** reference to the playback controller */
        Kwave::PlaybackController &m_playback_controller;

        /** The playback device used for configuring and testing playback */
        Kwave::PlayBackDevice *m_device;

        /** all parameters needed for playback */
        Kwave::PlayBackParam m_playback_params;

        /** map of playback methods/types */
        Kwave::PlayBackTypesMap m_methods_map;

        /** file filter for the "Select..." dialog (optional) */
        QString m_file_filter;

        /** map for items in the list view */
        QMap<QTreeWidgetItem *, QString> m_devices_list_map;

        /** if false, do nothing in setDevice */
        bool m_enable_setDevice;
    };
}

#endif /* _PLAY_BACK_DIALOG_H */

//***************************************************************************
//***************************************************************************
