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

#include <qdatetime.h>
#include <qdialog.h>
#include <qfileinfo.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qstring.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <kdatewidget.h>

#include "libkwave/FileInfo.h"
#include "libkwave/CompressionType.h"
#include "libkwave/SampleFormat.h"
#include "libgui/KwavePlugin.h"

#include "FileInfoDialog.h"
#include "SelectDateDialog.h"

//***************************************************************************
FileInfoDialog::FileInfoDialog(QWidget *parent, FileInfo &info)
    :FileInfoDlg(parent), m_info(info)
{
    setupFileInfo();
    setupContent();
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
    cbSampleFormat->setCurrentItem(sample_formats.findFromData(sample_format));

    /* compression */
    CompressionType compressions;
    initInfo(lblCompression, cbCompression, INF_COMPRESSION);
    cbCompression->insertStringList(compressions.allNames());
    int compression = QVariant(m_info.get(INF_COMPRESSION)).toInt();
    cbCompression->setCurrentItem(compressions.findFromData(compression));

}

//***************************************************************************
void FileInfoDialog::setupContent()
{
    /* name, subject, genre, title, author, copyright */
    initInfoText(lblName,      edName,      INF_NAME);
    initInfoText(lblSubject,   edSubject,   INF_SUBJECT);
    initInfoText(lblGenre,     edGenre,     INF_GENRE);
    initInfoText(lblAuthor,    edAuthor,    INF_AUTHOR);
    initInfoText(lblCopyright, edCopyright, INF_COPYRIGHT);

    /* date widget */
    initInfo(lblDate, dateEdit, INF_CREATION_DATE);
    QDate date;
    date = (m_info.contains(INF_CREATION_DATE)) ?
        QDate::fromString(QVariant(m_info.get(INF_CREATION_DATE)).toString(),
        Qt::ISODate) : QDate::currentDate();
    dateEdit->setDate(date);
    connect(btSelectDate, SIGNAL(clicked()), this, SLOT(selectDate()));
    connect(btSelectDateNow, SIGNAL(clicked()), this, SLOT(setDateNow()));
}

//***************************************************************************
void FileInfoDialog::selectDate()
{
    QDate date(dateEdit->date());
    SelectDateDialog date_dialog(this, date);
    if (date_dialog.exec() == QDialog::Accepted) {
	date = date_dialog.date();
	dateEdit->setDate(date);
    }
}

//***************************************************************************
void FileInfoDialog::setDateNow()
{
    dateEdit->setDate(QDate::currentDate());
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
void FileInfoDialog::acceptEdit(FileProperty property, QString value)
{
    value.simplifyWhiteSpace();
    if (!m_info.contains(property) && !value.length()) return;

    if (!value.length()) {
	m_info.set(property, 0);
    } else {
	m_info.set(property, value);
    }
}

//***************************************************************************
void FileInfoDialog::accept()
{
    debug("FileInfoDialog::accept()");
    m_info.dump();

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
    m_info.set(INF_COMPRESSION, (compression != AF_COMPRESSION_NONE) ?
        QVariant(compression) : 0);

    /* name, subject, genre, title, author, copyright */
    acceptEdit(INF_NAME,      edName->text());
    acceptEdit(INF_GENRE,     edGenre->text());
    acceptEdit(INF_AUTHOR,    edAuthor->text());
    acceptEdit(INF_COPYRIGHT, edCopyright->text());

    /* date */
    QDate date = dateEdit->date();
    if ((date != QDate::currentDate()) || m_info.contains(INF_CREATION_DATE))
	m_info.set(INF_CREATION_DATE, QVariant(date).asString());

    debug("FileInfoDialog::accept() --2--");
    m_info.dump();

    QDialog::accept();
}

//***************************************************************************
//***************************************************************************
