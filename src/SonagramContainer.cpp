#include "SonagramContainer.h"
#include "ImageView.h"

//****************************************************************************
SonagramContainer::SonagramContainer (QWidget *parent): QWidget (parent)
{
  this->view=0;
}
//****************************************************************************
void SonagramContainer::setObjects (ImageView *view,ScaleWidget *x,ScaleWidget *y,CornerPatchWidget *corner,OverViewWidget *overview)
{
  this->view=view;
  this->xscale=x;
  this->yscale=y;
  this->corner=corner;
  this->overview=overview;
}
//****************************************************************************
SonagramContainer::~SonagramContainer ()
{
}
//****************************************************************************
void SonagramContainer::resizeEvent	(QResizeEvent *)
{
  if (view)
    {
      int bsize=(QPushButton("test",this).sizeHint()).height();
      view->setGeometry	(bsize,0,width()-bsize,height()-bsize*2);  
      xscale->setGeometry	(bsize,height()-bsize*2,width()-bsize,bsize);  
      overview->setGeometry	(bsize,height()-bsize,width()-bsize,bsize);  
      yscale->setGeometry	(0,0,bsize,height()-bsize*2);
      corner->setGeometry	(0,height()-bsize*2,bsize,bsize);  
    }
}
