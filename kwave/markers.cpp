//This File includes methods of class SignalWidget that deal with markers
//it also contains methods of Marker and MarkerType class.

#include <qobject.h>
#include <qpainter.h>
#include <math.h>
#include <limits.h>
#include "classes.h"
#include "dialogs.h"
#include "pitchwidget.h"

#define	AUTOKORRWIN 320 
//windowsize for autocorellation, propably a little bit to short for
//lower frequencies, but this will get configurable in future

extern QList<MarkerType>*markertypes;
int findNextRepeat       (int *,int);
int findNextRepeatOctave (int *,int,double =1.005);
int findFirstMark  (int *,int);
float autotable  [AUTOKORRWIN];
float weighttable[AUTOKORRWIN];
//****************************************************************************
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
MarkerList::MarkerList ()
{
}
//****************************************************************************
MarkerList::~MarkerList ()
{
  clear();
}
//****************************************************************************
int MarkerList::compareItems (GCI a,GCI b)
{
  Marker *c = (Marker *)a;
  Marker *d = (Marker *)b;
  return c->pos-d->pos;
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
void SignalWidget::signalinserted (int start, int len)
{
  struct Marker *tmp;
  for (tmp=markers->first();tmp;tmp=markers->next()) 
      if (tmp->pos>start) tmp->pos+=len;
  refresh ();
}
//****************************************************************************
void SignalWidget::signaldeleted (int start, int len)
{
  struct Marker *tmp;
  for (tmp=markers->first();tmp;tmp=markers->next())
    {
      if ((tmp->pos>start)&&(tmp->pos<start+len)) //if marker position is within selected boundaries
	{
	  markers->remove ();
	  tmp=markers->first();
	}
      if (tmp->pos>=start+len) tmp->pos-=len;  //if it is greater correct position
    }
  refresh ();
}
//****************************************************************************
void SignalWidget::deleteMarks ()
{
  if (signal)
    {
      Marker *tmp;
      int l=signal->getLMarker();
      int r=signal->getRMarker();

      for (tmp=markers->first();tmp;tmp=markers->next())  
	if ((tmp->pos>=l)&&(tmp->pos<r))
	  {
	    markers->remove (tmp);
	    tmp=markers->first();
	  }
      refresh ();
    }
}
//****************************************************************************
void SignalWidget::loadMarks ()
{
  markers->clear(); //remove old marks...

  appendMarks ();
}
//****************************************************************************
void SignalWidget::appendMarks ()
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
		sscanf (buf,"Type %d %s %d %d %d %d",&num,&name[0],&named,&r,&g,&b);
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
	    sscanf (buf,"%d %d %s",&num,&pos,&name[0]);

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
void SignalWidget::saveMarks ()
{
  MarkSaveDialog dialog(this);
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

	  for (act=markertypes->first();act;act=markertypes->next())
	    //write out all label types
	    if (act->selected)
	      {
		sprintf (buf,"Type %d %s %d %d %d %d\n",num,act->name->data(),act->named,act->color->red(),act->color->green(),act->color->blue());
		act->selected=num++;
		out.writeBlock (&buf[0],strlen(buf));
	      }
	  out.writeBlock ("Samples\n",8);
	  for (tmp=markers->first();tmp;tmp=markers->next())  //write out labels
	    {
	      //type must be named, and qstring name must be non-null
	      if ((tmp->type->named)&&(tmp->name))
		sprintf (buf,"%d %d %s\n",tmp->type->selected,tmp->pos,tmp->name->data());
	      else 
		sprintf (buf,"%d %d\n",tmp->type->selected,tmp->pos);
	      out.writeBlock (&buf[0],strlen(buf));
	    }
	}
    }
}
//****************************************************************************
void SignalWidget::addMark ()
{
  if (signal&&markertype)
    {
      Marker *newmark=new Marker;

      newmark->pos=signal->getLMarker();
      newmark->type=markertype;
      if (markertype->named)
	{
	  StringEnterDialog dialog(this,"Please Enter Name :");
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
void SignalWidget::savePeriods ()
{
  if (signal)
    {
      MarkSaveDialog dialog (this,"Select label, which periodities are to be saved:",false);
      if (dialog.exec())
	{
	  dialog.getSelection();
	  MarkerType *act;
	  Marker *tmp;
	  int last=0;
	  int rate=signal->getRate ();

	  QString name=QFileDialog::getSaveFileName (0,"*.dat",this);
	  if (!name.isNull())
	    {
	      QFile out(name.data());
	      char buf[160];
	      float freq=0,time,lastfreq=0;
	      out.open (IO_WriteOnly);
	      int first=true;

	      for (act=markertypes->first();act;act=markertypes->next())
		//write only selected label type
		if (act->selected)
		  //traverse list of all labels
		  for (tmp=markers->first();tmp;tmp=markers->next())
		    {
		      if (tmp->type==act)
			{
			  freq=tmp->pos-last;
			  time=last*1000/rate;

			  if ((!first)&&(freq!=lastfreq))
			    {
			      lastfreq=freq;
			      freq=1/(freq/rate);
			      sprintf (buf,"%f %f\n",time,freq);
			      out.writeBlock (&buf[0],strlen(buf));
			    }
			  else lastfreq=freq;
			  first=false;
			  last=tmp->pos;
			}
		    }

	      if (!first) //make sure last tone gets its length
		{
		  time=last*1000/rate;
		  sprintf (buf,"%f %f\n",time,freq);
		  out.writeBlock (&buf[0],strlen(buf));
		}

	      out.close ();
	    }
	}
    }
}
//****************************************************************************
void SignalWidget::saveBlocks (int bit)
{
    if (signal)
    {
      QString filename;
      SaveBlockDialog dialog (this);
      if (dialog.exec ())
	{
	  struct MarkerType *start=markertypes->at(dialog.getType1());
	  struct MarkerType *stop=markertypes->at(dialog.getType2());
	  QDir *savedir=dialog.getDir();
	  
	  struct Marker *tmp;
	  struct Marker *tmp2;
	  int count=0;
	  int l=signal->getLMarker(); //save old marker positions...
	  int r=signal->getRMarker(); //

	  for (tmp=markers->first();tmp;tmp=markers->next())  //traverse list of markers
	    {
	      if (tmp->type==start)
		{
		  for (tmp2=tmp;tmp2;tmp2=markers->next())  //traverse rest of list to find next stop marker
		    if (tmp2->type==stop)
		      {
			char buf[128];
			sprintf (buf,"%s%04d.wav",dialog.getName(),count);
			//lets hope noone tries to save more than 10000 blocks...

			signal->setMarkers (tmp->pos,tmp2->pos);
			filename=savedir->absFilePath(buf);
			signal->save (&filename,bit,true);  //save selected range...
			count++;
			break;
		      }
		}
	    }
	  signal->setMarkers (l,r);
	}
    }
}
//****************************************************************************
void SignalWidget::markSignal ()
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
//****************************************************************************
void SignalWidget::markPeriods ()
{
  if (signal)
    {
      PitchDialog dialog (this,signal->getRate()) ;
      if (dialog.exec())
	{
	  int high   =signal->getRate()/dialog.getHigh();
	  int low    =signal->getRate()/dialog.getLow();
	  int octave =dialog.getOctave();
	  double adjust=1+((double)dialog.getAdjust()/1000);

	  for (int i=0;i<AUTOKORRWIN;i++)
	    autotable[i]=1-(((double)i*i*i)/(AUTOKORRWIN*AUTOKORRWIN*AUTOKORRWIN)); //generate static weighting function

	  if (octave) for (int i=0;i<AUTOKORRWIN;i++) weighttable[i]=1; //initialise moving weight table

	  Marker *newmark;
	  int next;
	  int len=signal->getLength();
	  int *sam=signal->getSample();
	  struct MarkerType *start=markertype;
	  int cnt=findFirstMark (sam,len);

	  ProgressDialog *dialog=new ProgressDialog (len-AUTOKORRWIN,"Correlating Signal to find Periods:");
	  dialog->show();

	  if (dialog)
	    {
	      newmark=new Marker();
	      newmark->pos=cnt;
	      newmark->type=start;
	      newmark->name=0;
	      markers->inSort (newmark);

	      while (cnt<len-2*AUTOKORRWIN)
		{
		  if (octave)
		    next=findNextRepeatOctave (&sam[cnt],high,adjust);
		  else
		    next=findNextRepeat (&sam[cnt],high);

		  if ((next<low)&&(next>high))
		    {
		      newmark=new Marker();
		      newmark->pos=cnt;
		      newmark->type=start;
		      newmark->name=0;
		      markers->inSort (newmark);
		    }
		  if (next<AUTOKORRWIN) cnt+=next;
		  else
		    if (cnt<len-AUTOKORRWIN)
		      {
			int a=findFirstMark (&sam[cnt],len-cnt);
			if (a>0) cnt+=a;
			else cnt+=high;
		      }
		    else cnt=len;

		  dialog->setProgress (cnt);
		}

	      delete dialog;

	      refresh ();
	    }
	}
    }
}
//*****************************************************************************
int findNextRepeat (int *sample,int high)
  //autocorellation of a windowed part of the sample
  //returns length of period, if found
{
  int	i,j;
  double gmax=0,max,c;
  int	maxpos=AUTOKORRWIN;
  int	down,up;	//flags

  max=0;
  for (j=0;j<AUTOKORRWIN;j++)
    gmax+=((double)sample[j])*sample [j];

  //correlate signal with itself for finding maximum integral

  down=0;
  up=0;
  i=high;
  max=0;
  while (i<AUTOKORRWIN)
    {
      c=0;
      for (j=0;j<AUTOKORRWIN;j++) c+=((double)sample[j])*sample [i+j];
      c=c*autotable[i]; //multiply window with weight for preference of high frequencies
      if (c>max) max=c,maxpos=i;
      i++;
    }
  return maxpos;
} 
//*****************************************************************************
int findNextRepeatOctave (int *sample,int high,double adjust=1.005)
  //autocorellation of a windowed part of the sample
  //same as above only with an adaptive weighting to decrease fast period changes
{
  int	i,j;
  double gmax=0,max,c;
  int	maxpos=AUTOKORRWIN;
  int	down,up;	//flags

  max=0;
  for (j=0;j<AUTOKORRWIN;j++)
    gmax+=((double)sample[j])*sample [j];

  //correlate signal with itself for finding maximum integral

  down=0;
  up=0;
  i=high;
  max=0;
  while (i<AUTOKORRWIN)
    {
      c=0;
      for (j=0;j<AUTOKORRWIN;j++) c+=((double)sample[j])*sample [i+j];
      c=c*autotable[i]*weighttable[i];
      //multiply window with weight for preference of high frequencies
      if (c>max) max=c,maxpos=i;
      i++;
    }
  
    for (int i=0;i<AUTOKORRWIN;i++) weighttable[i]/=adjust;

  weighttable[maxpos]=1;
  weighttable[maxpos+1]=.9;
  weighttable[maxpos-1]=.9;
  weighttable[maxpos+2]=.8;
  weighttable[maxpos-2]=.8;

  float buf[7];

  for (int i=0;i<7;buf[i++]=.1)

    //low pass filter
  for (int i=high;i<AUTOKORRWIN-3;i++)
    {
      buf[i%7]=weighttable[i+3];
       weighttable[i]=(buf[0]+buf[1]+buf[2]+buf[3]+buf[4]+buf[5]+buf[6])/7;
    }

  return maxpos;
} 
//*****************************************************************************
int findFirstMark (int *sample,int len)
  //finds first sample that is non-zero, or one that preceeds a zero crossing
{
  int i=1;
  int last=sample[0];
  int act=last;
  if ((last<100)&&(last>-100)) i=0;
  else
    while (i<len)
      {
	act=sample[i];
	if ((act<0)&&(last>=0)) break;
	if ((act>0)&&(last<=0)) break;
	last=act;
	i++;
      }
  return i;
}
//*****************************************************************************
void SignalWidget::convertMarkstoPitch ()
{
  if (signal)
    {
      MarkSaveDialog dialog (this,"Select type of labels to be converted :",false);
      if (dialog.exec())
	{
	  dialog.getSelection ();
	  MarkerType *act;
	  Marker     *tmp;
	  int   len=signal->getLength()/2;
	  float *data=new float[len];
	  float freq;
	  float rate=(float)signal->getRate();

	  for (int i=0;i<len;data[i++]=0);

	  for (act=markertypes->first();act;act=markertypes->next())
	    {
	      if (act->selected)
		{
		  int   last=0;
		  //traverse list of all labels of the selected type...
		  for (tmp=markers->first();tmp;tmp=markers->next())
		    {
		      if (tmp->type==act)
			{
			  if (tmp->pos!=last)
			    {
			      freq=rate/(tmp->pos-last);
			    }
			  else freq=0;

			  for (int i=last;i<tmp->pos;i+=2) data[i/2]=freq;

			  last=tmp->pos;
			}
		    }
		  PitchWindow *window=new PitchWindow (signal->getName());
		  window->show ();
		  if (window) window->setSignal (data,len,rate/2);
		}
	    }
	}
    }
}
