#ifndef _PITCH_WIDGET_H_
#define _PITCH_WIDGET_H_

#include "config.h"
#include <qwidget.h>

class QPixmap;
class QMouseEvent;
class QPaintEvent;

//***********************************************************************
class PitchWidget : public QWidget
{
    Q_OBJECT
public:
    PitchWidget(QWidget *parent = 0);
    ~PitchWidget();
    void mousePressEvent(QMouseEvent * );
    void mouseReleaseEvent(QMouseEvent * );
    void mouseMoveEvent(QMouseEvent * );
    void setSignal(float *, int);
    void refresh();

public slots:

signals:

    void freqRange(float, float);
    void pitch(float);
    void timeSamples(float);

protected:

    void getMaxMin();
    void paintEvent(QPaintEvent *);

private:

    float *data;
    float max, min;
    int len;
    int width, height;     //of widget
    bool redraw;

    QPixmap *pixmap;       //pixmap to be blitted to screen
}
;
//***********************************************************************
#endif // _PITCH_WIDGET_H_
