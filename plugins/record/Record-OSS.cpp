/*************************************************************************
         Record-OSS.cpp  -  device for audio recording via OSS
                             -------------------
    begin                : Sun Jul 24 2005
    copyright            : (C) 2005 by Thomas Eschenbacher
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
#ifdef HAVE_OSS_SUPPORT

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>

#include <QDir>
#include <QFile>
#include <QtGlobal>

#include "libkwave/CompressionType.h"
#include "libkwave/SampleFormat.h"

#include "Record-OSS.h"

// Linux 2.6.24 and above's OSS emulation supports 24 and 32 bit formats
// but did not declare them in <soundcard.h>
#ifndef AFMT_S24_LE
#define AFMT_S24_LE      0x00008000
#endif
#ifndef AFMT_S24_BE
#define AFMT_S24_BE      0x00010000
#endif
#ifndef AFMT_S32_LE
#define AFMT_S32_LE      0x00001000
#endif
#ifndef AFMT_S32_BE
#define AFMT_S32_BE      0x00002000
#endif

#ifndef SNDCTL_DSP_SPEED
#define SNDCTL_DSP_SPEED SOUND_PCM_WRITE_RATE
#endif

#ifndef SNDCTL_DSP_CHANNELS
#define SNDCTL_DSP_CHANNELS SOUND_PCM_WRITE_CHANNELS
#endif

#ifndef SOUND_PCM_SETFMT
#define SOUND_PCM_SETFMT SOUND_PCM_WRITE_BITS
#endif

#ifndef SNDCTL_DSP_SETFMT
#define SNDCTL_DSP_SETFMT SOUND_PCM_SETFMT
#endif


#define MAX_CHANNELS 2 /**< highest available number of channels */

//***************************************************************************
RecordOSS::RecordOSS()
    :RecordDevice(), m_fd(-1), m_oss_version(-1)
{
}

//***************************************************************************
RecordOSS::~RecordOSS()
{
    close();
}

//***************************************************************************
int RecordOSS::open(const QString &dev)
{
    // close the device if it is still open
    Q_ASSERT(m_fd < 0);
    if (m_fd >= 0) close();
    if (!dev.length()) return -1; // no device name

    // first of all: try to open the device itself
    int fd = ::open(dev.toLocal8Bit(), O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
	qWarning("open failed, fd=%d, errno=%d (%s)",
	         fd, errno, strerror(errno));
	return -errno;
    }

    // Query OSS driver version
    m_oss_version = 0x030000;
#ifdef OSS_GETVERSION
    ioctl(fd, OSS_GETVERSION, &m_oss_version);
#endif
    m_fd = fd;
    return m_fd;
}

//***************************************************************************
int RecordOSS::read(QByteArray &buffer, unsigned int offset)
{
    fd_set rfds;
    struct timeval tv;
    int retval;
    int read_bytes = 0;
    unsigned int length = buffer.size();

//     qDebug("RecordOSS::read(), offset=%d, length=%d", offset, length);
    Q_ASSERT(m_fd >= 0);
    Q_ASSERT(buffer.size());
    Q_ASSERT(length);
    Q_ASSERT(offset < length);
    if (m_fd < 0) return -EBADF; // file not opened
    if (buffer.isEmpty()) return -EINVAL; // buffer is null pointer
    if (!length) return -EINVAL;
    if (offset >= length) return -EINVAL;

    length -= offset;

#if 0
    int blocksize = length;
    int err = ioctl(m_fd, SNDCTL_DSP_GETBLKSIZE, &blocksize);
    Q_ASSERT(!err);
    qDebug("blocksize = %u", blocksize);
    if (err) {
	blocksize = length;
    }

    blocksize = (127 << 16) + 6;
    err = ioctl(m_fd, SNDCTL_DSP_SETFRAGMENT, &blocksize);
#endif

    // determine the timeout for reading, use safety factor 2
    int rate = static_cast<int>(sampleRate());
    if (rate < 1) rate = 1;

    unsigned int timeout = (length / rate) * 2;
    if (timeout < 2) timeout = 2;
    u_int8_t *buf = reinterpret_cast<u_int8_t *>(buffer.data()) + offset;

    int mask = 0;
    retval = ioctl(m_fd, SNDCTL_DSP_SETTRIGGER, &mask);
    Q_ASSERT(!retval);
    mask = PCM_ENABLE_INPUT;
    retval = ioctl(m_fd, SNDCTL_DSP_SETTRIGGER, &mask);
    Q_ASSERT(!retval);

    while (length) {
	FD_ZERO(&rfds);
	FD_SET(m_fd, &rfds);

	tv.tv_sec  = timeout;
	tv.tv_usec = 0;
	retval = select(m_fd+1, &rfds, 0, 0, &tv);

	if (retval == -1) {
	    if (errno == EINTR)
		return -errno; // return without warning

	    qWarning("RecordOSS::read() - select() failed errno=%d (%s)",
	             errno, strerror(errno));
	    return -errno;
	} else if (retval) {
	    int res = ::read(m_fd, buf, length);

	    if ((res == -1) && (errno == EINTR))
		return -errno; // interrupted, return without warning

	    if ((res == -1) && (errno == EAGAIN))
		continue;

	    if (res < 0) {
		qWarning("RecordOSS::read() - read error %d (%s)",
		         errno, strerror(errno));
		return read_bytes;
	    }
	    read_bytes += res;
	    length -= res;
	    buf += res;
	} else {
	    printf("No data within 5 seconds.\n");
	    return -EIO;
        }
    }

    return read_bytes;
}

//***************************************************************************
int RecordOSS::close()
{
    if (m_fd < 0) return 0; // already closed
    ::close(m_fd);
    m_fd = -1;
    m_oss_version = -1;

    return 0;
}

//***************************************************************************
static bool addIfExists(QStringList &list, const QString &name)
{
    QFile file;

    if (name.contains("%1")) {
	// test for the name without suffix first
	addIfExists(list, name.arg(""));

	// loop over the list and try until a suffix does not exist
	for (unsigned int index=0; index < 64; index++)
	    addIfExists(list, name.arg(index));
    } else {
	// check a single name
	file.setFileName(name);
	if (!file.exists())
	    return false;

	if (!list.contains(name))
	    list.append(name);
    }

    return true;
}

//***************************************************************************
static void scanFiles(QStringList &list, const QString &dirname,
                      const QString &mask)
{
    QStringList files;
    QDir dir;

    dir.setPath(dirname);
    dir.setNameFilters(mask.split(' '));
    dir.setFilter(QDir::Files | QDir::Readable | QDir::System);
    dir.setSorting(QDir::Name);
    files = dir.entryList();

    for (QStringList::Iterator it=files.begin(); it != files.end(); ++it) {
	QString devicename = dirname + QDir::separator() + (*it);
	addIfExists(list, devicename);
    }
}

//***************************************************************************
static void scanDirectory(QStringList &list, const QString &dir)
{
    scanFiles(list, dir, "*audio*");
    scanFiles(list, dir, "adsp*");
    scanFiles(list, dir, "dsp*");
    scanFiles(list, dir, "dio*");
    scanFiles(list, dir, "pcm*");
}

//***************************************************************************
QStringList RecordOSS::supportedDevices()
{
    QStringList list, dirlist;

    scanDirectory(list, "/dev");
    scanDirectory(list, "/dev/sound");
    scanFiles(dirlist, "/dev/oss", "[^.]*");
    foreach(QString dir, dirlist)
	scanDirectory(list, dir);
    list.append("#EDIT#");
    list.append("#SELECT#");

    return list;
}

//***************************************************************************
QString RecordOSS::fileFilter()
{
    QString filter;

    if (filter.length()) filter += "\n";
    filter += QString("audio*|") + i18n("OSS recording device (audio*)");
    filter += QString("dsp*|") + i18n("OSS recording device (dsp*)");

    if (filter.length()) filter += "\n";
    filter += QString("adsp*|") + i18n("ALSA recording device (adsp*)");

    if (filter.length()) filter += "\n";
    filter += QString("*|") + i18n("Any device (*)");

    return filter;
}

//***************************************************************************
int RecordOSS::detectTracks(unsigned int &min, unsigned int &max)
{
    Q_ASSERT(m_fd >= 0);
    int t;
    int err;

    // find the smalles number of tracks, limit to MAX_CHANNELS
    for (t=1; t < MAX_CHANNELS; t++) {
	int real_tracks = t;
	err = ioctl(m_fd, SNDCTL_DSP_CHANNELS, &real_tracks);
	Q_ASSERT(real_tracks == t);
	if (err >= 0) {
	    min = real_tracks;
	    break;
	}
    }
    if (t >= MAX_CHANNELS) {
	// no minimum track number found :-o
	qWarning("no minimum track number found, err=%d",err);
	min = 0;
	max = 0;
	return err;
    }

    // find the highest number of tracks, start from MAX_CHANNELS downwards
    for (t=MAX_CHANNELS; t >= static_cast<int>(min); t--) {
	int real_tracks = t;
	err = ioctl(m_fd, SNDCTL_DSP_CHANNELS, &real_tracks);
	Q_ASSERT(real_tracks == t);
	if (err >= 0) {
	    max = real_tracks;
	    break;
	}
    }
    max = t;
    m_tracks = max;

    qDebug("RecordOSS::detectTracks, min=%u, max=%u",min,max);

    return 0;
}

//***************************************************************************
int RecordOSS::setTracks(unsigned int &tracks)
{
    Q_ASSERT(m_fd >= 0);

    // set the number of tracks in the device (must already be opened)
    int t = tracks;
    int err = ioctl(m_fd, SNDCTL_DSP_CHANNELS, &t);
    if (err < 0) return err;

    m_tracks = t;
    // return the number of tracks if succeeded
    tracks = t;

    return 0;
}

//***************************************************************************
int RecordOSS::tracks()
{
    return m_tracks;
}

//***************************************************************************
QList<double> RecordOSS::detectSampleRates()
{
    QList<double> list;
    Q_ASSERT(m_fd >= 0);

    static const int known_rates[] = {
	  1000, // (just for testing)
	  2000, // (just for testing)
	  4000, // standard OSS
	  5125, // seen in Harmony driver (HP712, 715/new)
	  5510, // seen in AD1848 driver
	  5512, // seen in ES1370 driver
	  6215, // seen in ES188X driver
	  6615, // seen in Harmony driver (HP712, 715/new)
	  6620, // seen in AD1848 driver
	  7350, // seen in AWACS and Burgundy sound driver
	  8000, // standard OSS
	  8820, // seen in AWACS and Burgundy sound driver
	  9600, // seen in AD1848 driver
	 11025, // soundblaster
	 14700, // seen in AWACS and Burgundy sound driver
	 16000, // standard OSS
	 17640, // seen in AWACS and Burgundy sound driver
	 18900, // seen in Harmony driver (HP712, 715/new)
	 22050, // soundblaster
	 24000, // seen in NM256 driver
	 27428, // seen in Harmony driver (HP712, 715/new)
	 29400, // seen in AWACS and Burgundy sound driver
	 32000, // standard OSS
	 32768, // seen in CS4299 driver
	 33075, // seen in Harmony driver (HP712, 715/new)
	 37800, // seen in Harmony driver (HP712, 715/new)
	 44100, // soundblaster
	 48000, // AC97
	 64000, // AC97
	 88200, // seen in RME96XX driver
	 96000, // AC97
	128000, // (just for testing)
	176400, // Envy24ht
	192000, // AC97
	196000, // (just for testing)
	200000, // Lynx2
	256000  // (just for testing)
    };

    // try all known sample rates
    for (unsigned int i=0; i < sizeof(known_rates)/sizeof(int); i++) {
	int rate = known_rates[i];
	int err = ioctl(m_fd, SNDCTL_DSP_SPEED, &rate);
	if (err < 0) {
	    qDebug("RecordOSS::detectSampleRates(): "\
	           "sample rate %d Hz not supported", known_rates[i]);
	    continue;
	}

	// do not produce duplicates
	if (list.contains(rate)) continue;

	// qDebug("found rate %d Hz", rate);
	list.append(rate);
	m_rate = rate;
    }

    return list;
}

//***************************************************************************
int RecordOSS::setSampleRate(double &new_rate)
{
    Q_ASSERT(m_fd >= 0);
    // OSS supports only integer rates
    int rate = static_cast<int>(rint(new_rate));

    // set the rate of the device (must already be opened)
    int err = ioctl(m_fd, SNDCTL_DSP_SPEED, &rate);
    if (err < 0) return err;

    m_rate = rate;
    // return the sample rate if succeeded
    new_rate = static_cast<double>(rate);

    return 0;
}

//***************************************************************************
double RecordOSS::sampleRate()
{
    Q_ASSERT(m_fd >= 0);

    return m_rate;
}

//***************************************************************************
void RecordOSS::format2mode(int format, int &compression, int &bits,
                            SampleFormat &sample_format)
{

    switch (format) {
	case AFMT_MU_LAW:
	    compression   = AF_COMPRESSION_G711_ULAW;
	    sample_format = SampleFormat::Signed;
	    bits          = 16;
	    break;
	case AFMT_A_LAW:
	    compression   = AF_COMPRESSION_G711_ALAW;
	    sample_format = SampleFormat::Unsigned;
	    bits          = 16;
	    break;
	case AFMT_IMA_ADPCM:
	    compression   = AF_COMPRESSION_MS_ADPCM;
	    sample_format = SampleFormat::Signed;
	    bits          = 16;
	    break;
	case AFMT_U8:
	    compression   = AF_COMPRESSION_NONE;
	    sample_format = SampleFormat::Unsigned;
	    bits          = 8;
	    break;
	case AFMT_S16_LE:
	case AFMT_S16_BE:
	    compression   = AF_COMPRESSION_NONE;
	    sample_format = SampleFormat::Signed;
	    bits          = 16;
	    break;
	case AFMT_S8:
	    compression   = AF_COMPRESSION_NONE;
	    sample_format = SampleFormat::Signed;
	    bits          = 8;
	    break;
	case AFMT_U16_LE:
	case AFMT_U16_BE:
	    compression   = AF_COMPRESSION_NONE;
	    sample_format = SampleFormat::Unsigned;
	    bits          = 16;
	    break;
	case AFMT_MPEG:
	    compression   = static_cast<int>(CompressionType::MPEG_LAYER_II);
	    sample_format = SampleFormat::Signed;
	    bits          = 16;
	    break;
#if 0
	case AFMT_AC3: /* Dolby Digital AC3 */
	    compression   = AF_COMPRESSION_NONE;
	    sample_format = SampleFormat::Unknown;
	    bits          = 16;
	    break;
#endif
	case AFMT_S24_LE:
	case AFMT_S24_BE:
	    compression   = AF_COMPRESSION_NONE;
	    sample_format = SampleFormat::Signed;
	    bits          = 24;
	    break;
	case AFMT_S32_LE:
	case AFMT_S32_BE:
	    compression   = AF_COMPRESSION_NONE;
	    sample_format = SampleFormat::Signed;
	    bits          = 32;
	    break;
	default:
	    compression   = -1;
	    sample_format = SampleFormat::Unknown;
	    bits          = -1;
    }

}

//***************************************************************************
int RecordOSS::mode2format(int compression, int bits,
                           SampleFormat sample_format)
{
    // first level: compression
    if (compression == AF_COMPRESSION_G711_ULAW) return AFMT_MU_LAW;
    if (compression == AF_COMPRESSION_G711_ALAW) return AFMT_A_LAW;
    if (compression == AF_COMPRESSION_MS_ADPCM)  return AFMT_IMA_ADPCM;
    if (compression == static_cast<int>(CompressionType::MPEG_LAYER_II))
	return AFMT_MPEG;

    // non-compressed: switch by sample format
    if ((sample_format == SampleFormat::Unsigned) && (bits == 8))
	return AFMT_U8;
    if ((sample_format == SampleFormat::Signed) && (bits == 8))
        return AFMT_S8;

    // get supported modes, maybe one endianness is not supported
    int mask = 0;
    int err = ioctl(m_fd, SNDCTL_DSP_GETFMTS, &mask);
    if (err < 0) return bits;

    // unsigned 16 bit mode
    // note: we prefer the machine's native endianness if both are supported !
    if ((sample_format == SampleFormat::Unsigned) && (bits == 16)) {
	mask &= (AFMT_U16_LE | AFMT_U16_BE);
	if (mask != (AFMT_U16_LE | AFMT_U16_BE)) return mask;
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
	return AFMT_U16_BE;
#else
	return AFMT_U16_LE;
#endif
    }

    // signed 16 bit mode
    if ((sample_format == SampleFormat::Signed) && (bits == 16)) {
	mask &= (AFMT_S16_LE | AFMT_S16_BE);
	if (mask != (AFMT_S16_LE | AFMT_S16_BE)) return mask;
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
	return AFMT_S16_BE;
#else
	return AFMT_S16_LE;
#endif
    }

    if ((sample_format == SampleFormat::Signed) && (bits == 24)) {
	mask &= (AFMT_S24_LE | AFMT_S24_BE);
	if (mask != (AFMT_S24_LE | AFMT_S24_BE)) return mask;
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
	return AFMT_S24_BE;
#else
	return AFMT_S24_LE;
#endif
    }

    if ((sample_format == SampleFormat::Signed) && (bits == 32)) {
	mask &= (AFMT_S32_LE | AFMT_S32_BE);
	if (mask != (AFMT_S32_LE | AFMT_S32_BE)) return mask;
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
	return AFMT_S32_BE;
#else
	return AFMT_S32_LE;
#endif
    }

    qWarning("RecordOSS: unknown format: sample_format=%d, bits=%d",
             static_cast<int>(sample_format), bits);
    return 0;
}

//***************************************************************************
QList<int> RecordOSS::detectCompressions()
{
    Q_ASSERT(m_fd >= 0);
    QList<int> compressions;
    int err = 0;
    int mask = AFMT_QUERY;

    err = ioctl(m_fd, SNDCTL_DSP_GETFMTS, &mask);
    if (err < 0) return compressions;

    if (mask & AFMT_MPEG)
        compressions += static_cast<int>(CompressionType::MPEG_LAYER_II);
    if (mask & AFMT_A_LAW)     compressions += AF_COMPRESSION_G711_ALAW;
    if (mask & AFMT_MU_LAW)    compressions += AF_COMPRESSION_G711_ULAW;
    if (mask & AFMT_IMA_ADPCM) compressions += AF_COMPRESSION_MS_ADPCM;
    if (mask & (AFMT_U16_LE | AFMT_U16_BE | AFMT_S16_LE | AFMT_S16_BE | \
                AFMT_S24_LE | AFMT_S24_BE | AFMT_S32_LE | AFMT_S32_BE | \
                AFMT_S8 | AFMT_U8))
        compressions += AF_COMPRESSION_NONE;

    return compressions;
}

//***************************************************************************
int RecordOSS::setCompression(int new_compression)
{
    Q_ASSERT(m_fd >= 0);
    int compression, bits, format = AFMT_QUERY;
    SampleFormat sample_format;

    // read back current format
    int err = ioctl(m_fd, SNDCTL_DSP_SETFMT, &format);
    if (err < 0) return err;
    format2mode(format, compression, bits, sample_format);

    // modify the compression
    compression = new_compression;

    // activate new format
    format = mode2format(compression, bits, sample_format);
    int oldformat = format;
    err = ioctl(m_fd, SNDCTL_DSP_SETFMT, &format);
    if (err < 0) return err;
    if (oldformat != format) return -1;

    return 0;
}

//***************************************************************************
int RecordOSS::compression()
{
    Q_ASSERT(m_fd >= 0);
    int mask = AFMT_QUERY;
    int err = ioctl(m_fd, SNDCTL_DSP_SETFMT, &mask);
    if (err < 0) return AF_COMPRESSION_NONE;

    int c, b;
    SampleFormat s;
    format2mode(mask, c, b, s);
    return c;
}

//***************************************************************************
QList<unsigned int> RecordOSS::supportedBits()
{
    Q_ASSERT(m_fd >= 0);
    QList<unsigned int> bits;
    bits.clear();
    int err = 0;
    int mask = AFMT_QUERY;

    err = ioctl(m_fd, SNDCTL_DSP_GETFMTS, &mask);
    if (err < 0) return bits;

    // mask out all modes that do not match the current compression
    const int compression = this->compression();
    for (unsigned int bit=0; bit < (sizeof(mask) << 3); bit++) {
	if (!mask & (1 << bit)) continue;

	// format is supported, split into compression, bits, sample format
	int c, b;
	SampleFormat s;
	format2mode(1 << bit, c, b, s);
	if (b < 0) continue; // unknown -> skip

	// take the mode if compression matches and it is not already known
	if ((c == compression) && !(bits.contains(b))) {
	    bits += b;
	}
    }

#if 0
    if (mask | AFMT_AC3)
	qDebug("RecordOSS: your device supports AC3 which is not "\
	       "yet supported, sorry :-(");
#endif
    return bits;
}

//***************************************************************************
int RecordOSS::setBitsPerSample(unsigned int new_bits)
{
    Q_ASSERT(m_fd >= 0);
    int compression, bits, format = AFMT_QUERY;
    SampleFormat sample_format;

    // read back current format
    int err = ioctl(m_fd, SNDCTL_DSP_SETFMT, &format);
    if (err < 0) return err;
    format2mode(format, compression, bits, sample_format);

    // modify the bits per sample
    bits = new_bits;

    // activate new format
    int oldformat = format;
    format = mode2format(compression, bits, sample_format);
    err = ioctl(m_fd, SNDCTL_DSP_SETFMT, &format);
    if (err < 0) return err;
    if (oldformat != format) return -1;

    return 0;
}

//***************************************************************************
int RecordOSS::bitsPerSample()
{
    Q_ASSERT(m_fd >= 0);
    int mask = AFMT_QUERY;
    int err = ioctl(m_fd, SNDCTL_DSP_SETFMT, &mask);
    if (err < 0) return err;

    int c, b;
    SampleFormat s;
    format2mode(mask, c, b, s);
    return b;
}

//***************************************************************************
QList<SampleFormat> RecordOSS::detectSampleFormats()
{
    Q_ASSERT(m_fd >= 0);
    QList<SampleFormat> formats;
    formats.clear();
    int err = 0;
    int mask = AFMT_QUERY;

    err = ioctl(m_fd, SNDCTL_DSP_GETFMTS, &mask);
    if (err < 0) return formats;

    // mask out all modes that do not match the current compression
    // and bits per sample
    const int compression     = this->compression();
    const int bits_per_sample = this->bitsPerSample();
    for (unsigned int bit=0; bit < (sizeof(mask) << 3); bit++) {
	if (!mask & (1 << bit)) continue;

	// format is supported, split into compression, bits, sample format
	int c, b;
	SampleFormat s;
	format2mode(1 << bit, c, b, s);
	if (c < 0) continue; // unknown -> skip

	if ((c == compression) && (b == bits_per_sample)) {
	    // this mode matches -> append it if not already known
	    if (!formats.contains(s)) formats += s;
	}
    }

    return formats;
}

//***************************************************************************
int RecordOSS::setSampleFormat(SampleFormat new_format)
{
    Q_ASSERT(m_fd >= 0);
    int compression, bits, format = AFMT_QUERY;
    SampleFormat sample_format;

    // read back current format
    int err = ioctl(m_fd, SNDCTL_DSP_SETFMT, &format);
    if (err < 0) return err;
    format2mode(format, compression, bits, sample_format);

    // modify the sample format
    sample_format = new_format;

    // activate new format
    int oldformat = format;
    format = mode2format(compression, bits, sample_format);
    err = ioctl(m_fd, SNDCTL_DSP_SETFMT, &format);
    if (err < 0) return err;
    if (oldformat != format) return -1;

    return 0;
}

//***************************************************************************
SampleFormat RecordOSS::sampleFormat()
{
    Q_ASSERT(m_fd >= 0);
    int mask = AFMT_QUERY;
    int err = ioctl(m_fd, SNDCTL_DSP_SETFMT, &mask);
    if (err < 0) return SampleFormat::Unknown;

    int c, b;
    SampleFormat s;
    format2mode(mask, c, b, s);
    return s;
}

//***************************************************************************
byte_order_t RecordOSS::endianness()
{
    Q_ASSERT(m_fd >= 0);
    int mask = AFMT_QUERY;
    int err = ioctl(m_fd, SNDCTL_DSP_SETFMT, &mask);
    if (err < 0) return UnknownEndian;

    if (mask & (AFMT_U16_LE | AFMT_S16_LE | AFMT_S24_LE | AFMT_S32_LE))
	return LittleEndian;

    if (mask & (AFMT_U16_BE | AFMT_S16_BE | AFMT_S24_BE | AFMT_S32_BE))
	return BigEndian;

    if (mask & (AFMT_S8 | AFMT_U8))
	return CpuEndian;

    return UnknownEndian;
}

#endif /* HAVE_OSS_SUPPORT */

//***************************************************************************
//***************************************************************************
