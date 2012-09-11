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

#include <QtGlobal>
#include <QCheckBox>
#include <QDateTime>
#include <QDialog>
#include <QFileInfo>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QComboBox>
#include <QPushButton>
#include <QRadioButton>
#include <QSlider>
#include <QSpinBox>
#include <QString>
#include <QStringList>
#include <QTabWidget>
#include <QToolTip>
#include <QVector>
#include <QWhatsThis>

#include <kglobal.h>
#include <kconfig.h>
#include <kcombobox.h>
#include <kdatewidget.h>
#include <klocale.h>
#include <kmimetype.h>
#include <knuminput.h>
#include <ktoolinvocation.h>

#include "libkwave/CompressionType.h"
#include "libkwave/FileInfo.h"
#include "libkwave/SampleFormat.h"
#include "libkwave/Utils.h"

#include "BitrateWidget.h"
#include "CompressionWidget.h"
#include "FileInfoDialog.h"
#include "KeywordWidget.h"
#include "SelectDateDialog.h"

/** section in the config file for storing default settings */
#define CONFIG_DEFAULT_SECTION "plugin fileinfo - setup dialog"

//***************************************************************************
FileInfoDialog::FileInfoDialog(QWidget *parent, FileInfo &info)
    :QDialog(parent), Ui::FileInfoDlg(), m_info(info)
{
    setupUi(this);

    QString mimetype = QVariant(m_info.get(INF_MIMETYPE)).toString();
    if (!mimetype.length()) mimetype = "audio/x-wav"; // default mimetype

    m_is_mpeg = ((mimetype == "audio/x-mpga") ||
        (mimetype == "audio/x-mp2") || (mimetype == "audio/x-mp3") ||
        (mimetype == "audio/mpeg"));
    m_is_ogg = ((mimetype == "audio/x-ogg") ||
                (mimetype == "application/x-ogg") ||
                (mimetype == "application/ogg"));

    qDebug("mimetype = %s",mimetype.toLocal8Bit().data());

    connect(btHelp, SIGNAL(clicked()),
            this,   SLOT(invokeHelp()));

    // open config for reading default settings
    KConfigGroup cfg = KGlobal::config()->group(CONFIG_DEFAULT_SECTION);

    setupFileInfoTab();
    setupCompressionTab(cfg);
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
    widget->setToolTip(description);
    widget->setWhatsThis("<b>"+name+"</b><br>"+description);
}

//***************************************************************************
void FileInfoDialog::initInfo(QLabel *label, QWidget *widget,
                              FileProperty property)
{
    Q_ASSERT(label);
    Q_ASSERT(widget);
    if (label) label->setText(i18n(m_info.name(property).toAscii()) + ":");
    describeWidget(widget, i18n(m_info.name(property).toAscii()),
                   m_info.description(property));
}

//***************************************************************************
void FileInfoDialog::initInfoText(QLabel *label, QLineEdit *edit,
                                  FileProperty property)
{
    initInfo(label, edit, property);
    if (edit) edit->setText(QVariant(m_info.get(property)).toString());
}

//***************************************************************************
void FileInfoDialog::setupFileInfoTab()
{
    /* filename */
    initInfo(lblFileName, edFileName, INF_FILENAME);
    QFileInfo fi(QVariant(m_info.get(INF_FILENAME)).toString());
    edFileName->setText(fi.fileName());
    edFileName->setEnabled(fi.fileName().length() != 0);

    /* file size in bytes */
    initInfo(lblFileSize, edFileSize, INF_FILESIZE);
    if (m_info.contains(INF_FILESIZE)) {
	unsigned int size = QVariant(m_info.get(INF_FILESIZE)).toUInt();
	QString dotted = Kwave::dottedNumber(size);
	if (size < 10*1024) {
	    edFileSize->setText(i18n("%1 bytes", dotted));
	} else if (size < 10*1024*1024) {
	    edFileSize->setText(i18n("%1 kB (%2 byte)",
		QString::number(size / 1024), dotted));
	} else {
	    edFileSize->setText(i18n("%1 MB (%2 byte)",
		QString::number(size / (1024*1024)), dotted));
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
//    lblFileFormat->setText(i18n("File format:");
//    describeWidget(edFileFormat, lblFileFormat->text().left(
//        lblFileFormat->text().length()-1),
//        i18n("Format of the file from which the\n"
//             "audio data was loaded from"));
//    qDebug("mimetype='%s'",mimetype.local8Bit().data()); // ###
//    KMimeType::Ptr mime = KMimeType::mimeType(mimetype);
//    QString format =
//       (mime != KMimeType::mimeType(KMimeType::defaultMimeType())) ?
//        mime->comment() : mimetype;
//    qDebug("comment='%s'",mime->comment().local8Bit().data()); // ###
//    edFileFormat->setText(format);

    // use mimetype instead
    initInfoText(lblFileFormat,   edFileFormat,   INF_MIMETYPE);

    /* sample rate */
    lblSampleRate->setText(i18n("Sample rate:"));
    describeWidget(cbSampleRate, lblSampleRate->text().left(
        lblSampleRate->text().length()-1),
        i18n("Here you can select one of the predefined\n"
             "well-known sample rates or you can enter\n"
             "any sample rate on your own."));
    cbSampleRate->setEditText(QString::number(m_info.rate()));

    /* bits per sample */
    lblResolution->setText(i18n("Resolution:"));
    describeWidget(sbResolution, lblResolution->text().left(
        lblResolution->text().length()-1),
        i18n("Select a resolution in bits in which the file\n"
             "will be saved."));
    sbResolution->setValue(m_info.bits());

    /* number of tracks */
    lblChannels->setText(i18n("Tracks:"));
    describeWidget(sbChannels, lblChannels->text().left(
        lblChannels->text().length()-1),
        i18n("Shows the number of tracks of the signal.\n"
             "You can add or delete tracks via the Edit menu."));
    sbChannels->setMaximum(m_info.tracks());
    sbChannels->setMinimum(m_info.tracks());
    sbChannels->setValue(m_info.tracks());
    connect(sbChannels, SIGNAL(valueChanged(int)),
            this, SLOT(tracksChanged(int)));
    tracksChanged(sbChannels->value());

    /* length of the signal */
    lblLength->setText(i18n("Length:"));
    describeWidget(txtLength, lblLength->text().left(
        lblLength->text().length() - 1),
        i18n("Shows the length of the file in samples\n"
             "and if possible as time."));
    unsigned int samples = m_info.length();
    double rate = m_info.rate();
    if (rate != 0) {
	double ms = static_cast<double>(samples) * 1E3 / rate;
	txtLength->setText(i18n("%1 (%2 samples)",
	    Kwave::ms2string(ms),
	    Kwave::dottedNumber(samples)));
    } else {
	txtLength->setText(i18n("%1 samples",
	    Kwave::dottedNumber(samples)));
    }

    /* sample format */
    SampleFormat::Map sf;
    initInfo(lblSampleFormat, cbSampleFormat, INF_SAMPLE_FORMAT);
    cbSampleFormat->insertItems(-1, sf.allNames());
    SampleFormat format;
    format.fromInt(QVariant(m_info.get(INF_SAMPLE_FORMAT)).toInt());
    cbSampleFormat->setCurrentIndex(sf.findFromData(format));
    if (m_is_mpeg || m_is_ogg) cbSampleFormat->setEnabled(false);

}

//***************************************************************************
void FileInfoDialog::setupCompressionTab(KConfigGroup &cfg)
{
    /* compression */
    CompressionType compressions;
    initInfo(lblCompression, cbCompression, INF_COMPRESSION);
    cbCompression->insertItems(-1, compressions.allNames());
    int compression = QVariant(m_info.get(INF_COMPRESSION)).toInt();
    cbCompression->setCurrentIndex(compressions.findFromData(compression));
    if (m_is_mpeg || m_is_ogg) cbCompression->setEnabled(false);


    // enable/disable ABR/VBR controls, depending on mime type
    if (m_is_mpeg) {
	// MPEG file -> not supported yet
	compressionWidget->enableABR(false, false, false);
	compressionWidget->enableVBR(false);
    } else if (m_is_ogg) {
	// Ogg/Vorbis file
	bool lower = m_info.contains(INF_BITRATE_LOWER);
	bool upper = m_info.contains(INF_BITRATE_UPPER);
	compressionWidget->enableABR(true, lower, upper);
	compressionWidget->enableVBR(true);
    } else {
	// other...
	compressionWidget->enableABR(false, false, false);
	compressionWidget->enableVBR(false);
    }

    // ABR bitrate settings
    int abr_bitrate = m_info.contains(INF_BITRATE_NOMINAL) ?
                  QVariant(m_info.get(INF_BITRATE_NOMINAL)).toInt() :
                  cfg.readEntry("default_abr_nominal_bitrate", -1);
    int min_bitrate = m_info.contains(INF_BITRATE_LOWER) ?
                  QVariant(m_info.get(INF_BITRATE_LOWER)).toInt() :
                  cfg.readEntry("default_abr_lower_bitrate",-1);
    int max_bitrate = m_info.contains(INF_BITRATE_UPPER) ?
                  QVariant(m_info.get(INF_BITRATE_UPPER)).toInt() :
                  cfg.readEntry("default_abr_upper_bitrate",-1);
    compressionWidget->setBitrates(abr_bitrate, min_bitrate, max_bitrate);

    // VBR base quality
    int quality = m_info.contains(INF_VBR_QUALITY) ?
              QVariant(m_info.get(INF_VBR_QUALITY)).toInt() :
              cfg.readEntry("default_vbr_quality", -1);
    compressionWidget->setQuality(quality);

    compressionWidget->init(m_info);
    compressionWidget->setMode(m_info.contains(INF_VBR_QUALITY) ?
        CompressionWidget::VBR_MODE : CompressionWidget::ABR_MODE);

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
void FileInfoDialog::setupMpegTab()
{
    // the whole tab is only enabled in mpeg mode
    if (!m_is_mpeg) {
	InfoTab->setTabEnabled(2, false);
	return;
    }

    /* MPEG layer */
    initInfo(lblMpegLayer,   cbMpegLayer,    INF_MPEG_LAYER);
    int layer = m_is_mpeg ? QVariant(m_info.get(INF_MPEG_LAYER)).toInt() : 0;
    if (layer <= 0) {
	cbMpegLayer->setEditable(true);
	cbMpegLayer->clearEditText();
	cbMpegLayer->setEnabled(false);
    } else cbMpegLayer->setCurrentIndex(layer-1);

    /* MPEG version */
    initInfo(lblMpegVersion, cbMpegVersion,  INF_MPEG_VERSION);
    int ver = m_is_mpeg ? static_cast<int>(
        (2.0 * QVariant(m_info.get(INF_MPEG_VERSION)).toDouble())) : 0;
    // 0, 1, 2, 2.5 -> 0, 2, 4, 5
    if (ver > 3) ver++; // 0, 2, 4, 6
    ver >>= 1; // 0, 1, 2, 3
    ver--; // -1, 0, 1, 2
    if (ver < 0) {
	cbMpegVersion->setEditable(true);
	cbMpegVersion->clearEditText();
	cbMpegVersion->setEnabled(false);
    } else cbMpegVersion->setCurrentIndex(ver);

    /* Bitrate in bits/s */
    initInfo(lblMpegBitrate, cbMpegBitrate,  INF_BITRATE_NOMINAL);
    int bitrate = QVariant(m_info.get(INF_BITRATE_NOMINAL)).toInt();
    if (bitrate) {
	QString s;
	s.setNum(bitrate / 1000);
	s += "K";
	s = i18n(s.toAscii());
	int index = cbMpegBitrate->findText(s);
	cbMpegBitrate->setCurrentIndex(index);
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
	cbMpegModeExt->addItem(i18n("Bands 0 to 31"));
	cbMpegModeExt->addItem(i18n("Bands 8 to 31"));
	cbMpegModeExt->addItem(i18n("Bands 12 to 31"));
	cbMpegModeExt->addItem(i18n("Bands 16 to 31"));
	cbMpegModeExt->setCurrentIndex(modeext);
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
    if ((emphasis >= 0) && (emphasis <= 2))
	cbMpegEmphasis->setCurrentIndex(emphasis);
    else
	cbMpegEmphasis->setEnabled(false);

    /* Copyrighted */
    initInfo(lblMpegCopyrighted, chkMpegCopyrighted, INF_COPYRIGHTED);
    bool copyrighted = QVariant(m_info.get(INF_COPYRIGHTED)).toBool();
    chkMpegCopyrighted->setChecked(copyrighted);
    chkMpegCopyrighted->setText((copyrighted) ? i18n("Yes") : i18n("No"));

    /* Original */
    initInfo(lblMpegOriginal, chkMpegOriginal, INF_ORIGINAL);
    bool original = QVariant(m_info.get(INF_ORIGINAL)).toBool();
    chkMpegOriginal->setChecked(original);
    chkMpegOriginal->setText((original) ? i18n("Yes") : i18n("No"));

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

    initInfo(0, sbCDs, INF_CDS);
    int cds = (m_info.contains(INF_CDS)) ?
	QVariant(m_info.get(INF_CDS)).toInt() : 0;
    sbCDs->setValue(cds);

    initInfo(lblTrack, sbTrack, INF_TRACK);
    int track = (m_info.contains(INF_TRACK)) ?
	QVariant(m_info.get(INF_TRACK)).toInt() : 0;
    sbTrack->setValue(track);

    initInfo(0, sbTracks, INF_TRACKS);
    int tracks = (m_info.contains(INF_TRACKS)) ?
	QVariant(m_info.get(INF_TRACKS)).toInt() : 0;
    sbTracks->setValue(tracks);


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
    lblKeywords->setText(i18n(m_info.name(INF_KEYWORDS).toAscii()));
    lstKeywords->setWhatsThis("<b>" +
	i18n(m_info.name(INF_KEYWORDS).toAscii()) +
	"</b><br>"+m_info.description(INF_KEYWORDS));
    if (m_info.contains(INF_KEYWORDS)) {
	QString keywords = QVariant(m_info.get(INF_KEYWORDS)).toString();
	lstKeywords->setKeywords(keywords.split(";"));
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
	    lblChannelsVerbose->setText(i18n("(Mono)"));
	    break;
	case 2:
	    lblChannelsVerbose->setText(i18n("(Stereo)"));
	    break;
	case 4:
	    lblChannelsVerbose->setText(i18n("(Quadro)"));
	    break;
	default:
	    lblChannelsVerbose->setText("");
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
    list += edName->text().split(" ");
    list += edSubject->text().split(" ");
    list += edVersion->text().split(" ");
    list += edGenre->text().split(" ");
    list += edAuthor->text().split(" ");
    list += edOrganization->text().split(" ");
    list += edCopyright->text().split(" ");
    list += edLicense->text().split(" ");
    list += edSource->text().split(" ");
    list += edSourceForm->text().split(" ");
    list += edAlbum->text().split(" ");
    list += edProduct->text().split(" ");
    list += edArchival->text().split(" ");
    list += edContact->text().split(" ");
    list += edSoftware->text().split(" ");
    list += edTechnican->text().split(" ");
    list += edEngineer->text().split(" ");
    list += edCommissioned->text().split(" ");
    list += edISRC->text().split(" ");

    // filter out all useless stuff
    QMutableStringListIterator it(list);
    while (it.hasNext()) {
	QString token = it.next();

	// remove punktation characters like '.', ',', '!' from start and end
	while (token.length()) {
	    QString old_value = token;

	    QChar c = token[token.length()-1];
	    if (c.isPunct() || c.isMark() || c.isSpace())
		token = token.left(token.length()-1);
	    if (!token.length()) break;

	    c = token[0];
	    if (c.isPunct() || c.isMark() || c.isSpace())
		token = token.right(token.length()-1);

	    if (token == old_value) break; // no (more) change(s)
	}
	it.setValue(token);

	// remove empty entries
	if (!token.length()) {
	    it.remove();
	    continue;
	}

	// remove simple numbers and too short stuff
	bool ok;
	token.toInt(&ok);
	if ((ok) || (token.length() < 3)) {
	    it.remove(); // number or less than 3 characters -> remove
	    continue;
	}

	// remove duplicates that differ in case
	QMutableStringListIterator it2(list);
	while (it2.hasNext()) {
	    QString token2 = it2.next();
	    if (list.indexOf(token) == list.lastIndexOf(token2)) continue;
	    if (token2.compare(token, Qt::CaseInsensitive) == 0) {
		// take the one with less uppercase characters
		unsigned int upper1 = 0;
		unsigned int upper2 = 0;
		for (int i=0; i < token.length(); ++i)
		    if (token[i].category() == QChar::Letter_Uppercase)
			upper1++;
		for (int i=0; i < token2.length(); ++i)
		    if (token2[i].category() == QChar::Letter_Uppercase)
			upper2++;
		if (upper2 < upper1) token = token2;
		it2.remove();
	    }
	}

	it.setValue(token);
    }
    // other stuff like empty strings and duplicates are handled in
    // the list itself, we don't need to take care of that here :)

    lstKeywords->setKeywords(list);
}

//***************************************************************************
void FileInfoDialog::acceptEdit(FileProperty property, QString value)
{
    value = value.simplified();
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
    // save defaults for next time...
    KConfigGroup cfg = KGlobal::config()->group(CONFIG_DEFAULT_SECTION);
    cfg.sync();
    {
	int nominal, upper, lower;
	compressionWidget->getABRrates(nominal, lower, upper);
	cfg.writeEntry("default_abr_nominal_bitrate", nominal);
	cfg.writeEntry("default_abr_upper_bitrate", upper);
	cfg.writeEntry("default_abr_lower_bitrate", lower);

        int quality = compressionWidget->baseQuality();
	cfg.writeEntry("default_vbr_quality", quality);
    }
    cfg.sync();

    qDebug("FileInfoDialog::accept()");
    m_info.dump();

    /* bits per sample */
    m_info.setBits(sbResolution->value());

    /* sample rate */
    m_info.setRate(cbSampleRate->currentText().toDouble());

    /* sample format */
    SampleFormat::Map sample_formats;
    SampleFormat sample_format =
	sample_formats.data(cbSampleFormat->currentIndex());
    if (m_info.contains(INF_SAMPLE_FORMAT) ||
        (sample_format != sample_formats.data(0)))
    {
	m_info.set(INF_SAMPLE_FORMAT, QVariant(sample_format.toInt()));
    }

    /* compression */
    CompressionType compressions;
    int compression = compressions.data(cbCompression->currentIndex());
    m_info.set(INF_COMPRESSION, (compression != AF_COMPRESSION_NONE) ?
        QVariant(compression) : QVariant());

    /* bitrate in Ogg/Vorbis mode */
    if (m_is_ogg) {
        CompressionWidget::Mode mode = compressionWidget->mode();
        QVariant del;

        switch (mode) {
	    case CompressionWidget::ABR_MODE: {
	        int nominal, upper, lower;
	        compressionWidget->getABRrates(nominal, lower, upper);
	        bool use_lowest  = compressionWidget->lowestEnabled();
	        bool use_highest = compressionWidget->highestEnabled();

	        m_info.set(INF_BITRATE_NOMINAL, QVariant(nominal));
	        m_info.set(INF_BITRATE_LOWER,
	                   (use_lowest) ? QVariant(lower) : del);
	        m_info.set(INF_BITRATE_UPPER,
	                   (use_highest) ? QVariant(upper) : del);
	        m_info.set(INF_VBR_QUALITY, del);
	        break;
	    }
	    case CompressionWidget::VBR_MODE: {
	        int quality = compressionWidget->baseQuality();

	        m_info.set(INF_BITRATE_NOMINAL, del);
	        m_info.set(INF_BITRATE_LOWER, del);
	        m_info.set(INF_BITRATE_UPPER, del);
	        m_info.set(INF_VBR_QUALITY, QVariant(quality));
	        break;
	    }
	}

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
	m_info.set(INF_CREATION_DATE, QVariant(date).toString());

    /* source, source form, album */
    acceptEdit(INF_SOURCE,      edSource->text());
    acceptEdit(INF_SOURCE_FORM, edSourceForm->text());
    acceptEdit(INF_ALBUM,       edAlbum->text());

    /* CD and track */
    int cd     = sbCD->value();
    int cds    = sbCDs->value();
    int track  = sbTrack->value();
    int tracks = sbTracks->value();
    m_info.set(INF_CD,     (cd     != 0) ? QVariant(cd)     : QVariant());
    m_info.set(INF_CDS,    (cds    != 0) ? QVariant(cds)    : QVariant());
    m_info.set(INF_TRACK,  (track  != 0) ? QVariant(track)  : QVariant());
    m_info.set(INF_TRACKS, (tracks != 0) ? QVariant(tracks) : QVariant());

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

    qDebug("FileInfoDialog::accept() --2--");
    m_info.dump();

    QDialog::accept();
}

//***************************************************************************
void FileInfoDialog::invokeHelp()
{
    KToolInvocation::invokeHelp("fileinfo");
}

//***************************************************************************
#include "FileInfoDialog.moc"
//***************************************************************************
//***************************************************************************
