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
#include <klocale.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <kapp.h>
#include <kglobal.h>
#include <math.h>
#include <stdlib.h>

#include "libkwave/byteswap.h"
#include "libkwave/FileInfo.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleReader.h"
#include "libkwave/VirtualAudioFile.h"

#include "WavEncoder.h"
#include "WavFileFormat.h"

// some byteswapping helper macros
#if defined(ENDIANESS_BIG)
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
QValueList<FileProperty> WavEncoder::supportedProperties()
{
    QValueList<FileProperty> list;
    QMap<QCString, FileProperty>::Iterator it;
    for (it=m_property_map.begin(); it != m_property_map.end(); ++it) {
        list.append(it.data());
    }
    return list;
}

/***************************************************************************/
void WavEncoder::writeInfoChunk(QIODevice &dst, FileInfo &info)
{
    // create a list of chunk names and properties for the INFO chunk
    QMap<FileProperty, QVariant> properties(info.properties());
    QMap<QCString, QCString> info_chunks;
    unsigned int info_size = 0;

    QMap<FileProperty, QVariant>::Iterator it;
    for (it=properties.begin(); it!=properties.end(); ++it) {
	FileProperty property = it.key();
	if (!m_property_map.containsProperty(property)) continue;

	QCString chunk_id = m_property_map.findProperty(property);
	QCString value = QVariant(properties[property]).asString().utf8();
	info_chunks.insert(chunk_id, value);
	info_size += 4 + 4 + value.length();
	if (value.length() & 0x01) info_size++;
    }

    // if there are properties to save, create a LIST chunk
    if (!info_chunks.isEmpty()) {
	u_int32_t size;

	// enlarge the main RIFF chunk by the size of the LIST chunk
	info_size += 4 + 4 + 4; // add the size of LIST(INFO)
	dst.at(4);
	dst.readBlock((char *)&size, 4);
	size = CPU_TO_LE32(LE32_TO_CPU(size) + info_size);
	dst.at(4);
	dst.writeBlock((char *)&size, 4);

	// add the LIST(INFO) chunk itself
	dst.at(dst.size());
	dst.writeBlock("LIST", 4);
	size = CPU_TO_LE32(info_size - 8);
	dst.writeBlock((char *)&size, 4);
	dst.writeBlock("INFO", 4);

	// append the chunks to the end of the file
	QMap<QCString, QCString>::Iterator it;
	for (it=info_chunks.begin(); it != info_chunks.end(); ++it) {
	    QCString name  = it.key();
	    QCString value = it.data();

	    dst.writeBlock(name.data(), 4); // chunk name
	    u_int32_t size = value.length(); // length of the chunk
	    if (size & 0x01) size++;
	    size = CPU_TO_LE32(size);
	    dst.writeBlock((char *)&size, 4);
	    dst.writeBlock(value.data(), value.length());
	    if (value.length() & 0x01) {
		const char zero = 0;
		dst.writeBlock(&zero, 1);
	    }
	}
    }
}

/***************************************************************************/
void WavEncoder::writeLabels(QIODevice &dst, FileInfo &info)
{
    const unsigned int labels_count = info.labels().count();
    u_int32_t size, additional_size = 0, index, data;

    // shortcut: nothing to do if no labels present
    if (!labels_count) return;

    // easy things first: size of the cue list (has fixed record size)
    // without chunk name and chunk size
    const unsigned int size_of_cue_list =
	4 + /* number of entries */
	labels_count * (6 * 4); /* cue list entry: 6 x 32 bit */

    // now the size of the labels
    unsigned int size_of_labels = 4 + /* header entry: 'adtl' */
	labels_count * (3 * 4); /* per label 3 * 32 bit header */
    LabelListIterator it(info.labels());
    for (it.toFirst(); it.current(); ++it) {
	Label *label = it.current();
	Q_ASSERT(label);
	unsigned int name_len = label->name().utf8().size();
	size_of_labels += name_len;
	// padding if size is unaligned
	if (size_of_labels & 1) size_of_labels++;
    }

    // enlarge the main RIFF chunk by the size of the cue and LIST chunks
    additional_size += 4 + 4 + size_of_cue_list; // add size of 'cue '
    additional_size += 4 + 4 + size_of_labels;   // add size of LIST(adtl)

    dst.at(4);
    dst.readBlock((char *)&size, 4);
    size = CPU_TO_LE32(LE32_TO_CPU(size) + additional_size);
    dst.at(4);
    dst.writeBlock((char *)&size, 4);

    // seek to the end of the file
    dst.at(dst.size());

    // add the 'cue ' list
    dst.writeBlock("cue ", 4);
    size = CPU_TO_LE32(size_of_cue_list);
    dst.writeBlock((char *)&size, 4);

    // number of entries
    size = CPU_TO_LE32(labels_count);
    dst.writeBlock((char *)&size, 4);

    for (index=0, it.toFirst(); it.current(); ++it, ++index) {
	Label *label = it.current();
	Q_ASSERT(label);
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
	dst.writeBlock((char *)&data, 4); // dwIdentifier
	data = 0;
	dst.writeBlock((char *)&data, 4); // dwPosition
	dst.writeBlock("data", 4);        // fccChunk
	dst.writeBlock((char *)&data, 4); // dwChunkStart
	dst.writeBlock((char *)&data, 4); // dwBlockStart
	data = CPU_TO_LE32(label->pos());
	dst.writeBlock((char *)&data, 4); // dwSampleOffset
    }

    // add the LIST(adtl) chunk
    dst.writeBlock("LIST", 4);
    size = CPU_TO_LE32(size_of_labels);
    dst.writeBlock((char *)&size, 4);
    dst.writeBlock("adtl", 4);
    for (index=0, it.toFirst(); it.current(); ++it, ++index) {
	Label *label = it.current();
	Q_ASSERT(label);
	QCString name = label->name().utf8();

	/*
	 * typedef struct {
	 *     u_int32_t dwChunkID;    <- 'labl'
	 *     u_int32_t dwChunkSize;  (without padding !)
	 *     u_int32_t dwIdentifier; <- index
	 *     char    dwText[];       <- label->name()
	 * } label_list_entry_t;
	 */
        dst.writeBlock("labl", 4);                // dwChunkID
        data = CPU_TO_LE32(name.size() + 4);
	dst.writeBlock((char *)&data, 4);         // dwChunkSize
	data = CPU_TO_LE32(index);
	dst.writeBlock((char *)&data, 4);         // dwIdentifier
	dst.writeBlock(name.data(), name.size()); // dwText
	if (name.size() & 1) {
	    // padding if necessary
	    data = 0;
	    dst.writeBlock((char *)&data, 1);     // (padding)
	}
    }
}

/***************************************************************************/
bool WavEncoder::encode(QWidget *widget, MultiTrackReader &src,
                        QIODevice &dst, FileInfo &info)
{
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
    Q_ASSERT(tracks);
    Q_ASSERT(length);
    if ((!tracks) || (!length)) return false;
    Q_ASSERT(src.tracks() == tracks);
    if (src.tracks() != tracks) return false;

    // check if the choosen compression mode is supported for saving
    if ((compression != AF_COMPRESSION_NONE) &&
        (compression != AF_COMPRESSION_G711_ULAW) &&
        (compression != AF_COMPRESSION_G711_ALAW) )
    {
	qWarning("compression mode %d not supported!", compression);
	int what_now = KMessageBox::warningYesNoCancel(widget,
	    i18n("Sorry, the currently selected compression type can "
	         "not be used for saving. Do you want to use "
	         "G711 ULAW compression instead?"), 0,
	    KGuiItem(i18n("&Yes, use G711")),
	    KGuiItem(i18n("&No, store uncompressed"))
	);
	switch (what_now) {
	    case (KMessageBox::Yes):
		compression = AF_COMPRESSION_G711_ULAW;
		break;
	    case (KMessageBox::No):
		compression = AF_COMPRESSION_NONE;
		break;
	    default:
		return false; // bye bye, save later...
	}
    }

    // open the output device
    if (!dst.open(IO_ReadWrite | IO_Truncate)) {
	KMessageBox::error(widget,
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
	        reason = i18n("format or function is not implemented") /*+
		         "\n("+format_name+")"*/;
	        break;
	    case AF_BAD_MALLOC:
	        reason = i18n("out of memory");
	        break;
	    case AF_BAD_HEADER:
	        reason = i18n("file header is damaged");
	        break;
	    case AF_BAD_CODEC_TYPE:
	        reason = i18n("invalid codec type")/* +
		         "\n("+format_name+")"*/;
	        break;
	    case AF_BAD_OPEN:
	        reason = i18n("opening the file failed");
	        break;
	    case AF_BAD_READ:
	        reason = i18n("read access failed");
	        break;
	    case AF_BAD_SAMPFMT:
	        reason = i18n("invalid sample format");
	        break;
	    default:
		reason = reason.number(outfile.lastError());
	}

	QString text= i18n("An error occurred while opening the "\
	    "file:\n'%1'").arg(reason);
	KMessageBox::error(widget, text);

	return false;
    }

    // set up libaudiofile to produce Kwave's internal sample format
#if defined(ENDIANESS_BIG)
    afSetVirtualByteOrder(fh, AF_DEFAULT_TRACK, AF_BYTEORDER_BIGENDIAN);
#else
    afSetVirtualByteOrder(fh, AF_DEFAULT_TRACK, AF_BYTEORDER_LITTLEENDIAN);
#endif
    afSetVirtualSampleFormat(fh, AF_DEFAULT_TRACK,
	AF_SAMPFMT_TWOSCOMP, SAMPLE_STORAGE_BITS);

    // allocate a buffer for input data
    const unsigned int frame_size = (unsigned int)afGetVirtualFrameSize(fh,
	AF_DEFAULT_TRACK, 1);
    const unsigned int buffer_frames = (8*1024);
    int32_t *buffer = (int32_t *)malloc(buffer_frames * frame_size);
    Q_ASSERT(buffer);
    if (!buffer) return false;

    // read in from the sample readers
    unsigned int rest = length;
    while (rest) {
	// merge the tracks into the sample buffer
	int32_t *p = buffer;
	unsigned int count = buffer_frames;
	if (rest < count) count = rest;

	for (unsigned int pos=0; pos < count; pos++) {
	    for (unsigned int track = 0; track < tracks; track++) {
		SampleReader *stream = src[track];
		sample_t sample;
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

	rest -= count;

	// abort if the user pressed cancel
	// --> this would leave a corrupted file !!!
	if (src.isCancelled()) break;
    }

    // close the audiofile stuff, we need control over the
    // fixed-up file on our own
    outfile.close();

    // put the properties into the INFO chunk
    writeInfoChunk(dst, info);

    // write the labels list
    writeLabels(dst, info);

    // clean up the sample buffer
    if (buffer) free(buffer);
    afFreeFileSetup(setup);

    return true;
}

/***************************************************************************/
/***************************************************************************/
