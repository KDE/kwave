#include <qwidget.h>
#include "../libgui/ScaleWidget.h"

class PitchWidget;
class ScaleWidget;
class CornerPatchWidget;
//***********************************************************************
class PitchContainer : public QWidget
{
 Q_OBJECT
 public:
 	PitchContainer	(QWidget *parent);
 	~PitchContainer	();
	void 	setObjects	(PitchWidget *view,ScaleWidget *x,ScaleWidget *y,CornerPatchWidget *corner);

 public slots:

 signals:

 protected:

 void	resizeEvent	(QResizeEvent *);

 private:
 PitchWidget       *view;
 ScaleWidget       *xscale,*yscale;
 CornerPatchWidget *corner;
};

