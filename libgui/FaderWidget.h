#ifndef _FADER_WIDGET_H_
#define _FADER_WIDGET_H_ 1

#include <qstring.h>
#include <qwidget.h>

class FaderWidget : public QWidget {
    Q_OBJECT
public:
    FaderWidget(QWidget *parent = 0, int dir = 1);

    ~FaderWidget();

    QString getDegree();

public slots:

    void setCurve(int);

signals:

protected:

    void paintEvent(QPaintEvent *);

private:
    int dir;
    int curve;
    int width, height;            //of widget
};
//***********************************************************************
#endif // _FADER_WIDGET_H_
