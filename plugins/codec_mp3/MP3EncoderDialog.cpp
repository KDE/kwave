/***************************************************************************
   MP3EncoderDialog.cpp  - dialog for configuring the MP3 encoer
                           -------------------
    begin                : Sun Jun 03 2012
    copyright            : (C) 2012 by Thomas Eschenbacher
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

#include <QAbstractButton>
#include <QApplication>
#include <QBuffer>
#include <QCursor>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLatin1Char>
#include <QPointer>
#include <QProcess>
#include <QPushButton>
#include <QtGlobal>
#include <QLineEdit>

#include <KHelpClient>
#include <KProcess>

#include "libkwave/FileInfo.h"
#include "libkwave/MessageBox.h"
#include "libkwave/MetaDataList.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/ReaderMode.h"
#include "libkwave/SignalManager.h"
#include "libkwave/String.h"
#include "libkwave/Utils.h"

#include "libgui/FileDialog.h"

#include "MP3Encoder.h"
#include "MP3EncoderDialog.h"
#include "MP3EncoderSettings.h"

/** text of the last entry in the combo box, for user defined settings */
#define PRESET_NAME_USER_DEFINED i18n("(user defined)")

#ifdef EXECUTABLE_SUFFIX
#define EXE_SUFFIX EXECUTABLE_SUFFIX
#else
#define EXE_SUFFIX ""
#endif

/**
 * set of predefined encoder settings
 */
const Kwave::MP3EncoderSettings g_predefined_settings[] =
{
    {
	_("LAME"),                        // name
	_("lame" EXE_SUFFIX),             // path
	{
	    _("-r"),                      // raw format
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
	    _("--big-endian"),            // byte order
#else
	    _("--little-endian"),         // byte order
#endif
	    _("--signed")                 // signed
	},
	{
	    _("-s %1"),                   // sample rate
	    _("--bitwidth %1"),           // bits per sample
	    {
		_("-mm"),                 // mono
		_("-mj")                  // stereo
	    }
	},
	{
	    {
		_("--abr %1"),            // average bitrate
		_("-b %1"),               // minimum bitrate
		_("-B %1")                // maximum bitrate
	    }
	},
	{
	    {
		_("-en"),                 // no emphasis
		_("-e5"),                 // 50/15ms
		_("-ec")                  // CCIT J17
	    },
	    _("-q 2"),                    // noise shaping
	    _("--strictly-enforce-ISO")   // compatibility
	},
	{
	    _("-c"),                      // copyrighted
	    _("-o"),                      // original
	    _("-p"),                      // protect
	    _(""),                        // prepended
	    _("--silent")                 // appended
	},
	{
	    _("--longhelp"),              // encoder help
	    _("--version")                // encoder version
	}
    },
    /***********************************************************************/
    {
	_("TwoLAME"),                     // name
	_("twolame" EXE_SUFFIX),          // path
	{
	    _("--raw-input"),             // raw format
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
	    _("--byte-swap"),             // byte order
#else
	    _(""),                        // byte order
#endif
	    _("")                         // signed
	},
	{
	    _("--samplerate=%1"),         // sample rate
	    _("--samplesize=16"),         // bits per sample (supports only 16)
	    {
		_("--channels=1 --mode=mono"),  // mono
		_("--channels=2 --mode=joint")  // stereo
	    }
	},
	{
	    {
		_("--bitrate=%1"),        // average bitrate
		_(""),                    // minimum bitrate
		_("--max-bitrate=%1")     // maximum bitrate
	    }
	},
	{
	    {
		_("--deemphasis=n"),      // no emphasis
		_("--deemphasis=5"),      // 50/15ms
		_("--deemphasis=c")       // CCIT J17
	    },
	    _(""),                        // noise shaping
	    _("")                         // compatibility
	},
	{
	    _("--copyright"),             // copyrighted
	    _("--original"),              // original
	    _("--protect"),               // protect
	    _(""),                        // prepended
	    _("--quiet")                  // appended
	},
	{
	    _("--help"),                  // encoder help
	    _("--help")                   // encoder version
	}
    },
    /***********************************************************************/
    {
	_("tooLAME"),                     // name
	_("toolame" EXE_SUFFIX),          // path
	{
	    _(""),                        // raw format
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
	    _("-x"),                      // byte order
#else
	    _(""),                        // byte order
#endif
	    _("")                         // signed
	},
	{
	    _("-s [%khz]"),               // sample rate
	    _(""),                        // bits per sample (supports only 16)
	    {
		_("-mm"),                 // mono
		_("-mj")                  // stereo
	    }
	},
	{
	    {
		_("-b %1"),               // average bitrate
		_(""),                    // minimum bitrate
		_("")                     // maximum bitrate
	    }
	},
	{
	    {
		_("-dn"),                 // no emphasis
		_("-d5"),                 // 50/15ms
		_("-dc")                  // CCIT J17
	    },
	    _(""),                        // noise shaping
	    _("")                         // compatibility
	},
	{
	    _("-c"),                      // copyrighted
	    _("-o"),                      // original
	    _("-e"),                      // protect
	    _(""),                        // prepended
	    _("-t 0")                     // appended
	},
	{
	    _("-help"),                   // encoder help
	    _("-version")                 // encoder version
	}
    }
};

/***************************************************************************/

/**
 * load a text field with the content of the current settings
 * @param field member within m_settings
 * @param control the QLineEdit to fill with the value
 */
#define LOAD(field, control) control->setText(m_settings.field)

/**
 * take the content of a QLineEdit and save it back into m_settings
 * @param field member within m_settings
 * @param control the QLineEdit to fill with the value
 */
#define SAVE(field, control) \
    m_settings.field = QString(control->text()).simplified()

/**
 * connect the editingFinished() signal of a QLineEdit to our slot
 * switchToUserDefined()
 * @param control the QLineEdit to handle
 */
#define CONNECT(control) \
    connect(control, SIGNAL(editingFinished()), \
    this, SLOT(switchToUserDefined()))

/**
 * check if the content of a QLineEdit matches the corresponding member
 * of some "settings" record
 * @param field member within m_settings
 * @param control the QLineEdit to compare against
 * @return the bool variable "match" is updated (logical AND)
 */
#define CHECK(field, control) match &= \
    (settings.field == QString(control->text()).simplified())

#define ELEMENTS_OF(x) (sizeof(x) / sizeof(x[0]))

/***************************************************************************/
Kwave::MP3EncoderDialog::MP3EncoderDialog(QWidget *parent)
    :QDialog(parent), Ui::MP3EncoderDialogBase(),
     m_settings(g_predefined_settings[0])
{
    setupUi(this);
    setModal(true);

    // set up the combo box with all presets
    cbProgram->clear();
    for (unsigned int i = 0; i < ELEMENTS_OF(g_predefined_settings); i++) {
	QString name    = g_predefined_settings[i].m_name;
	QString path    = searchPath(g_predefined_settings[i].m_path);
	QString param   = g_predefined_settings[i].m_info.m_version;
	QString version = encoderVersion(path, param);
	if (version.length() >= name.length())
	    cbProgram->addItem(version);
	else
	    cbProgram->addItem(name);
    }
    cbProgram->addItem(PRESET_NAME_USER_DEFINED);

    // load the saved settings from the config file
    m_settings.load();
    load();

    // connect the combo box of the program selection
    connect(cbProgram, SIGNAL(activated(int)),
            this, SLOT(selectProgram(int)));

    // standard actions, reset / reset to defaults etc...
    connect(buttonBox, SIGNAL(clicked(QAbstractButton*)),
            this, SLOT(buttonClicked(QAbstractButton*)));
    connect(buttonBox_Help, SIGNAL(helpRequested()),
            this, SLOT(invokeHelp()));

    // auto-detect button
    connect(btDetect, SIGNAL(clicked()),      this, SLOT(autoDetect()));

    // locate file path button
    connect(btLocate, SIGNAL(clicked()),      this, SLOT(locatePath()));

    // search for program (file browser)
    connect(btSearch, SIGNAL(clicked()),      this, SLOT(browseFile()));

    // test setup
    connect(btTest, SIGNAL(clicked()),        this, SLOT(testSettings()));

    // builtin help of the encoder
    connect(btEncoderHelp, SIGNAL(clicked()), this, SLOT(encoderHelp()));

    // whenever a setting has been manally edited, check if that is a
    // user defined setting or a predefined set of parameters
    CONNECT(edPath);

    CONNECT(edRawFormat);
    CONNECT(edByteOrder);
    CONNECT(edSign);

    CONNECT(edSampleRate);
    CONNECT(edBitsPerSample);
    CONNECT(edMono);
    CONNECT(edStereo);

    CONNECT(edBitrateAvg);
    CONNECT(edBitrateMin);
    CONNECT(edBitrateMax);

    CONNECT(edEmphasisNone);
    CONNECT(edEmphasis5015ms);
    CONNECT(edEmphasisCCIT_J17);

    CONNECT(edNoiseShaping);
    CONNECT(edCompatibility);

    CONNECT(edCopyright);
    CONNECT(edOriginal);
    CONNECT(edProtect);
    CONNECT(edPrepend);
    CONNECT(edAppend);

    CONNECT(edEncoderHelp);
    CONNECT(edVersionInfo);

    // set the focus onto the "OK" button
    buttonBox->button(QDialogButtonBox::Ok)->setFocus();
}

/***************************************************************************/
Kwave::MP3EncoderDialog::~MP3EncoderDialog()
{
}

/***************************************************************************/
void Kwave::MP3EncoderDialog::load()
{
    // cbProgram
    unsigned int i = 0;
    bool use_preset = false;
    for (i = 0; i < ELEMENTS_OF(g_predefined_settings); i++) {
	if (g_predefined_settings[i].m_name == m_settings.m_name) {
	    QString path = m_settings.m_path;
	    m_settings = g_predefined_settings[i];
	    m_settings.m_path = path;
	    cbProgram->setCurrentIndex(i);
	    use_preset = true;
	    break;
	}
    }
    if ((!use_preset) && (cbProgram->currentIndex() != Kwave::toInt(i))) {
	// set the combo box to "user defined" and load the rest from the config
	cbProgram->setCurrentIndex(i);
    }

    // set all other dialog content according to the loaded settings
    LOAD(m_path,                           edPath);

    LOAD(m_input.m_raw_format,             edRawFormat);
    LOAD(m_input.m_byte_order,             edByteOrder);
    LOAD(m_input.m_signed,                 edSign);

    LOAD(m_format.m_sample_rate,           edSampleRate);
    LOAD(m_format.m_bits_per_sample,       edBitsPerSample);
    LOAD(m_format.m_channels.m_mono,       edMono);
    LOAD(m_format.m_channels.m_stereo,     edStereo);

    LOAD(m_quality.m_bitrate.m_avg,        edBitrateAvg);
    LOAD(m_quality.m_bitrate.m_min,        edBitrateMin);
    LOAD(m_quality.m_bitrate.m_max,        edBitrateMax);

    LOAD(m_encoding.m_emphasis.m_none,     edEmphasisNone);
    LOAD(m_encoding.m_emphasis.m_50_15ms,  edEmphasis5015ms);
    LOAD(m_encoding.m_emphasis.m_ccit_j17, edEmphasisCCIT_J17);

    LOAD(m_encoding.m_noise_shaping,       edNoiseShaping);
    LOAD(m_encoding.m_compatibility,       edCompatibility);

    LOAD(m_flags.m_copyright,              edCopyright);
    LOAD(m_flags.m_original,               edOriginal);
    LOAD(m_flags.m_protect,                edProtect);
    LOAD(m_flags.m_prepend,                edPrepend);
    LOAD(m_flags.m_append,                 edAppend);

    LOAD(m_info.m_help,                    edEncoderHelp);
    LOAD(m_info.m_version,                 edVersionInfo);

    updateEncoderInfo();
}

/***************************************************************************/
void Kwave::MP3EncoderDialog::save()
{
    // get the content of the combo box
    int index = cbProgram->currentIndex();
    if (index < Kwave::toInt(ELEMENTS_OF(g_predefined_settings))) {
	m_settings.m_name = g_predefined_settings[index].m_name;
    } else {
	m_settings.m_name = _("*");
    }

    // fetch all settings from the dialog content
    SAVE(m_path,                           edPath);

    SAVE(m_input.m_raw_format,             edRawFormat);
    SAVE(m_input.m_byte_order,             edByteOrder);
    SAVE(m_input.m_signed,                 edSign);

    SAVE(m_format.m_sample_rate,           edSampleRate);
    SAVE(m_format.m_bits_per_sample,       edBitsPerSample);
    SAVE(m_format.m_channels.m_mono,       edMono);
    SAVE(m_format.m_channels.m_stereo,     edStereo);

    SAVE(m_quality.m_bitrate.m_avg,        edBitrateAvg);
    SAVE(m_quality.m_bitrate.m_min,        edBitrateMin);
    SAVE(m_quality.m_bitrate.m_max,        edBitrateMax);

    SAVE(m_encoding.m_emphasis.m_none,     edEmphasisNone);
    SAVE(m_encoding.m_emphasis.m_50_15ms,  edEmphasis5015ms);
    SAVE(m_encoding.m_emphasis.m_ccit_j17, edEmphasisCCIT_J17);

    SAVE(m_encoding.m_noise_shaping,       edNoiseShaping);
    SAVE(m_encoding.m_compatibility,       edCompatibility);

    SAVE(m_flags.m_copyright,              edCopyright);
    SAVE(m_flags.m_original,               edOriginal);
    SAVE(m_flags.m_protect,                edProtect);
    SAVE(m_flags.m_prepend,                edPrepend);
    SAVE(m_flags.m_append,                 edAppend);

    SAVE(m_info.m_help,                    edEncoderHelp);
    SAVE(m_info.m_version,                 edVersionInfo);

    m_settings.save();
}

/***************************************************************************/
void Kwave::MP3EncoderDialog::selectProgram(int index)
{
    if (index < 0) return;
    if (index >= Kwave::toInt(ELEMENTS_OF(g_predefined_settings))) return;

    m_settings = g_predefined_settings[index];
    load();
}

/***************************************************************************/
void Kwave::MP3EncoderDialog::switchToUserDefined()
{
    int index = cbProgram->currentIndex();

    for (unsigned i = 0; i < ELEMENTS_OF(g_predefined_settings); i++) {
	const Kwave::MP3EncoderSettings &settings =
	    g_predefined_settings[i];
	bool match = true;

	match &= bool(edPath->text().simplified().contains(settings.m_path,
	    Qt::CaseInsensitive));

	CHECK(m_input.m_raw_format,             edRawFormat);
	CHECK(m_input.m_byte_order,             edByteOrder);
	CHECK(m_input.m_signed,                 edSign);

	CHECK(m_format.m_sample_rate,           edSampleRate);
	CHECK(m_format.m_bits_per_sample,       edBitsPerSample);
	CHECK(m_format.m_channels.m_mono,       edMono);
	CHECK(m_format.m_channels.m_stereo,     edStereo);

	CHECK(m_quality.m_bitrate.m_avg,        edBitrateAvg);
	CHECK(m_quality.m_bitrate.m_min,        edBitrateMin);
	CHECK(m_quality.m_bitrate.m_max,        edBitrateMax);

	CHECK(m_encoding.m_emphasis.m_none,     edEmphasisNone);
	CHECK(m_encoding.m_emphasis.m_50_15ms,  edEmphasis5015ms);
	CHECK(m_encoding.m_emphasis.m_ccit_j17, edEmphasisCCIT_J17);

	CHECK(m_encoding.m_noise_shaping,       edNoiseShaping);
	CHECK(m_encoding.m_compatibility,       edCompatibility);

	CHECK(m_flags.m_copyright,              edCopyright);
	CHECK(m_flags.m_original,               edOriginal);
	CHECK(m_flags.m_protect,                edProtect);
	CHECK(m_flags.m_prepend,                edPrepend);
	CHECK(m_flags.m_append,                 edAppend);

	CHECK(m_info.m_help,                    edEncoderHelp);
	CHECK(m_info.m_version,                 edVersionInfo);

	if (match) {
	    // found a match against known preset
	    if (Kwave::toInt(i) != index) {
		cbProgram->setCurrentIndex(i);
		updateEncoderInfo();
	    }
	    return;
	}
    }

    // fallback: "user defined"
    cbProgram->setCurrentIndex(ELEMENTS_OF(g_predefined_settings));
    updateEncoderInfo();
}

/***************************************************************************/
void Kwave::MP3EncoderDialog::buttonClicked(QAbstractButton *button)
{
    if (!button || !buttonBox) return;
    switch (buttonBox->standardButton(button)) {
	case QDialogButtonBox::Ok:
	    // save settings and accept
	    save();
	    break;
	case QDialogButtonBox::Reset:
	    // reset to last saved state
	    load();
	    break;
	case QDialogButtonBox::RestoreDefaults:
	    // reset to default settings == entry #0 in the combo box
	    selectProgram(0);
	    break;
	default:
	    break;
    }
}

/***************************************************************************/
void Kwave::MP3EncoderDialog::autoDetect()
{
    for (unsigned i = 0; i < ELEMENTS_OF(g_predefined_settings); ++i) {
	QFile f(searchPath(g_predefined_settings[i].m_path));
	if (f.exists()) {
	    // found it :)
	    cbProgram->setCurrentIndex(i);
	    selectProgram(i);
	    locatePath();
	    return;
	}
    }
}

/***************************************************************************/
void Kwave::MP3EncoderDialog::locatePath()
{
    const QString prog_orig = edPath->text().simplified();
    const QString prog      = searchPath(prog_orig);
    if (prog != prog_orig) {
	edPath->setText(prog);
	updateEncoderInfo();
    }
}

/***************************************************************************/
void Kwave::MP3EncoderDialog::browseFile()
{
    QString mask = _("*");
#ifdef EXECUTABLE_SUFFIX
    mask += QString(EXECUTABLE_SUFFIX);
#endif
    QPointer<Kwave::FileDialog> dlg = new(std::nothrow)
	Kwave::FileDialog(
	    _("kfiledialog:///kwave_mp3_encoder"),
	    Kwave::FileDialog::Opening,
	    _(""), this, true,
	    QUrl::fromLocalFile(_("file:/") + edPath->text().simplified()),
	    mask
	);
    if (!dlg) return;
    dlg->setWindowTitle(i18n("Select MP3 Encoder"));
    dlg->setDirectory(_("/usr/bin/"));
    if (dlg->exec() == QDialog::Accepted)
	edPath->setText(dlg->selectedUrl().toLocalFile());
    delete dlg;
}

/***************************************************************************/
void Kwave::MP3EncoderDialog::testSettings()
{
    const sample_index_t test_length = 128 * 1024; // 128k samples
    const double         sample_rate = 44100.0;
    const unsigned int   bits        = 16;
    const unsigned int   tracks      = 2;

    // save all data, so that the encoder can read it
    save();

    // us a dummy sink in memory
    QBuffer dst;

    // create some dummy audio data (stereo)
    Kwave::SignalManager manager(this);
    manager.newSignal(test_length, sample_rate, bits, tracks);
    Kwave::MetaDataList meta_data = manager.metaData();

    // add some dummy meta data, to cover all parameters of the encoder
    Kwave::FileInfo info(meta_data);
    info.set(Kwave::INF_BITRATE_NOMINAL, QVariant(128000));
    info.set(Kwave::INF_BITRATE_LOWER,   QVariant( 64000));
    info.set(Kwave::INF_BITRATE_UPPER,   QVariant(192000));
    info.set(Kwave::INF_MPEG_EMPHASIS,   QVariant(3));
    info.set(Kwave::INF_COPYRIGHTED,     QVariant(1));
    info.set(Kwave::INF_ORIGINAL,        QVariant(1));
    meta_data.replace(Kwave::MetaDataList(info));

    // create a multi track reader
    QList<unsigned int> track_list;
    track_list.append(0);
    track_list.append(1);
    sample_index_t first = 0;
    sample_index_t last  = test_length - 1;
    Kwave::MultiTrackReader src(Kwave::SinglePassForward,
	manager, track_list, first, last);

    // create an encoder
    MP3Encoder encoder;

    // pass the data through the encoder
    bool succeeded = encoder.encode(this, src, dst, meta_data);

    // check return code
    if (succeeded) {
	KMessageBox::information(this, i18n(
	    "Congratulation, the test was successful!"));
    } // else: the plugin has already shown an error message
}

/***************************************************************************/
void Kwave::MP3EncoderDialog::encoderHelp()
{
    // show complete help
    QString program = edPath->text().simplified();
    QString param   = QString(edEncoderHelp->text()).simplified();

    QString text    = callWithParam(program, param);

    KMessageBox::information(this, text);
}

/***************************************************************************/
QString Kwave::MP3EncoderDialog::callWithParam(const QString &path,
                                               const QString &param)
{
    QStringList params(param);

    // set hourglass cursor
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    QProcess process;
    process.setProcessChannelMode(QProcess::MergedChannels);
    process.start(path, params);
    process.waitForStarted();
    if (process.state() != QProcess::NotRunning) process.waitForFinished();

    QString text = QString::fromLocal8Bit(process.readAllStandardOutput());
    qDebug("stdout output: %s", DBG(text));

    // remove hourglass
    QApplication::restoreOverrideCursor();

    return text;
}

/***************************************************************************/
QString Kwave::MP3EncoderDialog::encoderVersion(const QString &path,
                                                const QString &param)
{
    QString text = callWithParam(path, param);

    QStringList lines = text.split(QLatin1Char('\n'));

    // take the first non-zero line
    while (lines.count() && !lines.first().simplified().length())
	lines.removeFirst();

    return (!lines.isEmpty()) ? lines.first().simplified() : QString();
}

/***************************************************************************/
QString Kwave::MP3EncoderDialog::searchPath(const QString &program)
{
    const QFile::Permissions executable =
	(QFile::ExeOwner | QFile::ExeUser | QFile::ExeGroup | QFile::ExeOther);
#ifdef Q_OS_WIN
    const QLatin1Char separator = QLatin1Char(';');
#else
    const QLatin1Char separator = QLatin1Char(':');
#endif
    QStringList path =
	_(qgetenv("PATH")).split(separator, QString::SkipEmptyParts);

    QFileInfo f(program);
    QString d = f.path();
    if (d.length()) path.prepend(d);

    foreach (const QString &dir, path) {
	QString p = dir;
	if (!p.endsWith(QDir::separator()))
	    p += QDir::separator();
	p += f.fileName();

	QFile file(p);
	qDebug("testing '%s'", DBG(p));
	if (file.exists() && (file.permissions() & executable)) {
	    // found it :)
	    return p;
	}
    }

    return program;
}

/***************************************************************************/
void Kwave::MP3EncoderDialog::updateEncoderInfo()
{
    int index = cbProgram->currentIndex();
    QString title;

    if (index >= Kwave::toInt(ELEMENTS_OF(g_predefined_settings))) {
	title = PRESET_NAME_USER_DEFINED;
    }

    // detect by using the currently selected path
    if (!title.length()) {
	// first try with user defined full path
	QString name    = g_predefined_settings[index].m_name;
	QString program = QFileInfo(edPath->text().simplified()).filePath();
	QString param   = edVersionInfo->text().simplified();
	QString version = encoderVersion(program, param);
	if (version.length() >= name.length()) {
	    title = version;
	} else {
	    // fallback: detect by using list of predefined settings
	    QString p  = g_predefined_settings[index].m_info.m_version;
	    program    = searchPath(g_predefined_settings[index].m_path);
	    version    = encoderVersion(program, p);
	    if (version.length() >= name.length())
		title = version;
	}
    }

    cbProgram->setItemText(index, title);
}

//***************************************************************************
void Kwave::MP3EncoderDialog::invokeHelp()
{
    KHelpClient::invokeHelp(_("plugin_sect_codec_mp3"));
}

/***************************************************************************/
/***************************************************************************/
