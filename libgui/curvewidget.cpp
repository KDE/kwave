#include <stdio.h>
#include <qimage.h>
#include <qpainter.h>
#include <qcursor.h>
#include <qkeycode.h>
#include <qaccel.h>
#include <qfiledlg.h>
#include <math.h>
#include <limits.h>
#include <libkwave/interpolation.h>
#include <libkwave/fileloader.h>
#include <libkwave/globals.h>
#include "curvewidget.h"

int knobcount=0;
QPixmap *knob=0;
QPixmap *selectedknob=0;

extern Global globals;
//****************************************************************************
CurveWidget::CurveWidget (QWidget *parent,const char *init,int keepborder) : QWidget (parent)
{
  last=0;

  this->keepborder=keepborder;

  if (init) points=new Curve (init);
  else points=new Curve ("curve (linear,0,0,1,1)");

  setBackgroundColor (black);

  menu=new QPopupMenu ();
  QPopupMenu *interpolation=new QPopupMenu ();
  QPopupMenu *del =new QPopupMenu ();
  QPopupMenu *transform =new QPopupMenu ();
  QPopupMenu *presets=new QPopupMenu ();
  transform->insertItem (klocale->translate("Flip Horizontal"),this,SLOT(HFlip()));
  transform->insertItem (klocale->translate("Flip Vertical"),this,SLOT(VFlip()));
  transform->insertSeparator();

  menu->insertItem (klocale->translate("Interpolation"),interpolation);
  menu->insertSeparator();
  menu->insertItem (klocale->translate("Transform"),transform);
  menu->insertItem (klocale->translate("Delete"),del);
  menu->insertItem (klocale->translate("Fit In"),this,SLOT(scaleFit()));
  menu->insertSeparator();
  menu->insertItem (klocale->translate("Presets"),presets);
  menu->insertItem (klocale->translate("Save Preset"),this,SLOT(savePreset()));

  transform->insertItem (klocale->translate ("into 1st half"),this,SLOT(firstHalf()));
  transform->insertItem (klocale->translate ("into 2nd half"),this,SLOT(secondHalf()));

  del->insertItem (klocale->translate ("recently selected Point"),this,SLOT(deleteLast()));
  del->insertItem (klocale->translate ("every 2nd Point"),this,SLOT(deleteSecond()));

  presetDir=new QDir(globals.localconfigDir);

  if (presetDir)
    {
      if (!presetDir->cd ("presets"))
	{
	  presetDir->mkdir ("presets");
	  presetDir->cd ("presets");
	}
      if (!presetDir->cd ("curves"))
	{
	  presetDir->mkdir ("curves");
	  presetDir->cd ("curves");
	}

      presetDir->setNameFilter ("*.curve");

      namelist=(QStrList *)presetDir->entryList ();

      for (char *tmp=namelist->first();tmp;tmp=namelist->next())
	{
	  char buf[strlen(tmp)-5];
	  strncpy (buf,tmp,strlen(tmp)-6);
	  buf[strlen(tmp)-6]=0;
	  presets->insertItem (buf);
	}

      connect( presets, SIGNAL(activated(int)), SLOT(loadPreset(int)) ); 
    }

  char **names=(char **) Interpolation::getTypes();
  int i=0;

  while (names[i]!=NULL)
    interpolation->insertItem (klocale->translate(names[i++]));

  connect( interpolation, SIGNAL(activated(int)), SLOT(setType(int)) ); 

  act=0;                           //set active point to none
  interpolationtype=0;             //default interpolation type
  
  if (knob==0)
    {
      QString dirname=globals.app->kde_datadir ();
      QDir dir (dirname.data());
      dir.cd ("kwave");
      dir.cd ("pics");

      knob=new QPixmap ();
      knob->convertFromImage (QImage (dir.filePath("knob.xpm")));
      selectedknob=new QPixmap ();
      selectedknob->convertFromImage (QImage (dir.filePath("selectedknob.xpm")));
      knobcount=0;
    }
  else
    knobcount++;

  setMouseTracking (true);

  QAccel *delkey=new QAccel (this);
  delkey->connectItem (delkey->insertItem(Key_Delete),this,SLOT (deleteLast()));
}
//****************************************************************************
CurveWidget::~CurveWidget ()
{
  knobcount--;

  if (knobcount<=0)
    {
      if (knob) delete knob;
      if (selectedknob) delete selectedknob;
      knob=0;
    }

  if (presetDir) delete presetDir;
  if (points) delete points;
  if (menu) delete menu;
}
//****************************************************************************
void CurveWidget::setCurve (const char *next)
{
  if (points) delete points;
  points=new Curve (next);
}
//****************************************************************************
void CurveWidget::setType(int type)
{
  points->setInterpolationType (Interpolation::getTypes()[type]);
  repaint ();
}
//****************************************************************************
void CurveWidget::savePreset()
{
  QString name=QFileDialog::getSaveFileName (presetDir->path(),"*.curve",this);

  if (name.find (".curve")==-1) name.append (".curve");
  QFile out(name.data());
  out.open (IO_WriteOnly);
  const char *buf=points->getCommand();
  out.writeBlock (buf,strlen(buf));
}
//****************************************************************************
void CurveWidget::loadPreset(int num)
{
  char *name=namelist->at(num);

  FileLoader loader (presetDir->absFilePath(name));

  if (points) delete points;
  points=new Curve (loader.getMem());

  repaint ();
}
//****************************************************************************
void CurveWidget::secondHalf ()
{
  points->secondHalf ();
  last=0;
  repaint();
}
//****************************************************************************
void CurveWidget::firstHalf ()
{
  points->firstHalf ();
  last=0;

  repaint();
}
//****************************************************************************
void CurveWidget::deleteSecond ()
{
  points->deleteSecondPoint();
  last=0;

  repaint ();
}
//****************************************************************************
void CurveWidget::deleteLast ()
{
  if (last)
    {
      points->deletePoint (last,false);
      last=0;
      repaint ();
    }
}
//****************************************************************************
void CurveWidget::HFlip ()
{
  points->HFlip();
  repaint ();
}
//****************************************************************************
void CurveWidget::VFlip ()
{
  points->VFlip();
  repaint ();
}
//****************************************************************************
void CurveWidget::scaleFit()
{
  points->scaleFit ();
  repaint ();
}
//****************************************************************************
void CurveWidget::addPoint (double newx,double newy)
{
  points->addPoint (newx,newy);
  last=0;
  repaint ();
}
//****************************************************************************
Point *CurveWidget::findPoint (int sx,int sy)
  // checks, if given coordinates fit to a control point in the list...
{
  return points->findPoint (((double)sx)/width,((double)height-sy)/height);
}
//****************************************************************************
void CurveWidget::mousePressEvent( QMouseEvent *e)
{
  if (e->button()==RightButton)
    {
      QPoint popup=QCursor::pos();
      menu->popup(popup);
    }
  else
    {
      act=findPoint (e->pos().x(),e->pos().y());

      if (act==0) //so, no matching point is found -> generate a new one !
	{
	  addPoint ((double) (e->pos().x())/width,
		  (double) (height-e->pos().y())/height);
	  act=findPoint (e->pos().x(),e->pos().y());
	}
      repaint ();
    }
}
//****************************************************************************
void CurveWidget::mouseReleaseEvent( QMouseEvent *)
{
  last=act;
  act=0;
  repaint();
}
//****************************************************************************
void CurveWidget::mouseMoveEvent( QMouseEvent *e )
{
  int x=e->pos().x();
  int y=e->pos().y();

  // if a point is selected...
  if (act)
    {
      act->x=(double) (x)/width;
      act->y=(double) (height-y)/height;

      if (act->x<0) act->x=0;
      if (act->y<0) act->y=0;
      if (act->x>1) act->x=1;
      if (act->y>1) act->y=1;

      Point *prev=points->previous(act);
      Point *next=points->next(act);

      if (prev) if (act->x<prev->x) act->x=prev->x+(1/(double) width);
      if (next) if (act->x>next->x) act->x=next->x-(1/(double) width);

      repaint ();
    }
  else
    if (findPoint (x,y)) setCursor (sizeAllCursor);
    else setCursor (arrowCursor);
}
//****************************************************************************
void CurveWidget::paintEvent  (QPaintEvent *)
{
  Point *tmp;

  height=rect().height();
  width=rect().width();

  p.begin (this);
  p.setPen (white);

  int kw=knob->width();
  int kh=knob->height();
  int lx,ly,ay;

  if (points)
    {
      double *y=points->getInterpolation (width);
      if (y)
	{
	  ly=height-(int)(y[0]*height);

	  for (int i=1;i<width;i++)
	    {
	      ay=height-(int)(y[i]*height);
	      p.drawLine (i-1,ly,i,ay);
	      ly=ay;
	    }

	  for ( tmp=points->first(); tmp ; tmp=points->next(tmp) )
	    {
	      lx=(int)(tmp->x*width);
	      ly=height-(int)(tmp->y*height);

	      if ((tmp==act)||(tmp==last)) bitBlt (this,lx-kw/2,ly-kh/2,selectedknob,0,0,kw,kh);
	      else bitBlt (this,lx-kw/2,ly-kh/2,knob,0,0,kw,kh);

	    }
	}
      else debug ("CurveWidget: could not get Interpolation !\n");
      p.end();
    }
  else debug ("CurveWidget: nothing to be drawn !\n");
}
//****************************************************************************



