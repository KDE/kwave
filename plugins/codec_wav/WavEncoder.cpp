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
    if (!dst.open(IO_WriteOnly)) {
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
	if (!m_property_map.contains(it.key())) {
	    // if it's only internal, we don't care...
	    if (info.isInternal(it.key())) used_properties--;
	
	    // remove unsupported property
	    properties.remove(it);
	}
    }

//    unsigned int supported_properties = properties.count();
//    if (supported_properties < used_properties) {
//	// show a warning to the user and ask him if he wants to continue
//	if (KMessageBox::warningContinueCancel(widget,
//	    i18n("Saving in this format will loose some additional "
//	         "file attributes. Do you still want to continue?")
//	    ) != KMessageBox::Continue) return false;
//    }

    // append some missing standard properties if they are missing
    if (!properties.contains(INF_SOFTWARE)) {
        // add our Kwave Software tag
	const KAboutData *about_data = KGlobal::instance()->aboutData();
	QString software = about_data->programName() + "-" +
	    about_data->version() +
	    " for KDE " + QString::fromLatin1(KDE_VERSION_STRING);
	properties.insert(INF_SOFTWARE, QVariant(software.utf8()));
    }
    if (!properties.contains(INF_CREATION_DATE)) {
	// add a date tag
	QDate now(QDate::currentDate());
	QString date;
	date = date.sprintf("%04d-%02d-%02d",
	    now.year(), now.month(), now.day());
	properties.insert(INF_CREATION_DATE, QVariant(date.utf8()));
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

    // if there are properties to save, create a LIST chunk
    // void afInitMiscIDs (AFfilesetup setup, int *ids, int nids)
    // int afWriteMisc (AFfilehandle file, int miscellaneousid, void *buf, int bytes)
    // afInitMiscIDs(setup, miscids, 2);
    // afInitMiscType(setup, 1, AF_MISC_COPY);
    // afInitMiscType(setup, 2, AF_MISC_NAME);
    // afInitMiscSize(setup, 1, strlen(copyright));
    // afInitMiscSize(setup, 2, strlen(name));
//    for (it=properties.begin(); it!=properties.end(); ++it) {
//	QString id = "";
//	debug("misc data, id='%s'", id);
//    }

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

    // clean up the sample buffer
    if (buffer) free(buffer);
    afFreeFileSetup(setup);

    return true;
}

/***************************************************************************/
/***************************************************************************/
