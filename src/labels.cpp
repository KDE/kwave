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
#include <kmsgbox.h>

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

int SignalWidget::mstosamples (double ms)
{
  return   (int)(ms*signalmanage->getRate()/1000);
}
//****************************************************************************
void selectMarkers (const char *command)
{
  KwaveParser parser(command);
} 
//****************************************************************************
MarkerType *findMarkerType (const char *txt)
{
  int cnt=0;
  MarkerType *act;

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
  for (tmp=labels->first();tmp;tmp=labels->next()) 
      if (tmp->pos>start) tmp->pos+=len;
  setRange (start,start+len); 
  refresh ();
}
//****************************************************************************
void SignalWidget::signaldeleted (int start, int len)
{
  Marker *tmp;
  for (tmp=labels->first();tmp;tmp=labels->next())
    {
      if ((tmp->pos>start)&&(tmp->pos<start+len)) //if marker position is within selected boundaries
	{
	  labels->remove ();
	  tmp=labels->first();
	}
      if (tmp->pos>=start+len) tmp->pos-=len;  //if it is greater correct position
    }
  setRange (start,start); 
  refresh ();
}
//****************************************************************************
void SignalWidget::deleteLabel ()
{
  if (signalmanage)
    {
      Marker *tmp;
      int l=signalmanage->getLMarker();
      int r=signalmanage->getRMarker();

      for (tmp=labels->first();tmp;tmp=labels->next()) 
	{
	  int pos=mstosamples (tmp->pos);
	  if ((pos>=l)&&(pos<r))
	    {
	      labels->remove (tmp);
	      tmp=labels->first();
	    }
	}
      refresh ();
    }
}
//****************************************************************************
void SignalWidget::loadLabel ()
{
  labels->clear(); //remove old Label...

  appendLabel ();
}
//****************************************************************************
void SignalWidget::appendLabel ()
{
  QString name=QFileDialog::getOpenFileName (0,"*.label",this);
  if (!name.isNull())
    {
      char *comstr=catString ("loadbatch (",name,")");
      globals.port->putMessage (comstr);
    }
  refresh ();
}
//****************************************************************************
void SignalWidget::saveLabel (const char *typestring)
{
  QString name=QFileDialog::getSaveFileName (0,"*.label",this);
  if (!name.isNull())
    {
      FILE *out;
      out=fopen (name.data(),"w");

      KwaveParser parser (typestring);
      Marker     *tmp;
      MarkerType *act;

      const char *actstring=parser.getFirstParam();

      while (actstring)
	{
	  printf ("selecting %s\n",actstring);
	  for (act=globals.markertypes.first();act;act=globals.markertypes.next())
	    if (strcmp(act->name,actstring)==0)
	      {
		printf ("selected\n");
		act->selected=true;
		break;
	      }
	  actstring=parser.getNextParam();
	}

      for (act=globals.markertypes.first();act;act=globals.markertypes.next())
	//write out all selected label types
	if (act->selected)
	  fprintf (out,"%s\n",act->getCommand());

      //ended writing of types, so go on with the labels...

      for (tmp=labels->first();tmp;tmp=labels->next())  //write out labels
	{
	  fprintf (out,tmp->getCommand());
	  fprintf (out,"\n");
	}

      fclose (out);
    }
}
//****************************************************************************
void SignalWidget::addLabel (const char *params)
{
  if (signalmanage&&markertype)
    {
      KwaveParser parser(params);
      Marker *newmark;

      if (parser.countParams()>0)
	{
	  newmark=new Marker (params);
	}
      else
	{
	  double pos=((double)signalmanage->getLMarker())*1000/signalmanage->getRate();
	  newmark=new Marker (pos,markertype);

	  //should it need a name ?
	  if (markertype->named)
	    {
	      KwaveDialog *dialog =
		DynamicLoader::getDialog ("stringenter",new DialogOperation("Enter name of label :",true)); //create a modal dialog
	      
	      if (dialog)
		{
		  dialog->show ();
	
		  if (dialog->result())
		    {
		      printf ("dialog:%s\n",dialog->getCommand());
		      newmark->setName (dialog->getCommand());
		      delete dialog;
		    }
		  else
		    {
		      delete newmark;
		      newmark=0;
		    }
		}
	      else
		{
		  KMsgBox::message (this,"Error",klocale->translate("Dialog not loaded !"));
		  delete newmark;
		  newmark=0;
		}
	    }
	}

      if (newmark)
	{
	  labels->inSort (newmark);
      
	  refresh();
	}
    }
}
//****************************************************************************
void SignalWidget::jumptoLabel ()
// another fine function contributed by Gerhard Zintel
// if lmarker == rmarker (no range selected) cursor jumps to the nearest label
// if lmarker <  rmarker (range is selected) lmarker jumps to next lower label or zero
// rmarker jumps to next higher label or end
{
  if (signalmanage)
    {
      int lmarker=signalmanage->getLMarker(), rmarker=signalmanage->getRMarker();
      bool RangeSelected = (rmarker - lmarker) > 0;
      if (labels)
      {
        Marker *tmp;
	int position = 0;
	for (tmp=labels->first();tmp;tmp=labels->next())
	  if (RangeSelected) {
	    if (tmp->pos < lmarker)
	      if (abs(lmarker-position)>abs(lmarker-mstosamples(tmp->pos))) position = mstosamples(tmp->pos);
	}
	else if (abs(lmarker-position)>abs(lmarker-mstosamples(tmp->pos))) position = mstosamples(tmp->pos);
	lmarker = position;
	position = signalmanage->getLength();
	for (tmp=labels->first();tmp;tmp=labels->next())
	  if (tmp->pos > rmarker)
	    if (abs(rmarker-position)>abs(rmarker-mstosamples(tmp->pos))) position = mstosamples(tmp->pos);
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
		  for (tmp=labels->first();tmp;tmp=labels->next())
		    {
		      if (tmp->getType()==act)
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
			  last=mstosamples(tmp->pos);
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

	  for (tmp=labels->first();tmp;tmp=labels->next())  //traverse list of labels
	    {
	      if (tmp->getType()==start)
		{
		  for (tmp2=tmp;tmp2;tmp2=labels->next())  //traverse rest of list to find next stop marker
		    if (tmp2->getType()==stop)
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
	  
      int level=(int) (parser.toDouble()/100*(1<<23));

      int len=signalmanage->getLength();
      int *sam=signalmanage->getSignal()->getSample();
      MarkerType *start=findMarkerType(parser.getNextParam());
      MarkerType *stop=findMarkerType (parser.getNextParam());
      int time=(int) (parser.toDouble ()*signalmanage->getRate()/1000);

      printf ("%d %d\n",level,time);
      printf ("%s %s\n",start->name,stop->name);

      ProgressDialog *dialog=
	new ProgressDialog (len,"Searching for Signal portions...");

      if (dialog&&start&&stop)
	{
	  dialog->show();

	  newmark=new Marker(0,start);  //generate initial marker

	  labels->inSort (newmark);

	  for (int i=0;i<len;i++)
	    {
	      if (abs(sam[i])<level)
		{
		  int j=i;
		  while ((i<len) &&(abs(sam[i])<level)) i++;

		  if (i-j>time)
		    {
		      //insert labels...
		      newmark=new Marker(i,start);
		      labels->inSort (newmark);

		      if (start!=stop)
			{
			  newmark=new Marker(j,stop);
			  labels->inSort (newmark);
			}
		    }
		}
	      dialog->setProgress (i);
	    }

	  newmark=new Marker(len-1,stop);
	  labels->inSort (newmark);

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
	  labels->inSort (newmark);

	  while (cnt<len-2*AUTOKORRWIN)
	    {
	      if (octave)
		next=findNextRepeatOctave (&sam[cnt],high,adjust);
	      else
		next=findNextRepeat (&sam[cnt],high);

	      if ((next<low)&&(next>high))
		{
		  newmark=new Marker(cnt,start);

		  labels->inSort (newmark);
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
void SignalWidget::addLabelType (MarkerType *marker)
{
  globals.markertypes.append (marker);
  if (manage) manage->addNumberedMenuEntry ("labeltypes",marker->name);
}
//*****************************************************************************
void SignalWidget::addLabelType (const char *str)
{
  MarkerType *marker=new MarkerType(str);
  if (marker) addLabelType (marker);
}
