#include <qimage.h>
#include <qpainter.h>
#include <qcursor.h>
#include <qkeycode.h>
#include <qaccel.h>
#include <qfiledlg.h>
#include <math.h>
#include <limits.h>
#include "interpolation.h"
#include "curvewidget.h"

struct Curve
{
  QString name;
  QList<CPoint> *points;
  int type;
};

int knobcount=0;
QPixmap *knob=0;
QPixmap *selectedknob=0;

extern QDir *configDir;
extern KApplication *app;

//****************************************************************************
CurveWidget::CurveWidget (QWidget *parent,const char *name,QList<CPoint> *init,int keepborder) : QWidget (parent,name)
{
  Interpolation ask(0);
  last=0;

  this->keepborder=keepborder;

  points=new QList<CPoint> ();
  points->setAutoDelete (true);

  if (init)
    {
      CPoint *tmp;
      init->setAutoDelete (false);
      for (tmp=init->first();tmp;tmp=init->first())
	{
	  init->removeRef (tmp);
	  points->append (tmp);
	}
   }
  else
   {
      CPoint *first=new CPoint;
      CPoint *last=new CPoint;
      first->x=0;
      first->y=0;
      last->x=1;
      last->y=1;
      points->append (first);
      points->append (last);
    }
 
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

  presetDir=new QDir(configDir->path());

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

  for (char *tmp=namelist->first();tmp!=0;tmp=namelist->next())
    {
      char buf[strlen(tmp)-5];
      strncpy (buf,tmp,strlen(tmp)-6);
      buf[strlen(tmp)-6]=0;
      presets->insertItem (buf);
    }

  connect( presets, SIGNAL(activated(int)), SLOT(loadPreset(int)) ); 

  char **names=(char **) ask.getTypes();
  int i=0;

  while (names[i]!=NULL)
    interpolation->insertItem (klocale->translate(names[i++]));

  connect( interpolation, SIGNAL(activated(int)), SLOT(setType(int)) ); 

  act=0; //set active point to none
  interpolationtype=LINEAR; //linear interpolation
  
  if (knob==0)
    {
      QString dirname=app->kde_datadir ();
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
CurveWidget::~CurveWidget (QWidget *parent,const char *name)
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
QList<CPoint> *CurveWidget::getPoints ()
{
  return points;
}
//****************************************************************************
void CurveWidget::setCurve (QList<CPoint> *curve)
{
  //make deep copy of list
  points->clear();
  CPoint *tmp;

  for (tmp=curve->first();tmp;tmp=curve->next())
    {
      CPoint *newpoint=new CPoint;
      newpoint->x=tmp->x;
      newpoint->y=tmp->y;
      points->append (newpoint);
    }
}
//****************************************************************************
void CurveWidget::setType(int type)
{
  interpolationtype=type;
  repaint ();
}
//****************************************************************************
void CurveWidget::savePreset()
{
  CPoint *tmp;
  char buf[80];
  QString name=QFileDialog::getSaveFileName (presetDir->path(),"*.curve",this);
  if (name.find (".curve")==-1) name.append (".curve");
  QFile out(name.data());
  out.open (IO_WriteOnly);
  sprintf (buf,"Type = %d\n",interpolationtype);
  out.writeBlock (&buf[0],strlen(buf));

  for ( tmp=points->first(); tmp != 0; tmp=points->next() )
    {
      sprintf (buf,"%e %e\n",tmp->x,tmp->y);
      out.writeBlock (&buf[0],strlen(buf));
    }
}
//****************************************************************************
void CurveWidget::loadPreset(int num)
{
  char *name=namelist->at(num);

  char buf[120];

  QFile *in=new QFile (presetDir->absFilePath(name));
  if ((in)&&(in->open (IO_ReadOnly)))
    {
      CPoint *newpoint;
      QList<CPoint> *newlist=new QList<CPoint>;
      newlist->setAutoDelete (true);

      in->readLine(buf,120);
      sscanf (buf,"Type = %d",&interpolationtype);
      while (in->readLine(buf,120)>0)
	{
	  float x,y;
	  newpoint=new CPoint;
	  sscanf (buf,"%f %f\n",&x,&y);

	  newpoint->x=x;
	  newpoint->y=y;
	  if (newpoint->x>1) newpoint->x=1;
	  if (newpoint->y>1) newpoint->y=1;
	  if (newpoint->x<0) newpoint->x=0;
	  if (newpoint->y<0) newpoint->y=0;

	  newlist->append (newpoint);
	}
      delete in;

      if (newlist->count()>=2)
	{
	  if (points) delete points;
	  points=newlist;
	}
    }
  else printf ("could not open file !\n");

  repaint ();
}
//****************************************************************************
void CurveWidget::doDouble ()
{
}
//****************************************************************************
void CurveWidget::secondHalf ()
{
  struct CPoint *tmp;
  
  for ( tmp=points->first(); tmp != 0; tmp=points->next() )
    tmp->x=.5+tmp->x/2;

  tmp=points->first();
  CPoint *newpoint=new CPoint;
  newpoint->x=0;
  newpoint->y=tmp->y;
  points->insert (0,newpoint);
  repaint();
}
//****************************************************************************
void CurveWidget::firstHalf ()
{
  struct CPoint *tmp;
  
  for ( tmp=points->first(); tmp != 0; tmp=points->next() )
    tmp->x=tmp->x/2;

  tmp=points->last();
  CPoint *newpoint=new CPoint;
  newpoint->x=1;
  newpoint->y=tmp->y;
  points->append (newpoint);
  repaint();
}
//****************************************************************************
void CurveWidget::deleteSecond ()
{
  struct CPoint *tmp;
  
  for ( tmp=points->first(); tmp != 0; tmp=points->next() )
    {
      tmp=points->next();
      if (tmp&&tmp!=points->last())
	{
	  points->removeRef(tmp);
	  tmp=points->prev();
	}
      //points is autodelete... no delete for object is required
    }
  last=0;
  repaint ();
}
//****************************************************************************
void CurveWidget::deleteLast ()
{
  if ((last)&&(last!=points->first())&&(last!=points->last()))
  //inhibit removal of first/last element
    {
      points->removeRef(last);
      //points is autodelete... no delete for object is required
      last=0;
      repaint ();
    }
}
//****************************************************************************
void CurveWidget::HFlip ()
{
  CPoint *tmp;
  QList<CPoint> *newlist=new QList<CPoint>;

  for ( tmp=points->last(); tmp != 0; tmp=points->prev() )
    {
      tmp->x=1-tmp->x;
      newlist->append (tmp);
    }

  points->setAutoDelete (false);
  delete points;

  points=newlist;
  points->setAutoDelete (true);
  repaint ();
}
//****************************************************************************
void CurveWidget::VFlip ()
{
  CPoint *tmp;
  for ( tmp=points->first(); tmp != 0; tmp=points->next() ) tmp->y=1-tmp->y;
  repaint ();
}
//****************************************************************************
int  CurveWidget::getInterpolationType()
{
 return interpolationtype;
}
//****************************************************************************
void CurveWidget::scaleFit()
{
  struct CPoint *tmp;
  double min=1000,max=0;

  tmp=points->first();
  if (tmp!=0)
    {
      Interpolation interpolation(interpolationtype);

      double *y=interpolation.getInterpolation (points,1024);
      for (int i=0;i<1024;i++)
	{
	  if (y[i]>max) max=y[i];
	  if (y[i]<min) min=y[i];
	}

      max-=min;
      tmp=points->first();
      for (;tmp!=0;tmp=points->next())
	{
	  tmp->y-=min;	  
	  tmp->y/=max;
	}
      repaint ();
    }
}
//****************************************************************************
void CurveWidget::addPoint (double newx,double newy)
{
  CPoint *tmp;

  act=new CPoint;
  act->x=newx;
  act->y=newy;

  tmp=points->first();

  while ((tmp)&&(tmp->x<act->x)) tmp=points->next();

  if (tmp!=0) points->insert (points->at(), act);
  last=0;
  repaint ();
}
//****************************************************************************
CPoint *CurveWidget::findPoint (int sx,int sy)
  // checks, if given coordinates fit to a control point in the list...
{
  double maxx=(double) (sx)/width+.05;
  double minx=(double) (sx)/width-.05;
  double maxy=(double) (height-sy)/height+.05;
  double miny=(double) (height-sy)/height-.05;
  CPoint *tmp;
  CPoint *act=0;
  double x,y;
  
  for ( tmp=points->first(); tmp != 0; tmp=points->next() )
    {
      x=tmp->x;
      y=tmp->y;

      if (x>maxx) break;
      //the list is sorted, a match cannot be
      //found anymore, because x is already to big

      if ((x>=minx)&&(x<=maxx)&&(y>=miny)&&(y<=maxy))
	{
	  act=tmp;
	  last=0;
	  break;
	}
    }
  return act;
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
	addPoint ((double) (e->pos().x())/width,(double) (height-e->pos().y())/height);
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

      if (points->find (act)!=-1)
	{
	  CPoint *prev=points->prev();
	  if (prev) points->next();  //item no first item
	  else points->first();     
	  CPoint *next=points->next();

	  if (prev) if (act->x<prev->x) act->x=prev->x+(1/(double) width);
	  if (next) if (act->x>next->x) act->x=next->x-(1/(double) width);
	}
      if (act==points->first()) act->x=0;
      if (act==points->last())  act->x=1;
            
      repaint ();
    }
  else
    if (findPoint (x,y)) setCursor (sizeAllCursor);
    else setCursor (arrowCursor);
}
//****************************************************************************
void CurveWidget::paintEvent  (QPaintEvent *)
{
  CPoint *tmp;
  Interpolation interpolation (interpolationtype);

  height=rect().height();
  width=rect().width();

  p.begin (this);
  p.setPen (darkGray);

  /*  p.drawLine (0,height/4,width,height/4);
  p.drawLine (0,height*3/4,width,height*3/4);
  p.drawLine (width/4,0,width/4,height);
  p.drawLine (width*3/4,0,width*3/4,height);
  */
  p.setPen (white);

  int kw=knob->width();
  int kh=knob->height();
  int lx,ly,ay;

  double *y=interpolation.getInterpolation (points,width);

  ly=height-(int)(points->first()->y*height);

  for (int i=1;i<width;i++)
    {
      ay=height-(int)(y[i]*height);
      p.drawLine (i-1,ly,i,ay);
      ly=ay;
    }

  for ( tmp=points->first(); tmp != 0; tmp=points->next() )
    {
      lx=(int)(tmp->x*width);
      ly=height-(int)(tmp->y*height);

      if ((tmp==act)||(tmp==last)) bitBlt (this,lx-kw/2,ly-kh/2,selectedknob,0,0,kw,kh);
      else bitBlt (this,lx-kw/2,ly-kh/2,knob,0,0,kw,kh);
    }
  p.end();
}
//****************************************************************************



