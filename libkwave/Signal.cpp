// I/O Functions such as loading/saving are in sampleio.cpp

#include <math.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

#include <qarray.h>
#include <qlist.h>

#include <klocale.h>
#include <kmessagebox.h>

#include "libkwave/MultiTrackReader.h"
#include "libkwave/MultiTrackWriter.h"
#include "libkwave/Track.h"
#include "libkwave/WindowFunction.h"

#include "Signal.h"
#include "Parser.h"
#include "gsl_fft.h"
#include "Interpolation.h"
#include "Curve.h"
#include "Filter.h"

#include "mt/SharedLockGuard.h"

#define MAXPRIME 512

//**********************************************************
//void Signal::getMaxMin (int &max, int &min, int begin, int len) {
//    int c, first, last;
//    min =  INT_MAX;
//    max =  INT_MIN;
//
//    // first assign left and right margin
//    first = begin;
//    last = begin + len - 1;
//
//    // then do some range checking
//    if (first < 0) first = 0;
//    if (first >= length) first = length - 1;
//    if (last < first) last = first;
//    if (last >= length) last = length - 1;
//
//    for (int i = first; i <= last; i++) {
//	c = sample[i];
//	if (c > max) max = c;
//	if (c < min) min = c;
//    }
//}

//***************************************************************************
Signal::Signal()
    :m_tracks(), m_lock_tracks(), m_rate(0), m_bits(0)
{
}

//***************************************************************************
Signal::Signal(unsigned int tracks, unsigned int length)
    :m_tracks(), m_lock_tracks(), m_rate(0), m_bits(0)
{
    while (tracks--) {
	appendTrack(length);
    }
}

//***************************************************************************
Signal::~Signal()
{
    close();
}

//***************************************************************************
void Signal::close()
{
    SharedLockGuard lock(m_lock_tracks, true);

    m_tracks.setAutoDelete(true);
    while (m_tracks.count()) {
	m_tracks.remove(m_tracks.last());
    }
    m_bits = 0;
    m_rate = 0;
}

//***************************************************************************
Track *Signal::insertTrack(unsigned int index, unsigned int length)
{
    unsigned int track_nr = 0;
    Track *t = 0;
    {
	SharedLockGuard lock(m_lock_tracks, true);
	
	t = new Track(length);
	ASSERT(t);
	if (!t) return 0;
	
	if (index < m_tracks.count()) {
	    // insert into the list
	    track_nr = index;
	    m_tracks.insert(index, t);
	} else {
	    // append to the end ot the list
	    track_nr = m_tracks.count();
	    m_tracks.append(t);
	}
	
	// connect to the track's signals
	connect(t, SIGNAL(sigSamplesDeleted(Track&, unsigned int,
	    unsigned int)),
	    this, SLOT(slotSamplesDeleted(Track&, unsigned int,
	    unsigned int)));
	connect(t, SIGNAL(sigSamplesInserted(Track&, unsigned int,
	    unsigned int)),
	    this, SLOT(slotSamplesInserted(Track&, unsigned int,
	    unsigned int)));
	connect(t, SIGNAL(sigSamplesModified(Track&, unsigned int,
	    unsigned int)),
	    this, SLOT(slotSamplesModified(Track&, unsigned int,
	    unsigned int)));
    }

    // track has been inserted at the end
    if (t) emit sigTrackInserted(track_nr, *t);
    return t;
}


//***************************************************************************
Track *Signal::appendTrack(unsigned int length)
{
    return insertTrack(tracks(), length);
}

//***************************************************************************
void Signal::deleteTrack(unsigned int index)
{
    // remove the track from the list but do not delete it
    Track *t = 0;
    {
	SharedLockGuard lock(m_lock_tracks, true);
	if (index > m_tracks.count()) return; // bail out if not in range
	
	t = m_tracks.at(index);
	m_tracks.setAutoDelete(false);
	m_tracks.remove(index);
    }

    // now emit a signal that the track has been deleted. Maybe
    // someone is still using a reference to it, so we have not
    // deleted it yet - still only unlinked from the track list!
    emit sigTrackDeleted(index);

    // as everybody now knows that the track is gone, we can safely
    // delete it now.
    if (t) delete t;
}

//***************************************************************************
SampleWriter *Signal::openSampleWriter(unsigned int track,
	InsertMode mode, unsigned int left, unsigned int right)
{
    SharedLockGuard lock(m_lock_tracks, false);

    ASSERT(track < m_tracks.count());
    if (track >= m_tracks.count()) {
	return 0; // track does not exist !
    }

    Track *t = m_tracks.at(track);
    ASSERT(t);
    return (t) ? t->openSampleWriter(mode, left, right) : 0;
}

//***************************************************************************
SampleReader *Signal::openSampleReader(unsigned int track,
	unsigned int left, unsigned int right)
{
    SharedLockGuard lock(m_lock_tracks, false);

    ASSERT(track < m_tracks.count());
    if (track >= m_tracks.count()) return 0; // track does not exist !

    Track *t = m_tracks.at(track);
    ASSERT(t);
    return (t) ? t->openSampleReader(left, right) : 0;
}

//***************************************************************************
void Signal::openMultiTrackReader(MultiTrackReader &readers,
    const QArray<unsigned int> &track_list,
    unsigned int first, unsigned int last)
{
    unsigned int count = track_list.count();
    unsigned int track;
    readers.setAutoDelete(true);
    readers.clear();
    readers.resize(count);

    for (unsigned int i=0; i < count; i++) {
	track = track_list[i];
	SampleReader *s = openSampleReader(track, first, last);
	ASSERT(s);
	readers.insert(i, s);
    }

}

//***************************************************************************
void Signal::openMultiTrackWriter(MultiTrackWriter &writers,
    const QArray<unsigned int> &track_list, InsertMode mode,
    unsigned int left, unsigned int right)
{
    unsigned int count = track_list.count();
    unsigned int track;
    writers.clear();
    writers.resize(count);

    for (unsigned int i=0; i < count; i++) {
	track = track_list[i];
	SampleWriter *s = openSampleWriter(track, mode, left, right);
	if (s) {
	    writers.insert(i, s);
	} else {
	    // out of memory or aborted
	    debug("Signal::openMultiTrackWriter: "\
	          "out of memory or aborted");
	    writers.clear();
	    return;
	}
    }
}

//***************************************************************************
const QArray<unsigned int> Signal::allTracks()
{
    unsigned int track;
    QArray<unsigned int> list(tracks());

    for (track=0; track < list.count(); track++) {
	list[track] = track;
    }

    return list;
}

//***************************************************************************
void Signal::deleteRange(unsigned int track, unsigned int offset,
                         unsigned int length)
{
    SharedLockGuard lock(m_lock_tracks, false);

    ASSERT(track < m_tracks.count());
    if (track >= m_tracks.count()) return; // track does not exist !

    Track *t = m_tracks.at(track);
    ASSERT(t);
    if (t) t->deleteRange(offset, length);
}

//***************************************************************************
unsigned int Signal::tracks()
{
    SharedLockGuard lock(m_lock_tracks, false);
    return m_tracks.count();
}

//***************************************************************************
unsigned int Signal::length()
{
    SharedLockGuard lock(m_lock_tracks, false);

    unsigned int max = 0;
    unsigned int len;

    QListIterator<Track> it(m_tracks);
    for ( ; it.current(); ++it) {
	len = it.current()->length();
	if (len > max) max = len;
    }
//    debug("Signal::length() = %d", max);
    return max;
}

//***************************************************************************
bool Signal::trackSelected(unsigned int track)
{
    SharedLockGuard lock(m_lock_tracks, false);

    if (track >= m_tracks.count()) return false;
    if (!m_tracks.at(track)) return false;

    return m_tracks.at(track)->selected();
}

//***************************************************************************
void Signal::selectTrack(unsigned int track, bool select)
{
    SharedLockGuard lock(m_lock_tracks, false);

    ASSERT(track < m_tracks.count());
    if (track >= m_tracks.count()) return;
    ASSERT(m_tracks.at(track));
    if (!m_tracks.at(track)) return;

    m_tracks.at(track)->select(select);
}

////**********************************************************
//int Signal::getSingleSample(int offset) {
//    if ((offset < 0) || (offset >= length)) return 0;
//    return sample[offset];
//}
//

//// now follow the various editing and effects functions
////**********************************************************
//int Signal::getChannelMaximum () {
//    int max = 0;
//    for (int i = 0; i < length; i++)
//	if (max < abs(sample[i])) max = abs(sample[i]);
//
//    return max;
//}
////*********************************************************
//void Signal::resample (const char *) {
//    int newrate = 44100;
//    // ### WRONG: BETTER USE LOWPASS INTERPOLATION !!! ###
//    Interpolation interpolation(INTPOL_SPLINE);   //get Spline Interpolation
//    Curve *points = new Curve;
//
//    //  int oldmax=getChannelMaximum();
//
//    int newlen = (int)((double)length * (double)newrate / rate);
//    int *newsample = getNewMem(newlen);
//
//    if (newsample && points) {
//	for (int i = 0; i < length; i++)
//	    points->append (((double) i) / length, (double) sample[i]);
//
//	QArray<double> *y = interpolation.interpolation(points, newlen);
//	if (y) {
//	    delete points;
//	    points = 0;
//
//	    getridof (sample);
//
//	    sample = newsample;
//	    for (int i = 0; i < newlen; i++) newsample[i] = (int)*y[i];
//
//	    length = newlen;
//	    begin = 0;
//	    len = length;
//	    //      amplifyChannelMax (oldmax);
//	    rate = newrate;
//	    delete y;
//	} else getridof (newsample);
//    } else getridof (newsample);
//    if (points) delete points;
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
//	max = 2;
//	tst /= 2;
//	while ((tst % 2) == 0) tst /= 2;    //remove prime factor 2
//    }
//
//
//    for (int i = 3; i <= sqrt(tst); i += 2)
//	if ((tst % i) == 0) {
//	    if (i > max) max = i;
//	    while ((tst % i) == 0) tst /= i;    //divide the current prime factor until it is not present any more
//	}
//
//
//    if (tst > max) max = tst;
//
//    return max;
//}
////*********************************************************
//void Signal::fft (int windowtype, bool accurate)
//{
//    complex *data = 0;
//
//    if (!accurate) {
//	int reduce = 1;
//	int max = getMaxPrimeFactor (len);    //get biggest prime factor
//
//	if (max > MAXPRIME) {
//	    while ((len - reduce > MAXPRIME) && (getMaxPrimeFactor(len - reduce) > MAXPRIME)) reduce++;
//	    len -= reduce;   //correct length of buffer to be transferred
//	}
//
//
//    }
//
//    data = new complex[len];
//
//    if (data) {
//	double rea, ima, max = 0;
//
//	for (int i = 0; i < len; i++) {
//	    data[i].real = ((double)(sample[begin + i]) / (1 << 23));
//	    data[i].imag = 0;
//	}
//
//	gsl_fft_complex_wavetable table;
//
//	gsl_fft_complex_wavetable_alloc (len, &table);
//
//	gsl_fft_complex_init (len, &table);
//
//	gsl_fft_complex_forward (data, len, &table);
//	gsl_fft_complex_wavetable_free (&table);
//
//	for (int i = 0; i < len; i++) {
//	    rea = data[i].real;
//	    ima = data[i].imag;
//	    rea = sqrt(rea * rea + ima * ima);              //get amplitude
//	    if (max < rea) max = rea;
//	}
//
//    } else {
//	if (data) delete data;
//	KMessageBox::error
//	(0, i18n("Info"), i18n("No Memory for FFT-buffers available !"), 2);
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
////	gsl_fft_complex_wavetable table;
////	gsl_fft_complex_wavetable_alloc (points, &table);
////	gsl_fft_complex_init (points, &table);
////
////	for (int i = 0; i < points; i++) {
////	    avgdata[i].real = 0;
////	    avgdata[i].imag = 0;
////	}
////
////	double rea, ima, max = 0;
////	int page = 0;
////
////	while (page < len) {
////	    if (page + points < len)
////		for (int i = 0; i < points; i++) {
////		    data[i].real = (windowfunction[i] * (double)(sample[begin + i]) / (1 << 23));
////		    data[i].imag = 0;
////		}
////	    else {
////		int i = 0;
////		for (; i < len - page; i++) {
////		    data[i].real = (windowfunction[i] * (double)(sample[begin + i]) / (1 << 23));
////		    data[i].imag = 0;
////		}
////		for (; i < points; i++) {
////		    data[i].real = 0;
////		    data[i].imag = 0;
////		}
////	    }
////
////	    page += points;
////	    count++;
////	    gsl_fft_complex_forward (data, points, &table);
////
////	    for (int i = 0; i < points; i++) {
////		rea = data[i].real;
////		ima = data[i].imag;
////		avgdata[i].real += sqrt(rea * rea + ima * ima);
////	    }
////	}
////
////	if (data) delete data;
////	gsl_fft_complex_wavetable_free (&table);
////
////	for (int i = 0; i < points; i++)        //find maximum
////	{
////	    avgdata[i].real /= count;
////	    rea = avgdata[i].real;
////	    if (max < rea) max = rea;
////	}
////
////	//create window for object
////    }
////    else {
////	if (data) delete data;
////	KMessageBox::error
////	(0, i18n("Info"), i18n("No Memory for FFT-buffers available !"), 2);
////    }
//}
////*********************************************************
//void Signal::movingFilter (Filter *filter, int tap, Curve *points, int low, int high)
//{
//    Interpolation interpolation (0);
//
//    QArray<double> *move = interpolation.interpolation (points, len);
//    if (move) {
//	for (int i = 0; i < len; i++)
//	    //rescale range of filtermovement...
//	    move[i] = ((double)low) / 1000 + (((double)(high - low)) / 1000 * *move[i]);
//
//	double val;
//	double addup = 0;
//	unsigned int max = 0;
//	unsigned int num = filter->count();
//
//	for (unsigned int j = 0; j < num; j++) {
//	    addup += fabs(filter->coeff(j));
//	    if (max < filter->delay(j)) max = filter->delay(j);   //find maximum offset
//	}
//
//	if (filter->isFIR()) {
//	    for (unsigned int i = begin + len - 1; i >= begin + max; i--) {
//		filter->setCoeff(tap, (*move[i - begin]));
//		val = filter->coeff(0) * sample[i];
//		for (unsigned int j = 1; j < filter->count(); j++)
//		    val += filter->coeff(j) * sample[i - filter->delay(j)];
//		sample[i] = (int)(val / addup);      //renormalize
//	    }
//
//
//	    // slower routine because of check, needed only in this range...
//	    for (unsigned int i = begin + max - 1; i >= begin; i--)
//	    {
//		filter->setCoeff(tap, *move[i - begin]);
//		val = filter->coeff(0) * sample[i];
//		for (unsigned int j = 1; j < filter->count(); j++)
//		    if (i - filter->delay(j) > 0)
//			val += filter->coeff(j) * sample[i - filter->delay(j)];
//		sample[i] = (int)(val / addup);      //renormalize
//	    }
//
//
//	} else {
//	    // basically the same,but the loops go viceversa
//	
//	    //slower routine because of check, needed only in this range...
//	    for (unsigned int i = begin; i < begin + max; i++) {
//		filter->setCoeff(tap, *move[i - begin]);
//		val = filter->coeff(0) * sample[i];
//		for (unsigned int j = 1; j < filter->count(); j++)
//		    if (i - filter->delay(j) > 0)
//			val += filter->coeff(j) * sample[i - filter->delay(j)];
//		sample[i] = (int)(val / addup);      //renormalize
//	    }
//
//
//	    for (unsigned int i = begin + max; i < begin + len; i++) {
//		filter->setCoeff(tap, *move[i - begin]);
//		val = filter->coeff(0) * sample[i];
//		for (unsigned int j = 1; j < filter->count(); j++)
//		    val += filter->coeff(j) * sample[i - filter->delay(j)];
//		sample[i] = (int)(val / addup);      //renormalize
//	    }
//
//
//	}
//	
//	delete move;
//    }
//}
//
////*********************************************************
//void Signal::replaceStutter (int len1, int len2) {
//    int *sample = &(this->sample[lmarker]);
//
//    int j;
//    int i = len2;
//    while (i < len - len1) {
//	for (j = 0; j < len1; j++) sample[i + j] = 0;
//	i += len1 + len2;
//	counter = i;
//    }
//    counter = -1;
//}
//
////**********************************************************
//void Signal::mixPaste (Signal *signal) {
//    int pastelength = signal->getLength ();
//    int *paste = signal->getSample();
//    int marked = (lmarker != rmarker) ? rmarker - lmarker + 1 : length;
//    if (pastelength > marked) pastelength = marked;
//    if (lmarker + pastelength > length) pastelength = length - lmarker;
//
//    int *sample = this->sample + lmarker;
//    while (pastelength--) {
//	*sample = (*paste + *sample) >> 1;
//	sample++;
//	paste++;
//    }

//***************************************************************************
unsigned int Signal::trackIndex(const Track &track)
{
    SharedLockGuard lock(m_lock_tracks, false);

    int index = m_tracks.findRef(&track);
    ASSERT(index >= 0);
    return (index >= 0) ? index : m_tracks.count();
}

//***************************************************************************
void Signal::slotSamplesInserted(Track &src, unsigned int offset,
                                 unsigned int length)
{
    unsigned int track = trackIndex(src);
    emit sigSamplesInserted(track, offset, length);
}

//***************************************************************************
void Signal::slotSamplesDeleted(Track &src, unsigned int offset,
                                unsigned int length)
{
    unsigned int track = trackIndex(src);
    emit sigSamplesDeleted(track, offset, length);
}

//***************************************************************************
void Signal::slotSamplesModified(Track &src, unsigned int offset,
                                 unsigned int length)
{
    unsigned int track = trackIndex(src);
    emit sigSamplesModified(track, offset, length);
}

//***************************************************************************
//***************************************************************************
