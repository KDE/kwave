#ifndef _FFT_WINDOW_H_
#define _FFT_WINDOW_H_ 1

#include <qobject.h>
#include <qwidget.h> // ###
//#include <ktmainwindow.h>

//#include "libkwave/gsl_fft.h"
//
//class FFTWidget;
//class FFTContainer;
//class QPopupMenu;
//class ScaleWidget;
//class QString;
//class CornerPatchWidget;
//class KStatusBar;

//***********************************************************************
class FFTWindow : public QWidget // KTMainWindow
{
    Q_OBJECT
public:

    FFTWindow(const char *name);

    virtual ~FFTWindow();

//    //reaches through to class FFTWidget...
//    void setSignal(complex *, double, int, int);
//
//public slots:
//
//    void setFreqInfo(int, int);
//    void setAmpInfo(int, int);
//    void setDBInfo(int, int);
//    void setPhaseInfo(int, int);
//    void setNoteInfo(int, int);
//    void phaseMode();
//    void dbMode(int);
//    void percentMode();
//    void askFreqRange();
//    void findPeak();
//
//signals:
//
//protected:
//
//private:
//    FFTContainer *mainwidget;
//    FFTWidget *fftview;
//    QPopupMenu *cursor;
//    ScaleWidget *xscale, *yscale;
//    CornerPatchWidget *corner;
//    KStatusBar *status;
//    int findPeakID;
};

#endif // _FFT_WINDOW_H_
