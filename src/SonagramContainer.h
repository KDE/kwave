#ifndef _SONOGRAM_CONTAINER_H_
#define _SONOGRAM_CONTAINER_H_ 1

#include <qwidget.h>

class ImageView;
class ScaleWidget;
class CornerPatchWidget;
class OverViewWidget;
class QResizeEvent;

//***********************************************************************
class SonagramContainer : public QWidget
{
 Q_OBJECT
 public:
 	SonagramContainer	(QWidget *parent);
 	~SonagramContainer	();
 void 	setObjects	(ImageView *view,ScaleWidget *x,ScaleWidget *y,
                         CornerPatchWidget *corner,OverViewWidget *overview=0);

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

#endif // _SONOGRAM_CONTAINER_H_