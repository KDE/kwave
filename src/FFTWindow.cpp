#include <math.h>

#include <qkeycode.h>
#include <qstring.h>
#include <qwidget.h>
#include <qpixmap.h>
#include <qtimer.h>

#include <kapp.h>
#include <kselect.h>
#include <ktopwidget.h>
#include <kmenubar.h>
#include <kbuttonbox.h>
#include <kstatusbar.h>

#include <libkwave/gsl_fft.h>

#include "libgui/FFTWidget.h"
#include "libgui/ScaleWidget.h"
#include "libgui/CornerPatchWidget.h"

#include "TopWidget.h"
#include "FFTContainer.h"

#include "FFTWindow.h"

//****************************************************************************
FFTWindow::FFTWindow (QString *name)
    :KTopLevelWidget(name ? name->data() : 0)
{
    corner = 0;
    cursor = 0;
    fftview = 0;
    findPeakID = 0;
    mainwidget = 0;
    status = 0;
    xscale = 0;
    yscale = 0;

    QPopupMenu *fft = new QPopupMenu();
    ASSERT(fft);
    if (!fft) return;

    QPopupMenu *view = new QPopupMenu();
    ASSERT(view);
    if (!view) return;

    cursor = new QPopupMenu();
    ASSERT(cursor);
    if (!cursor) return;

    QPopupMenu *edit = new QPopupMenu();
    ASSERT(edit);
    if (!edit) return;

    QPopupMenu *dbmenu = new QPopupMenu();
    ASSERT(dbmenu);
    if (!dbmenu) return;

    KMenuBar *bar = new KMenuBar (this);
    ASSERT(bar);
    if (!bar) return;

    bar->insertItem (i18n("&Spectral Data"), fft);
    bar->insertItem (i18n("&Edit"), edit);
    bar->insertItem (i18n("&View"), view);
    bar->insertItem (i18n("&Cursor"), cursor);

    status = new KStatusBar (this, "Frequencies Status Bar");
    status->insertItem ("Frequency:          0 Hz     ", 1);
    status->insertItem ("Amplitude:    0 %      ", 2);
    status->insertItem ("Phase:    0        ", 3);
    status->insertItem ("Note:    0        ", 4);

    mainwidget = new FFTContainer (this);
    ASSERT(mainwidget);
    if (!mainwidget) return;

    fftview = new FFTWidget (mainwidget);
    ASSERT(fftview);
    if (!fftview) return;

    xscale = new ScaleWidget (mainwidget, 0, 100, "Hz");
    ASSERT(xscale);
    if (!xscale) return;

    yscale = new ScaleWidget (mainwidget, 100, 0, "%");
    ASSERT(yscale);
    if (!yscale) return;

    corner = new CornerPatchWidget (mainwidget);
    ASSERT(corner);
    if (!corner) return;

    mainwidget->setObjects (fftview, xscale, yscale, corner);

    edit->insertItem(i18n("Multiply with graph"),
                    fftview, SLOT(amplify()));
    edit->insertItem(i18n("Multiply with formant pattern"),
                     fftview, SLOT(formant()));
    edit->insertItem(i18n("Smooth"),
                     fftview, SLOT(smooth()));
    edit->insertSeparator ();
    edit->insertItem(i18n("Kill phase"),
                     fftview, SLOT(killPhase()));
    cursor->insertItem(i18n("find Maximum"),
                       fftview, SLOT(findMaxPeak()), Key_M);
    cursor->insertItem(i18n("find Minimum"),
                       fftview, SLOT(findMinimum()), SHIFT + Key_M);
    findPeakID = cursor->insertItem(i18n("find nearest Peak"),
                                    this, SLOT(findPeak()), Key_Tab);
    cursor->setCheckable( TRUE );
    cursor->setItemChecked( findPeakID, true );

    fft->insertItem(i18n("Inverse FFT"), fftview, SLOT(iFFT()));

    view->insertItem(i18n("Amplitude in %"), this, SLOT(percentMode()));
    view->insertItem(i18n("Amplitude in dB"), dbmenu);

    for (int i = 0; i < 110; i += 10) {
	char buf[64];
	snprintf(buf, sizeof(buf), "0-%d dB", i + 50);
	dbmenu->insertItem (buf);
    }
    connect (dbmenu, SIGNAL (activated(int)), this, SLOT(dbMode(int)));

    view->insertItem (i18n("Phase"), this, SLOT(phaseMode()));

    connect(fftview, SIGNAL(freqInfo(int, int)),
            this, SLOT(setFreqInfo(int, int)));
    connect(fftview, SIGNAL(ampInfo(int, int)),
            this, SLOT(setAmpInfo(int, int)));
    connect(fftview, SIGNAL(noteInfo(int, int)),
            this, SLOT(setNoteInfo(int, int)));
    connect(fftview, SIGNAL(dbInfo(int, int)),
            this, SLOT(setDBInfo(int, int)));
    connect(fftview, SIGNAL(phaseInfo(int, int)),
            this, SLOT(setPhaseInfo(int, int)));

    setView(mainwidget);
    setStatusBar(status);
    setMenu(bar);

    QString *windowname = new QString("");
    ASSERT(windowname);
    if (!windowname) return;

    windowname->append("Frequencies of ");
    windowname->append(name->data());
    setCaption (windowname->data());
    resize (480, 300);
    setMinimumSize (480, 300);
}

//****************************************************************************
void FFTWindow::phaseMode() 
{
    ASSERT(fftview);
    ASSERT(yscale);
    if (!fftview) return;
    if (!yscale) return;

    fftview->phaseMode ();
    yscale->setMaxMin (180, -180);
    yscale->setUnit ("°");
}

//****************************************************************************
void FFTWindow::percentMode()
{
    ASSERT(fftview);
    ASSERT(yscale);
    if (!fftview) return;
    if (!yscale) return;

    fftview->percentMode ();
    yscale->setMaxMin (0, 100);
    yscale->setUnit ("%");
}

//****************************************************************************
void FFTWindow::dbMode(int mode) 
{
    ASSERT(fftview);
    ASSERT(yscale);
    if (!fftview) return;
    if (!yscale) return;

    fftview->dbMode (50 + mode*10);
    yscale->setMaxMin ( -(50 + mode*10), 0);
    yscale->setUnit ("db");
}

//****************************************************************************
void FFTWindow::setSignal(complex *data, double max, int size, int rate) 
{
    ASSERT(fftview);
    ASSERT(xscale);
    if (!fftview) return;
    if (!xscale) return;

    fftview->setSignal (data, size, rate);
    xscale ->setMaxMin (rate / 2, 0);
}

//****************************************************************************
void FFTWindow::findPeak() 
{
    ASSERT(fftview);
    ASSERT(cursor);
    if (!fftview) return;
    if (!cursor) return;

    bool snap = 0;
    fftview->togglefindPeak (&snap);
    cursor->setItemChecked( findPeakID, snap );
    if (snap) {
	fftview->repaint (false);
    }
}

//****************************************************************************
void FFTWindow::askFreqRange() 
{
}

//****************************************************************************
FFTWindow::~FFTWindow() 
{
}

//****************************************************************************
void FFTWindow::setFreqInfo(int hz, int err) 
{
    ASSERT(status);
    if (!status) return;

    char buf[128];
    snprintf(buf, sizeof(buf), "Frequency: %d +/-%d Hz", hz, err);
    status->changeItem (buf, 1);
}

//****************************************************************************
void FFTWindow::setAmpInfo(int amp, int err) 
{
    ASSERT(status);
    if (!status) return;

    char buf[128];
    snprintf(buf, sizeof(buf), "Amplitude: %d +/-%d %%", amp, err);
    status->changeItem (buf, 2);
}

//****************************************************************************
void FFTWindow::setDBInfo(int amp, int err) 
{
    ASSERT(status);
    if (!status) return;

    char buf[128];
    snprintf (buf, sizeof(buf), "Amplitude: %d dB", amp);
    status->changeItem (buf, 2);
}

//****************************************************************************
void FFTWindow::setPhaseInfo(int ph, int err) 
{
    ASSERT(status);
    if (!status) return;

    char buf[128];
    snprintf(buf, sizeof(buf), "Phase: %d +/- %d", ph, err);
    status->changeItem (buf, 3);
}

//****************************************************************************
//Note detection contributed by Gerhard Zintel
void FFTWindow::setNoteInfo (int hz, int x) 
{
    ASSERT(status);
    if (!status) return;

    char buf[128];
    float BaseNoteA = 440.0;    //We should be able to set this in a menu
    static char *notename[] = {
	"C", "C#/Db", "D", "D#/Eb", "E", "F", "F#/Gb", "G", "G#/Ab", "A", "A#/Bb", "B"
    };
    int octave, note;    // note = round(57 + 12 * (log(hz) - log(440)) / log(2))
    note = 45 + (int) (12.0 * (log(hz) - log(BaseNoteA)) / log(2) + 0.5);
    if (note < 0) note = 0;
    octave = note / 12;
    note = note % 12;

    snprintf(buf, sizeof(buf), "Note: %s%d ", notename[note], octave);
    status->changeItem (buf, 4);
}
