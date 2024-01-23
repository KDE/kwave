/***************************************************************************
 MP3EncoderSettings.cpp  -  settings for configuring the MP3 encoer
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

#include <QDir>
#include <QLatin1Char>

#include <KConfigGroup>
#include <KSharedConfig>

#include "libkwave/String.h"

#include "MP3EncoderSettings.h"

/** name of the section in the config file */
#define MP3_ENCODER_CONFIG_GROUP "MP3_Encoder_Settings"

/**
 * load from config file
 * @param field receives the value
 * @param key identifier in the config section
 */
#define LOAD(field, key) field = \
    cfg.readEntry(key, field)

/**
 * save to config file
 * @param field value to save
 * @param key identifier in the config section
 */
#define SAVE(field, key) \
    cfg.writeEntry(key, sanitized(field))

/***************************************************************************/
/**
 * Makes sure that the string does only contain allowed characters,
 * avoid misuse of the command line parameters
 * @param in potentially unsafe string
 * @return string with only digits, numbers and '-', '=' and '%'
 */
static QString sanitized(const QString &in)
{
    QString out = _("");
    QString str = in.simplified();

    for (int i = 0; i < str.length(); i++) {
        QCharRef c = str[i];
        if ( c.isLetterOrNumber() || c.isSpace() ||
             (c == QLatin1Char('-')) || (c == QLatin1Char('%')) ||
             (c == QLatin1Char('=')) || (c == QLatin1Char('.')) ||
             (c == QLatin1Char('[')) || (c == QLatin1Char(']')) ||
             (c == QDir::separator()) )
        {
            out += c;
        }
    }
    return out;
}

/***************************************************************************/
void Kwave::MP3EncoderSettings::load()
{
    KConfigGroup cfg = KSharedConfig::openConfig()->group(MP3_ENCODER_CONFIG_GROUP);

    LOAD(m_name,                           "name_______________________");
    LOAD(m_path,                           "path_______________________");

    LOAD(m_input.m_raw_format,             "input_raw_format___________");
    LOAD(m_input.m_byte_order,             "input_byte_order___________");
    LOAD(m_input.m_signed,                 "input_signed_______________");

    LOAD(m_format.m_sample_rate,           "format_sample_rate_________");
    LOAD(m_format.m_bits_per_sample,       "format_bits_per_sample_____");
    LOAD(m_format.m_channels.m_mono,       "format_channels_mono_______");
    LOAD(m_format.m_channels.m_stereo,     "format_channels_stereo_____");

    LOAD(m_quality.m_bitrate.m_avg,        "quality_bitrate_avg________");
    LOAD(m_quality.m_bitrate.m_min,        "quality_bitrate_min________");
    LOAD(m_quality.m_bitrate.m_max,        "quality_bitrate_max________");

    LOAD(m_encoding.m_emphasis.m_none,     "encoding_emphasis_none_____");
    LOAD(m_encoding.m_emphasis.m_50_15ms,  "encoding_emphasis_50_15ms__");
    LOAD(m_encoding.m_emphasis.m_ccit_j17, "encoding_emphasis_ccit_j17_");

    LOAD(m_encoding.m_noise_shaping,       "encoding_noise_shaping_____");
    LOAD(m_encoding.m_compatibility,       "encoding_compatibility_____");

    LOAD(m_flags.m_copyright,              "flags_copyright____________");
    LOAD(m_flags.m_original,               "flags_original_____________");
    LOAD(m_flags.m_protect,                "flags_protect______________");
    LOAD(m_flags.m_prepend,                "flags_prepend______________");
    LOAD(m_flags.m_append,                 "flags_append_______________");

    LOAD(m_info.m_help,                    "info_help__________________");
    LOAD(m_info.m_version,                 "info_version_______________");
}

/***************************************************************************/
void Kwave::MP3EncoderSettings::save()
{
    KConfigGroup cfg = KSharedConfig::openConfig()->group(MP3_ENCODER_CONFIG_GROUP);

    SAVE(m_name,                           "name_______________________");
    SAVE(m_path,                           "path_______________________");

    SAVE(m_input.m_raw_format,             "input_raw_format___________");
    SAVE(m_input.m_byte_order,             "input_byte_order___________");
    SAVE(m_input.m_signed,                 "input_signed_______________");

    SAVE(m_format.m_sample_rate,           "format_sample_rate_________");
    SAVE(m_format.m_bits_per_sample,       "format_bits_per_sample_____");
    SAVE(m_format.m_channels.m_mono,       "format_channels_mono_______");
    SAVE(m_format.m_channels.m_stereo,     "format_channels_stereo_____");

    SAVE(m_quality.m_bitrate.m_avg,        "quality_bitrate_avg________");
    SAVE(m_quality.m_bitrate.m_min,        "quality_bitrate_min________");
    SAVE(m_quality.m_bitrate.m_max,        "quality_bitrate_max________");

    SAVE(m_encoding.m_emphasis.m_none,     "encoding_emphasis_none_____");
    SAVE(m_encoding.m_emphasis.m_50_15ms,  "encoding_emphasis_50_15ms__");
    SAVE(m_encoding.m_emphasis.m_ccit_j17, "encoding_emphasis_ccit_j17_");

    SAVE(m_encoding.m_noise_shaping,       "encoding_noise_shaping_____");
    SAVE(m_encoding.m_compatibility,       "encoding_compatibility_____");

    SAVE(m_flags.m_copyright,              "flags_copyright____________");
    SAVE(m_flags.m_original,               "flags_original_____________");
    SAVE(m_flags.m_protect,                "flags_protect______________");
    SAVE(m_flags.m_prepend,                "flags_prepend______________");
    SAVE(m_flags.m_append,                 "flags_append_______________");

    SAVE(m_info.m_help,                    "info_help__________________");
    SAVE(m_info.m_version,                 "info_version_______________");
}

/***************************************************************************/
/***************************************************************************/
