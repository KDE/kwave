
#include "config.h"
#include "libgui/OverViewWidget.h"
#include "libgui/ScaleWidget.h"
#include "libgui/CornerPatchWidget.h"

#include "ImageView.h"
#include "SonagramContainer.h"

//****************************************************************************
SonagramContainer::SonagramContainer(QWidget *parent)
    :QWidget(parent)
{
    corner = 0;
    overview = 0;
    view = 0;
    xscale = 0;
    yscale = 0;
}

//****************************************************************************
void SonagramContainer::setObjects(ImageView *view, ScaleWidget *x,
                                   ScaleWidget *y, CornerPatchWidget *corner,
                                   OverViewWidget *overview)
{
    this->corner = corner;
    this->overview = overview;
    this->view = view;
    this->xscale = x;
    this->yscale = y;
}

//****************************************************************************
SonagramContainer::~SonagramContainer()
{
}

//****************************************************************************
void SonagramContainer::resizeEvent(QResizeEvent *)
{
    ASSERT(corner);
    ASSERT(overview);
    ASSERT(view);
    ASSERT(xscale);
    ASSERT(yscale);
    if (!corner) return;
    if (!overview) return;
    if (!view) return;
    if (!xscale) return;
    if (!yscale) return;

    int bsize = (QPushButton("test", this).sizeHint()).height();
    view->setGeometry (bsize, 0, width() - bsize, height() - bsize*2);
    xscale->setGeometry (bsize, height() - bsize*2, width() - bsize, bsize);
    overview->setGeometry (bsize, height() - bsize, width() - bsize, bsize);
    yscale->setGeometry (0, 0, bsize, height() - bsize*2);
    corner->setGeometry (0, height() - bsize*2, bsize, bsize);
}

//****************************************************************************
//****************************************************************************
