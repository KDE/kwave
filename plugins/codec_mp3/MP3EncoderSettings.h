/***************************************************************************
   MP3EncoderSettings.h  -  settings for configuring the MP3 encoer
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

#ifndef MP3_ENCODER_SETTINGS_H
#define MP3_ENCODER_SETTINGS_H

#include <QtGlobal>
#include <QString>

namespace Kwave
{

    typedef struct MP3EncoderSettings
    {

        /** load from a config file */
        void load();

        /** save to a config file */
        void save();

        QString m_name;                 /**< name of the program (preset) */
        QString m_path;                 /**< path to the executable       */

        struct {
            QString m_raw_format;       /**< raw format                   */
            QString m_byte_order;       /**< byte order                   */
            QString m_signed;           /**< signed format                */
        } m_input;

        struct {
            QString m_sample_rate;      /**< sample rate [1/sec]          */
            QString m_bits_per_sample;  /**< bits per sample              */
            struct {
                QString m_mono;         /**< mono                         */
                QString m_stereo;       /**< stereo                       */
            } m_channels;
        } m_format;

        struct {
            struct {
                QString m_avg;          /**< average bitrate             */
                QString m_min;          /**< minimum bitrate             */
                QString m_max;          /**< maximum bitrage             */
            } m_bitrate;
        } m_quality;

        struct {
            struct {
                QString m_none;         /**< no preemphasis              */
                QString m_50_15ms;      /**< 50/15ms                     */
                QString m_ccit_j17;     /**< CCIT J17                    */
            } m_emphasis;
            QString m_noise_shaping;    /**< noise shaping               */
            QString m_compatibility;    /**< compatibility               */
        } m_encoding;

        struct {
            QString m_copyright;        /**< copyrighted                 */
            QString m_original;         /**< original                    */
            QString m_protect;          /**< protect, CRC                */

            QString m_prepend;          /**< additional to prepended     */
            QString m_append;           /**< additional to append        */
        } m_flags;

        struct {
            QString m_help;             /**< encoder help                */
            QString m_version;          /**< encoder version             */
        } m_info;

    } MP3EncoderSettings;

}

#endif /* MP3_ENCODER_SETTINGS_H */

//***************************************************************************
//***************************************************************************
