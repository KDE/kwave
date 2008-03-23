/***************************************************************************
       FileProgress.cpp  -  progress window for loading/saving files
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

#include "config.h"
#include <math.h>
#include <sched.h> // for sched_yield()

#include <QApplication>
#include <QCloseEvent>
#include <QGridLayout>
#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>

#include <kpushbutton.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandardguiitem.h>

#include "libkwave/KwavePlugin.h"  // for ms2string
#include "libgui/FileProgress.h"

//***************************************************************************
FileProgress::FileProgress(QWidget *parent,
	const QUrl &url, unsigned int size,
	unsigned int samples, qreal rate, unsigned int bits,
	unsigned int tracks)
    :QDialog(parent),
    m_url(url),
    m_size(size),
    m_lbl_url(0),
    m_lbl_length(0),
    m_progress(0),
    m_stat_transfer(0),
    m_stat_bytes(0),
    m_time(),
    m_canceled(true),
    m_last_percent(0),
    m_bits_per_sample(bits),
    m_sample_rate(rate),
    m_tracks(tracks)
{
    setModal(true);

    QString text;

    // start the timer now
    m_time.start();

    // set the caption to the url
    setWindowTitle(m_url.toString());

    // toplevel uses a vbox layout
    QVBoxLayout *top_layout = new QVBoxLayout(this);
    Q_ASSERT(top_layout);
    if (!top_layout) return;
    top_layout->setMargin(10);
    top_layout->setSpacing(10);

    // sublayout for the lines with the file info
    QGridLayout *info_layout = new QGridLayout();
    Q_ASSERT(info_layout);
    if (!info_layout) return;
    info_layout->setSpacing(0);
    info_layout->setColumnStretch(0, 0);
    info_layout->setColumnStretch(1, 100);
    top_layout->addLayout(info_layout);

    // label with "source"
    if (!addInfoLabel(info_layout, i18n("source: "), 0, 0)) return;
    text = "?";
    m_lbl_url = addInfoLabel(info_layout, text, 0, 1);
    if (!m_lbl_url) return;

    // label with "length"
    if (!addInfoLabel(info_layout, i18n("length: "), 1, 0)) return;

    m_lbl_length = addInfoLabel(info_layout, "", 1, 1);
    if (!m_lbl_length) return;
    setLength(samples*tracks);

    // label with "rate:"
    if (!addInfoLabel(info_layout, i18n("sample rate: "), 2, 0)) return;
    text = i18n("%1 samples per second", rate);
    if (!addInfoLabel(info_layout, text, 2, 1)) return;

    // label with "resolution:"
    if (!addInfoLabel(info_layout, i18n("resolution: "), 3, 0)) return;
    text = i18n("%1 bits per sample", bits);
    if (!addInfoLabel(info_layout, text, 3, 1)) return;

    // label with "tracks:"
    if (!addInfoLabel(info_layout, i18n("tracks: "), 4, 0)) return;
    switch (tracks) {
	case 1:
	    text = i18n("1 (mono)");
	    break;
	case 2:
	    text = i18n("2 (stereo)");
	    break;
	case 4:
	    text = i18n("4 (quadro)");
	    break;
	default:
	    text = text.setNum(tracks);
    }
    if (!addInfoLabel(info_layout, text, 4, 1)) return;

    // progress bar
    m_progress = new QProgressBar(this);
    Q_ASSERT(m_progress);
    if (!m_progress) return;
    top_layout->addWidget(m_progress, 0, 0);
    m_progress->setMinimum(0);
    m_progress->setMaximum(100);

    // sublayout for the line with estimated time
    QGridLayout *status_layout = new QGridLayout();
    Q_ASSERT(status_layout);
    if (!status_layout) return;
    status_layout->setSpacing(1);
    status_layout->setColumnMinimumWidth(1, 20);
    top_layout->addLayout(status_layout);

    // create the statistic labels
    m_stat_transfer = addInfoLabel(status_layout, "-", 1, 0);
    if (!m_stat_transfer) return;
    m_stat_bytes = addInfoLabel(status_layout, "-", 1, 2);
    if (!m_stat_bytes) return;

    // some dummy update to get maximum size
    updateStatistics(9999999.9, (qreal)(99*24*60*60), size);

    // now correct the minimum sizes of the statistic entries
    m_stat_transfer->adjustSize();
    m_stat_transfer->setMinimumWidth(m_stat_transfer->sizeHint().width());
    m_stat_bytes->adjustSize();
    m_stat_bytes->setMinimumWidth(m_stat_bytes->sizeHint().width());

    // right lower edge: the "cancel" button
    KPushButton *bt_cancel =
	new KPushButton(KStandardGuiItem::cancel(), this);
    Q_ASSERT(bt_cancel);
    if (!bt_cancel) return;
    bt_cancel->setFixedSize(bt_cancel->sizeHint());
    bt_cancel->setFocus();
    bt_cancel->setShortcut(Qt::Key_Escape);
    connect(bt_cancel, SIGNAL(clicked()), this, SLOT(cancel()));
    top_layout->addWidget(bt_cancel, 0, Qt::AlignRight);

    // activate the layout and show the dialog
    top_layout->activate();
    setFixedHeight(sizeHint().height());
    setMinimumWidth(sizeHint().width());

    show();
    fitUrlLabel();

    // now set canceled to false, this dialog is ready for use
    m_canceled = false;
}

//***************************************************************************
void FileProgress::resizeEvent(QResizeEvent *)
{
    fitUrlLabel();
}

//***************************************************************************
void FileProgress::closeEvent(QCloseEvent *e)
{
    Q_ASSERT(e);
    if (!e) return;

    // if not already cancelled -> ask user to confirm
    if (!m_canceled) {
	if (KMessageBox::warningYesNo(this,
	    i18n("Do you really want to abort the operation?")) !=
	    KMessageBox::Yes)
	{
	    // the user was wise and said "No"
	    e->ignore();
	    return;
	} else {
	    // user pressed "Yes", he should know what he has done
	    m_canceled = true;
	}
    }

    // default action: accept
    e->accept();
}

//***************************************************************************
void FileProgress::fitUrlLabel()
{
    if (!m_lbl_url) return;

    int width = m_lbl_url->frameRect().width();
    QString url = m_url.toString();
    m_lbl_url->setText(url);
    int todel = 4;
    while (m_lbl_url->sizeHint().width() > width) {
	// delete characters in the middle of the string
	url = m_url.toString();
	int len = url.length();
	if (len <= todel) break;
	url = url.left((len-todel)/2) + "..." +
	    url.right((len-todel)/2 + 4);
	m_lbl_url->setText(url);
	todel++;
    }

    m_lbl_url->adjustSize();
}

//***************************************************************************
QLabel *FileProgress::addInfoLabel(QGridLayout *layout, const QString text,
	int row, int col)
{
    QLabel *label = new QLabel(this);
    Q_ASSERT(label);
    if (!label) return 0;

    label->setText(text);
    label->adjustSize();
    label->setFixedHeight(label->sizeHint().height());
    label->setMinimumWidth(label->sizeHint().width());

    layout->addWidget(label, row, col);

    return label;
}

//***************************************************************************
void FileProgress::updateStatistics(qreal rate, qreal rest,
	unsigned int pos)
{
    QString text;
    QString num;

    if (!m_stat_transfer) return;
    if (!m_stat_bytes) return;

    // left: transfer rate and estimated time
    num = num.sprintf("%1.1f", rate/1024.0);

    int h =  (int)floor(rest) / (60*60);
    int m = ((int)floor(rest) / 60) % 60;
    int s =  (int)floor(rest) % 60;
    if (h > 23) {
	h = 23;
	m = s = 59;
    }
    QTime time(h,m,s,0);
    text = i18n("%1 KB/s (%2 remain)", num, time.toString());
    m_stat_transfer->setText(text);

    // right: statistic over the transferred bytes
    text = i18n("%1 MB of %2 MB done",
	num.sprintf("%1.1f", pos / (1024.0*1024.0)),
	num.sprintf("%1.1f", m_size / (1024.0*1024.0)));
    m_stat_bytes->setText(text);
}

//***************************************************************************
void FileProgress::setValue(unsigned int pos)
{
    // position is in samples, we need bytes
    setBytePosition(pos * (m_bits_per_sample >> 3));
}

//***************************************************************************
void FileProgress::setBytePosition(unsigned int pos)
{
    if (!m_progress) return;

    // the easiest part: the progress bar and the caption
    int percent = (int)((qreal)pos / (qreal)m_size * 100.0);

    // not enough progress not worth showing ?
    if (percent <= m_last_percent) return;
    m_last_percent = percent;

    if (m_progress->value() != percent) {
	QString newcap;
	newcap = i18n("(%1%) %2", percent, m_url.toString());
	setWindowTitle(newcap);

	m_progress->setValue(percent);
    }

    // update the transfer statistics
    qreal seconds = m_time.elapsed() / 1000.0; // [sec]
    qreal rate = pos / seconds;                // [bytes/sec]
    qreal rest = 0;
    if (rate > 10) {
	rest = qreal(m_size - pos) / rate;     // [seconds]
    }
    updateStatistics(rate, rest, pos);

    // as this dialog is modal, we must take care of our events
    // manually !
    qApp->processEvents();

    // better be nice to other processes and let them also play a bit :)
    sched_yield();
}

//***************************************************************************
void FileProgress::setLength(unsigned int samples)
{
    QString text;

    // length in samples -> h:m:s
    if (m_sample_rate) {
	// length in ms
	text = KwavePlugin::ms2string((float)(samples/m_tracks)/
	                              (float)m_sample_rate*1000.0);
    } else {
	// fallback if no rate: length in samples
        text = i18n("%1 samples", samples);
    }
    m_lbl_length->setText(text);
}

//***************************************************************************
void FileProgress::cancel()
{
    close();
    if (m_canceled) emit canceled();
}

//***************************************************************************
#include "FileProgress.moc"
//***************************************************************************
//***************************************************************************
