/***************************************************************************
    PlayBack-Phonon.cpp  -  playback device for KDE4-Phonon
			     -------------------
    begin                : Fri May 15 2009
    copyright            : (C) 2009 by Thomas Eschenbacher
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
#ifdef HAVE_PHONON_SUPPORT

#include <errno.h>

#include "libkwave/memcpy.h"

#include "PlayBack-Phonon.h"
#include "SampleEncoderLinear.h"

//***************************************************************************
PlayBackPhonon::PlayBackPhonon()
    :PlayBackDevice(), m_buffer(), m_raw_buffer(), m_buffer_size(0),
     m_buffer_used(0), m_encoder(0),
     m_sem(0), m_output(0), m_path(), m_media_object(),
     m_media_source(this), m_first_write(true)
{
}

//***************************************************************************
PlayBackPhonon::~PlayBackPhonon()
{
}

//***************************************************************************
void PlayBackPhonon::createEncoder(unsigned int bits)
{
    // create the sample encoder
    // we assume that OSS is always little endian
    if (m_encoder) delete m_encoder;

    switch (bits) {
	case 8:
	    m_encoder = new SampleEncoderLinear(
		SampleFormat::Unsigned, 8, LittleEndian);
	    break;
	case 24:
	    m_encoder = new SampleEncoderLinear(
	    SampleFormat::Signed, 24, LittleEndian);
	    break;
	case 32:
	    m_encoder = new SampleEncoderLinear(
		SampleFormat::Signed, 32, LittleEndian);
	    break;
	default:
	    m_encoder = new SampleEncoderLinear(
		SampleFormat::Signed, 16, LittleEndian);
	    break;
    }
}

//***************************************************************************
void PlayBackPhonon::createHeader(double rate,
                                  unsigned int channels,
                                  unsigned int bits)
{
    m_header.resize(44);

    m_header[ 0] = 'R';
    m_header[ 1] = 'I';
    m_header[ 2] = 'F';
    m_header[ 3] = 'F';

    m_header[ 4] = 40;
    m_header[ 5] = 0;
    m_header[ 6] = 0;
    m_header[ 7] = 0;

    m_header[ 8] = 'W';
    m_header[ 9] = 'A';
    m_header[10] = 'V';
    m_header[11] = 'E';

    m_header[12] = 'f';
    m_header[13] = 'm';
    m_header[14] = 't';
    m_header[15] = ' ';

    m_header[16] = 16;
    m_header[17] = 0;
    m_header[18] = 0;
    m_header[19] = 0;

    m_header[20] = 0x01;
    m_header[21] = 0x00;

    m_header[22] = channels;
    m_header[23] = 0;

    int r = static_cast<int>(rate);
    m_header[24] = (r >>  0) & 0xFF;
    m_header[25] = (r >>  8) & 0xFF;
    m_header[26] = (r >> 16) & 0xFF;
    m_header[27] = (r >> 24) & 0xFF;

    int bps = r * ((channels * bits) / 8);
    m_header[28] = (bps >>  0) & 0xFF;
    m_header[29] = (bps >>  8) & 0xFF;
    m_header[30] = (bps >> 16) & 0xFF;
    m_header[31] = (bps >> 24) & 0xFF;

    m_header[32] = (((channels * bits) / 8) >> 0) & 0xFF;
    m_header[33] = (((channels * bits) / 8) >> 8) & 0xFF;

    m_header[34] = (bits >> 0) & 0xFF;
    m_header[35] = (bits >> 8) & 0xFF;

    m_header[36] = 'd';
    m_header[37] = 'a';
    m_header[38] = 't';
    m_header[39] = 'a';

    m_header[40] = static_cast<char>(0xFF);
    m_header[41] = static_cast<char>(0xFF);
    m_header[42] = static_cast<char>(0xFF);
    m_header[43] = static_cast<char>(0xFF);
}

//***************************************************************************
QString PlayBackPhonon::open(const QString &device, double rate,
                             unsigned int channels, unsigned int bits,
                             unsigned int bufbase)
{
    // close the previous device
    if (m_output) delete m_output;
    m_output = 0;

    // create a new Phonon output device
    m_output = new Phonon::AudioOutput(Phonon::MusicCategory);
    Q_ASSERT(m_output);
    if (!m_output) return i18n("out of memory");

    // try to find the matching device
    QList<Phonon::AudioOutputDevice> devices =
	Phonon::BackendCapabilities::availableAudioOutputDevices();

    Phonon::AudioOutputDevice dev;
    foreach(Phonon::AudioOutputDevice d, devices) {
	if (d.name() == device) {
	    dev = d;
	    break;
	}
    }
    if (!dev.isValid()) {
	return i18n("Opening the device '%1' failed.",
	    device.toLocal8Bit().data());
    }

    // set the output device name
    m_output->setOutputDevice(dev);

    // create the path from the stream to the output
    m_media_object.stop();
    m_media_object.clear();
    m_media_object.clearQueue();
    m_first_write = true;
    m_media_object.enqueue(m_media_source);
    m_path = Phonon::createPath(&m_media_object, m_output);

    // create a sample encoder
    createEncoder(bits);
    Q_ASSERT(m_encoder);
    if (!m_encoder) return i18n("out of memory");

    // calculate the new buffer size
    if (bufbase < 8)
	bufbase = 8;
    m_buffer_size = (1 << bufbase);

    // resize the raw buffer
    m_raw_buffer.resize(m_buffer_size);

    // resize our buffer (size in samples) and reset it
    m_buffer_size /= m_encoder->rawBytesPerSample();
    m_buffer.resize(m_buffer_size);

    // create a dummy wave RIFF header
    createHeader(rate, channels, bits);

    m_media_object.play();
    setStreamSize(44);

    return 0;
}

//***************************************************************************
int PlayBackPhonon::write(const Kwave::SampleArray &samples)
{
    Q_ASSERT (m_buffer_used <= m_buffer_size);
    if (m_buffer_used > m_buffer_size) {
	qWarning("PlayBackPhonon::write(): buffer overflow ?!");
	m_buffer_used = m_buffer_size;
	flush();
	return -EIO;
    }

    // number of samples left in the buffer
    unsigned int remaining = samples.size();
    unsigned int offset    = 0;
    while (remaining) {
	unsigned int length = remaining;
	if (m_buffer_used + length > m_buffer_size)
	    length = m_buffer_size - m_buffer_used;

	MEMCPY(&(m_buffer[m_buffer_used]),
	       &(samples[offset]),
	       length * sizeof(sample_t));
	m_buffer_used += length;
	offset        += length;
	remaining     -= length;

	// write buffer to device if it has become full
	if (m_buffer_used >= m_buffer_size) {
	    int err = flush();
	    if (err != -EAGAIN) return err;
	}
    }

    return 0;
}

//***************************************************************************
int PlayBackPhonon::flush()
{
    if (!m_buffer_used || !m_encoder) return 0; // nothing to do

    // convert into byte stream
    m_encoder->encode(m_buffer, m_buffer_used, m_raw_buffer);

    // wait until the Phonon layer has called needData()
    if (!m_sem.tryAcquire(1, 1000)) {
	qDebug("PlayBackPhonon::flush() - EAGAIN");
	m_buffer_used = 0;
	return -EAGAIN;
    }

    writeData(m_raw_buffer);

    m_buffer_used = 0;
    return 0;
}

//***************************************************************************
int PlayBackPhonon::close()
{
    m_media_object.stop();
    m_media_object.clearQueue();
    m_media_object.clear();

    // close the device
    if (m_output) delete m_output;
    m_output = 0;
    m_first_write = true;

    return 0;
}

//***************************************************************************
QStringList PlayBackPhonon::supportedDevices()
{
    QStringList list;

    // get the list of available audio output devices from Phonon
    QList<Phonon::AudioOutputDevice> devices =
	Phonon::BackendCapabilities::availableAudioOutputDevices();

    // get and use the device name(s) from the object description(s)
    foreach(Phonon::AudioOutputDevice device, devices) {
// 	qDebug("name='%s'", device.name().toLocal8Bit().data());
	list << device.name();

	// for debugging: list all properties
// 	foreach (const char *property, device.propertyNames()) {
// 	    qDebug("    '%s' = '%s'", property,
// 		device.property(property).toString().toLocal8Bit().data());
// 	}
    }

    return list;
}

//***************************************************************************
QString PlayBackPhonon::fileFilter()
{
    return "";
}

//***************************************************************************
QList<unsigned int> PlayBackPhonon::supportedBits(const QString &device)
{
    qDebug("%s", __FUNCTION__);
    Q_UNUSED(device);

    QList<unsigned int> list;
    list << 8;
    list << 16;

    return list;
}

//***************************************************************************
int PlayBackPhonon::detectChannels(const QString &device,
                                   unsigned int &min, unsigned int &max)
{
    Q_UNUSED(device);

    min = 1;
    max = 2;

    return 0;
}

//***************************************************************************
void PlayBackPhonon::reset()
{
}

//***************************************************************************
void PlayBackPhonon::needData()
{
    if (m_first_write) {
	m_first_write = false;
	writeData(m_header);
	return;
    }
    m_sem.release();
}

#endif /* HAVE_PHONON_SUPPORT */

//***************************************************************************
//***************************************************************************
