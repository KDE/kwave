/*************************************************************************
         RecordParams.h  -  holds parameters of the record plugin
                             -------------------
    begin                : Thu Sep 04 2003
    copyright            : (C) 2003 by Thomas Eschenbacher
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

#ifndef _RECORD_PARAMS_H_
#define _RECORD_PARAMS_H_

#include "config.h"
#include <qstringlist.h>

class RecordParams {

public:

    /** Constructor, initializes everything with defaults */
    RecordParams();

    /** Destructor */
    virtual ~RecordParams();

    /**
     * Parse from a QStringList
     * @param list the QStringList to parse
     * @return zero or -EINVAL if failed
     */
    virtual int fromList(const QStringList &list);

    /** Parse into a QStringList */
    virtual QStringList toList() const;

    bool pre_record_enabled;		/**< pre-record: feature enabled */
    unsigned int pre_record_time;	/**< pre-record: time in seconds */

    bool record_time_limited;		/**< record time: limited */
    unsigned int record_time;		/**< record time: limit in seconds */

    bool record_trigger_enabled;	/**< record trigger: feature enabled */
    unsigned int record_trigger;	/**< record trigger level in percent */

    bool amplification_enabled;		/**< amplification: feature enabled */
    int amplification;			/**< amplification: value in decibel */

    bool agc_enabled;			/**< agc: feature enabled */
    unsigned int agc_decay;		/**< agc: decay in milliseconds */

    bool fade_in_enabled;		/**< fade in: feature enabled */
    unsigned int fade_in_time;		/**< fade in: time in milliseconds */

    bool fade_out_enabled;		/**< fade out: feature enabled */
    unsigned int fade_out_time;		/**< fade out: time in milliseconds */

    QString device_name;		/**< name of the input device */
    unsigned int tracks;		/**< number of tracks */
    double sample_rate;			/**< sample rate in samples/second */
    int compression;			/**< compression index or -1 */
    unsigned int bits_per_sample;	/**< resolution in bits per sample */
    int sample_format;			/**< sample format index */

    unsigned int buffer_count;		/**< number of buffers */
    unsigned int buffer_size;		/**< power of the record buffer size */

    bool display_level_meter;		/**< enable level meter display */
    bool display_oscilloscope;		/**< enable oscilloscope display */
    bool display_fft;			/**< enable fft display */
    bool display_overview;		/**< enable overview display */
};

#endif /* _RECORD_PARAMS_H_ */
