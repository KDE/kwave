#ifndef _FFT_CONTAINER_H_
#define _FFT_CONTAINER_H_

#include "config.h"
#include <qwidget.h>

class ScaleWidget;
class CornerPatchWidget;
class FFTWidget;

class FFTContainer: public QWidget
{
    Q_OBJECT
public:
    FFTContainer(QWidget *parent);
    ~FFTContainer();
    void setObjects(FFTWidget *fftview, ScaleWidget *x, ScaleWidget *y,
		    CornerPatchWidget *corner);

public slots:

signals:

protected:

    void resizeEvent (QResizeEvent *);

private:
    FFTWidget *view;
    ScaleWidget *xscale, *yscale;
    CornerPatchWidget *corner;
};

#endif // _FFT_CONTAINER_H_
