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
#include <qtooltip.h>
#include <qwhatsthis.h>

#include "libkwave/FileInfo.h"
#include "libkwave/CompressionType.h"
#include "libkwave/SampleFormat.h"
#include "libgui/KwavePlugin.h"
#include "FileInfoDialog.h"

//***************************************************************************
FileInfoDialog::FileInfoDialog(QWidget *parent, FileInfo &info)
    :FileInfoDlg(parent), m_info(info)
{
    setupFileInfo();
}

//***************************************************************************
FileInfoDialog::~FileInfoDialog()
{
}

//***************************************************************************
void FileInfoDialog::describeWidget(QWidget *widget, const QString &name,
                                    const QString &description)
{
    if (!widget) return;
    QToolTip::add(widget, description);
    QWhatsThis::add(widget, "<b>"+name+"</b><br>"+description);
}

//***************************************************************************
void FileInfoDialog::initInfo(QLabel *label, QWidget *widget,
                              FileProperty property)
{
    ASSERT(label);
    ASSERT(widget);
    if (label) label->setText(m_info.name(property) + ":");
    describeWidget(widget, m_info.name(property),
                   m_info.description(property));
}

//***************************************************************************
void FileInfoDialog::initInfoText(QLabel *label, QLineEdit *edit,
                                  FileProperty property)
{
    initInfo(label, edit, property);
    if (edit) edit->setText(QVariant(m_info.get(property)).asString());
}

//***************************************************************************
void FileInfoDialog::setupFileInfo()
{
    /* filename */
    initInfo(lblFileName, edFileName, INF_FILENAME);
    QFileInfo fi(QVariant(m_info.get(INF_FILENAME)).asString());
    edFileName->setText(fi.fileName());
    edFileName->setEnabled(fi.fileName().length() != 0);

    /* file size in bytes */
    initInfo(lblFileSize, edFileSize, INF_FILESIZE);
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

    /* sample rate */
    lblSampleRate->setText(i18n("Sample Rate")+":");
    describeWidget(cbSampleRate, lblSampleRate->text().left(
        lblSampleRate->text().length()-1),
        i18n("Here you can select one of "
        "the predefined well-known sample rates or you can enter "
        "any sample rate on your own."));
    cbSampleRate->setCurrentText(QString::number(m_info.rate()));

    /* bits per sample */
    lblResolution->setText(i18n("Resolution")+":");
    describeWidget(sbResolution, lblResolution->text().left(
        lblResolution->text().length()-1),
        i18n("Select a resolution in bits in which the file will be saved. "
        "However, Kwave always uses 24 bits as it's own internal resolution "
        "to give the best results when processing the audio data."));
    sbResolution->setValue(m_info.bits());

    /* number of tracks */
    lblTracks->setText(i18n("Tracks")+":");
    describeWidget(sbTracks, lblTracks->text().left(
        lblTracks->text().length()-1),
        i18n("Shows the number of tracks of the signal. You can add or "
        "delete tracks via the Edit menu."));
    sbTracks->setMaxValue(m_info.tracks());
    sbTracks->setMinValue(m_info.tracks());
    sbTracks->setValue(m_info.tracks());
    connect(sbTracks, SIGNAL(valueChanged(int)),
            this, SLOT(tracksChanged(int)));
    tracksChanged(sbTracks->value());

    /* length of the signal */
    lblLength->setText(i18n("Length")+":");
    describeWidget(txtLength, lblLength->text().left(
        lblLength->text().length()-1),
        i18n("Shows the length of the file in samples and if "
        "possible as time."));
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

    /* sample format */
    SampleFormat sample_formats;
    initInfo(lblSampleFormat, cbSampleFormat, INF_SAMPLE_FORMAT);
    cbSampleFormat->insertStringList(sample_formats.allNames());
    int sample_format = QVariant(m_info.get(INF_SAMPLE_FORMAT)).toInt();
    cbSampleFormat->setEditText(sample_formats.name(
	sample_formats.findFromData(sample_format)));

    /* compression */
    CompressionType compressions;
    initInfo(lblCompression, cbCompression, INF_COMPRESSION);
    cbCompression->insertStringList(compressions.allNames());
    int compression = QVariant(m_info.get(INF_COMPRESSION)).toInt();
    cbCompression->setEditText(compressions.name(
	compressions.findFromData(compression)));

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

    /* sample format */
    SampleFormat sample_formats;
    int sample_format = sample_formats.data(cbSampleFormat->currentItem());
    m_info.set(INF_SAMPLE_FORMAT, QVariant(sample_format));

    /* compression */
    CompressionType compressions;
    int compression = compressions.data(cbCompression->currentItem());
    m_info.set(INF_COMPRESSION, QVariant(compression));

    QDialog::accept();
}

//***************************************************************************
//***************************************************************************
