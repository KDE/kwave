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

#ifndef _FILE_PROGRESS_H_
#define _FILE_PROGRESS_H_

#include "config.h"
#include <qdatetime.h>
#include <qsemimodal.h>
#include <qurl.h>

#include <kdialog.h>

class QCloseEvent;
class QGridLayout;
class QLabel;
class QResizeEvent;
class KProgress;

class FileProgress: public KDialog
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
	const QUrl &url, unsigned int size,
	unsigned int samples, double rate, unsigned int bits,
	unsigned int tracks);

    /**
     * Returns true if the dialog is unusable or the user
     * has pressed the "cancel" button.
     */
    inline bool isCancelled() { return m_cancelled; };

signals:

    /**
     * Emitted when the user has pressed the Cancel button and
     * has confirmed with "Yes".
     */
    void cancelled();

public slots:
    /**
     * Advances the progress to a given position within the file.
     * @param pos position within the file, in samples
     */
    void setValue(unsigned int pos);

    /**
     * Like setValue, but takes position in bytes as argument
     * @param pos position within the file, in bytes [0...m_size-1]
     */
    void setBytePosition(unsigned int pos);

    /**
     * Updates the length information, needed in stream mode.
     * @param samples total number of samples, must be divided through
     *        the number of tracks for getting the real length
     */
    void setLength(unsigned int samples);
    
protected slots:

    /**
     * Connected to the "cancel" button to set the "m_cancelled"
     * flag if the user wants to abort.
     */
    void cancel();

protected:

    /**
     * Fits again the URL label on resize events.
     * @see fitUrlLabel()
     */
    virtual void resizeEvent(QResizeEvent *);

    /**
     * Called if the window is to be closed.
     */
    virtual void closeEvent(QCloseEvent *e);

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
     * @param col the column within the layout [0...columns-1]
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
    void updateStatistics(double rate, double rest, unsigned int pos);

    /** url of the file */
    QUrl m_url;

    /** size of the file [Bytes] */
    unsigned int m_size;

    /** label with the url, shortened when too long */
    QLabel *m_lbl_url;

    /** label with length info, changes in streaming mode */
    QLabel *m_lbl_length;
    
    /** progress bar */
    KProgress *m_progress;

    /** label with transfer statistics */
    QLabel *m_stat_transfer;

    /** label with progress statistics */
    QLabel *m_stat_bytes;

    /** start time, set on initialization of this dialog */
    QTime m_time;

    /** true if the dialog is unusable or canceled by the user */
    bool m_cancelled;

    /** last displayed percent value */
    int m_last_percent;

    /** number of bits per sample */
    unsigned int m_bits_per_sample;

    /** number of samples per second, used for output */
    double m_sample_rate;

    /** number of tracks */
    unsigned int m_tracks;
    
};

#endif /* _FILE_PROGRESS_H_ */
