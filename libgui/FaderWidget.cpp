
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <qkeycode.h>
#include <qpainter.h>
#include <math.h>
#include <limits.h>
#include "FaderWidget.h"

//****************************************************************************
FaderWidget::FaderWidget(QWidget *parent, int dir)
    :QWidget(parent) 
{
    curve = 0;
    this->dir = dir;
    height = -1;
    width = 0;
    setBackgroundColor(QColor(black));
}

//****************************************************************************
FaderWidget::~FaderWidget()
{
}

//****************************************************************************
void FaderWidget::setCurve(int c)
{
    curve = c * c * c;
    repaint();
}

//****************************************************************************
QString FaderWidget::getDegree()
{
    QString deg;
    return deg.setNum(((float) (curve)) / 10);
}

//****************************************************************************
void FaderWidget::paintEvent (QPaintEvent *) 
{
    QPainter p;
    height = rect().height();
    width = rect().width();

    ASSERT(width);
    if (!width) return;

    p.begin (this);
    p.setPen (white);

    int lx = (dir == 1) ? 0 : width;
    int ly = height;

    if (curve == 0) {
	for (int i = 1; i < width; i++) {
	    if (dir == 1) {
		p.drawLine (lx, ly, i, (width - i)*height / width);
		lx = i;
	    } else {
		p.drawLine (lx, ly, width - i, (width - i)*height / width);
		lx = width - i;
	    }

	    ly = (width - i) * height / width;
	}
    } else
	if (curve < 0)
	    for (int i = 1; i < width; i++) {
		int y = height - (int)(log10(1 + ( -curve * ((double)i) / width)) * height / log10(1 - curve));
		if (dir == 1) {
		    p.drawLine (lx, ly, i, y);
		    lx = i;
		} else {
		    p.drawLine (lx, ly, width - i, y);
		    lx = width - i;
		}

		ly = y;
	    }
	else
	    for (int i = 1; i < width; i++) {
		int y = (int)(log10(1 + (curve * ((double)(width - i)) / width)) * height / log10(1 + curve));

		if (dir == 1) {
		    p.drawLine (lx, ly, i, y);
		    lx = i;
		} else {
		    p.drawLine (lx, ly, width - i, y);

		    lx = width - i;
		}

		ly = y;
	    }

    p.end();
}

//****************************************************************************
//****************************************************************************
