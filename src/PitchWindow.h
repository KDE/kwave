#ifndef _PITCH_WINDOW_H_
#define _PITCH_WINDOW_H_ 1

#include <ktopwidget.h>

class PitchWidget;
class PitchContainer;
class ScaleWidget;
class CornerPatchWidget;
class KStatusBar;

//***********************************************************************
class PitchWindow : public KTopLevelWidget
{
    Q_OBJECT
public:
    PitchWindow(const char *name);
    ~PitchWindow();
    void setSignal(float *, int, int);

public slots:

    void freqRange(float, float);
    void showPitch(float);
    void showTime(float);

signals:

protected:

private:
    PitchWidget *view;
    PitchContainer *mainwidget;
    ScaleWidget *xscale, *yscale;
    KStatusBar *status;
    CornerPatchWidget *corner;
    int rate;
};

#endif // _PITCH_WINDOW_H_
