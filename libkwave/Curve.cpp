#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "parser.h"
#include "curve.h"
#include "kwavestring.h"
#include "interpolation.h"

//****************************************************************************
Curve::Curve ()
{
  comstr=0;
  interpolation=new Interpolation (0);
  interpolationtype=0;
  points=new PointSet();
  points->setAutoDelete (true);
}
//****************************************************************************
Curve::Curve (const char *command)
{
  comstr=0;
  interpolation=new Interpolation (0);
  interpolationtype=0;
  points=new PointSet();
  points->setAutoDelete (true);

  KwaveParser parse(command);
  const char *type=parse.getFirstParam();

  if (type) setInterpolationType (type);
  else debug ("Curve::could not parse type\n");

  double x=1,y;

  while (!parse.isDone())
    {
      x=parse.toDouble();
      y=parse.toDouble();

      append (x,y);
    }
}
//****************************************************************************
const char *Curve::getCommand ()
{
  const char **names=Interpolation::getTypes();
  char buf[512];
  Point *tmp;
  char *tmpstr=strdup (names[interpolationtype]);

  comstr=catString ("curve (",tmpstr);
  if (tmpstr) free (tmpstr);
  tmpstr=comstr;

  for ( tmp=points->first(); tmp; tmp=points->next() )
    {
      sprintf (buf,",%f,%f",tmp->x,tmp->y);
      comstr=catString (tmpstr,buf);
      if (tmpstr) free (tmpstr);
      tmpstr=comstr;
    }

  comstr=catString (tmpstr,")");
  if (tmpstr) free (tmpstr);
  return comstr;
}
//****************************************************************************
double* Curve::getInterpolation (int width)
{
  return interpolation->getInterpolation (this,width);
};
//****************************************************************************
void Curve::setInterpolationType (const char *name)
{
  const char **names=Interpolation::getTypes();
  int cnt=0;

  while (names[cnt])
    {
      if (strcmp(names[cnt],name)==0)
	{
	  interpolationtype=cnt;
	  interpolation->setType (cnt);
	  break;
	}
      cnt++;
    }
};
//****************************************************************************
const char* Curve::getInterpolationType ()
{
  return (Interpolation::getTypes())[interpolationtype];
}
//****************************************************************************
void Curve::deletePoint (Point *last,bool check)
{
  if ((check) || ((last!=points->first())&&(last!=points->last())) )
    {
      points->removeRef (last);
    }
}
//****************************************************************************
void Curve::secondHalf ()
{
  Point *tmp;

  for ( tmp=points->first(); tmp; tmp=points->next() )
    tmp->x=.5+tmp->x/2;

  tmp=points->first();
  Point *newpoint=new Point;
  newpoint->x=0;
  newpoint->y=tmp->y;
  points->insert (0,newpoint);
}
//****************************************************************************
void Curve::deleteSecondPoint ()
{
  Point *tmp;
  
  for ( tmp=points->first(); tmp; tmp=points->next() )
    {
      tmp=points->next();
      if (tmp&&tmp!=points->last())
	{
	  //points should have autodelete... no delete for object is required
	  points->removeRef(tmp);
	  tmp=points->prev();
	}
    }
}
//****************************************************************************
void Curve::addPoint (Point *insert)
{
  Point *tmp;
  
  tmp=points->first();

 //linear search for position
  while ((tmp)&&(tmp->x<insert->x)) tmp=points->next();

  if (tmp) points->insert (points->at(), insert);
  else debug ("point is out of range !\n");
}
//****************************************************************************
void Curve::addPoint (double x,double y)
{
  Point *insert=new Point;
  if (insert)
    {
      insert->x=x;
      insert->y=y;
  
      addPoint (insert);
    }
  else debug ("Curve: could not get new Point\n");
}
//****************************************************************************
Point *Curve::previous (Point *act)
{
  points->findRef (act);
  return points->prev();
}
//****************************************************************************
Point *Curve::next (Point *act)
{
  points->findRef (act);
  return points->next();
}
//****************************************************************************
void Curve::append (double x,double y)
{
  Point *insert=new Point;
  insert->x=x;
  insert->y=y;

  append (insert);
}
//****************************************************************************
void Curve::firstHalf ()
{
  Point *tmp;
  for ( tmp=points->first(); tmp; tmp=points->next() )
    tmp->x=tmp->x/2;

  tmp=points->first();
  Point *newpoint=new Point;
  newpoint->x=1;
  newpoint->y=tmp->y;
  points->insert (0,newpoint);
}
//****************************************************************************
void Curve::VFlip ()
{
  Point *tmp;
  for ( tmp=points->first(); tmp != 0; tmp=points->next() ) tmp->y=1-tmp->y;
}
//****************************************************************************
void Curve::HFlip ()
{
  Point *tmp;
  PointSet *newlist=new PointSet;

  for (tmp=points->last(); tmp; tmp=points->prev())
    {
      tmp->x=1-tmp->x;
      newlist->append (tmp);
    }

  points->setAutoDelete (false);
  delete points;

  points=newlist;
  points->setAutoDelete (true);
}
//****************************************************************************
void Curve::scaleFit(int range)
{
  struct Point *tmp;
  double min=1000,max=0;

  tmp=points->first();
  if (tmp)
    {
      Interpolation interpolation(interpolationtype);

      double *y=interpolation.getInterpolation (this,range);
      for (int i=0;i<range;i++)
	{
	  if (y[i]>max) max=y[i];
	  if (y[i]<min) min=y[i];
	}

      max-=min;
      tmp=points->first();
      for (;tmp;tmp=points->next())
	{
	  tmp->y-=min;	  
	  tmp->y/=max;
	}
    }
}
//****************************************************************************
Point *Curve::findPoint (double px,double py,double tol)
// checks, if given coordinates fit to a control point in the list...
{
  double maxx=px+tol;
  double minx=px-tol;
  double maxy=py+tol;
  double miny=py-tol;

  Point *tmp;
  Point *act=0;
  
  for ( tmp=points->first(); tmp; tmp=points->next() )
    {
      double x=tmp->x;
      double y=tmp->y;

      if (x>maxx) break;
      //the list should be sorted, a match cannot be
      //found anymore, because x is already to big

      if ((x>=minx)&&(x<=maxx)&&(y>=miny)&&(y<=maxy))
	{
	  act=tmp;
	  break;
	}
    }
  return act;
}
//****************************************************************************
Curve::~Curve ()
{
  if (comstr) free (comstr);
}



