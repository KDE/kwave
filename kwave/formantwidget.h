#ifndef _FORMANTDIALOG_H_
#define _FORMANTDIALOG_H_ 1

#include <qapp.h>
#include <qpushbt.h>
#include <qstring.h>
#include <qwidget.h>
#include <qdialog.h>
#include <qslider.h>
#include <qlabel.h>
#include <kintegerline.h>
#include "scale.h"

class FormantWidget : public QWidget
{
 Q_OBJECT
 public:
 	FormantWidget	(QWidget *parent,int rate);
 	~FormantWidget	();
 double *getPoints (int);

 public slots:

 void   setFormants (int,int *,int *);

 signals:

 void   dbscale (int,int);

 protected:

 void	paintEvent(QPaintEvent *);

 private:

 int    *pos;                   //array of formant-positions
 int    *widths;                //array of formantwidth
 int    num;                    //number of formants
 int	width,height;		//of widget
 int    rate;
 double *points;
};
//***********************************************************************
class FormantDialog : public QDialog
{
 Q_OBJECT

 public:
 	FormantDialog 	(QWidget *parent,int rate);
 	~FormantDialog 	();
 int    getCurve ();
 void   getWidgets (int);
 double *FormantDialog::getPoints (int psize);

 public slots:

 void posChanged    (int);
 void setScale      (int,int);
 void widthChanged  (int);
 void numberChanged (const char *);
 void posChanged    (const char *);
 void widthChanged  (const char *);

 protected:

 void resizeEvent (QResizeEvent *);
 void refresh     ();

 private:

 ScaleWidget       *x,*y;
 CornerPatchWidget *corner;
 KIntegerLine   *num;
 KIntegerLine   **pos;
 KIntegerLine   **widths;
 QSlider        **widthslider;
 QSlider        **posslider;
 QLabel         *poslabel;
 QLabel         *numlabel;
 QPushButton	*ok,*cancel;
 FormantWidget	*formant;

 int            oldnum;
 int            rate;
 char           inwidget;
};
//***********************************************************************
#endif



