#include <unistd.h>
#include "sample.h"
#include <kmsgbox.h>
#include <kprogress.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define processid	0
#define stopprocess	1
#define samplepointer	2

extern MSignal *clipboard; 

//**********************************************************
void MSignal::doRangeOp (int num)
{
	switch (num)
	 {
		case PSTOP:
			stopplay();
		break;
		case PLAY:
			play();
		break;
		case LOOP:
			loop();
		break;
		case DELETE:
			deleteRange();
		break;
		case PASTE:
			pasteRange();
		break;
		case CROP:
			cropRange();
		break;
		case COPY:
			copyRange();
		break;
		case CUT:
			cutRange();
		break;
		case ZERO:
			zeroRange();
		break;
		case FLIP:
			flipRange();
		break;
		case REVERSE:
			reverseRange();
		break;
	 }
}
//**********************************************************
MSignal::MSignal (QWidget *par,int numsamples,int rate=48000) :QObject ()
{
 sample=new int[numsamples];

 if (sample)
  {
	parent=par;
	this->rate=rate;
	this->length=numsamples;
	memid=shmget(IPC_PRIVATE,sizeof(int)*4,IPC_CREAT+(6<<6)+(6<<3));
	msg= (int*) shmat (memid,0,0);
	msg[processid]=0;
	msg[stopprocess]=false;
	msg[samplepointer]=0;

	emit sampleChanged();
  }
 else KMsgBox::message (parent,"Info","Not enough Memory for Operation !",2);
}
//**********************************************************
MSignal::~MSignal ()	{	if (sample!=0) delete sample,sample=0;	}

int	MSignal::getRate	() {return rate;}
int* 	MSignal::getSample	() {return sample;}
int	MSignal::getLength	() {return length;} 
int	MSignal::getLMarker	() {return lmarker;} 
int	MSignal::getRMarker	() {return rmarker;} 
int	MSignal::getPlayPosition() {return msg[samplepointer];} 
//**********************************************************
void	MSignal::setMarkers (int l,int r )
{
	lmarker=l;
	rmarker=r;
}
//**********************************************************
void MSignal::copyRange ()
{
 MSignal *temp; 
 int	 *tsample;

 debug ("copy\n");

 if ((lmarker<length)&&(rmarker>0)&&(rmarker!=lmarker))
  {
	if (lmarker<0) lmarker=0;
	if (rmarker>length) rmarker=length;
	temp=new MSignal (parent,rmarker-lmarker,rate);
	if (temp)
	 {
		int j=0;
		tsample=temp->getSample();

		for (int i=lmarker;i<rmarker;tsample[j++]=sample[i++]);

		if (clipboard) delete clipboard;
		clipboard=temp;

		emit sampleChanged();
	 }
	else KMsgBox::message (parent,"Info","Not enough Memory for Operation !",2);
  }
}
//**********************************************************
void MSignal::pasteRange ()
{
 debug ("paste\n");
 if ((lmarker<length)&&(rmarker>0)&&(clipboard))
  {
	int *newsam;

	if (lmarker<0) lmarker=0;
	if (rmarker>length) rmarker=length;
	
	debug ("rl is%d %d\n",lmarker,rmarker);
	debug ("clip is %p, with %d samples\n",clipboard,clipboard->getLength());

	int 	*clipsam	=clipboard->getSample();
	int	cliplength	=clipboard->getLength();

	newsam=new int[length+cliplength-(rmarker-lmarker)];
	if (newsam)
	 {
		memcpy (newsam,sample,lmarker*sizeof(int));
		memcpy (&newsam[lmarker],clipsam,(cliplength)*sizeof(int));
		memcpy (&newsam[lmarker+cliplength],&sample[rmarker],(length-rmarker)*sizeof(int));
		delete sample;
		sample=newsam;
		length+=cliplength-(rmarker-lmarker);
		rmarker=lmarker+cliplength;
		emit sampleChanged();
	 }
	else KMsgBox::message (parent,"Info","Not enough Memory for Operation !",2);
  }
}
//**********************************************************
void MSignal::cutRange ()
{
 copyRange();
 if ((lmarker<length)&&(rmarker>0)&&(rmarker!=lmarker))
  {
	int *newsam;

	if (lmarker<0) lmarker=0;
	if (rmarker>length) rmarker=length;
	
	newsam=new int[length-(rmarker-lmarker)];
	if (newsam)
	 {
		memcpy (newsam,sample,lmarker*sizeof(int));
		memcpy (&newsam[lmarker],&sample[rmarker],(length-lmarker)*sizeof(int));
		delete sample;
		sample=newsam;
		length=length-(rmarker-lmarker);
		rmarker=lmarker;
		emit sampleChanged();
	 }
	else KMsgBox::message (parent,"Info","Not enough Memory for Operation !",2);
  }
}
//**********************************************************
void MSignal::deleteRange ()
{
 if ((lmarker<length)&&(rmarker>0)&&(rmarker!=lmarker))
  {
	int *newsam;

	if (lmarker<0) lmarker=0;
	if (rmarker>length) rmarker=length;
	
	newsam=new int[length-(rmarker-lmarker)];
	if (newsam)
	 {
		memcpy (newsam,sample,lmarker*sizeof(int));
		memcpy (&newsam[lmarker],&sample[rmarker],(length-rmarker)*sizeof(int));
		delete sample;
		sample=newsam;
		length=length-(rmarker-lmarker);
		rmarker=lmarker;
		emit sampleChanged();
	 }
	else KMsgBox::message (parent,"Info","Not enough Memory for Operation !",2);
  }
}
//**********************************************************
void MSignal::cropRange ()
{
 if ((lmarker<length)&&(rmarker>0)&&(rmarker!=lmarker))
  {
	int *newsam;

	if (lmarker<0) lmarker=0;
	if (rmarker>length) rmarker=length;
	
	newsam=new int[rmarker-lmarker];
	if (newsam)
	 {
		memcpy (newsam,&sample[lmarker],(rmarker-lmarker)*sizeof(int));
		delete sample;
		sample=newsam;
		length=rmarker-lmarker;
		emit sampleChanged();
	 }
	else KMsgBox::message (parent,"Info","Not enough Memory for Operation !",2);
  }
}
//**********************************************************
void MSignal::zeroRange ()
{
 if ((lmarker<length)&&(rmarker>0)&&(rmarker!=lmarker))
  {
	if (lmarker<0) lmarker=0;
	if (rmarker>length) rmarker=length;

	for (int i=lmarker;i<rmarker;sample[i++]=0);
	emit sampleChanged();
  }
}
//**********************************************************
void MSignal::flipRange ()
{
 if ((lmarker<length)&&(rmarker>0)&&(rmarker!=lmarker))
  {
	if (lmarker<0) lmarker=0;
	if (rmarker>length) rmarker=length;

	for (int i=lmarker;i<rmarker;sample[i]=-sample[i++]);
	emit sampleChanged();
  }
}
//**********************************************************
void MSignal::reverseRange ()
{
 if ((lmarker<length)&&(rmarker>0)&&(rmarker!=lmarker))
  {
	int i,x;
	if (lmarker<0) lmarker=0;
	if (rmarker>length) rmarker=length;

	for (i=0;i<(rmarker-lmarker)/2;i++)
	 {
		x=sample[lmarker+i];
		sample[lmarker+i]=sample[rmarker-i];
		sample[rmarker-i]=x;
	 }
	emit sampleChanged();
  }
}

