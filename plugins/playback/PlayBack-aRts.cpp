/***************************************************************************
      PlayBack-aRts.cpp  -  playback device for aRts sound daemon
			     -------------------
    begin                : Wed Jul 04 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
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
#ifdef HAVE_ARTS_SUPPORT

#include <errno.h>
#include <math.h>
#include <klocale.h>

#include "mt/ThreadsafeX11Guard.h"
#include "PlayBack-aRts.h"

/** use at least 2^8 = 256 bytes for playback buffer */
#define MIN_PLAYBACK_BUFFER 8U

/** use at most 2^16 = 65536 bytes for playback buffer */
#define MAX_PLAYBACK_BUFFER 16U

/** Usage counter for arts_init/arts_free */
static int g_arts_usage = 0;

//***************************************************************************
PlayBackArts::PlayBackArts(QMutex &arts_lock)
    :PlayBackDevice(),
    m_stream(0),
    m_rate(0),
    m_channels(0),
    m_bits(0),
    m_buffer(),
    m_buffer_size(0),
    m_buffer_used(0),
    m_lock_aRts(arts_lock),
    m_closed(true)
{
    qDebug("PlayBackArts::PlayBackArts(...)");
}

//***************************************************************************
PlayBackArts::~PlayBackArts()
{
}

//***************************************************************************
QString PlayBackArts::open(const QString &, double rate,
                           unsigned int channels, unsigned int bits,
                           unsigned int bufbase)
{
    qDebug("PlayBackArts::open(rate=%f,channels=%u,"\
	  "bits=%u, bufbase=%u)", rate, channels,
	  bits, bufbase);

    m_rate        = rate;
    m_channels    = channels;
    m_bits        = bits;
    m_buffer_size = 0;
    m_buffer_used = 0;
    m_stream      = 0;
    QMutexLocker lock(&m_lock_aRts);

    Q_ASSERT(m_closed);
    m_closed = false;

    // initialize aRts playback
    if (!g_arts_usage) {
	int errorcode = arts_init();
	if (errorcode < 0) {
	    qWarning("PlayBackArts::open(): arts_init error: %s",
	            arts_error_text(errorcode));
	    return artsErrorText(errorcode);
        }
    }
    g_arts_usage++;

    // open the stream
    m_stream = arts_play_stream((int)m_rate, m_bits, m_channels, "Kwave");
    Q_ASSERT(m_stream);
    if (!m_stream) return i18n("unable to open aRts playback stream");
    m_closed = false;

    // calculate the buffer size and limit it if necessary
    // ask aRts for the optimum and ignore the user's setting
    m_buffer_size = arts_stream_get(m_stream, ARTS_P_PACKET_SIZE);
    Q_ASSERT(m_buffer_size >= (1U << MIN_PLAYBACK_BUFFER));
    Q_ASSERT(m_buffer_size <= (1U << MAX_PLAYBACK_BUFFER));
    if (m_buffer_size < (1U << MIN_PLAYBACK_BUFFER))
	m_buffer_size = (1U << MIN_PLAYBACK_BUFFER);
    if (m_buffer_size > (1U << MAX_PLAYBACK_BUFFER))
	m_buffer_size = (1U << MAX_PLAYBACK_BUFFER);

    qDebug("PlayBackArts::open(): using %u bytes as buffer", m_buffer_size);
    m_buffer.resize(m_buffer_size);
    m_buffer.fill(0);

    return 0;
}

//***************************************************************************
int PlayBackArts::write(QMemArray<sample_t> &samples)
{
    Q_ASSERT (m_buffer_used < m_buffer_size);
    if (m_buffer_used >= m_buffer_size) {
	qWarning("PlayBackArts::write(): buffer overflow ?!");
	return -EIO;
    }
    Q_ASSERT(samples.count() == m_channels);

    // convert into byte stream
    unsigned int channel;
    for (channel=0; channel < m_channels; channel++) {
	sample_t sample = samples[channel];

	switch (m_bits) {
	    case 8:
		sample += 1 << 23;
		m_buffer[m_buffer_used++] = sample >> 16;
		break;
	    case 16:
		sample += 1 << 23;
		m_buffer[m_buffer_used++] = sample >> 8;
		m_buffer[m_buffer_used++] = (sample >> 16) + 128;
		break;
	    case 24:
		// play in 32 bit format
		m_buffer[m_buffer_used++] = 0x00;
		m_buffer[m_buffer_used++] = sample & 0x0FF;
		m_buffer[m_buffer_used++] = sample >> 8;
		m_buffer[m_buffer_used++] = (sample >> 16) + 128;
		break;
	    default:
		return -EINVAL;
	}
    }

    // write buffer to device if full
    if (m_buffer_used >= m_buffer_size) flush();

    return 0;
}

//***************************************************************************
void PlayBackArts::flush()
{
    if (!m_buffer_used) return; // nothing to do
    Q_ASSERT(m_buffer_used <= m_buffer_size);

    Q_ASSERT(m_stream);
    if (m_stream) {
	QMutexLocker lock(&m_lock_aRts);

	// fill rest of buffer with zeroes
	while (m_buffer_used < m_buffer_size)

	    m_buffer[m_buffer_used++] = 0;
	int errorcode = arts_write(m_stream, m_buffer.data(), m_buffer_used);
	if (errorcode < 0) {
	    qWarning("PlayBackArts: arts_write error: %s",
	             arts_error_text(errorcode));
	}

    }
    m_buffer_used = 0;
}

//***************************************************************************
int PlayBackArts::close()
{
    qDebug("PlayBackArts::close() [pid %d]",(int)pthread_self());
    flush();

    {
	QMutexLocker lock(&m_lock_aRts);

	// close the playback stream
	if (m_stream) arts_close_stream(m_stream);

	if (!m_closed && g_arts_usage && !--g_arts_usage) {
	    qDebug("PlayBackArts::close() [pid %d]: releasing aRts API",
	          (int)pthread_self());
	    arts_free();
	}

	m_stream = 0;
	m_closed = true;
    }

    return 0;
}

//***************************************************************************
QString PlayBackArts::artsErrorText(int errorcode)
{
    QString text = "?";
    switch (errorcode) {
	case ARTS_E_NOSERVER:
	    return(i18n("Can't connect to aRts soundserver. "\
	    "Maybe it is not properly installed or not running."));
	    break;
	case ARTS_E_NOBACKEND:
	    return(i18n("Loading the aRts backend library failed. "\
	    "This might be due to a problem with your aRts "\
	    "installation."));
	    break;
	case ARTS_E_NOSTREAM:
	    return(i18n("The aRts output stream is invalid."));
	    break;
	case ARTS_E_NOINIT:
	    return(i18n("aRts output is not available because the "\
	    "initialization of the aRts output has failed "\
	    "or not been done."));
	    break;
	case ARTS_E_NOIMPL:
	    return(i18n("The required aRts function is not yet "\
	    "implemented. Maybe you should get a newer version of "
	    "aRts or upgrade the kdelibs package."));
	    break;
	default:
	    // unknown error: fall-back to aRts built-in error message
	    text = i18n(arts_error_text(errorcode));
    };

    return text;
};

//***************************************************************************
QStringList PlayBackArts::supportedDevices()
{
    QStringList list; // list stays empty, no selectable devices
    list.append(i18n("[aRts sound daemon]"));
    return list;
}

//***************************************************************************
QValueList<unsigned int> PlayBackArts::supportedBits(const QString &device)
{
    (void)device;
    QValueList <unsigned int> bits;

    // aRts supports only 8 and 16 bits as far as I know...
    bits.append(8);
    bits.append(16);

    return bits;
}

//***************************************************************************
int PlayBackArts::detectChannels(const QString &device,
                                 unsigned int &min, unsigned int &max)
{
    (void) device;

    // aRts supports only mono and stereo as far as I know...
    min = 1;
    max = 2;
    return 0;
}

#endif /* HAVE_ARTS_SUPPORT */

//***************************************************************************
//***************************************************************************
