#ifndef _OVER_VIEW_WIDGET_H_
#define _OVER_VIEW_WIDGET_H_ 1

#include <qpushbt.h>
#include <qwidget.h>
#include <qcombo.h>
#include <qpixmap.h>
#include <qtimer.h>
#include <qframe.h>
#include <kselect.h>
//#include <ktopwidget.h>
#include <kbuttonbox.h>
#include <kbutton.h>

//class MainWidget;

//***********************************************************
//OverviewWidget is the scrollbar in the main window
//QScrollbar has proven to be unstable with high numbers.
//this one also features a small overview of the part of the sample being
//unseen
class OverViewWidget : public QWidget
{
    Q_OBJECT
public:
    OverViewWidget(QWidget *parent = 0, const char *name = 0);
//    OverViewWidget(MainWidget *parent = 0, const char *name = 0);
    ~OverViewWidget();
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void setSignal(char *);
    void setValue(int);
    void refresh();

    /** minimum size of the widtget, @see QWidget::minimumSize() */
    virtual const QSize minimumSize();
    /** optimal size for the widget, @see QWidget::sizeHint() */
    virtual const QSize sizeHint();

public slots:

    /*
     * Sets new range parameters of the slider, using a scale that is calculated
     * out of the slider's maximum position.
     * @param new_val position of the slider (left or top of the bar)
     * @param new_width width of the slider (bar)
     * @param new_length size of the slider's area
     */
    void setRange(int new_pos, int new_width, int new_length);

    void increase();

signals:

    void valueChanged (int);

protected:

    void paintEvent(QPaintEvent *);

private:

    int width, height;
    bool grabbed;
    int slider_width;
    int slider_length;
    int slider_pos;
    int dir;           //addup for direction...
    bool redraw;           //flag for redrawing pixmap
//    MainWidget *mparent;
    QWidget *parent;
    QTimer *timer;        //to spare user repeated pressing of the widget...
    QPixmap *pixmap;      //pixmap to be blitted to screen
}
;

#endif // _OVER_VIEW_WIDGET_H_
