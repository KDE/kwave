/***************************************************************************
     FileInfoDialog.cpp  -  dialog for editing file properties
                             -------------------
    begin                : Sat Jul 20 2002
    copyright            : (C) 2002 by Thomas Eschenbacher
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

#include <qfileinfo.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qstring.h>

#include "libkwave/FileInfo.h"
#include "libgui/KwavePlugin.h"
#include "FileInfoDialog.h"

//***************************************************************************
FileInfoDialog::FileInfoDialog(QWidget *parent, FileInfo &info)
    :FileInfoDlg(parent), m_info(info)
{
    /* filename */
    QFileInfo fi(QVariant(m_info.get(INF_FILENAME)).asString());
    edFileName->setText(fi.fileName());
    edFileName->setEnabled(fi.fileName().length() != 0);

    /* file size in bytes */
    if (m_info.contains(INF_FILESIZE)) {
	unsigned int size = QVariant(m_info.get(INF_FILESIZE)).asUInt();
	QString dotted = KwavePlugin::dottedNumber(size);
	if (size < 10*1024) {
	    edFileSize->setText(i18n("%1 bytes").arg(dotted));
	} else if (size < 10*1024*1024) {
	    edFileSize->setText(i18n("%1 kB (%2 byte)").arg(
		QString::number(size / 1024)).arg(dotted));
	} else {
	    edFileSize->setText(i18n("%1 MB (%2 byte)").arg(
		QString::number(size / (1024*1024))).arg(dotted));
	}
    } else {
	edFileSize->setEnabled(false);
    }

    /* sample rate, bits per sample */
    cbSampleRate->setCurrentText(QString::number(m_info.rate()));
    sbResolution->setValue(m_info.bits());

    /* number of tracks */
    sbTracks->setMaxValue(m_info.tracks());
    sbTracks->setMinValue(m_info.tracks());
    sbTracks->setValue(m_info.tracks());
    connect(sbTracks, SIGNAL(valueChanged(int)),
            this, SLOT(tracksChanged(int)));
    tracksChanged(sbTracks->value());

    /* length of the signal */
    unsigned int samples = m_info.length();
    double rate = m_info.rate();
    if (rate != 0) {
	double ms = (double)samples * 1E3 / rate;
	txtLength->setText(i18n("%1 (%2 samples)").arg(
	    KwavePlugin::ms2string(ms)).arg(
	    KwavePlugin::dottedNumber(samples)));
    } else {
	txtLength->setText(i18n("%2 samples").arg(
	    KwavePlugin::dottedNumber(samples)));
    }
}

//***************************************************************************
FileInfoDialog::~FileInfoDialog()
{
}

//***************************************************************************
void FileInfoDialog::tracksChanged(int tracks)
{
    switch (tracks) {
	case 1:
	    lblTracksVerbose->setText(i18n("(Mono)"));
	    break;
	case 2:
	    lblTracksVerbose->setText(i18n("(Stereo)"));
	    break;
	case 4:
	    lblTracksVerbose->setText(i18n("(Quadro)"));
	    break;
	default:
	    lblTracksVerbose->setText("");
	    break;
    }
}

//***************************************************************************
void FileInfoDialog::accept()
{
    debug("FileInfoDialog::accept()");

    /* bits per sample */
    m_info.setBits(sbResolution->value());

    /* sample rate */
    m_info.setRate(cbSampleRate->currentText().toDouble());

    QDialog::accept();
}

//***************************************************************************
//***************************************************************************
