/*************************************************************************
         WavEncoder.cpp  -  encoder for wav data
                             -------------------
    begin                : Sun Mar 10 2002
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

#include <config.h>

#include <math.h>
#include <stdlib.h>

#include <klocale.h>
#include <kmimetype.h>
#include <kglobal.h>

#include <QtCore/QByteArray>
#include <QtCore/QtEndian>
#include <QtCore/QtGlobal>

#include "libkwave/Compression.h"
#include "libkwave/FileInfo.h"
#include "libkwave/LabelList.h"
#include "libkwave/MessageBox.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleFormat.h"
#include "libkwave/SampleReader.h"
#include "libkwave/Utils.h"
#include "libkwave/VirtualAudioFile.h"

#include "WavEncoder.h"
#include "WavFileFormat.h"

/***************************************************************************/
Kwave::WavEncoder::WavEncoder()
    :Kwave::Encoder(), m_property_map()
{
    REGISTER_MIME_TYPES;
    REGISTER_COMPRESSION_TYPES;
}

/***************************************************************************/
Kwave::WavEncoder::~WavEncoder()
{
}

/***************************************************************************/
Kwave::Encoder *Kwave::WavEncoder::instance()
{
    return new Kwave::WavEncoder();
}

/***************************************************************************/
QList<Kwave::FileProperty> Kwave::WavEncoder::supportedProperties()
{
    return m_property_map.properties();
}

/***************************************************************************/
void Kwave::WavEncoder::fixAudiofileBrokenHeaderBug(QIODevice &dst,
                                                    Kwave::FileInfo &info,
                                                    unsigned int frame_size)
{
    const unsigned int length = Kwave::toUint(info.length());
    quint32 correct_size = length * frame_size;
    const int compression = info.contains(Kwave::INF_COMPRESSION) ?
                      info.get(Kwave::INF_COMPRESSION).toInt() :
	    Kwave::Compression::NONE;
    if (compression != Kwave::Compression::NONE) {
	qWarning("WARNING: libaudiofile might have produced a broken header!");
	return;
    }

    // just to be sure: at offset 36 we expect the chunk name "data"
    dst.seek(36);
    char chunk_name[5];
    memset(chunk_name, 0x00, sizeof(chunk_name));
    dst.read(&chunk_name[0], 4);
    if (strcmp("data", chunk_name)) {
	qWarning("WARNING: unexpected wav header format, check disabled");
	return;
    }

    // read the data chunk size that libaudiofile has written
    quint32 data_size;
    dst.seek(40);
    dst.read(reinterpret_cast<char *>(&data_size), 4);
    data_size = qFromLittleEndian<quint32>(data_size);
    if (data_size == length * frame_size) {
// 	qDebug("(data size written by libaudiofile is correct)");
	return;
    }

    qWarning("WARNING: libaudiofile wrote a wrong 'data' chunk size!");
    qWarning("         current=%u, correct=%u", data_size, correct_size);

    // write the fixed size of the "data" chunk
    dst.seek(40);
    data_size = qToLittleEndian<quint32>(correct_size);
    dst.write(reinterpret_cast<char *>(&data_size), 4);

    // also fix the "RIFF" size
    dst.seek(4);
    quint32 riff_size = static_cast<quint32>(dst.size()) - 4 - 4;
    riff_size = qToLittleEndian<quint32>(riff_size);
    dst.write(reinterpret_cast<char *>(&riff_size), 4);

}

/***************************************************************************/
void Kwave::WavEncoder::writeInfoChunk(QIODevice &dst, Kwave::FileInfo &info)
{
    // create a list of chunk names and properties for the INFO chunk
    QMap<Kwave::FileProperty, QVariant> properties(info.properties());
    QMap<QByteArray, QByteArray> info_chunks;
    unsigned int info_size = 0;

    for (QMap<Kwave::FileProperty, QVariant>::Iterator it = properties.begin();
	 it != properties.end(); ++it)
    {
	Kwave::FileProperty property = it.key();
	if (!m_property_map.containsProperty(property)) continue;

	QByteArray chunk_id = m_property_map.findProperty(property);
	if (info_chunks.contains(chunk_id)) continue; // already encoded

	QByteArray value = QVariant(properties[property]).toString().toUtf8();
	info_chunks.insert(chunk_id, value);
	info_size += 4 + 4 + value.length();
	if (value.length() & 0x01) info_size++;
    }

    // if there are properties to save, create a LIST chunk
    if (!info_chunks.isEmpty()) {
	quint32 size;

	// enlarge the main RIFF chunk by the size of the LIST chunk
	info_size += 4 + 4 + 4; // add the size of LIST(INFO)
	dst.seek(4);
	dst.read(reinterpret_cast<char *>(&size), 4);
	size = qToLittleEndian<quint32>(
	    qFromLittleEndian<quint32>(size) + info_size);
	dst.seek(4);
	dst.write(reinterpret_cast<char *>(&size), 4);

	// add the LIST(INFO) chunk itself
	dst.seek(dst.size());
	if (dst.pos() & 1) dst.write("\000", 1); // padding
	dst.write("LIST", 4);
	size = qToLittleEndian<quint32>(info_size - 8);
	dst.write(reinterpret_cast<char *>(&size), 4);
	dst.write("INFO", 4);

	// append the chunks to the end of the file
	for (QMap<QByteArray, QByteArray>::Iterator it = info_chunks.begin();
	     it != info_chunks.end(); ++it)
	{
	    QByteArray name  = it.key();
	    QByteArray value = it.value();

	    dst.write(name.data(), 4); // chunk name
	    size = value.length(); // length of the chunk
	    if (size & 0x01) size++;
	    size = qToLittleEndian<quint32>(size);
	    dst.write(reinterpret_cast<char *>(&size), 4);
	    dst.write(value.data(), value.length());
	    if (value.length() & 0x01) {
		const char zero = 0;
		dst.write(&zero, 1);
	    }
	}
    }
}

/***************************************************************************/
void Kwave::WavEncoder::writeLabels(QIODevice &dst,
                                    const Kwave::LabelList &labels)
{
    const unsigned int labels_count = labels.count();
    quint32 size, additional_size = 0, index, data;

    // shortcut: nothing to do if no labels present
    if (!labels_count) return;

    // easy things first: size of the cue list (has fixed record size)
    // without chunk name and chunk size
    const unsigned int size_of_cue_list =
	4 + /* number of entries */
	labels_count * (6 * 4); /* cue list entry: 6 x 32 bit */

    // now the size of the labels
    unsigned int size_of_labels = 0;
    foreach (const Kwave::Label &label, labels) {
	if (label.isNull()) continue;
	unsigned int name_len = label.name().toUtf8().size();
	if (!name_len) continue; // skip zero-length names
	size_of_labels += (3 * 4); // 3 * 4 byte
	size_of_labels += name_len;
	// padding if size is unaligned
	if (size_of_labels & 1) size_of_labels++;
    }
    if (size_of_labels) {
	size_of_labels += 4; /* header entry: 'adtl' */
	// enlarge the main RIFF chunk by the size of the LIST chunk
	additional_size += 4 + 4 + size_of_labels; // add size of LIST(adtl)
    }

    // enlarge the main RIFF chunk by the size of the cue chunks
    additional_size += 4 + 4 + size_of_cue_list; // add size of 'cue '

    dst.seek(4);
    dst.read(reinterpret_cast<char *>(&size), 4);
    size = qToLittleEndian<quint32>(
	qFromLittleEndian<quint32>(size) + additional_size);
    dst.seek(4);
    dst.write(reinterpret_cast<char *>(&size), 4);

    // seek to the end of the file
    dst.seek(dst.size());
    if (dst.pos() & 1) dst.write("\000", 1); // padding

    // add the 'cue ' list
    dst.write("cue ", 4);
    size = qToLittleEndian<quint32>(size_of_cue_list);
    dst.write(reinterpret_cast<char *>(&size), 4);

    // number of entries
    size = qToLittleEndian<quint32>(labels_count);
    dst.write(reinterpret_cast<char *>(&size), 4);

    index = 0;
    foreach (const Kwave::Label &label, labels) {
	if (label.isNull()) continue;
	/*
	 * typedef struct {
	 *     quint32 dwIdentifier; <- index
	 *     quint32 dwPosition;   <- 0
	 *     quint32 fccChunk;     <- 'data'
	 *     quint32 dwChunkStart; <- 0
	 *     quint32 dwBlockStart; <- 0
	 *     quint32 dwSampleOffset; <- label.pos()
	 * } cue_list_entry_t;
	 */
	data = qToLittleEndian<quint32>(index);
	dst.write(reinterpret_cast<char *>(&data), 4); // dwIdentifier
	data = 0;
	dst.write(reinterpret_cast<char *>(&data), 4); // dwPosition
	dst.write("data", 4);        // fccChunk
	dst.write(reinterpret_cast<char *>(&data), 4); // dwChunkStart
	dst.write(reinterpret_cast<char *>(&data), 4); // dwBlockStart
	data = qToLittleEndian<quint32>(Kwave::toUint(label.pos()));
	dst.write(reinterpret_cast<char *>(&data), 4); // dwSampleOffset
	index++;
    }

    // add the LIST(adtl) chunk
    if (size_of_labels) {
	dst.write("LIST", 4);
	size = qToLittleEndian<quint32>(size_of_labels);
	dst.write(reinterpret_cast<char *>(&size), 4);
	dst.write("adtl", 4);
	index = 0;
	foreach (const Kwave::Label &label, labels) {
	    if (label.isNull()) continue;
	    QByteArray name = label.name().toUtf8();

	    /*
	     * typedef struct {
	     *     quint32 dwChunkID;    <- 'labl'
	     *     quint32 dwChunkSize;  (without padding !)
	     *     quint32 dwIdentifier; <- index
	     *     char    dwText[];       <- label->name()
	     * } label_list_entry_t;
	     */
	    if (name.size()) {
		dst.write("labl", 4);                // dwChunkID
		data = qToLittleEndian<quint32>(name.size() + 4);

		// dwChunkSize
		dst.write(reinterpret_cast<char *>(&data), 4);
		data = qToLittleEndian<quint32>(index);

		// dwIdentifier
		dst.write(reinterpret_cast<char *>(&data), 4);
		dst.write(name.data(), name.size()); // dwText
		if (name.size() & 1) {
		    // padding if necessary
		    data = 0;
		    dst.write(reinterpret_cast<char *>(&data), 1);
		}
	    }
	    index++;
	}
    }
}

/***************************************************************************/
bool Kwave::WavEncoder::encode(QWidget *widget, Kwave::MultiTrackReader &src,
                               QIODevice &dst,
                               const Kwave::MetaDataList &meta_data)
{
    Kwave::FileInfo info(meta_data);

    /* first get and check some header information */
    const unsigned int   tracks = info.tracks();
    const sample_index_t length = info.length();
    unsigned int bits = info.bits();
    const double rate = info.rate();
    int sample_format = info.contains(Kwave::INF_SAMPLE_FORMAT) ?
                        info.get(Kwave::INF_SAMPLE_FORMAT).toInt() :
                        AF_SAMPFMT_TWOSCOMP;
    int compression = info.contains(Kwave::INF_COMPRESSION) ?
                      info.get(Kwave::INF_COMPRESSION).toInt() :
                      Kwave::Compression::NONE;

    // use default bit resolution if missing
    Q_ASSERT(bits);
    if (!bits) bits = 16;

    // check for a valid source
    if ((!tracks) || (!length)) return false;
    Q_ASSERT(src.tracks() == tracks);
    if (src.tracks() != tracks) return false;

    // check if the chosen compression mode is supported for saving
    if ((compression != Kwave::Compression::NONE) &&
        (compression != Kwave::Compression::G711_ULAW) &&
        (compression != Kwave::Compression::G711_ALAW))
    {
	qWarning("compression mode %d not supported!", compression);
	int what_now = Kwave::MessageBox::warningYesNoCancel(widget,
	    i18n("Sorry, the currently selected compression type cannot "
	         "be used for saving. Do you want to use "
	         "G711 ULAW compression instead?"), QString(),
	    i18n("&Yes, use G711"),
	    i18n("&No, store uncompressed")
	);
	switch (what_now) {
	    case (KMessageBox::Yes):
		info.set(Kwave::INF_COMPRESSION,
			 Kwave::Compression::G711_ULAW);
		compression = Kwave::Compression::G711_ULAW;
		break;
	    case (KMessageBox::No):
		info.set(Kwave::INF_COMPRESSION,
			 Kwave::Compression::NONE);
		compression = Kwave::Compression::NONE;
		break;
	    default:
		return false; // bye bye, save later...
	}
    }

    // check for unsupported compression/bits/sample format combinations
    // G.711 and MSADPCM support only 16 bit signed as input format!
    if ((compression == Kwave::Compression::G711_ULAW) ||
        (compression == Kwave::Compression::G711_ALAW))
    {
	if ((sample_format != AF_SAMPFMT_TWOSCOMP) ||
	    (bits          != 16))
	{
	    const Kwave::SampleFormat format(Kwave::SampleFormat::Signed);
	    info.set(Kwave::INF_SAMPLE_FORMAT, QVariant(format.toInt()));
	    info.setBits(16);
	    sample_format = AF_SAMPFMT_TWOSCOMP;
	    bits          = 16;
	    qDebug("auto-switching to 16 bit signed format");
	}
    } else if ((bits <= 8) && (sample_format != AF_SAMPFMT_UNSIGNED)) {
	const Kwave::SampleFormat format(Kwave::SampleFormat::Unsigned);
	info.set(Kwave::INF_SAMPLE_FORMAT, QVariant(format.toInt()));

	sample_format = AF_SAMPFMT_UNSIGNED;
	qDebug("auto-switching to unsigned format");
    } else if ((bits > 8) && (sample_format != AF_SAMPFMT_TWOSCOMP)) {
	const Kwave::SampleFormat format(Kwave::SampleFormat::Signed);
	info.set(Kwave::INF_SAMPLE_FORMAT, QVariant(format.toInt()));

	sample_format = AF_SAMPFMT_TWOSCOMP;
	qDebug("auto-switching to signed format");
    }

    // open the output device
    if (!dst.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
	Kwave::MessageBox::error(widget,
	    i18n("Unable to open the file for saving!"));
	return false;
    }

    // check for proper size: WAV supports only 32bit addressing
    if (length * ((bits + 7) / 8) >= UINT_MAX) {
	Kwave::MessageBox::error(widget, i18n("File or selection too large"));
	return false;
    }

    AFfilesetup setup;
    setup = afNewFileSetup();
    afInitFileFormat(setup, AF_FILE_WAVE);
    afInitChannels(setup, AF_DEFAULT_TRACK, tracks);
    afInitSampleFormat(setup, AF_DEFAULT_TRACK, sample_format, bits);
    afInitCompression(setup, AF_DEFAULT_TRACK, compression);
    afInitRate(setup, AF_DEFAULT_TRACK, rate);

    Kwave::VirtualAudioFile outfile(dst);
    outfile.open(&outfile, setup);

    AFfilehandle fh = outfile.handle();
    if (!fh || (outfile.lastError() >= 0)) {
	QString reason;

	switch (outfile.lastError()) {
	    case AF_BAD_NOT_IMPLEMENTED:
	        reason = i18n("Format or function is not implemented") /*+
		         "\n("+format_name+")"*/;
	        break;
	    case AF_BAD_MALLOC:
	        reason = i18n("Out of memory");
	        break;
	    case AF_BAD_HEADER:
	        reason = i18n("File header is damaged");
	        break;
	    case AF_BAD_CODEC_TYPE:
	        reason = i18n("Invalid codec type")/* +
		         "\n("+format_name+")"*/;
	        break;
	    case AF_BAD_OPEN:
	        reason = i18n("Opening the file failed");
	        break;
	    case AF_BAD_READ:
	        reason = i18n("Read access failed");
	        break;
	    case AF_BAD_SAMPFMT:
	        reason = i18n("Invalid sample format");
	        break;
	    default:
		reason = reason.number(outfile.lastError());
	}

	QString text= i18n("An error occurred while opening the "\
	    "file:\n'%1'", reason);
	Kwave::MessageBox::error(widget, text);

	return false;
    }

    // set up libaudiofile to produce Kwave's internal sample format
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
    afSetVirtualByteOrder(fh, AF_DEFAULT_TRACK, AF_BYTEORDER_BIGENDIAN);
#else
    afSetVirtualByteOrder(fh, AF_DEFAULT_TRACK, AF_BYTEORDER_LITTLEENDIAN);
#endif
    afSetVirtualSampleFormat(fh, AF_DEFAULT_TRACK,
	AF_SAMPFMT_TWOSCOMP, SAMPLE_STORAGE_BITS);

    // allocate a buffer for input data
    const unsigned int virtual_frame_size = Kwave::toUint(
	    afGetVirtualFrameSize(fh, AF_DEFAULT_TRACK, 1));
    const unsigned int buffer_frames = (8 * 1024);
    sample_storage_t *buffer = static_cast<sample_storage_t *>(
	malloc(buffer_frames * virtual_frame_size));
    if (!buffer) return false;

    // read in from the sample readers
    sample_index_t rest = length;
    while (rest) {
	// merge the tracks into the sample buffer
	sample_storage_t *p = buffer;
	unsigned int count = buffer_frames;
	if (rest < count) count = Kwave::toUint(rest);

	for (unsigned int pos = 0; pos < count; pos++) {
	    for (unsigned int track = 0; track < tracks; track++) {
		Kwave::SampleReader *stream = src[track];
		sample_t sample = 0;
		if (!stream->eof()) (*stream) >> sample;

		// the following cast is only necessary if
		// sample_t is not equal to sample_storage_t
		sample_storage_t act = static_cast<sample_storage_t>(sample);
		act *= (1 << (SAMPLE_STORAGE_BITS - SAMPLE_BITS));
		*p = act;
		p++;
	    }
	}

	// write out through libaudiofile
	count = afWriteFrames(fh, AF_DEFAULT_TRACK, buffer, count);

	// break if eof reached or disk full
	Q_ASSERT(count);
	if (!count) break;

	Q_ASSERT(rest >= count);
	rest -= count;

	// abort if the user pressed cancel
	// --> this would leave a corrupted file !!!
	if (src.isCanceled()) break;
    }

    // close the audiofile stuff, we need control over the
    // fixed-up file on our own
    outfile.close();

    // clean up the sample buffer
    free(buffer);
    afFreeFileSetup(setup);

    // due to a buggy implementation of libaudiofile
    // we have to fix up the length of the "data" and the "RIFF" chunk
    fixAudiofileBrokenHeaderBug(dst, info, (bits * tracks) >> 3);

    // put the properties into the INFO chunk
    writeInfoChunk(dst, info);

    // write the labels list
    writeLabels(dst, Kwave::LabelList(meta_data));

    return true;
}

/***************************************************************************/
/***************************************************************************/
