

#include <qapp.h>
#include <qpushbt.h>
#include <qstring.h>
#include <qwidget.h>
#include <qpainter.h>
#include <kapp.h>
#include <kslider.h>
#include <kselect.h>

class FaderWidget : public QWidget
{
 Q_OBJECT
 public:
 	FaderWidget	(QWidget *parent=0,int dir=1);
 	~FaderWidget	();
	int getCurve ();
 public slots:

 void   setCurve (int);

 signals:

 protected:

 void	paintEvent(QPaintEvent *);
 void	drawInterpolatedFFT	();

 private:

 int    dir;
 int    curve;
 int	width,height;		//of widget
 QPainter 	p;
};
//***********************************************************************
