#ifndef _FORMANT_WIDGET_H_
#define _FORMANT_WIDGET_H_

#include "config.h"
#include <qwidget.h>

//***********************************************************************
class FormantWidget : public QWidget {
    Q_OBJECT
public:
    FormantWidget (QWidget *parent, int rate);
    ~FormantWidget ();

    double *getPoints (int);

public slots:

    void setFormants (int, int *, int *);

signals:

    void dbscale (int, int);

protected:

    void paintEvent(QPaintEvent *);

private:

    int *pos;                      //array of formant-positions
    int *widths;                   //array of formantwidth
    int num;                       //number of formants
    int width, height;            //of widget
    int rate;
    double *points;
};
//***********************************************************************
#endif // _FORMANT_WIDGET_H_
