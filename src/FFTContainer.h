#include <qpainter.h>
#include <qwidget.h>
#include "../libgui/FFTWidget.h"
#include "../libgui/ScaleWidget.h"

class FFTContainer : public QWidget
{
 Q_OBJECT
 public:
	FFTContainer    (QWidget *parent);
	~FFTContainer   ();
 void   setObjects      (FFTWidget *fftview,ScaleWidget *x,ScaleWidget *y,CornerPatchWidget *corner);

 public slots:

 signals:

 protected:

 void   resizeEvent     (QResizeEvent *);

 private:
 FFTWidget     *view;
 ScaleWidget   *xscale,*yscale;
 CornerPatchWidget *corner;
};

