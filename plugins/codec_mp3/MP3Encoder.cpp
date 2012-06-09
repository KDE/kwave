/*************************************************************************
         MP3Encoder.cpp  -  export of MP3 data via "lame"
                             -------------------
    begin                : Sat May 19 2012
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

#include <id3/globals.h>
#include <id3/tag.h>
#include <id3/misc_support.h>

#include <QByteArray>
#include <QList>

#include <klocale.h>
#include <kmimetype.h>
#include <kapplication.h>
#include <kglobal.h>

#include "libkwave/FileInfo.h"
#include "libkwave/MetaDataList.h"
#include "libkwave/MessageBox.h"
#include "libkwave/MixerMatrix.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleReader.h"

#include "ID3_QIODeviceWriter.h"
#include "MP3CodecPlugin.h"
#include "MP3Encoder.h"
#include "MP3EncoderSettings.h"

/***************************************************************************/
static const struct {
    FileProperty property;
    ID3_FrameID id;
} supported_ids[] = {
    { INF_COMMENTS,      ID3FID_COMMENT    }, // Comments
    { INF_CONTACT,       ID3FID_OWNERSHIP  }, // contact info for creator
    { INF_ALBUM,         ID3FID_ALBUM      }, // Album/Movie/Show title
    { INF_GENRE,         ID3FID_CONTENTTYPE}, // Content type (Genre)
    { INF_COPYRIGHT,     ID3FID_COPYRIGHT},   // Copyright message.
    { INF_TECHNICAN,     ID3FID_ENCODEDBY},   // Encoded by.
    { INF_NAME,          ID3FID_TITLE},   // Title/songname/content description
    { INF_ANNOTATION,    ID3FID_SUBTITLE},// Subtitle/Description refinement
    { INF_MEDIUM,        ID3FID_MEDIATYPE},   // Medium type
    { INF_PERFORMER,     ID3FID_LYRICIST},    // Lyricist/Text writer
    { INF_AUTHOR,        ID3FID_ORIGARTIST},  // Original artist(s)/performer(s)
    { INF_LICENSE,       ID3FID_FILEOWNER},   // File owner/licensee
    { INF_PERFORMER,     ID3FID_LEADARTIST},  // Lead performer(s)/Soloist(s).
    { INF_VERSION,       ID3FID_MIXARTIST},// Interpreted, remixed / modified by
    { INF_CD,            ID3FID_PARTINSET},   // Part of a set
    { INF_ORGANIZATION,  ID3FID_PRODUCEDNOTICE}, // Produced notice
    { INF_ORGANIZATION,  ID3FID_PUBLISHER},   // Publisher
    { INF_TRACK,         ID3FID_TRACKNUM},    // Track number/Position in set
    { INF_ISRC,          ID3FID_ISRC},        // ISRC
    { INF_VERSION,       ID3FID_SETSUBTITLE}, // Set subtitle
    { INF_ANNOTATION,    ID3FID_USERTEXT}     // User defined text information
};

#define ELEMENTS_OF(__x__) (sizeof(__x__) / sizeof(__x__[0]))

/***************************************************************************/
Kwave::MP3Encoder::MP3Encoder()
    :Encoder(), m_lock(), m_dst(0), m_process(this), m_program(), m_params()
{
    LOAD_MIME_TYPES;

    connect(&m_process, SIGNAL(readyReadStandardOutput()),
	    this, SLOT(dataAvailable()));
}

/***************************************************************************/
Kwave::MP3Encoder::~MP3Encoder()
{
}

/***************************************************************************/
Encoder *Kwave::MP3Encoder::instance()
{
    return new MP3Encoder();
}

/***************************************************************************/
QList<FileProperty> Kwave::MP3Encoder::supportedProperties()
{
    QList<FileProperty> list;
    for (unsigned int i = 0; i < ELEMENTS_OF(supported_ids); ++i)
	list.append(supported_ids[i].property);
    return list;
}

/***************************************************************************/
void Kwave::MP3Encoder::encodeID3Tags(const Kwave::MetaDataList &meta_data,
                                      ID3_Tag &tag)
{
    const FileInfo info(meta_data);

    for (unsigned int i = 0; i < ELEMENTS_OF(supported_ids); ++i) {
	if (!info.contains(supported_ids[i].property)) continue;

	ID3_Frame *frame = new ID3_Frame;
	Q_ASSERT(frame);
	if (!frame) break;

	QVariant value = info.get(supported_ids[i].property);
	QString  str   = value.toString();

	// encode in UCS16
	frame->SetID(supported_ids[i].id);
	ID3_Field *field = frame->GetField(ID3FN_TEXT);
	Q_ASSERT(field);
	if (field) {
	    field->SetEncoding(ID3TE_UTF16);
	    field->Set(static_cast<const unicode_t *>(str.utf16()));
	}

	tag.AttachFrame(frame);
    }

    tag.Strip();
    tag.Update();
}

#define OPTION(__field__) \
    if (settings.__field__.length()) m_params.append(settings.__field__)

#define OPTION_P(__field__, __value__) \
    if (settings.__field__.length()) \
	m_params.append(settings.__field__.arg(__value__))

/***************************************************************************/
bool Kwave::MP3Encoder::encode(QWidget *widget, MultiTrackReader &src,
                               QIODevice &dst,
                               const Kwave::MetaDataList &meta_data)
{
    bool result = true;
    ID3_Tag id3_tag;
    Kwave::MP3EncoderSettings settings;

    settings.load();

    ID3_TagType id3_tag_type = ID3TT_ID3V2;
    id3_tag.SetSpec(ID3V2_LATEST);

    qDebug("MP3Encoder::encode()");
    const FileInfo info(meta_data);

    // get info: tracks, sample rate
    const unsigned int tracks     = src.tracks();
    const unsigned int length     = src.last() - src.first() + 1;
    const unsigned int bits       = qBound(8U, ((info.bits() + 7) & ~0x7), 32U);
    const double       rate       = info.rate();
    const unsigned int out_tracks = qMax(tracks, 2U);

    // when encoding track count > 2, show a warning that we will mix down
    // to stereo
    if (tracks > 2) {
	if (Kwave::MessageBox::warningContinueCancel(
	    widget,
	    i18n("The file format you have chosen supports only mono or"
		 "stereo. This file will be mixed down to stereo when"
		 "saving."),
	    QString(), QString(), QString(),
	    "accept_down_mix_on_export") != KMessageBox::Continue)
	{
	    return false;
	}
    }

    // open the output device
    if (!dst.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
	Kwave::MessageBox::error(widget,
	    i18n("Unable to open the file for saving!"));
	return false;
    }

    m_dst  = &dst;
    m_params.clear();

    // encode meta data into with id3lib
    ID3_QIODeviceWriter id3_writer(dst);
    encodeID3Tags(meta_data, id3_tag);

    OPTION(m_flags.m_prepend);          // optional paramters at the very start

    // mandantory audio input format and encoding options
    OPTION(m_input.m_raw_format);       // input is raw audio
    OPTION(m_input.m_byte_order);       // byte swapping
    OPTION(m_input.m_signed);           // signed sample format

    // supported sample rates [kHz]
    // 8 / 11.025 / 12 / 16 / 22.05 / 24 /32 / 44.1 / 48
    // if our rate is not supported, lame automatically resamples with the
    // next higher supported rate
    OPTION_P(m_format.m_sample_rate, rate); // sample rate

    // bits per sample, supported are: 8 / 16 / 24 / 32
    OPTION_P(m_format.m_bits_per_sample, bits);

    // encode one track as "mono" and two tracks as "joint-stereo"
    if (tracks == 1) {
	OPTION(m_format.m_channels.m_mono);
    } else {
	OPTION(m_format.m_channels.m_stereo);
    }

    // nominal / lower / upper bitrate
    int bitrate_min =   8;
    int bitrate_max = 320;
    int bitrate_nom = 128;
    if (info.contains(INF_BITRATE_NOMINAL)) {
	// nominal bitrate => use ABR mode
	bitrate_nom = info.get(INF_BITRATE_NOMINAL).toInt() / 1000;
	bitrate_nom = qBound(bitrate_min, bitrate_nom, bitrate_max);
	OPTION_P(m_quality.m_bitrate.m_avg, bitrate_nom);
    }
    if (info.contains(INF_BITRATE_LOWER)) {
	int bitrate = info.get(INF_BITRATE_LOWER).toInt() / 1000;
	bitrate_min = qBound(bitrate_min, bitrate, bitrate_nom);
	OPTION_P(m_quality.m_bitrate.m_min, bitrate_min);
    }
    if (info.contains(INF_BITRATE_UPPER)) {
	int bitrate = info.get(INF_BITRATE_UPPER).toInt() / 1000;
	bitrate_max = qBound(bitrate_nom, bitrate, bitrate_max);
	OPTION_P(m_quality.m_bitrate.m_max, bitrate_max);
    }
    //  INF_MPEG_LAYER,          /**< MPEG Layer, I/II/III */
    //  INF_MPEG_MODEEXT,        /**< MPEG mode extension */
    //  INF_MPEG_VERSION,        /**< MPEG version */

    /* MPEG emphasis mode */
    if (info.contains(INF_MPEG_EMPHASIS)) {
	int emphasis = info.get(INF_MPEG_EMPHASIS).toInt();
	switch (emphasis) {
	    case  1:
		OPTION(m_encoding.m_emphasis.m_50_15ms);   // 1 = 50/15ms
		break;
	    case  3:
		OPTION(m_encoding.m_emphasis.m_ccit_j17); // 3 = CCIT J.17
		break;
	    case  0: /* FALLTHROUGH */
	    default:
		OPTION(m_encoding.m_emphasis.m_none);      // 0 = none
		break;
	}
    }

    OPTION(m_encoding.m_noise_shaping); // noise shaping settings
    OPTION(m_encoding.m_compatibility); // compatibility options

    if (info.contains(INF_COPYRIGHTED) && info.get(INF_COPYRIGHTED).toBool()) {
	OPTION(m_flags.m_copyright);     // copyrighted
    }

    if (info.contains(INF_ORIGINAL) && !info.get(INF_ORIGINAL).toBool()) {
	OPTION(m_flags.m_original);     // original
    }

    OPTION(m_flags.m_protect);          // CRC protection
    OPTION(m_flags.m_append);           // optional paramters at the end

    m_params.append("-"); // infile  = stdin
    m_params.append("-"); // outfile = stdout

    m_program = settings.m_path;

    m_process.setReadChannel(QProcess::StandardOutput);

    m_process.start(m_program, m_params);
    QString stdError;
    if (!m_process.waitForStarted()) {
	qWarning("cannot start program '%s'", m_program.toLocal8Bit().data());
	stdError = QString::fromLocal8Bit(m_process.readAllStandardError());
	qWarning("stderr output: %s", stdError.toLocal8Bit().data());

	m_process.waitForFinished();
	result = false;
    }

    // if a ID3v2 tag is requested, the tag comes at the start
    if (id3_tag_type == ID3TT_ID3V2)
	id3_tag.Render(id3_writer, id3_tag_type);

    // MP3 supports only mono and stereo, prepare a mixer matrix
    // (not used in case of tracks <= 2)
    Kwave::MixerMatrix mixer(tracks, out_tracks);

    // read in from the sample readers
    const unsigned int buf_len = sizeof(m_write_buffer);
    const int bytes_per_sample = bits / 8;

    unsigned int rest = length;
    Kwave::SampleArray in_samples(tracks);
    Kwave::SampleArray out_samples(tracks);

    while (result && rest && (m_process.state() != QProcess::NotRunning)) {
	unsigned int x;
	unsigned int y;

	// merge the tracks into the sample buffer
	quint8 *dst = &(m_write_buffer[0]);
	unsigned int count = buf_len / (bytes_per_sample * tracks);
	if (rest < count) count = rest;

	unsigned int written = 0;
	for (written = 0; written < count; written++) {
	    sample_t *src_buf = 0;

	    // fill input buffer with samples
	    for (x = 0; x < tracks; x++) {
		in_samples[x] = 0;
		SampleReader *stream = src[x];
		Q_ASSERT(stream);
		if (!stream) continue;

		if (!stream->eof()) (*stream) >> in_samples[x];
	    }

	    if (tracks > 2) {
		// multiply matrix with input to get output
		for (y = 0; y < out_tracks; y++) {
		    double sum = 0;
		    for (x = 0; x < tracks; x++) {
			sum += static_cast<double>(in_samples[x]) * mixer[x][y];
		    }
		    out_samples[y] = static_cast<sample_t>(sum);
		}

		// use output of the matrix
		src_buf = out_samples.data();
	    } else {
		// use input buffer directly
		src_buf = in_samples.data();
	    }

	    // sample conversion from 24bit to raw PCM, native endian
	    for (y = 0; y < out_tracks; y++) {
		sample_t s = *(src_buf++);
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
		// big endian
		if (bits >= 8)
		    *(dst++) = (s >> 16);
		if (bits > 8)
		    *(dst++) = (s >> 8);
		if (bits > 16)
		    *(dst++) = (s & 0xFF);
		if (bits > 24)
		    *(dst++) = 0x00;
#else
		// little endian
		if (bits > 24)
		    *(dst++) = 0x00;
		if (bits > 16)
		    *(dst++) = s & 0xFF;
		if (bits > 8)
		    *(dst++) = (s >> 8);
		if (bits >= 8)
		    *(dst++) = (s >> 16);
#endif
	    }
	}

	// write out to the stdin of the external process
	unsigned int bytes_written = m_process.write(
	    reinterpret_cast<char *>(&(m_write_buffer[0])),
	    written * (bytes_per_sample * tracks)
	);

	// break if eof reached or disk full
	if (!bytes_written) break;

	// wait for write to take all data...
	m_process.waitForBytesWritten();

	// abort if the user pressed cancel
	// --> this would leave a corrupted file !!!
	if (src.isCanceled()) break;

	Q_ASSERT(rest >= written);
	rest -= written;
    }

    // flush and close the write channel
    m_process.closeWriteChannel();

    // wait until the process has finished
    qDebug("wait for finish of the process");
    while (m_process.state() != QProcess::NotRunning) {
	m_process.waitForFinished(100);
	if (src.isCanceled()) break;
    }

    int exit_code = m_process.exitCode();
    qDebug("exit code=%d", exit_code);
    if (!result || (exit_code != 0)) {
	result = false;
	stdError = QString::fromLocal8Bit(m_process.readAllStandardError());
	qWarning("stderr output: %s", stdError.toLocal8Bit().data());

	Kwave::MessageBox::error(widget,
	    i18nc("%1=name of the external program, %2=stderr of the program",
	    "An error occurred while calling the external encoder '%1':\n\n%2",
	   m_program, stdError
	));
    }

    // if a ID3v1 tag is requested, the tag comes at the end
    if (id3_tag_type != ID3TT_ID3V2)
	id3_tag.Render(id3_writer, id3_tag_type);

    {
	QMutexLocker _lock(&m_lock);
	m_dst  = 0;
	dst.close();
    }

    return result;
}

/***************************************************************************/
void Kwave::MP3Encoder::dataAvailable()
{
    while (m_process.bytesAvailable()) {
	qint64 len = m_process.read(&(m_read_buffer[0]), sizeof(m_read_buffer));
	if (len) {
	    QMutexLocker _lock(&m_lock);
	    if (m_dst) m_dst->write(&(m_read_buffer[0]), len);
	}
    }
}

/***************************************************************************/
#include "MP3Encoder.moc"
/***************************************************************************/
