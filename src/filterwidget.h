#ifndef _KWAVE_FILTER_WIDGET_H
#define _KWAVE_FILTER_WIDGET_H 1

#include <qlayout.h>
#include <qtooltip.h>
#include <qdir.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qbutton.h>
#include <qcombo.h>
#include <qdialog.h>
#include <qbttngrp.h>
#include <qchkbox.h>

#include "fftview.h"
#include "../libgui/curvewidget.h"

#include <krestrictedline.h>
#include <kintegerline.h>
#include <kmsgbox.h>


class MovingFilterDialog : public QDialog
{
  Q_OBJECT

    public:
  MovingFilterDialog 	(QWidget *parent=0,int num=1);
  ~MovingFilterDialog 	();

  const char*  getCommand ();
  int   getState ();
  int   getTap();
  int   getLow();  //for getting lower bound and
  int   getHigh(); //higher bound
  public slots:

    void toggleState();
    void checkTap(const char *);

 protected:

  void resizeEvent (QResizeEvent *);

 private:

  int num;
  QCheckBox     *usecurve;
  KRestrictedLine  *low,*high;
  KIntegerLine  *tap;
  QLabel        *lowlabel,*highlabel;
  QPushButton	*ok,*cancel;
  CurveWidget	*curve;
};
#endif /*filterwidget.h*/

