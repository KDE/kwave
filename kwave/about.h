#ifndef _ABOUT_H_
#define _ABOUT_H_ 1

#include <stdlib.h>
#include <qapp.h>
#include <qwidget.h>
#include <qpushbt.h>
#include <qdialog.h>
#include <qmlined.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qtimer.h>

#define MAXSIN 5

class LogoWidget : public QWidget
{
 Q_OBJECT

 public:

 	LogoWidget 	(QWidget *parent=NULL);
 	~LogoWidget 	();

 public slots:

 void doAnim ();

 protected:

 void   paintEvent(QPaintEvent *); 

 private:

 int width, height;
 int amp;
 int rpaint;          //flag for need of repaint of animation
 double deg[MAXSIN];  //phase of sinus for animation
 QPainter p;
 QPixmap  *pixmap;
 QPixmap  *img;
 QTimer   *timer;
};

class AboutDialog : public QDialog
{
 Q_OBJECT

 public:

 	AboutDialog 	(QWidget *parent=NULL);
 	~AboutDialog 	();

 public slots:

 protected:

 void resizeEvent (QResizeEvent *);

 private:

 QMultiLineEdit *abouttext;
 LogoWidget     *logo;
 QPushButton	*ok;
};


#endif  /* about.h */   

