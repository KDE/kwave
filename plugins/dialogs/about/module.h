#ifndef _ABOUT_H_
#define _ABOUT_H_ 1

#include <stdlib.h>
#include <qwidget.h>
#include <qpushbt.h>
#include <qmlined.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qtimer.h>
#include "../../../libgui/kwavedialog.h"
#include <libkwave/dialogoperation.h>
#include <libkwave/globals.h>
#include <kapp.h>

#define MAXSIN 5
//**********************************************************
class LogoWidget : public QWidget
{
 Q_OBJECT

 public:

 	LogoWidget 	(QWidget *parent,KApplication *app);
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
//**********************************************************
class AboutDialog : public KwaveDialog
{
 Q_OBJECT

 public:

 	AboutDialog 	(const Global *,bool);
 	~AboutDialog 	();

 const char *getCommand ();

 public slots:

 protected:

 void resizeEvent (QResizeEvent *);

 private:

 QMultiLineEdit *abouttext;
 LogoWidget     *logo;
 QPushButton	*ok;
};
#endif  /* about.h */   

