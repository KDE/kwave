//provides methods of multistateWidget a Class that switches the image it
// displays on clicking, used for the channel enable/disable lamps...
#include <stdio.h>
#include <qaccel.h>
#include <qpntarry.h>
#include <qdir.h>
#include "multistateimage.h"
#include <kapp.h>

QList<QPixmap> *pixmaps=0;
QStrList *pixnames=0;

extern KApplication *app;
//**********************************************************
MultiStateWidget::MultiStateWidget (QWidget *parent,int num,int count): QWidget (parent)
{
  this->number=num;
  this->states=new int[count];

  for (int i=0;i<count;i++)  states[i]=0;

  this->count=count;
  this->act=0;
  if (pixmaps==0) pixmaps=new QList<QPixmap>();
  if (pixnames==0) pixnames=new QStrList(true);
  resize (20,20);
}
//**********************************************************
int MultiStateWidget::addPixmap (char *name)
{
      int result=pixnames->find(name);
      if (result==-1)
	{
	  QPixmap *newpix=new QPixmap();
	  QString dirname=app->kde_datadir ();
	  QDir dir (dirname.data());
	  dir.cd ("kwave");
	  dir.cd ("pics");

	  QImage  *img=new QImage(dir.filePath(name).data());
	  if (newpix&&img)
	    {

	      newpix->convertFromImage(*img);
	      pixmaps->append (newpix);
	      pixnames->append (name);
	      return pixmaps->at();
	    }
	}
      else
	return result;

  return -1;
}
//**********************************************************
void MultiStateWidget::setStates (int *newstates)
{
  for (int i=0;i<count;i++) states[i]=newstates[i];
}
//**********************************************************
void MultiStateWidget::setState (int newstate)
{
  act=newstate;
  if (act>=count) act=count;
  if (act<0) act=0;
  repaint ();
}
//**********************************************************
void MultiStateWidget::nextState ()
{
      act++;
      if (act>=count) act=0;
      repaint ();
}
//**********************************************************
void MultiStateWidget::mouseReleaseEvent( QMouseEvent *e)
{
  if (e->button()==LeftButton)
    {
      nextState ();
      emit clicked (number);
    }
}
//**********************************************************
MultiStateWidget::~MultiStateWidget ()
{
  if (states) delete states;
}
//**********************************************************
void MultiStateWidget::paintEvent  (QPaintEvent *)
{
  QPixmap *img;

  img=pixmaps->at(states[act]);

  if (img)
        bitBlt (this,0,0,
  	      img,0,0,img->width(),img->height(),CopyROP);    
}
