//This File includes methods of class SignalWidget that deal with markers
//it also contains methods of Marker and MarkerType class.

#include <math.h>
#include <limits.h>
#include <qobject.h>
#include <qpainter.h>
#include "signalview.h"
#include "dialog_progress.h"
#include "pitchwidget.h"
#include "signalmanager.h"
#include <libkwave/parser.h>
#include <libkwave/markers.h>
#include <libkwave/globals.h>
#include <libkwave/dialogoperation.h>
#include <libkwave/dynamicloader.h>
#include "../libgui/kwavedialog.h"

extern Global globals;

#define	AUTOKORRWIN 320 
//windowsize for autocorellation, propably a little bit to short for
//lower frequencies, but this will get configurable somewhere in another
//dimension or for those of you who can't zap to other dimensions, it will
//be done in future

int findNextRepeat       (int *,int);
int findNextRepeatOctave (int *,int,double =1.005);
int findFirstMark  (int *,int);

float autotable  [AUTOKORRWIN];
float weighttable[AUTOKORRWIN];


//****************************************************************************
void selectMarkers (const char *command)
{
  KwaveParser parser(command);
} 
//****************************************************************************
MarkerType *findMarkerType (const char *txt)
{
  MarkerType *act;
  int cnt=0;

  for (act=globals.markertypes.first();act;act=globals.markertypes.next())
    {
      if (strcmp (act->name,txt)==0) return act;
      cnt++;
    }
  debug ("could not find Markertype %s\n",txt);
  return 0;
}
//****************************************************************************
void SignalWidget::signalinserted (int start, int len)
{
  Marker *tmp;
  for (tmp=markers->first();tmp;tmp=markers->next()) 
      if (tmp->pos>start) tmp->pos+=len;
  setRange (start,start+len); 
  refresh ();
}
//****************************************************************************
void SignalWidget::signaldeleted (int start, int len)
{
  Marker *tmp;
  for (tmp=markers->first();tmp;tmp=markers->next())
    {
      if ((tmp->pos>start)&&(tmp->pos<start+len)) //if marker position is within selected boundaries
	{
	  markers->remove ();
	  tmp=markers->first();
	}
      if (tmp->pos>=start+len) tmp->pos-=len;  //if it is greater correct position
    }
  setRange (start,start); 
  refresh ();
}
//****************************************************************************
void SignalWidget::deleteMarks ()
{
  if (signalmanage)
    {
      Marker *tmp;
      int l=signalmanage->getLMarker();
      int r=signalmanage->getRMarker();

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

	//Skip lines until keywords label follows
	while ((strncmp (buf,"Labels",6)!=0)&&(in.readLine(buf,120)>0));

	//unchoose all label types
	for (act=globals.markertypes.first();
	     act;
	     act=globals.markertypes.next()) act->selected=-1;

	while (in.readLine (buf,120)>0)
	  {
	    if (matchCommand(buf,"labeltype"))
	      {
		KwaveParser parser (buf);
		const char *name=parser.getFirstParam ();
		bool isthere=false;

		//check if label is already in list of known labels
		for (act=globals.markertypes.first()
		       ;act;
		     act=globals.markertypes.next())
		  if (strcmp (act->name,name)==0) isthere=true;


		if (!isthere) //nope ... ->generate new type...
		  {
		    MarkerType *newtype=new MarkerType (buf);
		    if (newtype) addMarkType (newtype);
		  }
	      }
	    else
	      //if keyword samples is following, break to read in data entries
	      if (strncmp(buf,"Samples",7)==0) break;
	  }
	//left above loop, so begin to pick up marker positions...

	int num;
	int pos;
	char *name;
	char *name2=new char[512];

	if (name2)
	  {
	    while (in.readLine (buf,120)>0)
	      {
		name=name2;
		name[0]=0;
		sscanf (buf,"%d %d %s",&num,&pos,&name[0]);

		if (globals.markertypes.current())
		  {
		    if (globals.markertypes.current()->selected!=num)
		      for (act=globals.markertypes.first();act->selected!=num;act=globals.markertypes.next());   
		  }
		else
		  for (act=globals.markertypes.first();act->selected!=num;act=globals.markertypes.next());   

		if (name[0]==0) name=0;
		if (!globals.markertypes.current()->named) name=0;

		Marker *newmark=new Marker(pos,globals.markertypes.current(),name);

		if (newmark)
		  markers->append (newmark);

	      }
	    delete [] name2;
	  }
      }
    }
  refresh ();
}
//****************************************************************************
void SignalWidget::saveMarks ()
{
  KwaveDialog *dialog =
    DynamicLoader::getDialog ("marksave",new DialogOperation(&globals,signalmanage->getRate(),0,0));

  if ((dialog)&&(dialog->exec()))
    {   
      selectMarkers (dialog->getCommand());

      QString name=QFileDialog::getSaveFileName (0,"*.label",this);
      if (!name.isNull())
	{
	  FILE *out;
	  out=fopen (name.data(),"w");
	  char buf[160];


	  //write file header for later identification
	  fprintf (out,"Labels\n");
	  Marker     *tmp;
	  MarkerType *act;

	  for (act=globals.markertypes.first();act;act=globals.markertypes.next())
	    //write out all selected label types
	    if (act->selected)
		fprintf (out,"%s\n",act->getCommand());

	  //ended writing of types, so go on with the labels...
	  fprintf (out,"Samples\n");
	  for (tmp=markers->first();tmp;tmp=markers->next())  //write out labels
	    {
	      //type must be named, and qstring name must be non-null
	      if ((tmp->type->named)&&(tmp->name))
		sprintf (buf,"%d %d %s\n",tmp->type->selected,tmp->pos,tmp->name);
	      else 
		sprintf (buf,"%d %d\n",tmp->type->selected,tmp->pos);
	      fprintf (out,buf);
	    }
	}
    }
}
//****************************************************************************
void SignalWidget::addMark ()
{
  if (signalmanage&&markertype)
    {
      Marker *newmark=new Marker (signalmanage->getLMarker(),markertype);

      //should it need a name ?
      if (markertype->named)
	{
	  KwaveDialog *dialog =
	    DynamicLoader::getDialog ("command",new DialogOperation("Enter name of label :",true)); //create a modal dialog

	  if ((dialog)&&(dialog->exec()))
	    {
	      newmark->name=duplicateString (dialog->getCommand());
	      markers->inSort (newmark);
	      delete dialog;
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
void SignalWidget::jumptoLabel ()
//another fine function contributed by Gerhard Zintel
// if lmarker == rmarker (no range selected) cursor jumps to the nearest label
// if lmarker <  rmarker (range is selected) lmarker jumps to next lower label or zero
// rmarker jumps to next higher label or end
{
  if (signalmanage)
    {
      int lmarker=signalmanage->getLMarker(), rmarker=signalmanage->getRMarker();
      bool RangeSelected = (rmarker - lmarker) > 0;
      if (markers)
      {
        Marker *tmp;
	int position = 0;
	for (tmp=markers->first();tmp;tmp=markers->next())
	  if (RangeSelected) {
	    if (tmp->pos < lmarker)
	      if (abs(lmarker-position)>abs(lmarker-tmp->pos)) position = tmp->pos;
	}
	else if (abs(lmarker-position)>abs(lmarker-tmp->pos)) position = tmp->pos;
	lmarker = position;
	position = signalmanage->getLength();
	for (tmp=markers->first();tmp;tmp=markers->next())
	  if (tmp->pos > rmarker)
	    if (abs(rmarker-position)>abs(rmarker-tmp->pos)) position = tmp->pos;
	rmarker = position;
	if (RangeSelected) signalmanage->setMarkers (lmarker,rmarker);
	else signalmanage->setMarkers (lmarker,lmarker);
	refresh ();
      }
    }
}   
//****************************************************************************
void SignalWidget::savePeriods ()
{
  if (signalmanage)
    {
      KwaveDialog *dialog =
	DynamicLoader::getDialog ("marksave",new DialogOperation(&globals,signalmanage->getRate(),0,0));

      if ((dialog)&&(dialog->exec()))
	{   
	  selectMarkers (dialog->getCommand());

	  MarkerType *act;
	  Marker *tmp;
	  int last=0;
	  int rate=signalmanage->getRate ();

	  QString name=QFileDialog::getSaveFileName (0,"*.dat",this);
	  if (!name.isNull())
	    {
	      QFile out(name.data());
	      char buf[160];
	      float freq=0,time,lastfreq=0;
	      out.open (IO_WriteOnly);
	      int first=true;

	      for (act=globals.markertypes.first();act;act=globals.markertypes.next())
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
    if (signalmanage)
    {
      KwaveDialog *dialog =
	DynamicLoader::getDialog ("saveblock",new DialogOperation(&globals,signalmanage->getRate(),0,0));

      if ((dialog)&&(dialog->exec()))
	{   
	  KwaveParser parser (dialog->getCommand());

	  const char *filename=parser.getFirstParam();
	  QDir *savedir=new QDir (parser.getNextParam());

	  MarkerType *start=findMarkerType(parser.getNextParam());
	  MarkerType *stop=findMarkerType (parser.getNextParam());
	  
	  Marker *tmp;
	  Marker *tmp2;
	  int count=0;
	  int l=signalmanage->getLMarker(); //save old marker positions...
	  int r=signalmanage->getRMarker(); //

	  for (tmp=markers->first();tmp;tmp=markers->next())  //traverse list of markers
	    {
	      if (tmp->type==start)
		{
		  for (tmp2=tmp;tmp2;tmp2=markers->next())  //traverse rest of list to find next stop marker
		    if (tmp2->type==stop)
		      {
			char buf[128];
			sprintf (buf,"%s%04d.wav",filename,count);
			//lets hope noone tries to save more than 10000 blocks...

			signalmanage->setMarkers (tmp->pos,tmp2->pos);
			filename=savedir->absFilePath(buf);
			signalmanage->save (filename,bit,true);  //save selected range...
			count++;
			break;
		      }
		}
	    }
	  signalmanage->setMarkers (l,r);
	}
    }
}
//****************************************************************************
void SignalWidget::markSignal (const char *str)
{
  if (signalmanage)
    {
      Marker *newmark;

      KwaveParser parser (str);
	  
      int level=(int) (parser.toDouble()*(1<<23)/100);

      int len=signalmanage->getLength();
      int *sam=signalmanage->getSignal()->getSample();
      MarkerType *start=findMarkerType(parser.getNextParam());
      MarkerType *stop=findMarkerType (parser.getNextParam());
      int time=(int) (parser.toDouble ()*signalmanage->getRate()/1000);

      ProgressDialog *dialog=
	new ProgressDialog (len,"Searching for Signal portions...");

      if (dialog&&start&&stop)
	{
	  dialog->show();

	  newmark=new Marker(0,start);  //generate initial marker

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
		      newmark=new Marker(i,start);
		      markers->inSort (newmark);

		      if (start!=stop)
			{
			  newmark=new Marker(j,stop);
			  markers->inSort (newmark);
			}
		    }
		}
	      dialog->setProgress (i);
	    }

	  newmark=new Marker(len-1,stop);
	  markers->inSort (newmark);

	  refresh ();
	  delete dialog;
	}
    }
}
//****************************************************************************
void SignalWidget::markPeriods (const char *str)
{
  if (signalmanage)
    {
      KwaveParser parser (str);

      int high   =signalmanage->getRate()/parser.toInt();
      int low    =signalmanage->getRate()/parser.toInt();
      int octave =parser.toBool ("true");
      double adjust=parser.toDouble ();

      for (int i=0;i<AUTOKORRWIN;i++)
	autotable[i]=1-(((double)i*i*i)/(AUTOKORRWIN*AUTOKORRWIN*AUTOKORRWIN)); //generate static weighting function

      if (octave) for (int i=0;i<AUTOKORRWIN;i++) weighttable[i]=1; //initialise moving weight table

      Marker *newmark;
      int next;
      int len=signalmanage->getLength();
      int *sam=signalmanage->getSignal()->getSample();
      MarkerType *start=markertype;
      int cnt=findFirstMark (sam,len);

      ProgressDialog *dialog=new ProgressDialog (len-AUTOKORRWIN,"Correlating Signal to find Periods:");
      if (dialog)
	{
	  dialog->show();

	  newmark=new Marker(cnt,start);
	  markers->inSort (newmark);

	  while (cnt<len-2*AUTOKORRWIN)
	    {
	      if (octave)
		next=findNextRepeatOctave (&sam[cnt],high,adjust);
	      else
		next=findNextRepeat (&sam[cnt],high);

	      if ((next<low)&&(next>high))
		{
		  newmark=new Marker(cnt,start);

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
void SignalWidget::setMarkType  (int num)
{
  this->setOp (SELECTMARK+num);
}
//*****************************************************************************
void SignalWidget::addMarkType (MarkerType *marker)
{
  globals.markertypes.append (marker);
  if (manage) manage->addNumberedMenuEntry ("labeltypes",marker->name);
}
//*****************************************************************************
void SignalWidget::addMarkType (const char *str)
{
  MarkerType *marker=new MarkerType(str);
  if (marker) addMarkType (marker);
}
//*****************************************************************************
void SignalWidget::convertMarkstoPitch (const char *)
{
  if (signalmanage)
    {
      KwaveDialog *dialog =
	DynamicLoader::getDialog ("marksave",new DialogOperation(&globals,signalmanage->getRate(),0,0));

      if ((dialog)&&(dialog->exec()))
	{   
	  selectMarkers (dialog->getCommand());

	  MarkerType *act;
	  Marker     *tmp;
	  int   len=signalmanage->getLength()/2;
	  float *data=new float[len];
	  float freq;
	  float rate=(float)signalmanage->getRate();

	  for (int i=0;i<len;data[i++]=0);

	  for (act=globals.markertypes.first();act;act=globals.markertypes.next())
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
		  PitchWindow *window=new PitchWindow (signalmanage->getName());
		  window->show ();
		  if (window) window->setSignal (data,len,rate/2);
		}
	    }
	}
    }
}
