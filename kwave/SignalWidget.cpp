#include "classes.h"
#include <qpainter.h>

void getminmax (int *sample,int len,int &max,int &min)
{
	int c;
	min=0;
	max=0;
	for (int i=0;i<len;i++)
	 {
		c=sample[i];
		if (c>max) max=c;
		if (c<min) min=c;
	 }
}
//****************************************************************************
SigWidget::SigWidget (QWidget *parent,const char *name) : QWidget (parent,name)
{
	timer=0;
	fit=true;
	signal=0;
	offset=0;
	zoom=1;
	down=false;
	reset=false;
	lastx=-1;
	firstx=-1;
	playpointer=-1;
	lastplaypointer=-1;
}
//****************************************************************************
SigWidget::~SigWidget (QWidget *parent,const char *name)
{
	if (signal!=0)	delete signal;
}
//****************************************************************************
void SigWidget::setRangeOp (int op)
{
	if (signal!=0)
	 {
		signal->doRangeOp (op);
		if ((op==PLAY)||(op==LOOP))
		 {
			if (timer==0)
			 {
				timer=new QTimer (this);
				connect (timer,SIGNAL(timeout()),this,SLOT(time()));
			 }
			timer->start (20);
		 }
		if (op==PSTOP)
		 {
			timer->stop ();
			playpointer=-1;
			if (lastplaypointer >=0) repaint (false);
		 }
	 }
}
//****************************************************************************
void SigWidget::setSignal  (MSignal *sig)
{
	signal=sig;
	offset=0;
	connect (signal,SIGNAL(sampleChanged()),this,SLOT(refresh()));
	emit	rateInfo	(signal->getRate());
	emit	lengthInfo	(signal->getLength());

	double length=signal->getLength()/signal->getRate()*1000;
	emit	timeInfo	((int) length);
}
//****************************************************************************
void SigWidget::setSignal  (QString *filename)
{
	signal=new MSignal (this,filename);
	offset=0;
	connect (signal,SIGNAL(sampleChanged()),this,SLOT(refresh()));
	emit	rateInfo	(signal->getRate());
	emit	lengthInfo	(signal->getLength());

	double length=signal->getLength()/signal->getRate()*1000;
	emit	timeInfo	((int) length);

}
//****************************************************************************
void SigWidget::setFit (int fit)
{
	this->fit=fit;
	repaint ();
}
//****************************************************************************
void SigWidget::time ()
{
	printf ("signal is %d\n",signal->getPlayPosition());
	int scr=signal->getPlayPosition()-offset;

	if (scr<width)
	 {
		playpointer=scr;
		repaint (false);
	 }
	else
	 if (lastplaypointer >=0) repaint (false);

	if (signal->getPlayPosition()==0)
		emit playingfinished();
}
//****************************************************************************
void SigWidget::setZoom (double zoom)
{
	printf ("newzoom is %E",zoom);
	this->fit=false;
	this->zoom=zoom;
	repaint ();
}
//****************************************************************************
void SigWidget::refresh	()
{
	repaint ();
	emit	rateInfo	(signal->getRate());
	emit	lengthInfo	(signal->getLength());
	emit	timeInfo	((signal->getLength()*1000)/signal->getRate());
};
//****************************************************************************
void SigWidget::setOffset	(int no)
 {
   if (signal!=0)
    {
	offset=no;
	repaint ();
    }
 }
//****************************************************************************
void SigWidget::mousePressEvent( QMouseEvent *e)
{
 down	= TRUE;
 reset	= TRUE;
 repaint(false);
 reset	= false;
 firstx =e->pos().x();
 lastx	=-1;
 nextx	=-1;
}
//****************************************************************************
void SigWidget::mouseReleaseEvent( QMouseEvent *e)
{
 if (down)
  {
	lastx=nextx;
	nextx=e->pos().x();             // add point
	if (nextx<0) nextx=0;
	if (nextx>width) nextx=width;

	repaint(false);
	if (nextx==firstx) lastx=firstx;
	
	down = FALSE;			// done recording points
	if (signal)
	 {
		if (firstx>lastx)
		 {
			signal->setMarkers (lastx+offset,firstx+offset);
			emit selectedtimeInfo((firstx-lastx)*10000/signal->getRate());
		 }
		else
		 {
			signal->setMarkers (firstx+offset,lastx+offset);
			emit selectedtimeInfo((lastx-firstx)*10000/signal->getRate());
		 }
	 }
  }
}
//****************************************************************************
void SigWidget::mouseMoveEvent( QMouseEvent *e )
{
    if (down) {
	lastx=nextx;
	nextx=e->pos().x();             // add point
	if (nextx<0) nextx=0;
	if (nextx>width) nextx=width;

	repaint (false);

	if (signal)
	 {
		if (firstx>lastx)
			emit selectedtimeInfo((firstx-lastx)*10000/signal->getRate());
		else
			emit selectedtimeInfo((lastx-firstx)*10000/signal->getRate());
	 }
    }
}
//****************************************************************************
void SigWidget::deleteLastRange  ()
{
	if (lastx>=0)
	if (lastx==firstx)	//is it a single line ?
	 {
		p.setPen (green);
		p.drawLine (firstx,-height/2,firstx,height);
		p.setPen (yellow);
	 }
	else
	 p.drawRect (firstx,-height/2,lastx-firstx,height);
}
//****************************************************************************
void SigWidget::drawRange  ()
{

	if (firstx>=0)
	if (nextx==firstx)
	 {
		p.setPen (green);
		p.drawLine (firstx,-height/2,firstx,height);
	 }
	else
	 {
		if (nextx>=0)
		p.drawRect (firstx,-height/2,nextx-firstx,height);
	 }
}
//****************************************************************************
void SigWidget::drawSignal (MSignal *signal)
{
	int*	sam=signal->getSample();

	if (sam!=0)
	 {
		int lx,x;
		lx=(sam[offset]/256)*height/65536;
		for (int i=1;((i<signal->getLength())&&(i<width));i++)
		 {
			x=(sam[offset+i]/256)*height/65536;
			p.drawLine (i-1,lx,i,x);
			lx=x;
		  }
	 }
}
//****************************************************************************
void SigWidget::paintEvent  (QPaintEvent *)
{
 height=rect().height();
 width=rect().width();

 p.begin (this);
 p.setPen (QPen(NoPen));
 p.setPen (gray);
 p.translate (0,height/2);

 if (playpointer>=0)
  {
	p.setRasterOp (XorROP);
	p.setPen (green);

	if (lastplaypointer >=0)
		p.drawLine (lastplaypointer,-height/2,lastplaypointer,height);

	p.drawLine (playpointer,-height/2,playpointer,height);
	lastplaypointer=playpointer;
	playpointer=-1;
	p.end();
  }
 else
 if (lastplaypointer >=0)
  {
	p.setRasterOp (XorROP);
	p.setPen (green);
	p.drawLine (lastplaypointer,-height/2,lastplaypointer,height);
	lastplaypointer=-1;
	p.end();
  }
 else 
 if (!down)
  {
	p.drawLine (0,0,width,0);
	p.setPen (white);
	if (signal!=0)
	  {
		int 	l=signal->getLMarker();
		int 	r=signal->getRMarker();

		drawSignal (signal);

		firstx=l-offset;
		lastx=r-offset;

		if ((firstx<width)&&(lastx>0))
		 {

			if (firstx<0) firstx=0;
			if (lastx>width) lastx=width;

			p.setBrush (yellow);
			p.setRasterOp (XorROP);

			if (r==l)
			 {
				p.setPen (green);
				p.drawLine (firstx,-height/2,firstx,height);
				p.setPen (yellow);
			 }
			else
			 {
				p.setPen (yellow);
				p.drawRect (firstx,-height/2,lastx-firstx,height);
			 }

		 }
		p.end();	//because next line also uses qpainters
		emit viewInfo	(offset,width,(int)(signal->getLength()/zoom)-width);
	  }
	 else p.end();
  }
 else
  {
	p.setBrush (yellow);
	p.setPen (yellow);
	p.setRasterOp (XorROP);

	if (reset)			//initial redraw after click
	 {				
		deleteLastRange ();
		lastx=-1;
	 }
	else 
	 {
		if (lastx>=0) deleteLastRange ();
		drawRange ();
	 }
	 p.end();
  }
}
