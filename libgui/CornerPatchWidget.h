#ifndef _CORNER_PATCH_WIDGET_H_
#define _CORNER_PATCH_WIDGET_H_

#include "config.h"
#include <qwidget.h>

class QPaintEvent;

//*****************************************************
/**
 * \class CornerPatchWidget
 * \deprecated WAS USED IN THE TIMES BEFORE LAYOUT MANAGEMENT
 */
class CornerPatchWidget:public QWidget
{
    Q_OBJECT

public:

    CornerPatchWidget(QWidget *parent = 0, int = 0);
    ~CornerPatchWidget();

signals:

public slots:

protected:

    void paintEvent(QPaintEvent *);

private:

    int pos;            //
};

#endif // _CORNER_PATCH_WIDGET_H_


