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
#include <qstringlist.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

#include <kdatewidget.h>
#include <klistbox.h>
#include <knuminput.h>

#include "libkwave/FileInfo.h"
#include "libkwave/CompressionType.h"
#include "libkwave/SampleFormat.h"
#include "libgui/KwavePlugin.h"

#include "FileInfoDialog.h"
#include "KeywordWidget.h"
#include "SelectDateDialog.h"

//***************************************************************************
FileInfoDialog::FileInfoDialog(QWidget *parent, FileInfo &info)
    :FileInfoDlg(parent), m_info(info)
{
    setupFileInfoTab();
    setupContentTab();
    setupSourceTab();
    setupMiscellaneousTab();
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
void FileInfoDialog::setupFileInfoTab()
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
void FileInfoDialog::setupContentTab()
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
void FileInfoDialog::setupSourceTab()
{
    /* source, source form, album */
    initInfoText(lblSource,     edSource,     INF_SOURCE);
    initInfoText(lblSourceForm, edSourceForm, INF_SOURCE_FORM);
    initInfoText(lblAlbum,      edAlbum,      INF_ALBUM);

    /* CD and track */
    initInfo(lblCD, sbCD, INF_CD);
    int cd = (m_info.contains(INF_CD)) ?
	QVariant(m_info.get(INF_CD)).toInt() : 0;
    sbCD->setValue(cd);

    initInfo(lblTrack, sbTrack, INF_TRACK);
    int track = (m_info.contains(INF_TRACK)) ?
	QVariant(m_info.get(INF_TRACK)).toInt() : 0;
    sbTrack->setValue(track);

    /* product and archival */
    initInfoText(lblProduct,  edProduct,  INF_PRODUCT);
    initInfoText(lblArchival, edArchival, INF_ARCHIVAL);

}

//***************************************************************************
void FileInfoDialog::setupMiscellaneousTab()
{
    /* software, engineer, technican, commissioned, keywords */
    initInfoText(lblSoftware,     edSoftware,     INF_SOFTWARE);
    initInfoText(lblEngineer,     edEngineer,     INF_ENGINEER);
    initInfoText(lblTechnican,    edTechnican,    INF_TECHNICAN);
    initInfoText(lblCommissioned, edCommissioned, INF_COMMISSIONED);

    /* list of keywords */
    initInfo(lblKeywords, lstKeywords, INF_KEYWORDS);
    if (m_info.contains(INF_KEYWORDS)) {
	QString keywords = QVariant(m_info.get(INF_KEYWORDS)).toString();
	lstKeywords->setKeywords(QStringList::split(";", keywords));
    }
    connect(lstKeywords, SIGNAL(autoGenerate()),
            this, SLOT(autoGenerateKeywords()));

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
void FileInfoDialog::autoGenerateKeywords()
{
    // start with the current list
    QStringList list = lstKeywords->keywords();

    // name, subject, genre, author, copyright, source, source form, album
    // product, archival, software, technican, engineer, commissioned,
    list += QStringList::split(" ", edName->text());
    list += QStringList::split(" ", edSubject->text());
    list += QStringList::split(" ", edGenre->text());
    list += QStringList::split(" ", edAuthor->text());
    list += QStringList::split(" ", edCopyright->text());
    list += QStringList::split(" ", edSource->text());
    list += QStringList::split(" ", edSourceForm->text());
    list += QStringList::split(" ", edAlbum->text());
    list += QStringList::split(" ", edProduct->text());
    list += QStringList::split(" ", edArchival->text());
    list += QStringList::split(" ", edSoftware->text());
    list += QStringList::split(" ", edTechnican->text());
    list += QStringList::split(" ", edEngineer->text());
    list += QStringList::split(" ", edCommissioned->text());

    // filter out all useless stuff
    for (QStringList::Iterator it = list.begin(); it != list.end(); ++it) {
	QString token = *it;

	// remove punktation characters like '.', ',', '!' from start and end
	while (token.length()) {
	    QChar c = token[token.length()-1];
	    if (c.isPunct() || c.isMark() || c.isSpace())
		token = token.left(token.length()-1);
	    c = token[0];
	    if (c.isPunct() || c.isMark() || c.isSpace())
		token = token.right(token.length()-1);
	    if ((*it) == token) break;
	    *it = token;
	}

	// remove empty entries
	if (!token.length()) {
	    list.remove(it);
	    it = list.begin();
	    continue;
	}
	
	// remove simple numbers and too short stuff
	bool ok;
	token.toInt(&ok);
	if ((ok) || (token.length() < 3)) {
	    list.remove(it); // number or less than 3 characters -> remove
	    it = list.begin();
	    continue;
	}

	// remove duplicates that differ in case
	QStringList::Iterator it2;
	for (it2 = list.begin(); it2 != list.end(); ++it2) {
	    if (it2 == it) continue;
	    if ((*it2).lower() == token.lower()) {
		// take the one with less uppercase characters
		unsigned int upper1 = 0;
		unsigned int upper2 = 0;
		unsigned int i;
		for (i=0; i < token.length(); ++i)
		    if (token[i].category() == QChar::Letter_Uppercase)
			upper1++;
		for (i=0; i < (*it2).length(); ++i)
		    if ((*it2)[i].category() == QChar::Letter_Uppercase)
			upper2++;
		if (upper2 < upper1) (*it) = (*it2);
		list.remove(it2);
		it2 = list.begin();
	    }
	}
    }
    // other stuff like empty strings and duplicates are handled in
    // the list itself, we don't need to take care of that here :)

    lstKeywords->setKeywords(list);
}

//***************************************************************************
void FileInfoDialog::acceptEdit(FileProperty property, QString value)
{
    value.simplifyWhiteSpace();
    if (!m_info.contains(property) && !value.length()) return;

    if (!value.length()) {
	m_info.set(property, QVariant());
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
        QVariant(compression) : QVariant());

    /* name, subject, genre, title, author, copyright */
    acceptEdit(INF_NAME,      edName->text());
    acceptEdit(INF_SUBJECT,   edSubject->text());
    acceptEdit(INF_GENRE,     edGenre->text());
    acceptEdit(INF_AUTHOR,    edAuthor->text());
    acceptEdit(INF_COPYRIGHT, edCopyright->text());

    /* date */
    QDate date = dateEdit->date();
    if ((date != QDate::currentDate()) || m_info.contains(INF_CREATION_DATE))
	m_info.set(INF_CREATION_DATE, QVariant(date).asString());

    /* source, source form, album */
    acceptEdit(INF_SOURCE,      edSource->text());
    acceptEdit(INF_SOURCE_FORM, edSourceForm->text());
    acceptEdit(INF_ALBUM,       edAlbum->text());

    /* CD and track */
    int cd    = sbCD->value();
    int track = sbTrack->value();
    m_info.set(INF_CD,    (cd    != 0) ? QVariant(cd)    : QVariant());
    m_info.set(INF_TRACK, (track != 0) ? QVariant(track) : QVariant());

    /* product and archival */
    acceptEdit(INF_PRODUCT,     edProduct->text());
    acceptEdit(INF_ARCHIVAL,    edArchival->text());

    /* software, engineer, technican, commissioned, keywords */
    acceptEdit(INF_SOFTWARE,    edSoftware->text());
    acceptEdit(INF_ENGINEER,    edEngineer->text());
    acceptEdit(INF_TECHNICAN,   edTechnican->text());
    acceptEdit(INF_COMMISSIONED,edCommissioned->text());

    // list of keywords
    acceptEdit(INF_KEYWORDS,    lstKeywords->keywords().join("; "));

    debug("FileInfoDialog::accept() --2--");
    m_info.dump();

    QDialog::accept();
}

//***************************************************************************
//***************************************************************************
