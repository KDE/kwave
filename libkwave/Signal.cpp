// I/O Functions such as loading/saving are in sampleio.cpp

//Here choose biggest prime factor to be tolerated before
//popping up a requester, when doing a fft
#include <math.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

#include <qarray.h>
#include <qlist.h>

#include <klocale.h>
#include <kmessagebox.h>

#include "Signal.h"
#include "Parser.h"
#include "gsl_fft.h"
#include "libkwave/WindowFunction.h"
#include "Interpolation.h"
#include "Curve.h"
#include "Filter.h"
#include "FileFormat.h"

#include "mt/MutexGuard.h"

#include "libkwave/Track.h"

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
    :m_tracks(), m_lock_tracks(), m_rate(0), m_bits(0),
     m_selected(true), m_selection_start(0), m_selection_end(0)
{
}

//***************************************************************************
Signal::Signal(unsigned int tracks, unsigned int length)
    :m_tracks(), m_lock_tracks(), m_rate(0), m_bits(0),
     m_selected(true), m_selection_start(0), m_selection_end(0)
{
    while (tracks--) {
	appendTrack(length);
    }
}

//***************************************************************************
Signal::~Signal()
{
}

//***************************************************************************
void Signal::close()
{
    MutexGuard lock(m_lock_tracks);

    debug("Signal::close()");
    m_tracks.setAutoDelete(true);
    while (m_tracks.count()) {
	m_tracks.remove(m_tracks.last());
    }
}

//***************************************************************************
Track *Signal::appendTrack(unsigned int length)
{
    unsigned int track_nr = 0;
    Track *t = 0;
    {
	MutexGuard lock(m_lock_tracks);

	t = new Track(length);
	ASSERT(t);
	if (!t) return 0;

	m_tracks.append(t);

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
	
	track_nr = m_tracks.count()-1;
    }

    // track has been inserted at the end
    emit sigTrackInserted(track_nr);
    return t;
}

//***************************************************************************
SampleInputStream *Signal::openInputStream(unsigned int track,
	InsertMode mode, unsigned int left, unsigned int right)
{
    MutexGuard lock(m_lock_tracks);

    ASSERT(track < m_tracks.count());
    if (track >= m_tracks.count()) {
	return 0; // track does not exist !
    }

    return m_tracks.at(track)->openInputStream(mode, left, right);
}

//***************************************************************************
unsigned int Signal::tracks()
{
    MutexGuard lock(m_lock_tracks);
    return m_tracks.count();
}

//***************************************************************************
unsigned int Signal::length()
{
    MutexGuard lock(m_lock_tracks);

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

////**********************************************************
//int Signal::getSingleSample(int offset) {
//    if ((offset < 0) || (offset >= length)) return 0;
//    return sample[offset];
//}
//
////**********************************************************
//void Signal::setMarkers (int l, int r )
////this one sets the internal markers, most operations use
//// and does all of the necessary range checks
//{
//    if (l > r) {
//	register int h = l;
//	l = r;
//	r = h;
//    }
//    if (l < 0) l = 0;
//    if (r >= length) r = length - 1;
//    lmarker = l;
//    rmarker = r;
//}
//// now follow the various editing and effects functions
////**********************************************************
//int Signal::getChannelMaximum () {
//    int max = 0;
//    for (int i = 0; i < length; i++)
//	if (max < abs(sample[i])) max = abs(sample[i]);
//
//    return max;
//}
////**********************************************************
//void Signal::changeRate (int newrate) {
//    rate = newrate;
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
////Following are clipboard functions of Signal Class
////**********************************************************
//void Signal::noMemory () {
//    KMessageBox::error(0, i18n("Info"), i18n("Not enough Memory for Operation !"), 2);
//}
////**********************************************************
//Signal *Signal::copyRange()
//{
//    if (rmarker != lmarker) //this function makes no sense if no Range is selected
//    {
//	int len = rmarker - lmarker + 1;
//	sample_t *newsam = getNewMem(len);
//	if (newsam) {
//	    memcpy(newsam, sample + lmarker, len*sizeof(int));
//	    return new Signal(newsam, len, rate);
//	} else noMemory();
//    }
//    return 0;
//}
////**********************************************************
//// shouldn't this be "insert()" instead of "insertPaste()" ?
//void Signal::insertPaste (Signal *signal) {
//    int insertlength = signal->getLength ();
//    int *insert = signal->getSample();
//
//    if (insert && insertlength) {
//	int newlength = length + insertlength;
//	int *newsam = (int *)realloc(sample, newlength * sizeof(int));
//	if (newsam) {
//	    memmove(newsam + lmarker + insertlength, newsam + lmarker,
//		    (length - lmarker)*sizeof(int));
//	    memcpy(newsam + lmarker, insert, insertlength*sizeof(int));
//
//	    sample = newsam;
//	    length = newlength;
//	} else noMemory ();
//    } else KMessageBox::error(0, i18n("Info"), i18n("signal is empty !"), 2);
//}
////**********************************************************
//void Signal::overwritePaste (Signal *signal) {
//    int pastelength = signal->getLength ();
//    int *paste = signal->getSample();
//    int marked = (lmarker != rmarker) ? rmarker - lmarker + 1 : length;
//    if (pastelength > marked) pastelength = marked;
//    if (lmarker + pastelength > length) pastelength = length - lmarker;
//
//    memcpy(sample + lmarker, paste, pastelength*sizeof(int));
//}
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
//}
////**********************************************************
//Signal *Signal::cutRange ()
////fine example for reusal of code, my prof would say, don't show him the rest
//// of my code
//{
//    Signal *tmp = copyRange ();
//    if (tmp) deleteRange ();
//    return tmp;
//}
////**********************************************************
//void Signal::deleteRange () {
//    // example:
//    // sample = [01(23)45678], length = 9
//    // lmarker = 2, rmarker = 3
//    // -> selected = 3-2+1 = 2
//    // -> rest = 9-2 = 7
//    // -> left = [0..1] (length=2)
//    // -> right = [4..8] (length=9-3-1=5)
//    int selected = rmarker - lmarker + 1;   // selected samples
//    int rest = length - selected;       // rest after delete
//
//    if (rest && length && sample) {
//	if (rmarker < length - 1) {
//	    // copy data after the right marker
//	    memmove(sample + lmarker, sample + rmarker + 1,
//		    (length - rmarker - 1)*sizeof(int));
//	}
//	int *newsam = (int *)realloc(sample, rest * sizeof(int));
//
//	if (newsam != 0) {
//	    sample = newsam;
//	    length = rest;
//	} else {
//	    // oops, not enough memory !
//	    noMemory();
//	}
//    } else {
//	// delete everything
//	if (sample != 0) delete []sample;
//	sample = 0;
//	length = 0;
//    }
//
//    // correct the right marker
//    setMarkers(lmarker - 1, lmarker - 1);
//}
////**********************************************************
//void Signal::cropRange () {
//    if (rmarker != lmarker) {
//	int *newsam;
//	int newlength = rmarker - lmarker + 1;
//
//	memmove(sample, sample + lmarker, newlength*sizeof(int));
//	length = newlength;
//
//	newsam = (int *)realloc(sample, newlength * sizeof(int));
//	if (newsam != 0) sample = newsam;
//	// NOTE: if the realloc failed, the old memory
//	// will remain allocated and only length will be
//	// reduced - so the user won't see that.
//	// The (dead) memory will be freed on the next operation
//	// that calls "delete sample".
//    }
//
//    // correct the markers
//
//
//    setMarkers(lmarker, rmarker);
//}

//***************************************************************************
unsigned int Signal::trackIndex(const Track &track)
{
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
