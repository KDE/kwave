#ifndef _CURVE_WIDGET_H_
#define _CURVE_WIDGET_H_ 1

#include <qstring.h>
#include <qstrlist.h>
#include <qwidget.h>
#include <qpainter.h>

#include <libkwave/Interpolation.h>

class Curve;
class QMouseEvent;
class QPaintEvent;
class QPopupMenu;
class Point;
class QPixmap;
class QDir;

class CurveWidget : public QWidget
{
    Q_OBJECT

public:

    CurveWidget(QWidget *parent = 0, const char *init = 0, int = false);
    ~CurveWidget();

    QString getCommand();
    void setCurve(const char *);
    void addPoint(double, double);
    Point* findPoint(int, int);

public slots:

    /**
     * Sets a new interpolation type
     * @see Interpolation
     * @see interpolation_t
     */
    void setType(interpolation_t type);

    void scaleFit();
    void VFlip();
    void HFlip();
    void deleteLast();
    void deleteSecond();
    void firstHalf();
    void secondHalf();
    void savePreset();
    void loadPreset(int);

signals:

protected:

    void mousePressEvent(QMouseEvent * );
    void mouseReleaseEvent(QMouseEvent * );
    void mouseMoveEvent(QMouseEvent * );
    void paintEvent(QPaintEvent *);

private:

    int width, height;            //of widget
    int interpolationtype;         //type of interpolation
    int keepborder;                //flag denying acces to first and last point...e
    double x[7], y[7];            //buffer for polynomial coefficients

    Curve *points;        //Points set by User
    QPopupMenu *menu;
    Point *act;
    Point *last;                  //last Point clicked remembered for deleting
    QPainter p;
    QPixmap *pixmap;          //pixmap to be blitted to screen
    QStrList namelist;

    bool down;
    int knobcount;
    QPixmap *knob;
    QPixmap *selectedknob;

};
//***********************************************************************
#endif // _CURVE_WIDGET_H_
