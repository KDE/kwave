
#include <qpainter.h>
#include <qpushbt.h>

#include "libgui/FFTWidget.h"
#include "libgui/ScaleWidget.h"
#include "libgui/CornerPatchWidget.h"

#include "FFTContainer.h"

//****************************************************************************
FFTContainer::FFTContainer (QWidget *parent):
QWidget (parent)
{
    this->corner = 0;
    this->view = 0;
    this->xscale = 0;
    this->yscale = 0;
}

//****************************************************************************
void FFTContainer::setObjects(FFTWidget *view, ScaleWidget *x,
                              ScaleWidget *y, CornerPatchWidget *corner)
{
    this->corner = corner;
    this->view = view;
    this->xscale = x;
    this->yscale = y;
}

//****************************************************************************
FFTContainer::~FFTContainer()
{
}

//****************************************************************************
void FFTContainer::resizeEvent(QResizeEvent *)
{
    ASSERT(corner);
    ASSERT(view);
    ASSERT(xscale);
    ASSERT(yscale);
    if (!corner) return;
    if (!view) return;
    if (!xscale) return;
    if (!yscale) return;

    int bsize = (QPushButton("test", this).sizeHint()).height();
    view->setGeometry (bsize, 0, width() - bsize, height() - bsize);
    xscale->setGeometry (bsize, height() - bsize, width() - bsize, bsize);
    yscale->setGeometry (0, 0, bsize, height() - bsize);
    corner->setGeometry (0, height() - bsize, bsize, bsize);
}
