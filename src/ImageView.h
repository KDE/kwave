#ifndef _IMAGEVIEW_H_
#define _IMAGEVIEW_H_ 1

#include <qwidget.h>
#include <qpainter.h>
#include <qpixmap.h>

class ImageView : public QWidget
{
    Q_OBJECT
public:
    ImageView(QWidget *parent = 0);
    ~ImageView();
    void mouseMoveEvent(QMouseEvent * );
    void setImage(const QImage *image);
    int getWidth();
    int getOffset();

public slots:

    void setOffset(int);

signals:

    void viewInfo(int, int, int);
    void info (double, double);

protected:

    void paintEvent(QPaintEvent *);

private:
    int height, width;
    int offset;
    int lh, lw;
    const QImage *image;
    QPixmap map;
};
//***********************************************************************
#endif // _IMAGEVIEW_H_
