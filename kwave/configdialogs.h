#ifndef _CONFIGDIALOGS_H_
#define _CONFIGDIALOGS_H_ 1

#include <stdlib.h>
#include <qapp.h>
#include <qdir.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qbutton.h>
#include <qcombo.h>
#include <qdialog.h>
#include <qbttngrp.h>
#include <qradiobt.h> 
#include <qslider.h> 
#include <qchkbox.h> 
#include <qfiledlg.h> 
#include <qlistbox.h>
#include <kapp.h>
#include <kintegerline.h>

//*****************************************************************************
class PlayBackDialog : public QDialog
{
 Q_OBJECT

 public:

 PlayBackDialog 	(QWidget *parent=0,int =false,int =5);
 ~PlayBackDialog 	();
 int getResolution ();
 int getBufferSize ();

 public slots:

   void setBufferSize (int);

 protected:

 void resizeEvent (QResizeEvent *);

 private:

 QLabel		*bufferlabel;
 QSlider        *buffersize;
 QLabel         *devicelabel;
 QComboBox      *devicebox;
 QCheckBox      *stereo;
 QLabel		*label1,*label2;
 QButtonGroup	*bg;
 QRadioButton	*b16,*b8;

 QPushButton	*ok,*cancel;
};
//*****************************************************************************
class MemoryDialog : public QDialog
{
 Q_OBJECT

 public:

 MemoryDialog 	  (QWidget *parent=0);
 ~MemoryDialog 	  ();
 char *getDir     ();
 int  getThreshold();

 public slots:

 protected:

 void resizeEvent (QResizeEvent *);

 private:

 QLabel		*dirlabel;
 QLineEdit	*dir;
 QPushButton	*browse;
 QLabel		*memlabel;
 KIntegerLine   *mem;

 QPushButton	*ok,*cancel;
};

#endif  /* configdialogs.h */   






