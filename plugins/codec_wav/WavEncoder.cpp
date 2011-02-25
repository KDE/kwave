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

#include "config.h"

#include <math.h>
#include <stdlib.h>

#include <klocale.h>
#include <kmimetype.h>
#include <kglobal.h>

#include <QByteArray>
#include <QtGlobal>

#include "libkwave/byteswap.h"
#include "libkwave/FileInfo.h"
#include "libkwave/MessageBox.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleFormat.h"
#include "libkwave/SampleReader.h"
#include "libkwave/VirtualAudioFile.h"

#include "WavEncoder.h"
#include "WavFileFormat.h"

// some byteswapping helper macros
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
#define CPU_TO_LE32(x) (bswap_32(x))
#define LE32_TO_CPU(x) (bswap_32(x))
#else
#define CPU_TO_LE32(x) (x)
#define LE32_TO_CPU(x) (x)
#endif

/***************************************************************************/
WavEncoder::WavEncoder()
    :Encoder(), m_property_map()
{
    LOAD_MIME_TYPES;
}

/***************************************************************************/
WavEncoder::~WavEncoder()
{
}

/***************************************************************************/
Encoder *WavEncoder::instance()
{
    return new WavEncoder();
}

/***************************************************************************/
QList<FileProperty> WavEncoder::supportedProperties()
{
    return m_property_map.properties();
}

/***************************************************************************/
void WavEncoder::fixAudiofileBrokenHeaderBug(QIODevice &dst, FileInfo &info,
                                             unsigned int frame_size)
{
    const unsigned int length = info.length();
    u_int32_t correct_size = length * frame_size;
    const int compression = info.contains(INF_COMPRESSION) ?
                      info.get(INF_COMPRESSION).toInt() :
	    AF_COMPRESSION_NONE;
    if (compression != AF_COMPRESSION_NONE) {
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
    u_int32_t data_size;
    dst.seek(40);
    dst.read(reinterpret_cast<char *>(&data_size), 4);
    data_size = LE32_TO_CPU(data_size);
    if (data_size == length * frame_size) {
// 	qDebug("(data size written by libaudiofile is correct)");
	return;
    }

    qWarning("WARNING: libaudiofile wrote a wrong 'data' chunk size!");
    qWarning("         current=%u, correct=%u", data_size, correct_size);

    // write the fixed size of the "data" chunk
    dst.seek(40);
    data_size = CPU_TO_LE32(correct_size);
    dst.write(reinterpret_cast<char *>(&data_size), 4);

    // also fix the "RIFF" size
    dst.seek(4);
    u_int32_t riff_size = dst.size() - 4 - 4;
    riff_size = CPU_TO_LE32(riff_size);
    dst.write(reinterpret_cast<char *>(&riff_size), 4);

}

/***************************************************************************/
void WavEncoder::writeInfoChunk(QIODevice &dst, FileInfo &info)
{
    // create a list of chunk names and properties for the INFO chunk
    QMap<FileProperty, QVariant> properties(info.properties());
    QMap<QByteArray, QByteArray> info_chunks;
    unsigned int info_size = 0;

    QMap<FileProperty, QVariant>::Iterator it;
    for (it = properties.begin(); it != properties.end(); ++it) {
	FileProperty property = it.key();
	if (!m_property_map.containsProperty(property)) continue;

	QByteArray chunk_id = m_property_map.findProperty(property);
	QByteArray value = QVariant(properties[property]).toString().toUtf8();
	info_chunks.insert(chunk_id, value);
	info_size += 4 + 4 + value.length();
	if (value.length() & 0x01) info_size++;
    }

    // if there are properties to save, create a LIST chunk
    if (!info_chunks.isEmpty()) {
	u_int32_t size;

	// enlarge the main RIFF chunk by the size of the LIST chunk
	info_size += 4 + 4 + 4; // add the size of LIST(INFO)
	dst.seek(4);
	dst.read(reinterpret_cast<char *>(&size), 4);
	size = CPU_TO_LE32(LE32_TO_CPU(size) + info_size);
	dst.seek(4);
	dst.write(reinterpret_cast<char *>(&size), 4);

	// add the LIST(INFO) chunk itself
	dst.seek(dst.size());
	if (dst.pos() & 1) dst.write("\000", 1); // padding
	dst.write("LIST", 4);
	size = CPU_TO_LE32(info_size - 8);
	dst.write(reinterpret_cast<char *>(&size), 4);
	dst.write("INFO", 4);

	// append the chunks to the end of the file
	QMap<QByteArray, QByteArray>::Iterator it;
	for (it=info_chunks.begin(); it != info_chunks.end(); ++it) {
	    QByteArray name  = it.key();
	    QByteArray value = it.value();

	    dst.write(name.data(), 4); // chunk name
	    u_int32_t size = value.length(); // length of the chunk
	    if (size & 0x01) size++;
	    size = CPU_TO_LE32(size);
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
void WavEncoder::writeLabels(QIODevice &dst, const LabelList &labels)
{
    const unsigned int labels_count = labels.count();
    u_int32_t size, additional_size = 0, index, data;

    // shortcut: nothing to do if no labels present
    if (!labels_count) return;

    // easy things first: size of the cue list (has fixed record size)
    // without chunk name and chunk size
    const unsigned int size_of_cue_list =
	4 + /* number of entries */
	labels_count * (6 * 4); /* cue list entry: 6 x 32 bit */

    // now the size of the labels
    unsigned int size_of_labels = 0;
    foreach (const Label &label, labels) {
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
    size = CPU_TO_LE32(LE32_TO_CPU(size) + additional_size);
    dst.seek(4);
    dst.write(reinterpret_cast<char *>(&size), 4);

    // seek to the end of the file
    dst.seek(dst.size());
    if (dst.pos() & 1) dst.write("\000", 1); // padding

    // add the 'cue ' list
    dst.write("cue ", 4);
    size = CPU_TO_LE32(size_of_cue_list);
    dst.write(reinterpret_cast<char *>(&size), 4);

    // number of entries
    size = CPU_TO_LE32(labels_count);
    dst.write(reinterpret_cast<char *>(&size), 4);

    index = 0;
    foreach (const Label &label, labels) {
	if (label.isNull()) continue;
	/*
	 * typedef struct {
	 *     u_int32_t dwIdentifier; <- index
	 *     u_int32_t dwPosition;   <- 0
	 *     u_int32_t fccChunk;     <- 'data'
	 *     u_int32_t dwChunkStart; <- 0
	 *     u_int32_t dwBlockStart; <- 0
	 *     u_int32_t dwSampleOffset; <- label.pos()
	 * } cue_list_entry_t;
	 */
	data = CPU_TO_LE32(index);
	dst.write(reinterpret_cast<char *>(&data), 4); // dwIdentifier
	data = 0;
	dst.write(reinterpret_cast<char *>(&data), 4); // dwPosition
	dst.write("data", 4);        // fccChunk
	dst.write(reinterpret_cast<char *>(&data), 4); // dwChunkStart
	dst.write(reinterpret_cast<char *>(&data), 4); // dwBlockStart
	data = CPU_TO_LE32(label.pos());
	dst.write(reinterpret_cast<char *>(&data), 4); // dwSampleOffset
	index++;
    }

    // add the LIST(adtl) chunk
    if (size_of_labels) {
	dst.write("LIST", 4);
	size = CPU_TO_LE32(size_of_labels);
	dst.write(reinterpret_cast<char *>(&size), 4);
	dst.write("adtl", 4);
	index = 0;
	foreach (const Label &label, labels) {
	    if (label.isNull()) continue;
	    QByteArray name = label.name().toUtf8();

	    /*
	     * typedef struct {
	     *     u_int32_t dwChunkID;    <- 'labl'
	     *     u_int32_t dwChunkSize;  (without padding !)
	     *     u_int32_t dwIdentifier; <- index
	     *     char    dwText[];       <- label->name()
	     * } label_list_entry_t;
	     */
	    if (name.size()) {
		dst.write("labl", 4);                // dwChunkID
		data = CPU_TO_LE32(name.size() + 4);

		// dwChunkSize
		dst.write(reinterpret_cast<char *>(&data), 4);
		data = CPU_TO_LE32(index);

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
bool WavEncoder::encode(QWidget *widget, MultiTrackReader &src,
                        QIODevice &dst, const Kwave::MetaDataList &meta_data)
{
    FileInfo info = meta_data.fileInfo();

    /* first get and check some header information */
    const unsigned int tracks = info.tracks();
    const unsigned int length = info.length();
    unsigned int bits = info.bits();
    const double rate = info.rate();
    int sample_format = info.contains(INF_SAMPLE_FORMAT) ?
                        info.get(INF_SAMPLE_FORMAT).toInt() :
                        AF_SAMPFMT_TWOSCOMP;
    int compression = info.contains(INF_COMPRESSION) ?
                      info.get(INF_COMPRESSION).toInt() :
                      AF_COMPRESSION_NONE;

    // use default bit resolution if missing
    Q_ASSERT(bits);
    if (!bits) bits = 16;

    // check for a valid source
    if ((!tracks) || (!length)) return false;
    Q_ASSERT(src.tracks() == tracks);
    if (src.tracks() != tracks) return false;

    // check if the choosen compression mode is supported for saving
    if ((compression != AF_COMPRESSION_NONE) &&
        (compression != AF_COMPRESSION_G711_ULAW) &&
        (compression != AF_COMPRESSION_G711_ALAW))
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
		compression = AF_COMPRESSION_G711_ULAW;
		info.set(INF_COMPRESSION, AF_COMPRESSION_G711_ULAW);
		break;
	    case (KMessageBox::No):
		info.set(INF_COMPRESSION, AF_COMPRESSION_NONE);
		compression = AF_COMPRESSION_NONE;
		break;
	    default:
		return false; // bye bye, save later...
	}
    }

    // check for unsupported compression/bits/sample format combinations
    // G.711 and MSADPCM support only 16 bit signed as input format!
    if ((compression == AF_COMPRESSION_G711_ULAW) ||
        (compression == AF_COMPRESSION_G711_ALAW))
    {
	if ((sample_format != AF_SAMPFMT_TWOSCOMP) &&
	    (bits          != 16))
	{
	    const SampleFormat format(SampleFormat::Signed);
	    info.set(INF_SAMPLE_FORMAT, QVariant(format.toInt()));
	    info.setBits(16);
	    sample_format = AF_SAMPFMT_TWOSCOMP;
	    bits          = 16;
	    qDebug("auto-switching to 16 bit signed format");
	}
    } else if ((bits <= 8) && (sample_format != AF_SAMPFMT_UNSIGNED)) {
	const SampleFormat format(SampleFormat::Unsigned);
	info.set(INF_SAMPLE_FORMAT, QVariant(format.toInt()));

	sample_format = AF_SAMPFMT_UNSIGNED;
	qDebug("auto-switching to unsigned format");
    } else if ((bits > 8) && (sample_format != AF_SAMPFMT_TWOSCOMP)) {
	const SampleFormat format(SampleFormat::Signed);
	info.set(INF_SAMPLE_FORMAT, QVariant(format.toInt()));

	sample_format = AF_SAMPFMT_TWOSCOMP;
	qDebug("auto-switching to signed format");
    }

    // open the output device
    if (!dst.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
	Kwave::MessageBox::error(widget,
	    i18n("Unable to open the file for saving!"));
	return false;
    }
    AFfilesetup setup;
    setup = afNewFileSetup();
    afInitFileFormat(setup, AF_FILE_WAVE);
    afInitChannels(setup, AF_DEFAULT_TRACK, tracks);
    afInitSampleFormat(setup, AF_DEFAULT_TRACK, sample_format, bits);
    afInitCompression(setup, AF_DEFAULT_TRACK, compression);
    afInitRate (setup, AF_DEFAULT_TRACK, rate);

    VirtualAudioFile outfile(dst);
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
    const unsigned int virtual_frame_size = static_cast<const unsigned int>(
	    afGetVirtualFrameSize(fh, AF_DEFAULT_TRACK, 1));
    const unsigned int buffer_frames = (8*1024);
    int32_t *buffer = static_cast<int32_t *>(
	malloc(buffer_frames * virtual_frame_size));
    Q_ASSERT(buffer);
    if (!buffer) return false;

    // read in from the sample readers
    unsigned int rest = length;
    while (rest) {
	// merge the tracks into the sample buffer
	int32_t *p = buffer;
	unsigned int count = buffer_frames;
	if (rest < count) count = rest;

	for (unsigned int pos = 0; pos < count; pos++) {
	    for (unsigned int track = 0; track < tracks; track++) {
		SampleReader *stream = src[track];
		sample_t sample;
		Q_ASSERT(!stream->eof());
		(*stream) >> sample;

		// the following cast is only necessary if
		// sample_t is not equal to a 32bit int
		register __uint32_t act = static_cast<__uint32_t>(sample);
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
    if (buffer) free(buffer);
    afFreeFileSetup(setup);

    // due to a buggy implementation of libaudiofile
    // we have to fix up the length of the "data" and the "RIFF" chunk
    fixAudiofileBrokenHeaderBug(dst, info, (bits * tracks) >> 3);

    // put the properties into the INFO chunk
    writeInfoChunk(dst, info);

    // write the labels list
    writeLabels(dst, meta_data.labels());

    return true;
}

/***************************************************************************/
/***************************************************************************/
