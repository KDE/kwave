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

#include <QtCore/QtGlobal>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtCore/QDateTime>
#include <QtGui/QDialog>
#include <QtCore/QFileInfo>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QListWidget>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QSlider>
#include <QtGui/QSpinBox>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtGui/QTabWidget>
#include <QtGui/QToolTip>
#include <QtCore/QVector>
#include <QtGui/QWhatsThis>

#include <kglobal.h>
#include <kconfig.h>
#include <kcombobox.h>
#include <kdatewidget.h>
#include <klocale.h>
#include <kmimetype.h>
#include <knuminput.h>
#include <ktoolinvocation.h>

#include "libkwave/CodecManager.h"
#include "libkwave/CompressionType.h"
#include "libkwave/Encoder.h"
#include "libkwave/FileInfo.h"
#include "libkwave/GenreType.h"
#include "libkwave/SampleFormat.h"
#include "libkwave/String.h"
#include "libkwave/Utils.h"

#include "BitrateWidget.h"
#include "CompressionWidget.h"
#include "FileInfoDialog.h"
#include "KeywordWidget.h"
#include "SelectDateDialog.h"

/** section in the config file for storing default settings */
#define CONFIG_DEFAULT_SECTION "plugin fileinfo - setup dialog"

//***************************************************************************
Kwave::FileInfoDialog::FileInfoDialog(QWidget *parent, Kwave::FileInfo &info)
    :QDialog(parent), Ui::FileInfoDlg(), m_info(info)
{
    setupUi(this);

    QString mimetype = QVariant(m_info.get(Kwave::INF_MIMETYPE)).toString();
    if (!mimetype.length())
	mimetype = _("audio/x-wav"); // default mimetype

    m_is_mpeg = ((mimetype == _("audio/x-mpga")) ||
        (mimetype == _("audio/x-mp2")) ||
        (mimetype == _("audio/x-mp3")) ||
        (mimetype == _("audio/mpeg")));
    m_is_ogg = ((mimetype == _("audio/x-ogg")) ||
                (mimetype == _("application/x-ogg")) ||
                (mimetype == _("application/ogg")));

    qDebug("mimetype = %s",mimetype.toLocal8Bit().data());

    connect(cbCompression, SIGNAL(currentIndexChanged(int)),
            this, SLOT(compressionChanged()));
    connect(cbMpegLayer, SIGNAL(currentIndexChanged(int)),
            this, SLOT(mpegLayerChanged()));
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
Kwave::FileInfoDialog::~FileInfoDialog()
{
}

//***************************************************************************
void Kwave::FileInfoDialog::describeWidget(QWidget *widget,
                                           const QString &name,
                                           const QString &description)
{
    if (!widget) return;
    widget->setToolTip(description);
    widget->setWhatsThis(_("<b>") + name + _("</b><br>") + description);
}

//***************************************************************************
void Kwave::FileInfoDialog::initInfo(QLabel *label, QWidget *widget,
                                     Kwave::FileProperty property)
{
    if (label) label->setText(i18n(__(m_info.name(property))) + _(":"));
    if (widget) describeWidget(widget, i18n(__(m_info.name(property))),
                               m_info.description(property));
}

//***************************************************************************
void Kwave::FileInfoDialog::initInfoText(QLabel *label, QLineEdit *edit,
                                         Kwave::FileProperty property)
{
    initInfo(label, edit, property);
    if (edit) edit->setText(QVariant(m_info.get(property)).toString());
}

//***************************************************************************
void Kwave::FileInfoDialog::setupFileInfoTab()
{
    /* filename */
    initInfo(lblFileName, edFileName, Kwave::INF_FILENAME);
    QFileInfo fi(QVariant(m_info.get(Kwave::INF_FILENAME)).toString());
    edFileName->setText(fi.fileName());
    edFileName->setEnabled(fi.fileName().length() != 0);

    /* file size in bytes */
    initInfo(lblFileSize, edFileSize, Kwave::INF_FILESIZE);
    if (m_info.contains(Kwave::INF_FILESIZE)) {
	unsigned int size = QVariant(m_info.get(Kwave::INF_FILESIZE)).toUInt();
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
//    QString mimetype = QVariant(m_info.get(Kwave::INF_MIMETYPE)).toString();
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
    initInfoText(lblFileFormat,   edFileFormat,   Kwave::INF_MIMETYPE);

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
    initInfo(lblSampleFormat, cbSampleFormat, Kwave::INF_SAMPLE_FORMAT);
    cbSampleFormat->clear();
    Kwave::SampleFormat::Map sf;
    foreach (int k, sf.allKeys())
	cbSampleFormat->addItem(sf.description(k, true), sf.data(k));

    int format = QVariant(m_info.get(Kwave::INF_SAMPLE_FORMAT)).toInt();
    cbSampleFormat->setCurrentIndex(cbSampleFormat->findData(format));
}

//***************************************************************************
void Kwave::FileInfoDialog::setupCompressionTab(KConfigGroup &cfg)
{

    /*
     * mime type @ file info -> available compressions
     * compression @ file info / available compressions -> selected compression
     * selected compression -> mime type (edit field)
     * mime type -> mpeg/ogg/flac mode
     * mgeg layer -> compression
     */

    /* compression */
    updateAvailableCompressions();
    initInfo(lblCompression, cbCompression, Kwave::INF_COMPRESSION);

    compressionWidget->init(m_info);
    compressionWidget->setMode(m_info.contains(Kwave::INF_VBR_QUALITY) ?
        Kwave::CompressionWidget::VBR_MODE :
        Kwave::CompressionWidget::ABR_MODE);

    // enable/disable ABR/VBR controls, depending on mime type
    bool lower = m_info.contains(Kwave::INF_BITRATE_LOWER);
    bool upper = m_info.contains(Kwave::INF_BITRATE_UPPER);
    if (m_is_mpeg) {
	// MPEG file
	compressionWidget->enableABR(true, lower, upper);
	compressionWidget->enableVBR(false);
    } else if (m_is_ogg) {
	// Ogg/Vorbis file
	compressionWidget->enableABR(true, lower, upper);
	compressionWidget->enableVBR(true);
    } else {
	// other...
	compressionWidget->enableABR(false, false, false);
	compressionWidget->enableVBR(false);
    }

    // ABR bitrate settings
    int abr_bitrate = m_info.contains(Kwave::INF_BITRATE_NOMINAL) ?
                  QVariant(m_info.get(Kwave::INF_BITRATE_NOMINAL)).toInt() :
                  cfg.readEntry("default_abr_nominal_bitrate", -1);
    int min_bitrate = m_info.contains(Kwave::INF_BITRATE_LOWER) ?
                  QVariant(m_info.get(Kwave::INF_BITRATE_LOWER)).toInt() :
                  cfg.readEntry("default_abr_lower_bitrate",-1);
    int max_bitrate = m_info.contains(Kwave::INF_BITRATE_UPPER) ?
                  QVariant(m_info.get(Kwave::INF_BITRATE_UPPER)).toInt() :
                  cfg.readEntry("default_abr_upper_bitrate",-1);
    compressionWidget->setBitrates(abr_bitrate, min_bitrate, max_bitrate);

    // VBR base quality
    int quality = m_info.contains(Kwave::INF_VBR_QUALITY) ?
              QVariant(m_info.get(Kwave::INF_VBR_QUALITY)).toInt() :
              cfg.readEntry("default_vbr_quality", -1);
    compressionWidget->setQuality(quality);

    compressionChanged();

//    // this is not visible, not implemented yet...
//    InfoTab->setCurrentPage(5);
//    QWidget *page = InfoTab->currentPage();
//    InfoTab->removePage(page);
//    InfoTab->setCurrentPage(0);
//    return;
}

//***************************************************************************
void Kwave::FileInfoDialog::setupMpegTab()
{
    // the whole tab is only enabled in mpeg mode
    InfoTab->setTabEnabled(2, m_is_mpeg);

    /* MPEG layer */
    initInfo(lblMpegLayer,   cbMpegLayer,    Kwave::INF_MPEG_LAYER);
    int layer = m_info.get(Kwave::INF_MPEG_LAYER).toInt();
    if ((layer < 1) || (layer > 3))
	layer = 3; // default = layer III
    cbMpegLayer->setCurrentIndex(layer - 1);

    /* MPEG version */
    initInfo(lblMpegVersion, cbMpegVersion,  Kwave::INF_MPEG_VERSION);
    int ver = static_cast<int>(
        (2.0 * QVariant(m_info.get(Kwave::INF_MPEG_VERSION)).toDouble()));
    // 1, 2, 2.5 -> 2, 4, 5
    if ((ver < 1) || (ver > 5)) ver = 4; // default = version 2
    if (ver > 3) ver++; // 2, 4, 6
    ver >>= 1;          // 1, 2, 3
    ver--;              // 0, 1, 2
    if ((ver < 0) || (ver > 2)) ver = 1; // default = version 2
    cbMpegVersion->setCurrentIndex(ver);

    /* Mode extension */
    initInfo(lblMpegModeExt, cbMpegModeExt, Kwave::INF_MPEG_MODEEXT);
    // only in "Joint Stereo" mode, then depends on Layer
    //
    // Layer I+II          |  Layer III
    //                     |  Intensity stereo MS Stereo
    //--------------------------------------------------
    // 0 - bands  4 to 31  |  off              off  -> 4
    // 1 - bands  8 to 31  |  on               off  -> 5
    // 2 - bands 12 to 31  |  off              on   -> 6
    // 3 - bands 16 to 31  |  on               on   -> 7
    int modeext = QVariant(m_info.get(Kwave::INF_MPEG_MODEEXT)).toInt();
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
    initInfo(lblMpegEmphasis, cbMpegEmphasis, Kwave::INF_MPEG_EMPHASIS);
    int emphasis = QVariant(m_info.get(Kwave::INF_MPEG_EMPHASIS)).toInt();
    if ((emphasis >= 0) && (emphasis <= 2))
	cbMpegEmphasis->setCurrentIndex(emphasis);
    else
	cbMpegEmphasis->setEnabled(false);

    /* Copyrighted */
    initInfo(lblMpegCopyrighted, chkMpegCopyrighted, Kwave::INF_COPYRIGHTED);
    bool copyrighted = QVariant(m_info.get(Kwave::INF_COPYRIGHTED)).toBool();
    chkMpegCopyrighted->setChecked(copyrighted);
    chkMpegCopyrighted->setText((copyrighted) ? i18n("Yes") : i18n("No"));

    /* Original */
    initInfo(lblMpegOriginal, chkMpegOriginal, Kwave::INF_ORIGINAL);
    bool original = QVariant(m_info.get(Kwave::INF_ORIGINAL)).toBool();
    chkMpegOriginal->setChecked(original);
    chkMpegOriginal->setText((original) ? i18n("Yes") : i18n("No"));

    mpegLayerChanged();
}

//***************************************************************************
void Kwave::FileInfoDialog::setupContentTab()
{
    /* name, subject, version, genre, title, author, organization,
       copyright, license */
    initInfoText(lblName,         edName,         Kwave::INF_NAME);
    initInfoText(lblSubject,      edSubject,      Kwave::INF_SUBJECT);
    initInfoText(lblVersion,      edVersion,      Kwave::INF_VERSION);

    // genre type
    cbGenre->addItems(Kwave::GenreType::allTypes());
    QString genre = m_info.get(Kwave::INF_GENRE).toString();
    int genre_id = Kwave::GenreType::id(genre);
    if (genre_id >= 0) {
	// well known genre type
	genre = Kwave::GenreType::name(genre_id, true);
    } else {
	// user defined genre type
	cbGenre->addItem(genre);
    }
    initInfo(lblGenre,            cbGenre,        Kwave::INF_GENRE);
    cbGenre->setCurrentIndex(cbGenre->findText(genre));

    /* date widget */
    initInfo(lblDate, dateEdit, Kwave::INF_CREATION_DATE);
    QDate date;
    QString date_str = QVariant(m_info.get(Kwave::INF_CREATION_DATE)).toString();
    if (m_info.contains(Kwave::INF_CREATION_DATE)) {
        if (date_str.length())
	    date = QDate::fromString(date_str, Qt::ISODate);
    }
    if (!date.isValid()) {
	// fall back to "year only"
	int year = date_str.toInt();
	if ((year > 0) && (year <= 9999))
	    date = QDate(year, 1, 1);
    }
    if (!date.isValid()) {
	// fall back to "now"
	date = QDate::currentDate();
    }

    dateEdit->setDate(date);
    connect(btSelectDate, SIGNAL(clicked()), this, SLOT(selectDate()));
    connect(btSelectDateNow, SIGNAL(clicked()), this, SLOT(setDateNow()));
}

//***************************************************************************
void Kwave::FileInfoDialog::setupSourceTab()
{
    /* source, source form */
    initInfoText(lblSource,     edSource,     Kwave::INF_SOURCE);
    initInfoText(lblSourceForm, edSourceForm, Kwave::INF_SOURCE_FORM);

    /* Album, CD and track */
    initInfoText(lblAlbum,      edAlbum,      Kwave::INF_ALBUM);
    initInfo(lblCD, sbCD, Kwave::INF_CD);
    int cd = (m_info.contains(Kwave::INF_CD)) ?
	QVariant(m_info.get(Kwave::INF_CD)).toInt() : 0;
    sbCD->setValue(cd);

    initInfo(0, sbCDs, Kwave::INF_CDS);
    int cds = (m_info.contains(Kwave::INF_CDS)) ?
	QVariant(m_info.get(Kwave::INF_CDS)).toInt() : 0;
    sbCDs->setValue(cds);

    initInfo(lblTrack, sbTrack, Kwave::INF_TRACK);
    int track = (m_info.contains(Kwave::INF_TRACK)) ?
	QVariant(m_info.get(Kwave::INF_TRACK)).toInt() : 0;
    sbTrack->setValue(track);

    initInfo(0, sbTracks, Kwave::INF_TRACKS);
    int tracks = (m_info.contains(Kwave::INF_TRACKS)) ?
	QVariant(m_info.get(Kwave::INF_TRACKS)).toInt() : 0;
    sbTracks->setValue(tracks);

    /* software, engineer, technican */
    initInfoText(lblSoftware,     edSoftware,     Kwave::INF_SOFTWARE);
    initInfoText(lblEngineer,     edEngineer,     Kwave::INF_ENGINEER);
    initInfoText(lblTechnican,    edTechnican,    Kwave::INF_TECHNICAN);

}

//***************************************************************************
void Kwave::FileInfoDialog::setupAuthorCopyrightTab()
{
    /* author organization, copyright, license, ISRC */
    initInfoText(lblAuthor,       edAuthor,       Kwave::INF_AUTHOR);
    initInfoText(lblOrganization, edOrganization, Kwave::INF_ORGANIZATION);
    initInfoText(lblCopyright,    edCopyright,    Kwave::INF_COPYRIGHT);
    initInfoText(lblLicense,      edLicense,      Kwave::INF_LICENSE);
    initInfoText(lblISRC,         edISRC,         Kwave::INF_ISRC);

    /* product, archival, contact */
    initInfoText(lblProduct,  edProduct,  Kwave::INF_PRODUCT);
    initInfoText(lblArchival, edArchival, Kwave::INF_ARCHIVAL);
    initInfoText(lblContact,  edContact,  Kwave::INF_CONTACT);
}

//***************************************************************************
void Kwave::FileInfoDialog::setupMiscellaneousTab()
{
    /* commissioned */
    initInfoText(lblCommissioned, edCommissioned, Kwave::INF_COMMISSIONED);

    /* list of keywords */
    lblKeywords->setText(i18n(m_info.name(Kwave::INF_KEYWORDS).toAscii()));
    lstKeywords->setWhatsThis(_("<b>") +
	i18n(m_info.name(Kwave::INF_KEYWORDS).toAscii()) +
	_("</b><br>") + m_info.description(Kwave::INF_KEYWORDS));
    if (m_info.contains(Kwave::INF_KEYWORDS)) {
	QString keywords = QVariant(m_info.get(Kwave::INF_KEYWORDS)).toString();
	lstKeywords->setKeywords(keywords.split(_(";")));
    }
    connect(lstKeywords, SIGNAL(autoGenerate()),
            this, SLOT(autoGenerateKeywords()));

}

//***************************************************************************
void Kwave::FileInfoDialog::selectDate()
{
    QDate date(dateEdit->date());
    Kwave::SelectDateDialog date_dialog(this, date);
    if (date_dialog.exec() == QDialog::Accepted) {
	date = date_dialog.date();
	dateEdit->setDate(date);
    }
}

//***************************************************************************
void Kwave::FileInfoDialog::setDateNow()
{
    dateEdit->setDate(QDate::currentDate());
}

//***************************************************************************
void Kwave::FileInfoDialog::tracksChanged(int tracks)
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
	    lblChannelsVerbose->setText(_(""));
	    break;
    }
}

//***************************************************************************
void Kwave::FileInfoDialog::updateAvailableCompressions()
{
    cbCompression->blockSignals(true);

    Kwave::CompressionType compressions;
    QList<int> supported_compressions;
    QString mime_type = m_info.get(Kwave::INF_MIMETYPE).toString();

    // switch by mime type:
    if (mime_type.length()) {
	// mime type is present -> offer only matching compressions
	Kwave::Encoder *encoder = Kwave::CodecManager::encoder(mime_type);
	if (encoder) supported_compressions = encoder->compressionTypes();
    } else {
	// no mime type -> allow all mimetypes suitable for encoding
	supported_compressions.append(AF_COMPRESSION_NONE);

	QStringList mime_types = Kwave::CodecManager::encodingMimeTypes();
	foreach (QString m, mime_types) {
	    Kwave::Encoder *encoder = Kwave::CodecManager::encoder(m);
	    if (!encoder) continue;
	    QList<int> comps = encoder->compressionTypes();
	    foreach (int c, comps)
		if (!supported_compressions.contains(c))
		    supported_compressions.append(c);
	}
    }

    // if nothing is supported, then use only "none"
    if (supported_compressions.isEmpty())
	supported_compressions.append(AF_COMPRESSION_NONE);

    // add supported compressions to the combo box
    cbCompression->clear();
    foreach (int compression, supported_compressions) {
	cbCompression->addItem(compressions.description(
		compressions.findFromData(compression), true),
	    compression);
    }

    cbCompression->blockSignals(false);

    int c = QVariant(m_info.get(Kwave::INF_COMPRESSION)).toInt();
    int old_index = cbCompression->currentIndex();
    int new_index = cbCompression->findData(c);
    if (new_index != old_index) {
	// selection of the compression type changed

	// take the highest supported compression if changed to "invalid"
	// (assuming that the last entry in the list is the best one)
	if (new_index < 0)
	    new_index = cbCompression->count() - 1;

	cbCompression->setCurrentIndex(new_index);
    }
}

//***************************************************************************
void Kwave::FileInfoDialog::compressionChanged()
{
    if (!cbCompression || !edFileFormat) return;


    int compression = cbCompression->itemData(
	cbCompression->currentIndex()).toInt();

    // selected compression -> mime type (edit field)
    QString file_mime_type = m_info.get(Kwave::INF_MIMETYPE).toString();
    if (!file_mime_type.length()) {
	// if mime type is given by file info -> keep it
	// otherwise select one by evaluating the compression
	QStringList mime_types = Kwave::CodecManager::encodingMimeTypes();
	foreach (const QString &mime_type, mime_types) {
	    Kwave::Encoder *encoder = Kwave::CodecManager::encoder(mime_type);
	    if (!encoder) continue;
	    QList<int> comps = encoder->compressionTypes();
	    if (comps.contains(compression)) {
		edFileFormat->setText(mime_type);
		break;
	    }
	}
    }

    // if mpeg mode selected -> select mpeg layer
    int mpeg_layer = -1;
    switch (compression)
    {
	case Kwave::CompressionType::MPEG_LAYER_I:
	    mpeg_layer = 1;
	    m_is_mpeg = true;
	    m_is_ogg  = false;
	    break;
	case Kwave::CompressionType::MPEG_LAYER_II:
	    mpeg_layer = 2;
	    m_is_mpeg = true;
	    m_is_ogg  = false;
	    break;
	case Kwave::CompressionType::MPEG_LAYER_III:
	    mpeg_layer = 3;
	    m_is_mpeg = true;
	    m_is_ogg  = false;
	    break;
	case Kwave::CompressionType::OGG_VORBIS:
	    m_is_mpeg = false;
	    m_is_ogg  = true;
	    break;
	case Kwave::CompressionType::FLAC:
	case AF_COMPRESSION_NONE: /* FALLTHROUGH */
	default:
	    m_is_mpeg = false;
	    m_is_ogg  = false;
	    break;
    }

    InfoTab->setTabEnabled(2, m_is_mpeg);
    if (m_is_mpeg || m_is_ogg) cbSampleFormat->setEnabled(false);

    if ((mpeg_layer > 0) && (cbMpegLayer->currentIndex() != (mpeg_layer - 1)))
	cbMpegLayer->setCurrentIndex(mpeg_layer - 1);

    // enable/disable ABR/VBR controls, depending on mime type
    bool lower = compressionWidget->lowestEnabled();
    bool upper = compressionWidget->highestEnabled();
    if (m_is_mpeg) {
	// MPEG file
	compressionWidget->enableABR(true, lower, upper);
	compressionWidget->enableVBR(false);
	compressionWidget->setMode(Kwave::CompressionWidget::ABR_MODE);
    } else if (m_is_ogg) {
	// Ogg/Vorbis file
	compressionWidget->enableABR(true, lower, upper);
	compressionWidget->enableVBR(true);
    } else {
	// other...
	compressionWidget->enableABR(false, lower, upper);
	compressionWidget->enableVBR(false);
    }

}

//***************************************************************************
void Kwave::FileInfoDialog::mpegLayerChanged()
{
    if (!cbMpegLayer || !m_is_mpeg) return;

    int layer = cbMpegLayer->currentIndex() + 1;
    int compression = AF_COMPRESSION_NONE;
    switch (layer) {
	case 1:
	    compression = Kwave::CompressionType::MPEG_LAYER_I;
	    break;
	case 2:
	    compression = Kwave::CompressionType::MPEG_LAYER_II;
	    break;
	case 3:
	    compression = Kwave::CompressionType::MPEG_LAYER_III;
	    break;
    }

    if (compression != AF_COMPRESSION_NONE) {
	int index = cbCompression->findData(compression);
	if (index >= 0) cbCompression->setCurrentIndex(index);
    }
}

//***************************************************************************
void Kwave::FileInfoDialog::autoGenerateKeywords()
{
    // start with the current list
    QStringList list = lstKeywords->keywords();

    // name, subject, version, genre, author, organization,
    // copyright, license, source, source form, album,
    // product, archival, contact, software, technican, engineer,
    // commissioned, ISRC
    const QString space = _(" ");
    list += edName->text().split(space);
    list += edSubject->text().split(space);
    list += edVersion->text().split(space);
    list += cbGenre->currentText();
    list += edAuthor->text().split(space);
    list += edOrganization->text().split(space);
    list += edCopyright->text().split(space);
    list += edLicense->text().split(space);
    list += edSource->text().split(space);
    list += edSourceForm->text().split(space);
    list += edAlbum->text().split(space);
    list += edProduct->text().split(space);
    list += edArchival->text().split(space);
    list += edContact->text().split(space);
    list += edSoftware->text().split(space);
    list += edTechnican->text().split(space);
    list += edEngineer->text().split(space);
    list += edCommissioned->text().split(space);
    list += edISRC->text().split(space);

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
void Kwave::FileInfoDialog::acceptEdit(Kwave::FileProperty property,
                                       QString value)
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
void Kwave::FileInfoDialog::accept()
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

    /* mime type */
    m_info.set(Kwave::INF_MIMETYPE, edFileFormat->text());

    /* bits per sample */
    m_info.setBits(sbResolution->value());

    /* sample rate */
    m_info.setRate(cbSampleRate->currentText().toDouble());

    /* sample format */
    Kwave::SampleFormat::Map sample_formats;
    int sample_format =
	cbSampleFormat->itemData(cbSampleFormat->currentIndex()).toInt();
    m_info.set(Kwave::INF_SAMPLE_FORMAT, QVariant(sample_format));

    /* compression */
    Kwave::CompressionType compressions;
    int compression =
	cbCompression->itemData(cbCompression->currentIndex()).toInt();
    m_info.set(Kwave::INF_COMPRESSION, (compression != AF_COMPRESSION_NONE) ?
        QVariant(compression) : QVariant());

    /* MPEG layer */
    if (m_is_mpeg) {
	int layer = cbMpegLayer->currentIndex() + 1;
	m_info.set(Kwave::INF_MPEG_LAYER, layer);
    }

    /* bitrate in Ogg/Vorbis or MPEG mode */
    if (m_is_ogg || m_is_mpeg) {
        Kwave::CompressionWidget::Mode mode = compressionWidget->mode();
        QVariant del;

        switch (mode) {
	    case Kwave::CompressionWidget::ABR_MODE: {
	        int nominal, upper, lower;
	        compressionWidget->getABRrates(nominal, lower, upper);
	        bool use_lowest  = compressionWidget->lowestEnabled();
	        bool use_highest = compressionWidget->highestEnabled();

	        m_info.set(Kwave::INF_BITRATE_NOMINAL, QVariant(nominal));
	        m_info.set(Kwave::INF_BITRATE_LOWER,
	                   (use_lowest) ? QVariant(lower) : del);
	        m_info.set(Kwave::INF_BITRATE_UPPER,
	                   (use_highest) ? QVariant(upper) : del);
	        m_info.set(Kwave::INF_VBR_QUALITY, del);
	        break;
	    }
	    case Kwave::CompressionWidget::VBR_MODE: {
	        int quality = compressionWidget->baseQuality();

	        m_info.set(Kwave::INF_BITRATE_NOMINAL, del);
	        m_info.set(Kwave::INF_BITRATE_LOWER, del);
	        m_info.set(Kwave::INF_BITRATE_UPPER, del);
	        m_info.set(Kwave::INF_VBR_QUALITY, QVariant(quality));
	        break;
	    }
	}
    }

    /* name, subject, version, genre, title, author, organization,
       copyright */
    acceptEdit(Kwave::INF_NAME,         edName->text());
    acceptEdit(Kwave::INF_SUBJECT,      edSubject->text());
    acceptEdit(Kwave::INF_VERSION,      edVersion->text());
    acceptEdit(Kwave::INF_GENRE,        cbGenre->currentText());
    acceptEdit(Kwave::INF_AUTHOR,       edAuthor->text());
    acceptEdit(Kwave::INF_ORGANIZATION, edOrganization->text());
    acceptEdit(Kwave::INF_COPYRIGHT,    edCopyright->text());
    acceptEdit(Kwave::INF_LICENSE,      edLicense->text());

    /* date */
    QDate date = dateEdit->date();
    if ( (date != QDate::currentDate()) ||
	m_info.contains(Kwave::INF_CREATION_DATE) )
    {
	m_info.set(Kwave::INF_CREATION_DATE, QVariant(date).toString());
    }

    /* source, source form, album */
    acceptEdit(Kwave::INF_SOURCE,      edSource->text());
    acceptEdit(Kwave::INF_SOURCE_FORM, edSourceForm->text());
    acceptEdit(Kwave::INF_ALBUM,       edAlbum->text());

    /* CD and track */
    int cd     = sbCD->value();
    int cds    = sbCDs->value();
    int track  = sbTrack->value();
    int tracks = sbTracks->value();
    m_info.set(Kwave::INF_CD,     (cd     != 0) ? QVariant(cd)     : QVariant());
    m_info.set(Kwave::INF_CDS,    (cds    != 0) ? QVariant(cds)    : QVariant());
    m_info.set(Kwave::INF_TRACK,  (track  != 0) ? QVariant(track)  : QVariant());
    m_info.set(Kwave::INF_TRACKS, (tracks != 0) ? QVariant(tracks) : QVariant());

    /* product, archival, contact */
    acceptEdit(Kwave::INF_PRODUCT,     edProduct->text());
    acceptEdit(Kwave::INF_ARCHIVAL,    edArchival->text());
    acceptEdit(Kwave::INF_CONTACT,     edContact->text());

    /* software, engineer, technican, commissioned, ISRC, keywords */
    acceptEdit(Kwave::INF_SOFTWARE,    edSoftware->text());
    acceptEdit(Kwave::INF_ENGINEER,    edEngineer->text());
    acceptEdit(Kwave::INF_TECHNICAN,   edTechnican->text());
    acceptEdit(Kwave::INF_COMMISSIONED,edCommissioned->text());
//  acceptEdit(Kwave::INF_ISRC,        edISRC->text()); <- READ-ONLY

    // list of keywords
    acceptEdit(Kwave::INF_KEYWORDS,
	lstKeywords->keywords().join(_("; ")));

    qDebug("FileInfoDialog::accept() --2--");
    m_info.dump();

    QDialog::accept();
}

//***************************************************************************
void Kwave::FileInfoDialog::invokeHelp()
{
    KToolInvocation::invokeHelp(_("fileinfo"));
}

//***************************************************************************
#include "FileInfoDialog.moc"
//***************************************************************************
//***************************************************************************
