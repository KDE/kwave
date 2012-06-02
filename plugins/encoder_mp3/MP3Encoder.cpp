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

#include "MP3EncoderPlugin.h"
#include "MP3Encoder.h"

/***************************************************************************/
static const struct {
    FileProperty property;
    const char *parameter;
} supported_properties[] = {
	{ INF_NAME,         "--tt %1" },
// 	{ INF_VERSION,      "VERSION" },
	{ INF_ALBUM,        "--tl %1" },
	{ INF_TRACK,        "--tn %1" },
	{ INF_AUTHOR,       "--ta %1" },
// 	{ INF_PERFORMER,    "PERFORMER" },
// 	{ INF_COPYRIGHT,    "COPYRIGHT" },
// 	{ INF_LICENSE,      "LICENSE" },
// 	{ INF_ORGANIZATION, "ORGANIZATION" },
// 	{ INF_SUBJECT,      "DESCRIPTION" },
	{ INF_GENRE,        "--tg %1" },
// 	{ INF_SOURCE,       "LOCATION" },
// 	{ INF_CONTACT,      "CONTACT" },
// 	{ INF_ISRC,         "ISRC" },
// 	{ INF_SOFTWARE,     "ENCODER" },
// 	{ INF_CREATION_DATE,"DATE" },
// 	{ INF_VBR_QUALITY,  "VBR_QUALITY" },
	{ INF_MIMETYPE,     0 }
};

/***************************************************************************/
Kwave::MP3Encoder::MP3Encoder()
    :Encoder(), m_lock(), m_dst(0), m_process(this), m_program(), m_params()
{
    /* included in KDE: */
    addMimeType("audio/x-mpga",   i18n("MPEG layer I audio"),
                "*.mpga *.mpg *.mp1");
    addMimeType("audio/x-mp2",    i18n("MPEG layer II audio"), "*.mp2");
    addMimeType("audio/x-mp3",    i18n("MPEG layer III audio"), "*.mp3");

    /* like defined in RFC3003 */
    addMimeType("audio/mpeg",     i18n("MPEG audio"), "*.mpga *.mpg *.mp1");
    addMimeType("audio/mpeg",     i18n("MPEG layer II audio"), "*.mp2");
    addMimeType("audio/mpeg",     i18n("MPEG layer III audio"), "*.mp3");

    // NOTE: all mime types above should be recognized in the
    //       fileinfo plugin!

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

    for (unsigned int i = 0; i < sizeof(supported_properties) /
                                 sizeof(supported_properties[0]); ++i)
    {
	list.append(supported_properties[i].property);
    }

    return list;
}


/***************************************************************************/
bool Kwave::MP3Encoder::encode(QWidget *widget, MultiTrackReader &src,
                               QIODevice &dst,
                               const Kwave::MetaDataList &meta_data)
{
    bool result = true;

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
	int res = Kwave::MessageBox::warningContinueCancel(
	    widget,
	    i18n("The file format you have chosen supports only mono or"
		 "stereo. This file will be mixed down to stereo when"
		 "saving."),
	    QString(), QString(), QString(),
	    "accept_down_mix_on_export");
	if (res != KMessageBox::Continue) {
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

    // encode ID3 tags with id3lib into the output stream
    // TODO ...

    // mandantory audio input format and encoding options
    m_params.append("-r"); // input is raw audio

    // supported sample rates [kHz]
    // 8 / 11.025 / 12 / 16 / 22.05 / 24 /32 / 44.1 / 48
    // if our rate is not supported, lame automatically resamples with the
    // next higher supported rate
    m_params.append(QString("-s %1").arg(rate));

    // bits per sample, supported are: 8 / 16 / 24 / 32
    m_params.append(QString("-bitwidth=%1").arg(bits));

    m_params.append("--signed");        // always signed

#if Q_BYTE_ORDER == Q_BIG_ENDIAN
    m_params.append("--big-endian");
#else
    m_params.append("--little-endian");
#endif
    // encode one track as "mono" and two tracks as "joint-stereo"
    m_params.append(QString("-m%1").arg((tracks == 1) ? "m" : "j"));

    m_params.append("--silent");
    m_params.append("-"); // infile  = stdin
    m_params.append("-"); // outfile = stdout

    m_program = "lame";

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
