#ifndef _SCALE_WIDGET_H_
#define _SCALE_WIDGET_H_ 1

#include <qwidget.h>

class QPainter;

class ScaleWidget : public QWidget
{
    Q_OBJECT

public:

    ScaleWidget(QWidget *parent = 0, int = 0, int = 100, char *unittext = "%");
    ~ScaleWidget();
    void paintText(QPainter *, int, int, int, int, char *);
    void setMaxMin(int, int);
    void setUnit(char *);
    void setLogMode(bool);
    void drawLinear(QPainter *, int, int);
    void drawLog(QPainter *, int, int);

signals:

public slots:

protected:

    void paintEvent(QPaintEvent *);

private:

    int low, high;       //range of display
    bool logmode;        //conditional: logarithmic mode or not
    char *unittext;      //string containing the name of the unit
};

#endif // SCALE_WIDGET_H
