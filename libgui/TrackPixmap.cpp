/***************************************************************************
        TrackPixmap.cpp  -  buffered pixmap for displaying a kwave track
                             -------------------
    begin                : Tue Mar 20 2001
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

#include <new>

#include <QMutexLocker>
#include <QPainter>
#include <QPolygon>
#include <QTime>

#include "libkwave/SampleReader.h"
#include "libkwave/Track.h"

#include "libgui/TrackPixmap.h"

/**
 * This factor determines how many times the order is over than
 * the minimum required order. Higher values give less problems with
 * aliasing but some amplitude errors. Lower values make less
 * amplitude errors but more aliasing problems.
 * This should be a good compromise...
 */
#define INTERPOLATION_PRECISION 4

/**
 * If the zoom factor is below this margin, an interpolation
 * will be used.
 */
#define INTERPOLATION_ZOOM 0.10

//***************************************************************************
Kwave::TrackPixmap::TrackPixmap(Kwave::Track &track)
    :QObject(), m_pixmap(), m_track(track), m_offset(0), m_zoom(0.0),
    m_vertical_zoom(1.0), m_minmax_mode(false),
    m_sample_buffer(), m_min_buffer(), m_max_buffer(),
    m_modified(false), m_valid(0), m_lock_buffer(),
    m_interpolation_order(0), m_interpolation_alpha(),
    m_colors(Kwave::Colors::Normal)
{
    // connect all the notification signals of the track
    connect(&track,
        SIGNAL(sigSamplesInserted(Kwave::Track *, sample_index_t,
        sample_index_t)), this,
        SLOT(slotSamplesInserted(Kwave::Track *,
        sample_index_t, sample_index_t)));
    connect(&track, SIGNAL(sigSamplesDeleted(Kwave::Track *, sample_index_t,
        sample_index_t)), this, SLOT(slotSamplesDeleted(Kwave::Track *,
        sample_index_t, sample_index_t)));
    connect(&track, SIGNAL(sigSamplesModified(Kwave::Track *, sample_index_t,
        sample_index_t)), this, SLOT(slotSamplesModified(Kwave::Track *,
        sample_index_t, sample_index_t)));
    connect(&track, SIGNAL(sigSelectionChanged(bool)),
            this, SLOT(selectionChanged()));
}

//***************************************************************************
Kwave::TrackPixmap::~TrackPixmap()
{
    QMutexLocker lock(&m_lock_buffer);
    m_interpolation_alpha.clear();
}

//***************************************************************************
void Kwave::TrackPixmap::setOffset(sample_index_t offset)
{
    QMutexLocker lock(&m_lock_buffer);
    if (offset == m_offset) return; // no change
    const unsigned int buflen = static_cast<unsigned int>(m_valid.size());

    if (m_minmax_mode) {
        // move content of min and max buffer
        // one buffer element = one screen pixel
        Q_ASSERT(buflen == m_min_buffer.size());
        Q_ASSERT(buflen == m_max_buffer.size());
        if ((buflen != m_min_buffer.size()) ||
            (buflen != m_max_buffer.size()))
        {
            qDebug("TrackPixmap::setOffset(): buflen = %u", buflen);
            qDebug("TrackPixmap::setOffset(): min_buffer : %u",
                   m_min_buffer.size());
            qDebug("TrackPixmap::setOffset(): max_buffer : %u",
                   m_max_buffer.size());
        }

        // check for misaligned offset changes
        if ((  offset % static_cast<sample_index_t>(ceil(m_zoom))) !=
            (m_offset % static_cast<sample_index_t>(ceil(m_zoom)))) {

#ifdef CURRENTLY_UNUSED
// this will become interesting later, with the offset/zoom optimizations
            qWarning("TrackPixmap::setOffset(): oh nooo, "\
                "offset %u is misaligned by %u sample(s), please fix this!",
                offset, offset % pixels2samples(1));
            qWarning("TrackPixmap::setOffset(): "\
                "now I have to throw away the whole buffer :-((");

            qDebug("TrackPixmap::setOffset(%u): "\
                "misaligned->invalidating buffer", offset);
#endif
            invalidateBuffer();
        } else if (offset > m_offset) {
            // move left
            int diff = samples2pixels(offset - m_offset);
//          qDebug("TrackPixmap::setOffset(): moving left (min/max): %u",diff);
            Q_ASSERT(diff);
            Q_ASSERT(buflen);
            if (diff && buflen) {
                unsigned int src = Kwave::toUint(diff);
                unsigned int dst = 0;
                while (src < buflen) {
                    m_min_buffer[dst] = m_min_buffer[src];
                    m_max_buffer[dst] = m_max_buffer[src];
                    m_valid[dst++]    = m_valid[src++];
                }
                while (dst < buflen) m_valid.clearBit(dst++);
            }
        } else {
            // move right
            int diff = Kwave::toInt(samples2pixels(m_offset - offset));
//          qDebug("TrackPixmap::setOffset(): moving right (min/max): %u",diff);
            Q_ASSERT(diff);
            Q_ASSERT(buflen);
            if (diff && buflen) {
                int dst = buflen - 1;
                while (dst >= diff) {
                    int src = dst - diff;
                    m_min_buffer[dst] = m_min_buffer[src];
                    m_max_buffer[dst] = m_max_buffer[src];
                    m_valid[dst--]    = m_valid[src--];
                }
                diff = dst + 1;
                while (diff--) m_valid.clearBit(dst--);
            }
        }
    } else {
        // move content of sample buffer
        // one buffer element = one sample
        Q_ASSERT(buflen == m_sample_buffer.size());

        if (offset > m_offset) {
            // move left
//          qDebug("TrackPixmap::setOffset(): moving left (normal)");
            unsigned int diff = Kwave::toUint(offset - m_offset);
            unsigned int src  = Kwave::toUint(diff);
            unsigned int dst  = 0;
            while (src < buflen) {
                m_sample_buffer[dst] = m_sample_buffer[src];
                m_valid[dst++]       = m_valid[src++];
            }
            while (dst < buflen) m_valid.clearBit(dst++);
        } else {
            // move right
//          qDebug("TrackPixmap::setOffset(): moving right (normal)");
            int diff = Kwave::toInt(m_offset - offset);
            Q_ASSERT(buflen);
            if (buflen) {
                int dst = buflen - 1;
                while (dst >= diff) {
                    int src = dst - diff;
                    m_sample_buffer[dst] = m_sample_buffer[src];
                    m_valid[dst--]       = m_valid[src];
                }
                diff = dst + 1;
                while (diff--) m_valid.clearBit(dst--);
            }
        }
    }

    m_offset = offset;
    m_modified = true;
}

//***************************************************************************
void Kwave::TrackPixmap::resizeBuffer()
{
    bool ok = true;
    int buflen;
    int oldlen = static_cast<int>(m_valid.size());
    int w      = width();
    Q_ASSERT(w >= 0);

    if (m_minmax_mode) {
        // one buffer index == one screen pixel
        buflen = w;
        ok &= m_min_buffer.resize(buflen);
        Q_ASSERT(ok);
        ok &= m_max_buffer.resize(buflen);
        Q_ASSERT(ok);
    } else {
        // one buffer index == one sample
        buflen = Kwave::toInt(pixels2samples(w));
        ok &= m_sample_buffer.resize(buflen);
        Q_ASSERT(ok);
    }
    m_valid.resize(buflen);
    while (oldlen < buflen) m_valid.clearBit(oldlen++);
}

//***************************************************************************
void Kwave::TrackPixmap::setZoom(double zoom)
{
    QMutexLocker lock(&m_lock_buffer);

    Q_ASSERT(zoom >= 0.0);
    if (qFuzzyCompare(zoom, m_zoom)) return; // no change

//    qDebug("TrackPixmap::setZoom(%0.3f)", zoom);
    if ((zoom > 1.0) && !m_minmax_mode) {
        // switch to min/max mode
//      qDebug("TrackPixmap::setZoom(): switch to min/max mode");
        invalidateBuffer();
        m_minmax_mode = true;
    } else if ((zoom <= 1.0) && m_minmax_mode) {
        // switch to normal mode
//      qDebug("TrackPixmap::setZoom(): switch to normal mode");
        invalidateBuffer();
        m_minmax_mode = false;
    }

    // take the new zoom and resize the buffer
    m_zoom = zoom;
    if (m_minmax_mode) {
        // TODO: some clever caching instead of throwing away everything
        resizeBuffer();
        invalidateBuffer();
    } else {
        resizeBuffer();
    }

    m_modified = true;
}

//***************************************************************************
void Kwave::TrackPixmap::resize(int width, int height)
{
    QMutexLocker lock(&m_lock_buffer);

    int old_width = m_pixmap.width();
    int old_height = m_pixmap.height();
    if ((old_width == width) && (old_height == height)) return; // no change

    m_pixmap = QPixmap(width, height);
    if (width != old_width) resizeBuffer();

    m_modified = true;
}

//***************************************************************************
void Kwave::TrackPixmap::invalidateBuffer()
{
    m_valid.fill(false);
    m_modified = true;
}

//***************************************************************************
bool Kwave::TrackPixmap::validateBuffer()
{
    int first = 0;
    int last = 0;
    int buflen = static_cast<int>(m_valid.size());

    sample_index_t left  = m_offset;
    sample_index_t right = (m_track.length()) ? (m_track.length() - 1) : 0;
    Kwave::SampleReader *reader = m_track.openReader(
        Kwave::SinglePassForward, left, right);
    Q_ASSERT(reader);
    if (!reader) return false;

    if (m_minmax_mode) {
        Q_ASSERT(Kwave::toInt(m_min_buffer.size()) == buflen);
        Q_ASSERT(Kwave::toInt(m_max_buffer.size()) == buflen);
    } else {
        Q_ASSERT(Kwave::toInt(m_sample_buffer.size()) == buflen);
    }

    // work-around for missing extra buffer, delete the whole buffer
    // instead. this should not do any harm, in this mode we only
    // have few samples and redrawing will be fast
    if (m_zoom < INTERPOLATION_ZOOM) invalidateBuffer();

    while (first < buflen) {
        // find the first invalid index
        for (first = last; (first < buflen) && m_valid.testBit(first);
                ++first)
        {}

        // break if the first index is out of range
        if (first >= buflen) {
            delete reader;
            return false; // buffer is already ok
        }

        // find the last invalid index
        for (last = first; (last < buflen) && !m_valid[last]; ++last) {
        }

        if (last >= buflen) last = buflen - 1;
        if ((last > first) && (m_valid[last])) --last;

        // fill our array(s) with fresh sample data
        if (m_minmax_mode) {
            // indices are in pixels, convert to samples

            // first sample in first pixel
            sample_index_t s1 = m_offset +
                static_cast<sample_index_t>(floor(first * m_zoom));
            // last sample of last pixel
            sample_index_t s2 = m_offset +
                static_cast<sample_index_t>(floor((last+1) * m_zoom)) - 1;

            while (first <= last) {
                s2 = m_offset + static_cast<sample_index_t>(
                    floor((first + 1) * m_zoom));

                // get min/max for interval [s1...s2]
                sample_t min;
                sample_t max;
                reader->minMax(s1, s2, min, max);

                m_min_buffer[first] = min;
                m_max_buffer[first] = max;
                m_valid.setBit(first);

                // advance to the next position
                ++first;
                s1 = s2 + 1;
            }
        } else {
            // each index is one sample
            // -> read directly into the buffer
            reader->seek(m_offset + first);
            unsigned int count = reader->read(m_sample_buffer,
                first, last - first + 1);
            while (count) {
                m_valid.setBit(first++);
                count--;
            }

            // fill the rest with zeroes
            while (first <= last) {
                m_valid.setBit(first);
                m_sample_buffer[first++] = 0;
            }

        }

        Q_ASSERT(first >= last);
        ++last;
    }

#ifdef DEBUG
    for (first = 0; first < m_valid.size(); first++) {
        if (!m_valid[first]) qWarning("TrackPixmap::validateBuffer(): "\
                "still invalid index: %u", first);
    }
#endif /* DEBUG */

    delete reader;

    return true;
}

//***************************************************************************
void Kwave::TrackPixmap::repaint()
{
    QMutexLocker lock(&m_lock_buffer);

    int w = width();
    int h = height();

    if (!w || !h) return; // not valid yet

    if (m_track.selected()) {
        m_colors = Kwave::Colors::Normal;
    } else {
        m_colors = Kwave::Colors::Disabled;
    }

    QPainter p(&m_pixmap);
    p.fillRect(0, 0, w, h, m_colors.background);

    if (m_zoom > 0) {
        // first make the buffer valid
        validateBuffer();

        // then draw the samples
        if (m_minmax_mode) {
            drawOverview(p, h >> 1, h, 0, w - 1);
        } else {
            if (m_zoom < INTERPOLATION_ZOOM) {
                drawInterpolatedSignal(p, w, h >> 1, h);
            } else {
                drawPolyLineSignal(p, w, h >> 1, h);
            }
        }

        // draw the zero-line
        int last = (m_track.length() > m_offset) ?
            samples2pixels(m_track.length() - 1 - m_offset) : 0;
        p.setPen(m_colors.zero);
        if (last >= w) {
            p.drawLine(0, h >> 1, w - 1, h >> 1);
        } else {
            p.drawLine(0, h >> 1, last, h >> 1);
            p.setPen(m_colors.zero_unused);
            p.drawLine(last, h >> 1, w, h >> 1);
        }
    }

    // now we are no longer "modified"
    m_modified = false;
}

//***************************************************************************
void Kwave::TrackPixmap::setVerticalZoom(double zoom)
{
    QMutexLocker lock(&m_lock_buffer);

    if (qFuzzyCompare(zoom, m_vertical_zoom)) return;
    m_vertical_zoom = zoom;
    m_modified = true;
}

//***************************************************************************
bool Kwave::TrackPixmap::isModified()
{
    QMutexLocker lock(&m_lock_buffer);
    return m_modified;
}

//***************************************************************************
void Kwave::TrackPixmap::selectionChanged()
{
    QMutexLocker lock(&m_lock_buffer);
    m_modified = true;
}

//***************************************************************************
void Kwave::TrackPixmap::drawOverview(QPainter &p, int middle, int height,
                                      int first, int last)
{
    const Kwave::SampleArray &min_buffer = m_min_buffer;
    const Kwave::SampleArray &max_buffer = m_max_buffer;

    Q_ASSERT(m_minmax_mode);
    Q_ASSERT(width() <= Kwave::toInt(min_buffer.size()));
    Q_ASSERT(width() <= Kwave::toInt(max_buffer.size()));

    // scale_y: pixels per unit
    double scale_y = (m_vertical_zoom * height) / (1 << SAMPLE_BITS);

    p.setPen(m_colors.sample);
    int last_min = Kwave::toInt(min_buffer[first] * scale_y);
    int last_max = Kwave::toInt(max_buffer[first] * scale_y);
    for (int i = first; i <= last; i++) {
        Q_ASSERT(m_valid[i]);
        int max = Kwave::toInt(max_buffer[i] * scale_y);
        int min = Kwave::toInt(min_buffer[i] * scale_y);

        // make sure there is a connection between this
        // section and the one before, avoid gaps
        if (min > last_max + 1) min = last_max + 1;
        if (max + 1 < last_min) max = last_min - 1;

        p.drawLine(i, middle - max, i, middle - min);

        last_min = min;
        last_max = max;
    }
}

//***************************************************************************
void Kwave::TrackPixmap::calculateInterpolation()
{
    double f;
    double Fg;
    int k;
    int N;

//    qDebug("TrackPixmap::calculateInterpolation()");

    // remove all previous coefficents and signal buffer
    m_interpolation_alpha.clear();

    Q_ASSERT(!qFuzzyIsNull(m_zoom));
    if (qFuzzyIsNull(m_zoom)) return;

    // offset: index of first visible sample (left) [0...length-1]
    // m_zoom: number of samples / pixel

    // approximate the 3dB frequency of the low pass as
    // Fg = f_g / f_a
    // f_a: current "sample rate" of display (pixels) = 1.0
    // f_g: signal rate = (m_zoom/2)
    Fg = m_zoom / 2;

    // N: order of the filter, at least 2 * (1 / m_zoom)
    N = samples2pixels(INTERPOLATION_PRECISION);
    N |= 0x01;    // make N an odd number !

    // allocate a buffer for the coefficients
    m_interpolation_alpha = QVector<double>(N + 1);
    m_interpolation_order = N;

    Q_ASSERT(m_interpolation_alpha.count() == (N + 1));
    if (m_interpolation_alpha.count() != (N + 1)) return;

    // calculate the raw coefficients and
    // apply a Hamming window
    //
    //                    sin( (2k-N) * Pi * Fg )                       2kPi
    // alpha_k = 2 * Fg * ----------------------- * [ 0,54 - 0,46 * cos ---- ]
    //                      (2k - N) * Pi * Fg                            N
    //
    f = 0.0;    // (store the sum of all coefficients in "f")
    for (k = 0; k <= N; ++k) {
        m_interpolation_alpha[k] =
            sin((2 * k - N) * M_PI * Fg) / ((2 * k - N) * M_PI * Fg);
        m_interpolation_alpha[k] *= (0.54 - 0.46 * cos(2 * k * M_PI / N));
        f += m_interpolation_alpha[k];
    }
    // norm the coefficients to 1.0 / m_zoom
    f *= m_zoom;
    for (k = 0; k <= N; ++k)
        m_interpolation_alpha[k] /= f;

}

//***************************************************************************
void Kwave::TrackPixmap::drawInterpolatedSignal(QPainter &p, int width,
        int middle, int height)
{
    float *sig;
    float *sig_buffer;
    float scale_y;
    int i;
    int k;
    int N;
    int sample;
    int x;
    int buflen = static_cast<int>(m_valid.size());
    const Kwave::SampleArray &sample_buffer = m_sample_buffer;

//     qDebug("TrackPixmap::drawInterpolatedSignal()");

    Q_ASSERT(m_zoom > 0);
    if (m_zoom <= 0) return;

    // scale_y: pixels per unit
    scale_y = static_cast<float>(m_vertical_zoom * height) /
              static_cast<float>((SAMPLE_MAX + 1) << 1);

    // N: order of the filter, at least 2 * (1/m_zoom)
    N = samples2pixels(INTERPOLATION_PRECISION);
    N |= 0x01;    // make N an odd number !

    // re-calculate the interpolation's filter and buffers
    // if the current order has changed
    if (m_interpolation_order != N) {
        calculateInterpolation();
        N = m_interpolation_order;
    }

    Q_ASSERT(m_interpolation_alpha.count() == (N + 1));
    if (m_interpolation_alpha.count() != (N + 1)) return;

    // buffer for intermediate resampled data
    sig_buffer = new(std::nothrow) float[width + N + 2];
    Q_ASSERT(sig_buffer);
    if (!sig_buffer) return;

    // fill the sample buffer with zeroes
    for (i = 0; i < width + N + 2; ++i)
        sig_buffer[i] = 0.0;

    // resample
    x = -1 * samples2pixels(2);
    sample = -2;    // start some samples left of the window
    sig = sig_buffer + (N / 2);
    while (x <= width + N / 2) {
        if ((x >= -N / 2) && (sample > 0) && (sample < buflen)) {
            sig[x] = static_cast<float>(sample_buffer[sample]) * scale_y;
        }
        sample++;
        x = Q_LIKELY(sample >= 0) ?
            samples2pixels(sample) :
            (-1 * samples2pixels(-1 * sample));
    }

    // array with sample points
    QPolygon points;

    // pass the signal data through the filter
    for (i = 0; i < width; ++i) {
        sig = sig_buffer + (i + N);
        float y = 0.0;
        for (k = 0; k <= N; ++k, --sig)
            y += (*sig) * static_cast<float>(m_interpolation_alpha[k]);
        points.append(QPoint(i, middle - Kwave::toInt(y)));
    }

    // display the filter's interpolated output
    p.setPen(m_colors.interpolated);
    p.drawPolyline(points);

    // display the original samples
    sample = 0;
    x = samples2pixels(sample);
    sig = sig_buffer + (N / 2);
    p.setPen(m_colors.sample);
    i = 0;
    points.clear();
    while (x < width) {
        if (x >= 0) {
            // mark original samples
            points.append(QPoint(x, middle - Kwave::toInt(sig[x])));
        }
        sample++;
        x = samples2pixels(sample);
    }
    p.drawPoints(points);

    delete[] sig_buffer;
}

//***************************************************************************
void Kwave::TrackPixmap::drawPolyLineSignal(QPainter &p, int width,
        int middle, int height)
{
    const Kwave::SampleArray &sample_buffer = m_sample_buffer;
    double scale_y;
    unsigned int sample;
    unsigned int buflen = sample_buffer.size();

    // scale_y: pixels per unit
    scale_y = (m_vertical_zoom * static_cast<double>(height)) /
        static_cast<double>((SAMPLE_MAX + 1) << 1);

    // array with sample points
    QPolygon points;

    // display the original samples
    sample = 0;
    int x = 0;
    int y;
    while (x < width) {
        // mark original samples
        sample_t value = (sample < buflen) ? sample_buffer[sample] : 0;
        y = Kwave::toInt(value * scale_y);
        points.append(QPoint(x, middle - y));

        sample++;
        x = samples2pixels(sample);
    }

    // interpolate the rest of the display if necessary
    if (samples2pixels(sample - 1) <= width) {
        int x1;
        int x2;
        double y1;
        double y2;

        x1 = samples2pixels(sample - 1);
        x2 = samples2pixels(sample);

        y1 = ((sample) && (sample <= buflen)) ? (scale_y *
            static_cast<double>(sample_buffer[sample - 1])) : 0.0;
        y2 = (sample < buflen) ? (scale_y *
            static_cast<double>(sample_buffer[sample    ])) : 0.0;

        x = width - 1;
        y = Kwave::toInt(static_cast<double>(x - x1) * (y2 - y1) /
                         static_cast<double>(x2 - x1));

        points.append(QPoint(x, middle - y));
    }

    if (m_zoom >= 1.0) {
        // show only poly-line (bright)
        p.setPen(Qt::white);
        p.drawPolyline(points);
    } else {
        // show the poly-line (dark)
        p.setPen(Qt::darkGray);
        p.drawPolyline(points);

        // show the original points (bright)
        p.setPen(Qt::white);
        p.drawPoints(points);
    }
}

//***************************************************************************
void Kwave::TrackPixmap::slotSamplesInserted(Kwave::Track *,
                                             sample_index_t offset,
                                             sample_index_t length)
{
    {
        QMutexLocker lock(&m_lock_buffer);

        convertOverlap(offset, length);
        if (!length) return; // false alarm

        // mark all positions from here to right end as "invalid"
        const qsizetype first = offset;
        const qsizetype last  = m_valid.size();
        Q_ASSERT(first < m_valid.size());
        Q_ASSERT(last  > first);
        m_valid.fill(false, first, last);

        // repaint of the signal is needed
        m_modified = true;
    }

    // notify our owner about changed data -> screen refresh?
    emit sigModified();
}

//***************************************************************************
void Kwave::TrackPixmap::slotSamplesDeleted(Kwave::Track *,
                                            sample_index_t offset,
                                            sample_index_t length)
{
    {
        QMutexLocker lock(&m_lock_buffer);

        convertOverlap(offset, length);
        if (!length) return; // false alarm

        // mark all positions from here to right end as "invalid"
        const qsizetype first = offset;
        const qsizetype last  = m_valid.size();
        Q_ASSERT(first < m_valid.size());
        Q_ASSERT(last  > first);
        m_valid.fill(false, first, last);

        // repaint of the signal is needed
        m_modified = true;
    }

    // notify our owner about changed data -> screen refresh?
    emit sigModified();
}

//***************************************************************************
void Kwave::TrackPixmap::slotSamplesModified(Kwave::Track *,
                                             sample_index_t offset,
                                             sample_index_t length)
{
    {
        QMutexLocker lock(&m_lock_buffer);

        convertOverlap(offset, length);
        if (!length) return; // false alarm

        // mark all overlapping positions as "invalid"
        const int first = Kwave::toInt(offset);
        const int last  = Kwave::toInt(offset + length);
        Q_ASSERT(first < m_valid.size());
        Q_ASSERT(last  > first);
        m_valid.fill(false, first, last);

        // repaint of the signal is needed
        m_modified = true;
    }

    // notify our owner about changed data -> screen refresh?
    emit sigModified();
}

//***************************************************************************
void Kwave::TrackPixmap::convertOverlap(sample_index_t &offset,
                                        sample_index_t &length)
{
    if (m_zoom <= 0.0) length = 0;
    if (!length) return;
    if ((offset + length) <= m_offset) {
        length = 0;
        return; // not yet in view
    }

    unsigned int buflen = static_cast<unsigned int>(m_valid.size());
    if (!buflen) {
        offset = 0;
        length = 0;
        return;
    }

    // calculate the length
    if (m_minmax_mode) {
        // attention: round up the length int this mode!
        if (offset >= m_offset + static_cast<sample_index_t>(
            ceil(buflen * m_zoom)))
        {
            length = 0; // out of view
            return;
        } else {
            length = static_cast<sample_index_t>(
                ceil(static_cast<double>(length) / m_zoom));
        }
    } else {
        if (offset >= m_offset + buflen) {
            length = 0; // out of view
            return;
        }
    }

    // convert the offset
    offset = (offset > m_offset) ? offset - m_offset : 0;
    if (m_minmax_mode) {
        // attention: round down in this mode!
        double         ofs_d = static_cast<double>(offset);
        sample_index_t ofs = static_cast<sample_index_t>(floor(ofs_d / m_zoom));

        // if offset was rounded down, increment length
        if (ofs != static_cast<sample_index_t>(ceil(ofs_d / m_zoom)))
            length++;
        offset = ofs;
    }

    // limit the offset (maybe something happened when rounding)
    if (offset >= buflen) offset = buflen - 1;

    // limit the length to the end of the buffer
    if (offset + length > buflen) length = buflen - offset;

    Q_ASSERT(length);
}

//***************************************************************************
//***************************************************************************

#include "moc_TrackPixmap.cpp"
