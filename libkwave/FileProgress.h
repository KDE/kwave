/***************************************************************************
         FileProgress.h  -  progress window for loading/saving files
                             -------------------
    begin                : Mar 11 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
    email                : Thomas Eschenbacher <thomas.eschenbacher@gmx.de>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef FILE_PROGRESS_H
#define FILE_PROGRESS_H

#include "config.h"

#include <QtGlobal>
#include <QDialog>
#include <QElapsedTimer>
#include <QLabel>
#include <QUrl>

#include "libkwave/Sample.h"

class QCloseEvent;
class QGridLayout;
class QLabel;
class QProgressBar;
class QResizeEvent;
class KProgress;

namespace Kwave
{

    class Q_DECL_EXPORT FileProgress: public QDialog
    {
        Q_OBJECT
    public:
        /**
         * Constructor
         * @param parent the parent widget
         * @param url the URL of the file
         * @param size the size of the file in bytes
         * @param samples the number of samples
         * @param rate sample rate in samples per second
         * @param bits number of bits per sample
         * @param tracks number of tracks
         */
        FileProgress(QWidget *parent,
            const QUrl &url, quint64 size,
            sample_index_t samples, double rate, unsigned int bits,
            unsigned int tracks);

        /** Destructor */
        virtual ~FileProgress() Q_DECL_OVERRIDE {}

        /**
         * Returns true if the dialog is unusable or the user
         * has pressed the "cancel" button.
         */
        inline bool isCanceled() { return m_canceled; }

    signals:

        /**
         * Emitted when the user has pressed the Cancel button and
         * has confirmed with "Yes".
         */
        void canceled();

    public slots:

        /**
         * Advances the progress to a given position within the file.
         * @param percent position within the file, in percent
         */
        void setValue(qreal percent);

        /**
         * Like setValue, but takes position in bytes as argument
         * @param pos position within the file, in bytes [0...m_size-1]
         */
        void setBytePosition(quint64 pos);

        /**
         * Updates the length information, needed in stream mode.
         * @param samples total number of samples, must be divided through
         *        the number of tracks for getting the real length
         */
        void setLength(quint64 samples);

    protected slots:

        /**
         * Connected to the "cancel" button to set the "m_canceled"
         * flag if the user wants to abort.
         */
        void cancel();

    protected:

        /**
         * Fits again the URL label on resize events.
         * @see fitUrlLabel()
         */
        virtual void resizeEvent(QResizeEvent *) Q_DECL_OVERRIDE;

        /**
         * Called if the window is to be closed.
         */
        virtual void closeEvent(QCloseEvent *e) Q_DECL_OVERRIDE;

        /**
         * Fits the URL text into the available area, with
         * shortening it if necessary.
         */
        void fitUrlLabel();

        /**
         * Adds a label to an info field. Used within the constructor.
         * @param layout the QGridLayout with the labels
         * @param text the content of the label, localized
         * @param row the row within the layout [0...rows-1]
         * @param column the column within the layout [0...columns-1]
         * @return the label if successful, 0 if failed
         * @internal
         */
        QLabel *addInfoLabel(QGridLayout *layout, const QString text,
            int row, int column);

        /**
         * Updates the statistics of the transferred bytes and
         * transfer rate.
         * @param rate transfer rate in kilobytes per seconds
         * @param rest remaining time in seconds
         * @param pos position in the file
         * @internal
         */
        void updateStatistics(double rate, double rest, quint64 pos);

    protected:

        /** url of the file */
        QUrl m_url;

        /** size of the file [Bytes] */
        quint64 m_size;

        /** label with the url, shortened when too long */
        QLabel *m_lbl_url;

        /** label with length info, changes in streaming mode */
        QLabel *m_lbl_length;

        /** progress bar */
        QProgressBar *m_progress;

        /** label with transfer statistics */
        QLabel *m_stat_transfer;

        /** label with progress statistics */
        QLabel *m_stat_bytes;

        /** start time, set on initialization of this dialog */
        QElapsedTimer m_time;

        /** true if the dialog is unusable or canceled by the user */
        bool m_canceled;

        /** last displayed percent value */
        int m_last_percent;

        /** number of bits per sample */
        unsigned int m_bits_per_sample;

        /** number of samples per second, used for output */
        double m_sample_rate;

        /** number of tracks */
        unsigned int m_tracks;

    };

}

#endif /* FILE_PROGRESS_H */

//***************************************************************************
//***************************************************************************
