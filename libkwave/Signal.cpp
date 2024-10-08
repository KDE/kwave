/***************************************************************************
   Signal.cpp - representation of a Kwave signal with multiple tracks
                             -------------------
    begin                : Sat Feb 03 2001
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

#include <math.h>
#include <stdlib.h>

#include <algorithm>
#include <new>

#include <QReadLocker>
#include <QWriteLocker>

#include <KLocalizedString>

#include "libkwave/Curve.h"
#include "libkwave/Filter.h"
#include "libkwave/Interpolation.h"
#include "libkwave/MessageBox.h"
#include "libkwave/Signal.h"
#include "libkwave/Track.h"
#include "libkwave/Utils.h"
#include "libkwave/WindowFunction.h"

//***************************************************************************
Kwave::Signal::Signal()
    :m_tracks(), m_lock_tracks()
{
}

//***************************************************************************
Kwave::Signal::Signal(unsigned int tracks, sample_index_t length)
    :m_tracks(), m_lock_tracks()
{
    while (tracks--) {
        insertTrack(0, length, nullptr);
    }
}

//***************************************************************************
Kwave::Signal::~Signal()
{
    close();
}

//***************************************************************************
void Kwave::Signal::close()
{
    unsigned int count;
    while ((count = tracks()))
        deleteTrack(count - 1);
}

//***************************************************************************
Kwave::Track *Kwave::Signal::insertTrack(unsigned int index,
                                         sample_index_t length,
                                         QUuid *uuid)
{
    Kwave::Track *t = nullptr;
    {
        QWriteLocker lock(&m_lock_tracks);

        t = new(std::nothrow) Kwave::Track(length, uuid);
        Q_ASSERT(t);
        if (!t) return nullptr;

        // insert / append to the list
        unsigned int tracks = static_cast<unsigned int>(m_tracks.size());
        if (index < tracks)
            m_tracks.insert(m_tracks.begin() + index, t);
        else
        {
            index = tracks;
            m_tracks.push_back(t);
        }

        // connect to the new track's signals
        connect(t, SIGNAL(sigSamplesDeleted(Kwave::Track*, sample_index_t,
            sample_index_t)),
            this, SLOT(slotSamplesDeleted(Kwave::Track*, sample_index_t,
            sample_index_t)));
        connect(t, SIGNAL(sigSamplesInserted(Kwave::Track*, sample_index_t,
            sample_index_t)),
            this, SLOT(slotSamplesInserted(Kwave::Track*, sample_index_t,
            sample_index_t)));
        connect(t, SIGNAL(sigSamplesModified(Kwave::Track*, sample_index_t,
            sample_index_t)),
            this, SLOT(slotSamplesModified(Kwave::Track*, sample_index_t,
            sample_index_t)));

        connect(t, SIGNAL(sigSelectionChanged(bool)),
            this, SIGNAL(sigTrackSelectionChanged(bool)));
    }

    // track has been inserted
    emit sigTrackInserted(index, t);
    return t;
}

//***************************************************************************
void Kwave::Signal::deleteTrack(unsigned int index)
{
    // remove the track from the list but do not delete it
    Kwave::Track *t = nullptr;
    {
        QWriteLocker lock(&m_lock_tracks);
        if (static_cast<size_t>(index) > m_tracks.size())
            return; // bail out if not in range

        t = m_tracks.at(index);
        m_tracks.erase(m_tracks.begin() + index);
    }

    // now emit a signal that the track has been deleted. Maybe
    // someone is still using a reference to it, so we have not
    // deleted it yet - still only unlinked from the track list!
    emit sigTrackDeleted(index, t);

    // now everybody knows that the track is gone
    // -> we can safely delete it
    delete t;
}

//***************************************************************************
Kwave::Writer *Kwave::Signal::openWriter(Kwave::InsertMode mode,
        unsigned int track, sample_index_t left, sample_index_t right)
{
    QReadLocker lock(&m_lock_tracks);

    Q_ASSERT(static_cast<size_t>(track) < m_tracks.size());
    if (static_cast<size_t>(track) >= m_tracks.size())
        return nullptr; // track does not exist !

    Kwave::Track *t = m_tracks.at(track);
    Q_ASSERT(t);
    return (t) ? t->openWriter(mode, left, right) : nullptr;
}

//***************************************************************************
Kwave::SampleReader *Kwave::Signal::openReader(Kwave::ReaderMode mode,
                                               unsigned int track,
                                               sample_index_t left,
                                               sample_index_t right)
{
    QReadLocker lock(&m_lock_tracks);

    if (static_cast<size_t>(track) >= m_tracks.size())
        return nullptr; // track does not exist !

    Kwave::Track *t = m_tracks.at(track);
    Q_ASSERT(t);
    return (t) ? t->openReader(mode, left, right) : nullptr;
}

//***************************************************************************
Kwave::Stripe::List Kwave::Signal::stripes(unsigned int track,
                                           sample_index_t left,
                                           sample_index_t right)
{
    QReadLocker lock(&m_lock_tracks);

    if (static_cast<size_t>(track) < m_tracks.size()) {
        Kwave::Track *t = m_tracks.at(track);
        Q_ASSERT(t);
        if (t) return t->stripes(left, right);
    }
    return Kwave::Stripe::List(); // track does not exist !
}

//***************************************************************************
bool Kwave::Signal::mergeStripes(const Kwave::Stripe::List &stripes,
                                 unsigned int track)
{
    QReadLocker lock(&m_lock_tracks);

    if (static_cast<size_t>(track) >= m_tracks.size())
        return false; // track does not exist !

    Kwave::Track *t = m_tracks.at(track);
    Q_ASSERT(t);
    if (!t) return false;

    return t->mergeStripes(stripes);
}

//***************************************************************************
QVector<unsigned int> Kwave::Signal::allTracks()
{
    unsigned int track;
    unsigned int tracks = this->tracks();
    QVector<unsigned int> list;

    for (track = 0; track < tracks; track++) {
        list.append(track);
    }

    return list;
}

//***************************************************************************
void Kwave::Signal::deleteRange(unsigned int track,
                                sample_index_t offset,
                                sample_index_t length)
{
    QReadLocker lock(&m_lock_tracks);

    Q_ASSERT(static_cast<size_t>(track) < m_tracks.size());
    if (static_cast<size_t>(track) >= m_tracks.size())
        return; // track does not exist !

    Kwave::Track *t = m_tracks.at(track);
    Q_ASSERT(t);
    if (t) t->deleteRange(offset, length);
}

//***************************************************************************
void Kwave::Signal::insertSpace(unsigned int track, sample_index_t offset,
                                sample_index_t length)
{
    QReadLocker lock(&m_lock_tracks);

    Q_ASSERT(static_cast<size_t>(track) < m_tracks.size());
    if (static_cast<size_t>(track) >= m_tracks.size())
        return; // track does not exist !

    Kwave::Track *t = m_tracks.at(track);
    Q_ASSERT(t);
    if (t) t->insertSpace(offset, length);
}

//***************************************************************************
unsigned int Kwave::Signal::tracks()
{
    QReadLocker lock(&m_lock_tracks);
    return static_cast<unsigned int>(m_tracks.size());
}

//***************************************************************************
sample_index_t Kwave::Signal::length()
{
    QReadLocker lock(&m_lock_tracks);

    sample_index_t max = 0;
    for (Kwave::Track *track : m_tracks) {
        if (!track) continue;
        sample_index_t len = track->length();
        if (len > max) max = len;
    }
    return max;
}

//***************************************************************************
bool Kwave::Signal::trackSelected(unsigned int track)
{
    QReadLocker lock(&m_lock_tracks);

    if (static_cast<size_t>(track) >= m_tracks.size()) return false;
    if (!m_tracks.at(track)) return false;

    return m_tracks.at(track)->selected();
}

//***************************************************************************
void Kwave::Signal::selectTrack(unsigned int track, bool select)
{
    QReadLocker lock(&m_lock_tracks);

    Q_ASSERT(static_cast<size_t>(track) < m_tracks.size());
    if (static_cast<size_t>(track) >= m_tracks.size()) return;
    Q_ASSERT(m_tracks.at(track));
    if (!m_tracks.at(track)) return;

    m_tracks.at(track)->select(select);
}

//***************************************************************************
QUuid Kwave::Signal::uuidOfTrack(unsigned int track)
{
    QReadLocker lock(&m_lock_tracks);

    if (static_cast<size_t>(track) >= m_tracks.size()) return QUuid();
    Q_ASSERT(m_tracks.at(track));
    if (!m_tracks.at(track)) return QUuid();

    return m_tracks.at(track)->uuid();
}

//// now follow the various editing and effects functions
////**********************************************************
//#define MAXPRIME 512
//int Signal::getChannelMaximum () {
//    int max = 0;
//    for (int i = 0; i < length; i++)
//      if (max < qAbs(sample[i])) max = qAbs(sample[i]);
//
//    return max;
//}
////*********************************************************
//int getMaxPrimeFactor (int len) {
//    int max = 1;
//    int tst = len;
//
//    //here follows the canonical slow prime factor search, but it does its job
//    //with small numbers, greater ones should not occur within this program...
//
//    if (((tst % 2)) == 0) {
//      max = 2;
//      tst /= 2;
//      while ((tst % 2) == 0) tst /= 2;    // remove prime factor 2
//    }
//
//
//    for (int i = 3; i <= sqrt(tst); i += 2)
//      if ((tst % i) == 0) {
//          if (i > max) max = i;
//          while ((tst % i) == 0)
//              tst /= i; // divide the current prime factor until
//                        // it is not present any more
//      }
//
//
//    if (tst > max) max = tst;
//
//    return max;
//}
////*********************************************************
/** @todo needs to be ported to fftw, moved to a plugin and re-activated */
//void Signal::fft (int windowtype, bool accurate)
//{
//    complex *data = 0;
//
//    if (!accurate) {
//      int reduce = 1;
//      int max = getMaxPrimeFactor (len);    //get biggest prime factor
//
//      if (max > MAXPRIME) {
//          while ((len - reduce > MAXPRIME) && (
//                 getMaxPrimeFactor(len - reduce) > MAXPRIME)) reduce++;
//          len -= reduce;   //correct length of buffer to be transferred
//      }
//
//
//    }
//
//    data = new complex[len];
//
//    if (data) {
//      double rea, ima, max = 0;
//
//      for (int i = 0; i < len; i++) {
//          data[i].real = ((double)(sample[begin + i]) / (1 << 23));
//          data[i].imag = 0;
//      }
//
//      gsl_fft_complex_wavetable table;
//
//      gsl_fft_complex_wavetable_alloc (len, &table);
//
//      gsl_fft_complex_init (len, &table);
//
//      gsl_fft_complex_forward (data, len, &table);
//      gsl_fft_complex_wavetable_free (&table);
//
//      for (int i = 0; i < len; i++) {
//          rea = data[i].real;
//          ima = data[i].imag;
//          rea = sqrt(rea * rea + ima * ima);              //get amplitude
//          if (max < rea) max = rea;
//      }
//
//    } else {
//      if (data) delete data;
//      Kwave::MessageBox::error
//      (0, i18n("Info"), i18n("No Memory for FFT-buffers available."), 2);
//    }
//}
//
////*********************************************************
//void Signal::averageFFT (int points, window_function_t windowtype) {
////    complex *data = new complex[points];
////    complex *avgdata = new complex[points];
////    WindowFunction func(windowtype);
////
////    double *windowfunction = func.getFunction (points);
////
////    int count = 0;
////
////    if (data && avgdata && windowfunction) {
////    gsl_fft_complex_wavetable table;
////    gsl_fft_complex_wavetable_alloc (points, &table);
////    gsl_fft_complex_init (points, &table);
////
////    for (int i = 0; i < points; i++) {
////        avgdata[i].real = 0;
////        avgdata[i].imag = 0;
////    }
////
////    double rea, ima, max = 0;
////    int page = 0;
////
////    while (page < len) {
////        if (page + points < len)
////            for (int i = 0; i < points; i++) {
////                data[i].real = (windowfunction[i] *
////                    (double)(sample[begin + i]) / (1 << 23));
////                data[i].imag = 0;
////            }
////        else {
////            int i = 0;
////            for (; i < len - page; i++) {
////                data[i].real = (windowfunction[i] *
////                    (double)(sample[begin + i]) / (1 << 23));
////                data[i].imag = 0;
////            }
////            for (; i < points; i++) {
////                data[i].real = 0;
////                data[i].imag = 0;
////            }
////        }
////
////        page += points;
////        count++;
////        gsl_fft_complex_forward (data, points, &table);
////
////        for (int i = 0; i < points; i++) {
////            rea = data[i].real;
////            ima = data[i].imag;
////            avgdata[i].real += sqrt(rea * rea + ima * ima);
////        }
////    }
////
////    if (data) delete data;
////    gsl_fft_complex_wavetable_free (&table);
////
////    for (int i = 0; i < points; i++)        //find maximum
////    {
////        avgdata[i].real /= count;
////        rea = avgdata[i].real;
////        if (max < rea) max = rea;
////    }
////
////    //create window for object
////    }
////    else {
////    if (data) delete data;
////    Kwave::MessageBox::error
////    (0, i18n("Info"), i18n("No Memory for FFT-buffers available."), 2);
////    }
//}
////*********************************************************
//void Kwave::Signal::movingFilter (Kwave::Filter *filter, int tap,
//                                  Kwave::Curve *points, int low, int high)
//{
//    Kwave::Interpolation interpolation (0);
//
//    QMemArray<double> *move = interpolation.interpolation (points, len);
//    if (move) {
//      for (int i = 0; i < len; i++)
//          //rescale range of filtermovement...
//          move[i] = ((double)low) / 1000 + (((double)(high - low)) / 1000 *
//                    *move[i]);
//
//      double val;
//      double addup = 0;
//      unsigned int max = 0;
//      unsigned int num = filter->count();
//
//      for (unsigned int j = 0; j < num; j++) {
//          addup += fabs(filter->coeff(j));
//          if (max < filter->delay(j))
//              max = filter->delay(j);   // find maximum offset
//      }
//
//      if (filter->isFIR()) {
//          for (unsigned int i = begin + len - 1; i >= begin + max; i--) {
//              filter->setCoeff(tap, (*move[i - begin]));
//              val = filter->coeff(0) * sample[i];
//              for (unsigned int j = 1; j < filter->count(); j++)
//                  val += filter->coeff(j) * sample[i - filter->delay(j)];
//              sample[i] = (int)(val / addup);      //renormalize
//          }
//
//
//          // slower routine because of check, needed only in this range...
//          for (unsigned int i = begin + max - 1; i >= begin; i--)
//          {
//              filter->setCoeff(tap, *move[i - begin]);
//              val = filter->coeff(0) * sample[i];
//              for (unsigned int j = 1; j < filter->count(); j++)
//                  if (i - filter->delay(j) > 0)
//                      val += filter->coeff(j) * sample[i - filter->delay(j)];
//              sample[i] = (int)(val / addup);      //renormalize
//          }
//
//
//      } else {
//          // basically the same,but the loops go viceversa
//
//          //slower routine because of check, needed only in this range...
//          for (unsigned int i = begin; i < begin + max; i++) {
//              filter->setCoeff(tap, *move[i - begin]);
//              val = filter->coeff(0) * sample[i];
//              for (unsigned int j = 1; j < filter->count(); j++)
//                  if (i - filter->delay(j) > 0)
//                      val += filter->coeff(j) * sample[i - filter->delay(j)];
//              sample[i] = (int)(val / addup);      //renormalize
//          }
//
//
//          for (unsigned int i = begin + max; i < begin + len; i++) {
//              filter->setCoeff(tap, *move[i - begin]);
//              val = filter->coeff(0) * sample[i];
//              for (unsigned int j = 1; j < filter->count(); j++)
//                  val += filter->coeff(j) * sample[i - filter->delay(j)];
//              sample[i] = (int)(val / addup);      //renormalize
//          }
//
//
//      }
//
//      delete move;
//    }
//}
//
////*********************************************************
//void Kwave::Signal::replaceStutter (int len1, int len2) {
//    int *sample = &(this->sample[lmarker]);
//
//    int j;
//    int i = len2;
//    while (i < len - len1) {
//      for (j = 0; j < len1; j++) sample[i + j] = 0;
//      i += len1 + len2;
//      counter = i;
//    }
//    counter = -1;
//}
//
////**********************************************************
//void Kwave::Signal::mixPaste (Signal *signal) {
//    int pastelength = signal->getLength ();
//    int *paste = signal->getSample();
//    int marked = (lmarker != rmarker) ? rmarker - lmarker + 1 : length;
//    if (pastelength > marked) pastelength = marked;
//    if (lmarker + pastelength > length) pastelength = length - lmarker;
//
//    int *sample = this->sample + lmarker;
//    while (pastelength--) {
//      *sample = (*paste + *sample) >> 1;
//      sample++;
//      paste++;
//    }

//***************************************************************************
unsigned int Kwave::Signal::trackIndex(const Kwave::Track *track)
{
    QReadLocker lock(&m_lock_tracks);

    const auto it = std::find(m_tracks.cbegin(), m_tracks.cend(), track);
    return static_cast<unsigned int>( (it != m_tracks.cend()) ?
        std::distance(m_tracks.cbegin(), it) :
        m_tracks.size());
}

//***************************************************************************
void Kwave::Signal::slotSamplesInserted(Kwave::Track *src,
                                        sample_index_t offset,
                                        sample_index_t length)
{
    unsigned int track = trackIndex(src);
    emit sigSamplesInserted(track, offset, length);
}

//***************************************************************************
void Kwave::Signal::slotSamplesDeleted(Kwave::Track *src,
                                       sample_index_t offset,
                                       sample_index_t length)
{
    unsigned int track = trackIndex(src);
    emit sigSamplesDeleted(track, offset, length);
}

//***************************************************************************
void Kwave::Signal::slotSamplesModified(Kwave::Track *src,
                                        sample_index_t offset,
                                        sample_index_t length)
{
    unsigned int track = trackIndex(src);
    emit sigSamplesModified(track, offset, length);
}

//***************************************************************************
//***************************************************************************

#include "moc_Signal.cpp"
