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

#include "config.h"
#include <stdlib.h> // for abs()

#include <qcheckbox.h>
#include <qdatetime.h>
#include <qdialog.h>
#include <qfileinfo.h>
#include <qarray.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qslider.h>
#include <qspinbox.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qtabwidget.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

#include <kcombobox.h>
#include <kdatewidget.h>
#include <klistbox.h>
#include <kmimetype.h>
#include <knuminput.h>

#include "libkwave/CompressionType.h"
#include "libkwave/FileInfo.h"
#include "libkwave/KwavePlugin.h"
#include "libkwave/SampleFormat.h"

#include "BitrateWidget.h"
#include "FileInfoDialog.h"
#include "KeywordWidget.h"
#include "SelectDateDialog.h"

//***************************************************************************
FileInfoDialog::FileInfoDialog(QWidget *parent, FileInfo &info)
    :FileInfoDlg(parent), m_info(info)
{
    QString mimetype = QVariant(m_info.get(INF_MIMETYPE)).toString();
    if (!mimetype.length()) mimetype = "audio/x-wav"; // default mimetype
    
    m_is_mpeg = ((mimetype == "audio/x-mpga") ||
        (mimetype == "audio/x-mp2") || (mimetype == "audio/x-mp3") ||
        (mimetype == "audio/mpeg"));
    m_is_ogg = ((mimetype == "audio/x-ogg") ||
                (mimetype == "application/x-ogg") ||
                (mimetype == "application/ogg"));

    debug("mimetype = %s",mimetype.data());
   
    setupFileInfoTab();
    setupCompressionTab();
    setupMpegTab();
    setupContentTab();
    setupSourceTab();
    setupAuthorCopyrightTab();
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

    /* file format (from mime type) */

/** @todo using description instead of bare mime type would
          be fine, but doesn't work yet. Currently it gives
          only "Unknown" :-(
*/
//    QString mimetype = QVariant(m_info.get(INF_MIMETYPE)).toString();
//    lblFileFormat->setText(i18n("File Format")+":");
//    describeWidget(edFileFormat, lblFileFormat->text().left(
//        lblFileFormat->text().length()-1),
//        i18n("Format of the file from which the\n"
//             "audio data was loaded from"));
//    debug("mimetype='%s'",mimetype.data()); // ###
//    KMimeType::Ptr mime = KMimeType::mimeType(mimetype);
//    QString format =
//       (mime != KMimeType::mimeType(KMimeType::defaultMimeType())) ?
//        mime->comment() : mimetype;
//    debug("comment='%s'",mime->comment().data()); // ###
//    edFileFormat->setText(format);

    // use mimetype instead
    initInfoText(lblFileFormat,   edFileFormat,   INF_MIMETYPE);

    /* sample rate */
    lblSampleRate->setText(i18n("Sample Rate")+":");
    describeWidget(cbSampleRate, lblSampleRate->text().left(
        lblSampleRate->text().length()-1),
        i18n("Here you can select one of the predefined\n"
             "well-known sample rates or you can enter\n"
             "any sample rate on your own."));
    cbSampleRate->setCurrentText(QString::number(m_info.rate()));

    /* bits per sample */
    lblResolution->setText(i18n("Resolution")+":");
    describeWidget(sbResolution, lblResolution->text().left(
        lblResolution->text().length()-1),
        i18n("Select a resolution in bits in which the file\n"
             "will be saved."));
    sbResolution->setValue(m_info.bits());

    /* number of tracks */
    lblTracks->setText(i18n("Tracks")+":");
    describeWidget(sbTracks, lblTracks->text().left(
        lblTracks->text().length()-1),
        i18n("Shows the number of tracks of the signal.\n"
             "You can add or delete tracks via the Edit menu."));
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
        i18n("Shows the length of the file in samples\n"
             "and if possible as time."));
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
    if (m_is_mpeg || m_is_ogg) cbSampleFormat->setEnabled(false);

}

//***************************************************************************
void FileInfoDialog::setupCompressionTab()
{

    /* compression */
    CompressionType compressions;
    initInfo(lblCompression, cbCompression, INF_COMPRESSION);
    cbCompression->insertStringList(compressions.allNames());
    int compression = QVariant(m_info.get(INF_COMPRESSION)).toInt();
    cbCompression->setCurrentItem(compressions.findFromData(compression));
    if (m_is_mpeg || m_is_ogg) cbCompression->setEnabled(false);

    if (m_is_mpeg) {
	// MPEG file -> not supported yet
	rbCompressionABR->setEnabled(false);
	rbCompressionVBR->setEnabled(false);
	abrBitrate->setEnabled(false);
	lblCompressionNominalBitrate->setEnabled(false);
    } else if (m_is_ogg) {
	// Ogg/Vorbis file
	rbCompressionVBR->setEnabled(false); // not supported yet
	rbCompressionABR->setEnabled(true);
	abrBitrate->setEnabled(true);
	lblCompressionNominalBitrate->setEnabled(true);
    } else {
	// other...    
	rbCompressionABR->setEnabled(false);
	rbCompressionVBR->setEnabled(false);
	abrBitrate->setEnabled(false);
	lblCompressionNominalBitrate->setEnabled(false);
    }

    // use well-known bitrates from MP3
    QValueList<int> rates;
    rates.append(  8000);
    rates.append( 16000);
    rates.append( 24000);
    rates.append( 32000);
    rates.append( 40000);
    rates.append( 56000);
    rates.append( 64000);
    rates.append( 80000);
    rates.append( 96000);
    rates.append(112000);
    rates.append(128000);
    rates.append(144000);
    rates.append(160000);
    rates.append(176000);
    rates.append(192000);
    rates.append(224000);
    rates.append(256000);
    rates.append(288000);
    rates.append(320000);
    rates.append(352000);
    rates.append(384000);
    rates.append(416000);
    rates.append(448000);
    abrBitrate->allowRates(rates);

    int bitrate = m_info.contains(INF_BITRATE_NOMINAL) ?
                  QVariant(m_info.get(INF_BITRATE_NOMINAL)).toInt() : 64000;
    abrBitrate->setValue(bitrate);
    abrHighestBitrate->setSpecialValueText(i18n("no limit"));
    abrLowestBitrate->setSpecialValueText(i18n("no limit"));

    connect(rbCompressionABR, SIGNAL(toggled(bool)),
            this, SLOT(compressionSelectABR(bool)));
    
//    // this is not visible, not implemented yet...
//    InfoTab->setCurrentPage(5);
//    QWidget *page = InfoTab->currentPage();
//    InfoTab->removePage(page);
//    InfoTab->setCurrentPage(0);
//    return;
}

//***************************************************************************
//void FileInfoDialog::sampleRateChanged(int)
//{
//    int rate = (int)m_info.rate();
//    switch (rate) {
//	 8000:
//	11025:
//	22050:
//	32000:
//	44100:
//    }
//}

//***************************************************************************
void FileInfoDialog::compressionSelectABR(bool checked)
{
    checked = false; // ### <- not implemented yet
    abrHighestBitrate->setEnabled(checked && chkHighestBitrate->isChecked());
    abrLowestBitrate->setEnabled( checked && chkLowestBitrate->isChecked());
}

//***************************************************************************
void FileInfoDialog::setupMpegTab()
{
    // the whole tab is only enabled in mpeg mode
    if (!m_is_mpeg) {
	InfoTab->setCurrentPage(2);
	QWidget *page = InfoTab->currentPage();
	InfoTab->setTabEnabled(page, false);
	InfoTab->setCurrentPage(0);
	return;
    }

    /* MPEG layer */
    initInfo(lblMpegLayer,   cbMpegLayer,    INF_MPEG_LAYER);
    int layer = m_is_mpeg ? QVariant(m_info.get(INF_MPEG_LAYER)).toInt() : 0;
    if (layer <= 0) {
	cbMpegLayer->setEditable(true);
	cbMpegLayer->clearEdit();
	cbMpegLayer->setEnabled(false);
    } else cbMpegLayer->setCurrentItem(layer-1);

    /* MPEG version */
    initInfo(lblMpegVersion, cbMpegVersion,  INF_MPEG_VERSION);
    int ver = m_is_mpeg ?
        (int)(2.0 * QVariant(m_info.get(INF_MPEG_VERSION)).toDouble()) : 0;
    // 0, 1, 2, 2.5 -> 0, 2, 4, 5
    if (ver > 3) ver++; // 0, 2, 4, 6
    ver >>= 1; // 0, 1, 2, 3
    ver--; // -1, 0, 1, 2
    if (ver < 0) {
	cbMpegVersion->setEditable(true);
	cbMpegVersion->clearEdit();
	cbMpegVersion->setEnabled(false);
    } else cbMpegVersion->setCurrentItem(ver);

    /* Bitrate in bits/s */
    initInfo(lblMpegBitrate, cbMpegBitrate,  INF_BITRATE_NOMINAL);
    int bitrate = QVariant(m_info.get(INF_BITRATE_NOMINAL)).toInt();
    if (bitrate) {
	QString s;
	s.setNum(bitrate / 1000);
	s += "K";
	s = i18n(s);
	QListBoxItem *item = cbMpegBitrate->listBox()->findItem(s);
	int index = cbMpegBitrate->listBox()->index(item);
	cbMpegBitrate->setCurrentItem(index);
    }

    /* Mode extension */
    initInfo(lblMpegModeExt, cbMpegModeExt, INF_MPEG_MODEEXT);
    // only in "Joint Stereo" mode, then depends on Layer
    //
    // Layer I+II          |  Layer III
    //                     |  Intensity stereo MS Stereo
    //--------------------------------------------------
    // 0 - bands  4 to 31  |  off              off  -> 4
    // 1 - bands  8 to 31  |  on               off  -> 5
    // 2 - bands 12 to 31  |  off              on   -> 6
    // 3 - bands 16 to 31  |  on               on   -> 7
    int modeext = QVariant(m_info.get(INF_MPEG_MODEEXT)).toInt();
    if ((modeext >= 0) && (modeext <= 3)) {
	cbMpegModeExt->insertItem(i18n("bands 0 to 31"));
	cbMpegModeExt->insertItem(i18n("bands 8 to 31"));
	cbMpegModeExt->insertItem(i18n("bands 12 to 31"));
	cbMpegModeExt->insertItem(i18n("bands 16 to 31"));
	cbMpegModeExt->setCurrentItem(modeext);
	cbMpegIntensityStereo->setEnabled(false);
	cbMpegMSStereo->setEnabled(false);
    } else if ((modeext >= 4) && (modeext <= 7)) {
	cbMpegModeExt->setEnabled(false);
	cbMpegIntensityStereo->setChecked(modeext & 0x01);
	cbMpegMSStereo->setChecked(modeext & 0x02);
    } else {
	cbMpegModeExt->setEnabled(false);
	cbMpegIntensityStereo->setEnabled(false);
	cbMpegMSStereo->setEnabled(false);
    }

    /* Emphasis */
    initInfo(lblMpegEmphasis, cbMpegEmphasis, INF_MPEG_EMPHASIS);
    int emphasis = QVariant(m_info.get(INF_MPEG_EMPHASIS)).toInt();
    switch (emphasis) {
	case 0: cbMpegEmphasis->setCurrentItem(0); break;
	case 1: cbMpegEmphasis->setCurrentItem(1); break;
	case 3: cbMpegEmphasis->setCurrentItem(2); break;
	default: cbMpegEmphasis->setEnabled(false);
    }

    /* Copyrighted */
    initInfo(lblMpegCopyrighted, chkMpegCopyrighted, INF_COPYRIGHTED);
    bool copyrighted = QVariant(m_info.get(INF_COPYRIGHTED)).toBool();
    chkMpegCopyrighted->setChecked(copyrighted);
    chkMpegCopyrighted->setText((copyrighted) ? i18n("yes") : i18n("no"));

    /* Original */
    initInfo(lblMpegOriginal, chkMpegOriginal, INF_ORIGINAL);
    bool original = QVariant(m_info.get(INF_ORIGINAL)).toBool();
    chkMpegOriginal->setChecked(original);
    chkMpegOriginal->setText((original) ? i18n("yes") : i18n("no"));

}

//***************************************************************************
void FileInfoDialog::setupContentTab()
{
    /* name, subject, version, genre, title, author, organization,
       copyright, license */
    initInfoText(lblName,         edName,         INF_NAME);
    initInfoText(lblSubject,      edSubject,      INF_SUBJECT);
    initInfoText(lblVersion,      edVersion,      INF_VERSION);
    initInfoText(lblGenre,        edGenre,        INF_GENRE);

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
    /* source, source form */
    initInfoText(lblSource,     edSource,     INF_SOURCE);
    initInfoText(lblSourceForm, edSourceForm, INF_SOURCE_FORM);

    /* Album, CD and track */
    initInfoText(lblAlbum,      edAlbum,      INF_ALBUM);
    initInfo(lblCD, sbCD, INF_CD);
    int cd = (m_info.contains(INF_CD)) ?
	QVariant(m_info.get(INF_CD)).toInt() : 0;
    sbCD->setValue(cd);

    initInfo(lblTrack, sbTrack, INF_TRACK);
    int track = (m_info.contains(INF_TRACK)) ?
	QVariant(m_info.get(INF_TRACK)).toInt() : 0;
    sbTrack->setValue(track);

    
    /* software, engineer, technican */
    initInfoText(lblSoftware,     edSoftware,     INF_SOFTWARE);
    initInfoText(lblEngineer,     edEngineer,     INF_ENGINEER);
    initInfoText(lblTechnican,    edTechnican,    INF_TECHNICAN);

}

//***************************************************************************
void FileInfoDialog::setupAuthorCopyrightTab()
{
    /* author organization, copyright, license, ISRC */
    initInfoText(lblAuthor,       edAuthor,       INF_AUTHOR);
    initInfoText(lblOrganization, edOrganization, INF_ORGANIZATION);
    initInfoText(lblCopyright,    edCopyright,    INF_COPYRIGHT);
    initInfoText(lblLicense,      edLicense,      INF_LICENSE);
    initInfoText(lblISRC,         edISRC,         INF_ISRC);

    /* product, archival, contact */
    initInfoText(lblProduct,  edProduct,  INF_PRODUCT);
    initInfoText(lblArchival, edArchival, INF_ARCHIVAL);
    initInfoText(lblContact,  edContact,  INF_CONTACT);
}

//***************************************************************************
void FileInfoDialog::setupMiscellaneousTab()
{
    /* commissioned */
    initInfoText(lblCommissioned, edCommissioned, INF_COMMISSIONED);

    /* list of keywords */
    lblKeywords->setText(m_info.name(INF_KEYWORDS));
    QWhatsThis::add(lstKeywords, "<b>"+m_info.name(INF_KEYWORDS)+
        "</b><br>"+m_info.description(INF_KEYWORDS));
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

    // name, subject, version, genre, author, organization,
    // copyright, license, source, source form, album,
    // product, archival, contact, software, technican, engineer,
    // commissioned, ISRC
    list += QStringList::split(" ", edName->text());
    list += QStringList::split(" ", edSubject->text());
    list += QStringList::split(" ", edVersion->text());
    list += QStringList::split(" ", edGenre->text());
    list += QStringList::split(" ", edAuthor->text());
    list += QStringList::split(" ", edOrganization->text());
    list += QStringList::split(" ", edCopyright->text());
    list += QStringList::split(" ", edLicense->text());
    list += QStringList::split(" ", edSource->text());
    list += QStringList::split(" ", edSourceForm->text());
    list += QStringList::split(" ", edAlbum->text());
    list += QStringList::split(" ", edProduct->text());
    list += QStringList::split(" ", edArchival->text());
    list += QStringList::split(" ", edContact->text());
    list += QStringList::split(" ", edSoftware->text());
    list += QStringList::split(" ", edTechnican->text());
    list += QStringList::split(" ", edEngineer->text());
    list += QStringList::split(" ", edCommissioned->text());
    list += QStringList::split(" ", edISRC->text());

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
    if (m_info.contains(INF_SAMPLE_FORMAT) ||
        (sample_format != sample_formats.data(0)))
    {
	m_info.set(INF_SAMPLE_FORMAT, QVariant(sample_format));
    }
    
    /* compression */
    CompressionType compressions;
    int compression = compressions.data(cbCompression->currentItem());
    m_info.set(INF_COMPRESSION, (compression != AF_COMPRESSION_NONE) ?
        QVariant(compression) : QVariant());

    /* bitrate in Ogg/Vorbis mode */
    if (m_is_ogg) {
	m_info.set(INF_BITRATE_NOMINAL, QVariant(abrBitrate->value()));
    }
    
    /* name, subject, version, genre, title, author, organization,
       copyright */
    acceptEdit(INF_NAME,         edName->text());
    acceptEdit(INF_SUBJECT,      edSubject->text());
    acceptEdit(INF_VERSION,      edVersion->text());
    acceptEdit(INF_GENRE,        edGenre->text());
    acceptEdit(INF_AUTHOR,       edAuthor->text());
    acceptEdit(INF_ORGANIZATION, edOrganization->text());
    acceptEdit(INF_COPYRIGHT,    edCopyright->text());
    acceptEdit(INF_LICENSE,      edLicense->text());

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

    /* product, archival, contact */
    acceptEdit(INF_PRODUCT,     edProduct->text());
    acceptEdit(INF_ARCHIVAL,    edArchival->text());
    acceptEdit(INF_CONTACT,     edContact->text());

    /* software, engineer, technican, commissioned, ISRC, keywords */
    acceptEdit(INF_SOFTWARE,    edSoftware->text());
    acceptEdit(INF_ENGINEER,    edEngineer->text());
    acceptEdit(INF_TECHNICAN,   edTechnican->text());
    acceptEdit(INF_COMMISSIONED,edCommissioned->text());
//  acceptEdit(INF_ISRC,        edISRC->text()); <- READ-ONLY

    // list of keywords
    acceptEdit(INF_KEYWORDS,    lstKeywords->keywords().join("; "));

    debug("FileInfoDialog::accept() --2--");
    m_info.dump();

    QDialog::accept();
}

//***************************************************************************
//***************************************************************************
