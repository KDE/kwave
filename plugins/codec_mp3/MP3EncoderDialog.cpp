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
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QLineEdit>
#include <QtGlobal>

#include "libkwave/MessageBox.h"
#include "libgui/KwaveFileDialog.h"

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
	"LAME",                        // name
	"lame" EXE_SUFFIX,             // path
	{
	    "-r",                      // raw format
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
	    "--big-endian",            // byte order
#else
	    "--little-endian",         // byte order
#endif
	    "--signed"                 // signed
	},
	{
	    "-s %1",                   // sample rate
	    "-bitwidth=%1",            // bits per sample
	    {
		"-m m",                // mono
		"-m j"                 // stereo
	    }
	},
	{
	    {
		"--abr %1",            // average bitrate
		"-b %1",               // minimum bitrate
		"-B %1"                // maximum bitrate
	    }
	},
	{
	    {
		"-e n",                // no emphasis
		"-e 5",                // 50/15ms
		"-e c"                 // CCIT J17
	    },
	    "-q 2",                    // noise shaping
	    "--strictly-enforce-ISO"   // compatibility
	},
	{
	    "-c",                      // copyrighted
	    "-o",                      // original
	    "-p",                      // protect
	    "",                        // prepended
	    "--silent"                 // appended
	}
    },
    /***********************************************************************/
    {
	"TwoLAME",                     // name
	"twolame" EXE_SUFFIX,          // path
	{
	    "--raw-input",             // raw format
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
	    "--byte-swap",             // byte order
#else
	    "",                        // byte order
#endif
	    ""                         // signed
	},
	{
	    "--samplerate %1",         // sample rate
	    "--samplesize %1",         // bits per sample
	    {
		"--channels 1 --mode mono",  // mono
		"--channels 2 --mode joint"  // stereo
	    }
	},
	{
	    {
		"--bitrate %1",        // average bitrate
		"",                    // minimum bitrate
		"--max-bitrate %1"     // maximum bitrate
	    }
	},
	{
	    {
		"--deemphasis n",      // no emphasis
		"--deemphasis 5",      // 50/15ms
		"--deemphasis c"       // CCIT J17
	    },
	    "",                        // noise shaping
	    ""                         // compatibility
	},
	{
	    "--copyright",             // copyrighted
	    "--original",              // original
	    "--protect",               // protect
	    "",                        // prepended
	    "--quiet"                  // appended
	}
    },
    /***********************************************************************/
    {
	"tooLAME",                     // name
	"toolame" EXE_SUFFIX,          // path
	{
	    "",                        // raw format
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
	    "-x",                      // byte order
#else
	    "",                        // byte order
#endif
	    ""                         // signed
	},
	{
	    "-s %1",                   // sample rate
	    "-bitwidth=%1",            // bits per sample
	    {
		"-m m",                // mono
		"-m j"                 // stereo
	    }
	},
	{
	    {
		"-b %1",               // average bitrate
		"",                    // minimum bitrate
		""                     // maximum bitrate
	    }
	},
	{
	    {
		"-d n",                // no emphasis
		"-d 5",                // 50/15ms
		"-d c"                 // CCIT J17
	    },
	    "",                        // noise shaping
	    ""                         // compatibility
	},
	{
	    "-c",                      // copyrighted
	    "-o",                      // original
	    "-e",                      // protect
	    "",                        // prepended
	    "-t 0"                     // appended
	}
    }
};

/***************************************************************************/

/**
 * load a text field with the content of the current settings
 * @param __field__ member within m_settings
 * @param __control__ the QLineEdit to fill with the value
 */
#define LOAD(__field__, __control__) __control__->setText(m_settings.__field__)

/**
 * take the content of a QLineEdit and save it back into m_settings
 * @param __field__ member within m_settings
 * @param __control__ the QLineEdit to fill with the value
 */
#define SAVE(__field__, __control__) \
    m_settings.__field__ = QString(__control__->text()).simplified()

/**
 * connect the editingFinished() signal of a QLineEdit to our slot
 * switchToUserDefined()
 * @param __control__ the QLineEdit to handle
 */
#define CONNECT(__control__) \
    connect(__control__, SIGNAL(editingFinished()), \
    this, SLOT(switchToUserDefined()))

/**
 * check if the content of a QLineEdit matches the corresponding member
 * of some "settings" record
 * @param __field__ member within m_settings
 * @param __control__ the QLineEdit to compare against
 * @return the bool variable "match" is updated (logical AND)
 */
#define CHECK(__field__, __control__) match &= \
    (settings.__field__ == QString(__control__->text()).simplified())

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
    for (unsigned int i = 0; i < ELEMENTS_OF(g_predefined_settings); i++)
	cbProgram->addItem(g_predefined_settings[i].m_name, QVariant(i));
    cbProgram->addItem(PRESET_NAME_USER_DEFINED, QVariant(-1));

    // load the saved settings from the config file
    m_settings.load();
    load();

    // connect the combo box of the program selection
    connect(cbProgram, SIGNAL(activated(int)),
	    this, SLOT(selectProgram(int)));

    // standard actions, reset / reset to defaults etc...
    connect(buttonBox, SIGNAL(clicked(QAbstractButton *)),
	    this, SLOT(buttonClicked(QAbstractButton *)));

    // auto-detec button
    connect(btDetect, SIGNAL(clicked()), this, SLOT(autoDetect()));

    // search for program (file browser)
    connect(btSearch, SIGNAL(clicked()), this, SLOT(browseFile()));

    // test setup
    connect(btTest, SIGNAL(clicked()),   this, SLOT(testSettings()));

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
	    m_settings = g_predefined_settings[i];
	    cbProgram->setCurrentIndex(i);
	    use_preset = true;
	    break;
	}
    }
    if ((!use_preset) && (cbProgram->currentIndex() != static_cast<int>(i))) {
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
}

/***************************************************************************/
void Kwave::MP3EncoderDialog::save()
{
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

    m_settings.save();
}

/***************************************************************************/
void Kwave::MP3EncoderDialog::selectProgram(int index)
{
    if (index < 0) return;
    if (index >= static_cast<int>(ELEMENTS_OF(g_predefined_settings))) return;

    m_settings = g_predefined_settings[index];
    load();
}

/***************************************************************************/
void Kwave::MP3EncoderDialog::switchToUserDefined()
{
    for (unsigned i = 0; i < ELEMENTS_OF(g_predefined_settings); i++) {
	const Kwave::MP3EncoderSettings &settings =
	    g_predefined_settings[i];
	bool match = true;

	match &= bool(settings.m_path.contains(edPath->text().simplified(),
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

	if (match) {
	    // found a match against known preset
	    cbProgram->setCurrentIndex(i);
	    return;
	}
    }

    // fallback: "user defined"
    cbProgram->setCurrentIndex(ELEMENTS_OF(g_predefined_settings));
}

/***************************************************************************/
void Kwave::MP3EncoderDialog::buttonClicked(QAbstractButton *button)
{
    if (!button || !buttonBox) return;
    switch (buttonBox->standardButton(button)) {
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
    const QFile::Permissions executable =
	(QFile::ExeOwner | QFile::ExeUser | QFile::ExeGroup | QFile::ExeOther);
    QStringList path =
	QString(qgetenv("PATH")).split(":", QString::SkipEmptyParts);

    for (unsigned i = 0; i < ELEMENTS_OF(g_predefined_settings); i++) {
	const QString prog = g_predefined_settings[i].m_path;

	foreach (const QString &dir, path) {
	    QString p = dir;
	    if (!p.endsWith(QDir::separator()))
		p += QDir::separator();
	    p += prog;

	    QFile f(p);
	    qDebug("testing '%s'", p.toLocal8Bit().data());
	    if (f.exists() && (f.permissions() & executable)) {
		// found it :)
		cbProgram->setCurrentIndex(i);
		edPath->setText(p);
		return;
	    }
	}
    }
}

/***************************************************************************/
void Kwave::MP3EncoderDialog::browseFile()
{
    QString mask = QString("*");
#ifdef EXECUTABLE_SUFFIX
    mask += QString(EXECUTABLE_SUFFIX);
#endif
    KwaveFileDialog dlg("kfiledialog:///kwave_mp3_encoder", "", this,
	true, "file:/" + edPath->text().simplified(), mask);
    dlg.setKeepLocation(true);
    dlg.setOperationMode(KFileDialog::Opening);
    dlg.setCaption(i18n("Select MP3 Encoder"));
    dlg.setUrl(KUrl("file:/usr/bin/"));
    if (dlg.exec() != QDialog::Accepted) return;

    edPath->setText(dlg.selectedFile());
}

/***************************************************************************/
void Kwave::MP3EncoderDialog::testSettings()
{
    qDebug("%s",__FUNCTION__);
    Kwave::MessageBox::sorry(this,
	i18n("This is not implemented yet."),
	i18n("Sorry"));
}

/***************************************************************************/
#include "MP3EncoderDialog.moc"
/***************************************************************************/
/***************************************************************************/
