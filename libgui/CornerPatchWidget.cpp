
#include "config.h"
#include <qpainter.h>
#include "CornerPatchWidget.h"

//****************************************************************************
CornerPatchWidget::CornerPatchWidget(QWidget *parent, int pos)
    :QWidget(parent)
{
    this->pos = pos;
}

//****************************************************************************
CornerPatchWidget::~CornerPatchWidget()
{
}

//****************************************************************************
void CornerPatchWidget::paintEvent(QPaintEvent *)
{
    int h = height();
    int w = width();
    QPainter p;
    p.begin (this);
    p.setPen (colorGroup().light());
    p.drawLine (0, 0, 0, h - 1);
    p.setPen (colorGroup().dark());
    p.drawLine (0, h - 1, w, h - 1);
    p.end ();
}

//****************************************************************************
//****************************************************************************
