#include <qobject.h>
#include <qpainter.h>
#include <math.h>
#include <limits.h>
#include "classes.h"
#include "dialogs.h"

extern QList<MarkerType>*markertypes;

Marker::Marker ()
{
  name=0;
}
//****************************************************************************
Marker::~Marker ()
{
  if (name) delete name;
}
//****************************************************************************
MarkerType::MarkerType ()
{
  name=0;
  color=0;
}
//****************************************************************************
MarkerType::~MarkerType ()
{
  if (name) delete name;
  if (color)delete color;
}
//****************************************************************************
void SigWidget::signalinserted (int start, int len)
{
  struct Marker *tmp;
  for (tmp=markers->first();tmp;tmp=markers->next())  //write out labels
      if (tmp->pos>start) tmp->pos+=len;
  refresh ();
}
//****************************************************************************
void SigWidget::signaldeleted (int start, int len)
{
  struct Marker *tmp;
  for (tmp=markers->first();tmp;tmp=markers->next())  //write out labels
      if ((tmp->pos>start)&&(tmp->pos<start+len)) markers->removeRef (tmp);
  refresh ();
}
//****************************************************************************
void SigWidget::deleteMarks ()
{
  if (signal)
    {
      Marker *tmp;
      int l=signal->getLMarker();
      int r=signal->getRMarker();

      for (tmp=markers->first();tmp;tmp=markers->next())  
	if ((tmp->pos>=l)&&(tmp->pos<r)) markers->removeRef (tmp);

      refresh ();
    }
}
//****************************************************************************
void SigWidget::loadMarks ()
{
  markers->clear(); //remove old marks...

  appendMarks ();
}
//****************************************************************************
void SigWidget::appendMarks ()
{
  char buf[120];
  QString name=QFileDialog::getOpenFileName (0,"*.label",this);
  if (!name.isNull())
    {
      QFile in(name.data());
      in.open (IO_ReadOnly);
      {
	MarkerType *act;
	while ((strncmp (buf,"Labels",6)!=0)&&(in.readLine(buf,120)>0));

	for (act=markertypes->first();act;act=markertypes->next()) act->selected=-1;

	while (in.readLine (buf,120)>0)
	  {
	    if (strncmp(buf,"Type",4)==0)
	      {
		int num;
		int named;
		char name[120];
		int r,g,b;
		int set=false;
		sscanf (buf,"Type %d %s %d %d %d %d",&num,&name,&named,&r,&g,&b);
		for (act=markertypes->first();act;act=markertypes->next()) //linear search for label type ...
		  if (strcmp (act->name->data(),name)==0)
		    {
		      set=true;
		      act->selected=num;
		    }
		if (!set) //generate new type...
		  {
		    struct MarkerType *newtype=new MarkerType;
		    if (newtype)
		      {
			newtype->name=new QString (name);
			newtype->named=named;
			newtype->selected=num;
			newtype->color=new QColor (r,g,b);

			emit addMarkerType (newtype);
		      }
		  }
	      }
	    if (strncmp(buf,"Samples",7)==0) break;
	  }
	//left above loop, so begin to pick up marker positions...

	int num;
	int pos;
	char name [120];

	while (in.readLine (buf,120)>0)
	  {
	    name[0]=0;
	    sscanf (buf,"%d %d %s",&num,&pos,&name);

	    struct Marker *newmark=new Marker;

	    if (newmark)
	      {
		newmark->pos=pos;

		if (markertypes->current())
		  {
		    if (markertypes->current()->selected!=num)
		      for (act=markertypes->first();act->selected!=num;act=markertypes->next());   
		  }
		else
		  for (act=markertypes->first();act->selected!=num;act=markertypes->next());   
		newmark->type=markertypes->current();

		if (markertypes->current()->named)
		  if (name[0]!=0) newmark->name=new QString (name);
		  else newmark->name=0;

		markers->append (newmark);
	      }
	  }
      }
    }
  refresh ();
}
//****************************************************************************
void SigWidget::saveMarks ()
{
  MarkSaveDialog dialog;
  if (dialog.exec())
    {
      dialog.getSelection();
      QString name=QFileDialog::getSaveFileName (0,"*.label",this);
      if (!name.isNull())
	{
	  QFile out(name.data());
	  int num=0;
	  char buf[160];
	  out.open (IO_WriteOnly);
	  out.writeBlock ("Labels\n",7);
	  Marker     *tmp;
	  MarkerType *act;

	  for (act=markertypes->first();act;act=markertypes->next()) //write out label types
	    if (act->selected)
	      {
		sprintf (buf,"Type %d %s %d %d %d %d\n",num,act->name->data(),act->named,act->color->red(),act->color->green(),act->color->blue());
		act->selected=num++;
		out.writeBlock (&buf[0],strlen(buf));
	      }
	  out.writeBlock ("Samples\n",8);
	  for (tmp=markers->first();tmp;tmp=markers->next())  //write out labels
	    {
	      if (tmp->type->named)
		sprintf (buf,"%d %d %s\n",tmp->type->selected,tmp->pos,tmp->name->data());
	      else 
		sprintf (buf,"%d %d\n",tmp->type->selected,tmp->pos);
	      out.writeBlock (&buf[0],strlen(buf));
	    }
	}
    }
}
//****************************************************************************
void SigWidget::addMark ()
{
  if (signal&&markertype)
    {
      Marker *newmark=new Marker;
	    
      newmark->pos=signal->getLMarker();
      newmark->type=markertype;
      if (markertype->named)
	{
	  StringEnterDialog dialog(this);
	  if (dialog.exec())
	    {   
	      newmark->name=new QString (dialog.getString());
	      markers->inSort (newmark);
	    }
	  else delete newmark;
	}
      else
	{
	  newmark->name=0;
	  markers->inSort (newmark);
	}
      refresh();
    }
}
//****************************************************************************
void SigWidget::saveBlocks (int bit)
{
    if (signal)
    {
      SaveBlockDialog dialog (this);
      if (dialog.exec ())
	{
	}
    }
}
//****************************************************************************
void SigWidget::markSignal ()
{
  if (signal)
    {
      Marker *newmark;
      MarkSignalDialog dialog (this,signal->getRate());
      if (dialog.exec ())
	{
	  int level=(int) ((((double)dialog.getLevel ())*(1<<23))/100);
	  int time= dialog.getTime ();
	  int len=signal->getLength();
	  int *sam=signal->getSample();
	  struct MarkerType *start=markertypes->at(dialog.getType1());
	  struct MarkerType *stop=markertypes->at (dialog.getType2());

	  newmark=new Marker();  //generate initial marker
	  newmark->pos=0;
	  newmark->type=start;
	  newmark->name=0;
	  markers->inSort (newmark);

	  for (int i=0;i<len;i++)
	    {
	      if (abs(sam[i])<level)
		{
		  int j=i;
		  while ((i<len) &&(abs(sam[i])<level)) i++;
		  if (i-j>time)
		    {
		      //insert markers...
		      newmark=new Marker();
		      newmark->pos=i;
		      newmark->type=start;
		      newmark->name=0;
		      markers->inSort (newmark);

		      if (start!=stop)
			{
			  newmark=new Marker();
			  newmark->pos=j;
			  newmark->type=stop;
			  newmark->name=0;
			  markers->inSort (newmark);
			}
		    }
		}
	    }

	  newmark=new Marker();  //generate final marker
	  newmark->pos=len-1;
	  newmark->type=stop;
	  newmark->name=0;
	  markers->inSort (newmark);


	  refresh ();
	}
    }
}


