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

#include <qaccel.h>
#include <qapplication.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qnamespace.h> // for ESCAPE key (accelerator)

#include <kpushbutton.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprogress.h>

#include "libgui/KwavePlugin.h"  // for ms2string
#include "libgui/FileProgress.h"

//***************************************************************************
FileProgress::FileProgress(QWidget *parent,
	const QUrl &url, unsigned int size,
	unsigned int samples, unsigned int rate, unsigned int bits,
	unsigned int tracks)
    :QSemiModal(parent, "FileProgress", true),
    m_url(url),
    m_size(size),
    m_lbl_url(0),
    m_progress(0),
    m_stat_transfer(0),
    m_stat_bytes(0),
    m_time(),
    m_cancelled(true),
    m_last_percent(0)
{
    QString text;

    // start the timer now
    m_time.start();

    // set the caption to the url
    setCaption(m_url.toString());

    // toplevel uses a vbox layout
    QVBoxLayout *top_layout = new QVBoxLayout(this, 10, 10);
    ASSERT(top_layout);
    if (!top_layout) return;

    // sublayout for the lines with the file info
    QGridLayout *info_layout = new QGridLayout(5, 2, 0);
    ASSERT(info_layout);
    if (!info_layout) return;
    info_layout->setColStretch(0,0);
    info_layout->setColStretch(1,100);
    top_layout->addLayout(info_layout);

    // label with "source"
    if (!addInfoLabel(info_layout, i18n("source: "), 0, 0)) return;
    text = "?";
    m_lbl_url = addInfoLabel(info_layout, text, 0, 1);
    if (!m_lbl_url) return;

    // label with "length"
    if (!addInfoLabel(info_layout, i18n("length: "), 1, 0)) return;

    // length in samples -> h:m:s
    if (rate) {
	// length in ms
	text = KwavePlugin::ms2string((float)samples/(float)rate*1000.0);
    } else {
	// fallback if no rate: length in samples
        text = i18n("%1 samples").arg(samples);
    }
    if (!addInfoLabel(info_layout, text, 1, 1)) return;

    // label with "rate:"
    if (!addInfoLabel(info_layout, i18n("sample rate: "), 2, 0)) return;
    text = i18n("%1 samples per second").arg(rate);
    if (!addInfoLabel(info_layout, text, 2, 1)) return;

    // label with "resolution:"
    if (!addInfoLabel(info_layout, i18n("resolution: "), 3, 0)) return;
    text = i18n("%1 bits per sample").arg(bits);
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
    m_progress = new KProgress(100, this);
    ASSERT(m_progress);
    if (!m_progress) return;
    top_layout->addWidget(m_progress, 0, 0);

    // sublayout for the line with estimated time
    QGridLayout *status_layout = new QGridLayout(1, 3, 1);
    ASSERT(status_layout);
    if (!status_layout) return;
    status_layout->addColSpacing(1, 20);
    top_layout->addLayout(status_layout);

    // create the statistic labels
    m_stat_transfer = addInfoLabel(status_layout, "-", 1, 0);
    if (!m_stat_transfer) return;
    m_stat_bytes = addInfoLabel(status_layout, "-", 1, 2);
    if (!m_stat_bytes) return;

    // some dummy update to get maximum size
    updateStatistics(9999999.9, (double)(99*24*60*60), size);

    // now correct the minimum sizes of the statistic entries
    m_stat_transfer->adjustSize();
    m_stat_transfer->setMinimumWidth(m_stat_transfer->sizeHint().width());
    m_stat_bytes->adjustSize();
    m_stat_bytes->setMinimumWidth(m_stat_bytes->sizeHint().width());

    // right lower edge: the "cancel" button
    KPushButton *bt_cancel = new KPushButton(this);
    ASSERT(bt_cancel);
    if (!bt_cancel) return;
    bt_cancel->setText(i18n("&Cancel"));
    bt_cancel->setFixedSize(bt_cancel->sizeHint());
    bt_cancel->setAccel(Key_Escape);
    bt_cancel->setFocus();
    connect(bt_cancel, SIGNAL(clicked()), this, SLOT(cancel()));
    top_layout->addWidget(bt_cancel, 0, AlignRight);

    // activate the layout and show the dialog
    top_layout->activate();
    setFixedHeight(sizeHint().height());
    setMinimumWidth(sizeHint().width());

    show();
    fitUrlLabel();

    // now set cancelled to false, this dialog is ready for use
    m_cancelled = false;
}

//***************************************************************************
void FileProgress::resizeEvent(QResizeEvent *)
{
    fitUrlLabel();
}

//***************************************************************************
void FileProgress::closeEvent(QCloseEvent *e)
{
    ASSERT(e);
    if (!e) return;

    // if not already cancelled -> ask user to confirm
    if (!m_cancelled) {
	if (KMessageBox::warningYesNo(this,
	    i18n("Do you really want to abort the operation?")) !=
	    KMessageBox::Yes)
	{
	    // the user was wise and said "No"
	    e->ignore();
	    return;
	} else {
	    // user pressed "Yes", he should know what he has done
	    m_cancelled = true;
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
    ASSERT(label);
    if (!label) return 0;

    label->setText(text);
    label->adjustSize();
    label->setFixedHeight(label->sizeHint().height());
    label->setMinimumWidth(label->sizeHint().width());
    label->setAutoMask(true);

    layout->addWidget(label, row, col);

    return label;
}

//***************************************************************************
void FileProgress::updateStatistics(double rate, double rest,
	unsigned int pos)
{
    QString text;
    QString num;

    if (!m_stat_transfer) return;
    if (!m_stat_bytes) return;

    // left: transfer rate and estimated time
    num = num.sprintf("%1.1f", rate/1024.0);
    text = i18n("%1 KB/s (%2 remain)");
    text = text.arg(num);

    int h =  (int)floor(rest) / (60*60);
    int m = ((int)floor(rest) / 60) % 60;
    int s =  (int)floor(rest) % 60;
    if (h > 23) {
	h = 23;
	m = s = 59;
    }
    QTime time(h,m,s,0);
    text = text.arg(time.toString());
    m_stat_transfer->setText(text);

    // right: statistic over the transferred bytes
    text = i18n("%1 MB of %2 MB done");
    num = num.sprintf("%1.1f", pos / (1024.0*1024.0));
    text = text.arg(num);
    num = num.sprintf("%1.1f", m_size / (1024.0*1024.0));
    text = text.arg(num);
    m_stat_bytes->setText(text);
}

//***************************************************************************
void FileProgress::setValue(unsigned int pos)
{
    if (!m_progress) return;

    // the easiest part: the progress bar and the caption
    int percent = (int)((double)pos / (double)m_size * 100.0);

    // not enough progress not worth showing ?
    if (percent <= m_last_percent) return;
    m_last_percent = percent;

    if (m_progress->value() != percent) {
	QString newcap;
	newcap = i18n("(%1%) %2");
	newcap = newcap.arg(percent);
	newcap = newcap.arg(m_url.toString());
	setCaption(newcap);

	m_progress->setValue(percent);
    }

    // update the transfer statistics
    double seconds = m_time.elapsed() / 1000.0; // [sec]
    double rate = pos / seconds;                // [bytes/sec]
    double rest = 0;
    if (rate > 10) {
	rest = double(m_size - pos) / rate;     // [seconds]
    }
    updateStatistics(rate, rest, pos);

    // as this dialog is modal, we must take care of our events
    // manually !
    qApp->processEvents();

    // better be nice to other processes and let them also play a bit :)
    sched_yield();
}

//***************************************************************************
void FileProgress::cancel()
{
    close();
    if (m_cancelled) emit cancelled();
}

//***************************************************************************
//***************************************************************************
