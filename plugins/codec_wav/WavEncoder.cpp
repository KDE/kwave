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
#include <byteswap.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <kapp.h>
#include <kglobal.h>
#include <kaboutdata.h>
#include <math.h>
#include <stdlib.h>

#include "libkwave/FileInfo.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleReader.h"
#include "libkwave/VirtualAudioFile.h"

#include "WavEncoder.h"
#include "WavFileFormat.h"

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
    int compression = /* info.contains(INF_COMPRESSION) ?
                      info.get(INF_COMPRESSION).toInt() : */
                      AF_COMPRESSION_NONE;

    // use default bit resolution if missing
    ASSERT(bits);
    if (!bits) bits = 16;

    // check for a valid source
    ASSERT(tracks);
    ASSERT(length);
    if ((!tracks) || (!length)) return false;
    ASSERT(src.count() == tracks);
    if (src.count() != tracks) return false;

    // open the output device
    if (!dst.open(IO_ReadWrite | IO_Truncate)) {
	KMessageBox::error(widget,
	    i18n("Unable to open the file for saving!"));
	return false;
    }

    // find out if we have properties that would get lost
    // and get a list of properties that we have to save
    QMap<FileProperty, QVariant> properties(info.properties());
    unsigned int used_properties = properties.count();
    QMap<FileProperty, QVariant>::Iterator it;
    for (it=properties.begin(); it!=properties.end(); ++it) {
	if (! (m_property_map.containsProperty(it.key())) ) {
	    // if it's only internal, we don't care...
	    if (!info.canLoadSave(it.key())) used_properties--;
	    else warning("WavEncoder: unsupported property '%s'",
		info.name(it.key()).data());
	
	    // remove unsupported property
	    properties.remove(it);
	}
    }

    unsigned int supported_properties = properties.count();
    if (supported_properties < used_properties) {
	// show a warning to the user and ask him if he wants to continue
	if (KMessageBox::warningContinueCancel(widget,
	    i18n("Saving in this format will loose some additional "
	         "file attributes. Do you still want to continue?")
	    ) != KMessageBox::Continue) return false;
    }

    // append some missing standard properties if they are missing
    if (!properties.contains(INF_SOFTWARE)) {
        // add our Kwave Software tag
	const KAboutData *about_data = KGlobal::instance()->aboutData();
	QString software = about_data->programName() + "-" +
	    about_data->version() +
	    i18n(" for KDE ") + i18n(QString::fromLatin1(KDE_VERSION_STRING));
	QVariant value = software.utf8();
	debug("WavEncoder: adding software tag: '%s'", software.data());
	properties.insert(INF_SOFTWARE, value);
    }
    if (!properties.contains(INF_CREATION_DATE)) {
	// add a date tag
	QDate now(QDate::currentDate());
	QString date;
	date = date.sprintf("%04d-%02d-%02d",
	    now.year(), now.month(), now.day());
	QVariant value = date.utf8();
	debug("WavEncoder: adding date tag: '%s'", date.data());
	properties.insert(INF_CREATION_DATE, value);
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
//	    case AF_BAD_NOT_IMPLEMENTED:
//	        reason = i18n("format or function is not implemented") +
//		         "\n("+format_name+")";
//	        break;
	    case AF_BAD_MALLOC:
	        reason = i18n("out of memory");
	        break;
	    case AF_BAD_HEADER:
	        reason = i18n("file header is damaged");
	        break;
//	    case AF_BAD_CODEC_TYPE:
//	        reason = i18n("invalid codec type") +
//		         "\n("+format_name+")";
//	        break;
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
#if defined(IS_BIG_ENDIAN)
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
    ASSERT(buffer);
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
	ASSERT(count);
	if (!count) break;
	
	rest -= count;
	
	// abort if the user pressed cancel
	// --> this would leave a corrupted file !!!
	if (src.isCancelled()) break;
    }

    // close the audiofile stuff, we need control over the
    // fixed-up file on our own
    outfile.close();

    // create a list of chunk names and properties for the INFO chunk
    QMap<QCString, QCString> info_chunks;
    unsigned int info_size = 0;
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
	dst.readBlock((char*)&size, 4);
#if defined(IS_BIG_ENDIAN)
	size = bswap_32(bswap_32(size) + info_size);
#else
	size += info_size;
#endif
	dst.at(4);
	dst.writeBlock((char*)&size, 4);
	
	// add the LIST(INFO) chunk itself
	dst.at(dst.size());
	dst.writeBlock("LIST", 4);
#if defined(IS_BIG_ENDIAN)
	size = bswap_32(info_size - 8);
#else
	size = info_size - 8;
#endif
	dst.writeBlock((char*)&size, 4);
	dst.writeBlock("INFO", 4);
	
	// append the chunks to the end of the file
	QMap<QCString, QCString>::Iterator it;
	for (it=info_chunks.begin(); it != info_chunks.end(); ++it) {
	    QCString name  = it.key();
	    QCString value = it.data();
	
	    dst.writeBlock(name.data(), 4); // chunk name
	    u_int32_t size = value.length(); // length of the chunk
	    if (size & 0x01) size++;
#if defined(IS_BIG_ENDIAN)
	    size = bswap_32(size);
#endif
	    dst.writeBlock((char*)&size, 4);
	    dst.writeBlock(value.data(), value.length());
	    if (value.length() & 0x01) {
		const char zero = 0;
		dst.writeBlock(&zero, 1);
	    }
	}
    }

    // clean up the sample buffer
    if (buffer) free(buffer);
    afFreeFileSetup(setup);

    return true;
}

/***************************************************************************/
/***************************************************************************/
