#include <qwidget.h>
#include "../libgui/OverViewWidget.h"
#include "../libgui/ScaleWidget.h"

class ImageView;
//***********************************************************************
class SonagramContainer : public QWidget
{
 Q_OBJECT
 public:
 	SonagramContainer	(QWidget *parent);
 	~SonagramContainer	();
 void 	setObjects	(ImageView *view,ScaleWidget *x,ScaleWidget *y,CornerPatchWidget *corner,OverViewWidget *overview=0);

 public slots:

 signals:

 protected:

 void	resizeEvent	(QResizeEvent *);

 private:
 ImageView     *view;
 ScaleWidget   *xscale,*yscale;
 CornerPatchWidget *corner;
 OverViewWidget *overview;
};
