
#include "config.h"
//#include <kapp.h>
//#include <kstatusbar.h>
//
//#include "libgui/ScaleWidget.h"
//#include "libgui/CornerPatchWidget.h"
//
//#include "PitchContainer.h"
//#include "PitchWidget.h"
#include "PitchWindow.h"
//
//extern char *mstotimec (int ms);

//****************************************************************************
PitchWindow::PitchWindow (const char */*name*/)
//    :KTopLevelWidget()
{
//    corner = 0;
//    mainwidget = 0;
//    rate = 0;
//    status = 0;
//    view = 0;
//    xscale = 0;
//    yscale = 0;
//
//    QPopupMenu *pitch = new QPopupMenu();
//    ASSERT(pitch);
//    if (!pitch) return;
//
//    KMenuBar *bar = new KMenuBar (this);
//    ASSERT(bar);
//    if (!bar) return;
//
//    bar->insertItem (i18n("&Pitch"), pitch);
//
//    status = new KStatusBar (this);
//    ASSERT(status);
//    if (!status) return;
//
//    status->insertItem (i18n("Time:         0 ms      "), 1);
//    status->insertItem (i18n("Frequency:          0 Hz"), 2);
//
//    mainwidget = new PitchContainer(this);
//    ASSERT(mainwidget);
//    if (!mainwidget) return;
//
//    view = new PitchWidget (mainwidget);
//    ASSERT(view);
//    if (!view) return;
//
//    xscale = new ScaleWidget (mainwidget, 0, 100, "ms");
//    ASSERT(xscale);
//    if (!xscale) return;
//
//    yscale = new ScaleWidget (mainwidget, 20000, 0, "Hz");
//    ASSERT(yscale);
//    if (!yscale) return;
//
//    corner = new CornerPatchWidget (mainwidget);
//    ASSERT(corner);
//    if (!corner) return;
//
//    mainwidget->setObjects (view, xscale, yscale, corner);
//
//    setView (mainwidget);
//    setStatusBar (status);
//    setMenu (bar);
//    setMinimumSize (320, 200);
//
//    connect (view, SIGNAL(freqRange(float, float)), this, SLOT(freqRange(float, float)));
//    connect (view, SIGNAL(pitch(float)), this, SLOT(showPitch(float)));
//    connect (view, SIGNAL(timeSamples(float)), this, SLOT(showTime(float)));
//
//    QString *windowname = new QString (QString (i18n("Pitch of ")) + QString(name));
//    ASSERT(windowname);
//    if (!windowname) return;
//
//    setCaption (windowname->data());
}

////****************************************************************************
//void PitchWindow::freqRange(float min, float max)
//{
//    ASSERT(yscale);
//    if (!yscale) return;
//
//    yscale->setMaxMin ((int)min, (int)max);
//}
//
////****************************************************************************
//void PitchWindow::showPitch (float freq)
//{
//    ASSERT(status);
//    if (!status) return;
//
//    char buf[64];
//    snprintf(buf, sizeof(buf), i18n("Frequency : %.1f Hz\n"), freq);
//    status->changeItem (buf, 2);
//}
//
////****************************************************************************
//void PitchWindow::showTime (float time)
//{
//    ASSERT(status);
//    if (!status) return;
//
//    char buf[64];
//    snprintf(buf, sizeof(buf), i18n("Time : %s\n"),
//	mstotimec((int)((time*1000 / rate))));
//    status->changeItem (buf, 1);
//}
//
////****************************************************************************
//void PitchWindow::setSignal (float *data, int len, int rate)
////reaches through to class PitchWidget, same Method, last int is ommited, since only used for scales, that are managed from this class...
//{
//    ASSERT(view);
//    ASSERT(xscale);
//    if (!view) return;
//    if (!xscale) return;
//
//    view->setSignal (data, len);
//    xscale->setMaxMin ((int)(((double)len)*10000 / rate), 0);
//    this->rate = rate;
//}

//****************************************************************************
PitchWindow::~PitchWindow () 
{
}

//****************************************************************************
//****************************************************************************
