#ifndef _SCALE_WIDGET_H_
#define _SCALE_WIDGET_H_

#include <qstring.h>
#include <qwidget.h>

class QPainter;
class QSize;

class ScaleWidget : public QWidget
{
public:
    /**
     * Primitve Constructor for usage in a Qt designer's dialog.
     */
    ScaleWidget(QWidget *parent, const char *name) {};

    ScaleWidget(QWidget *parent = 0, int = 0, int = 100, const char *unittext = "%");

    /** Destructor */
    ~ScaleWidget();

    void paintText(QPainter *, int, int, int, int, char *);
    void setMaxMin(int, int);
    void setUnit(const char *text);
    void setLogMode(bool);
    void drawLinear(QPainter *, int, int);
    void drawLog(QPainter *, int, int);

    /** minimum size of the widtget, @see QWidget::minimumSize() */
    virtual const QSize minimumSize();

    /** optimal size for the widget, @see QWidget::sizeHint() */
    virtual const QSize sizeHint();

protected:

    void paintEvent(QPaintEvent *);

private:

    int low, high;       //range of display
    bool logmode;        //conditional: logarithmic mode or not

    /** string containing the name of the unit */
    QString m_unittext;

};

#endif // SCALE_WIDGET_H
