#include <stdio.h>
#include <stdlib.h>
#include <qpushbutton.h>
#include <qkeycode.h>
#include <qfiledialog.h>
#include <qlayout.h>
#include <qbttngrp.h>
#include <qtooltip.h>
#include "module.h"
#include <kapp.h>

const char *version="1.0";
const char *author="Martin Wilz";
const char *name="filter";
//**********************************************************
KwaveDialog *getDialog (DialogOperation *operation)
{
  return new FilterDialog(operation->isModal(),operation->getRate());
}
//**********************************************************
FilterDialog::FilterDialog (bool modal,int rate): KwaveDialog (modal)
{
  comstr=0;
  setCaption	(klocale->translate("Choose filter parameters:"));

  filter=new Filter (rate);

  if (filter)
    {
      ok	 = new QPushButton (klocale->translate("&Filter"),this);
      cancel = new QPushButton (klocale->translate("Cancel"),this);
      filterwidget = new FFTWidget (this);
      phasewidget  = new FFTWidget (this);

      load	 = new QPushButton (klocale->translate("&load filter"),this);
      save	 = new QPushButton (klocale->translate("&save filter"),this);

      QToolTip::add( filterwidget,klocale->translate("resulting changes in spectrum"));
      QToolTip::add( phasewidget ,klocale->translate("resulting changes in phase"));

      phasewidget->setAutoDelete (false);
      int bsize=ok->sizeHint().height();
      QVBoxLayout *vbox;
      bg = new QButtonGroup( this);
      bg->setTitle(klocale->translate("Filter Type"));  
      vbox = new QVBoxLayout(bg, 10);
      vbox->addSpacing( bg->fontMetrics().height() );
      fir = new QRadioButton( bg );
      fir->setText( "&FIR" );
      QToolTip::add( fir, klocale->translate("Use normal filtering, e.g. the impulse response is finite !"));
      vbox->addWidget(fir);
      fir->setMinimumSize(bsize*3,bsize);

      iir = new QRadioButton( bg );
      iir->setText(klocale->translate ("&IIR"));
      vbox->addWidget(iir);
      iir->setMinimumSize(bsize*3,bsize);
      QToolTip::add( iir, klocale->translate("Use recursive filtering, e.g. the impulse response could be infinite !"));
      iir->setMinimumSize( iir->sizeHint());
      fir->setChecked (true);

      taps=new KIntegerLine (this);
      taps->setText ("10");
      taplabel=new QLabel (klocale->translate("# of :"),this);
      QToolTip::add( taps ,klocale->translate("Number of filter coefficients\n Keep this low and you won't need any coffee break..."));

      label=0;
      mult=0;
      offset=0;

      getNTaps (10);

      ampx=new ScaleWidget (this,0,rate/2,"Hz");
      ampy=new ScaleWidget (this,100,0,"%");
      ampcorner=new CornerPatchWidget (this);

      phasex=new ScaleWidget (this,0,rate/2,"Hz");
      phasey=new ScaleWidget (this,180,-180,"°");
      phasecorner=new CornerPatchWidget (this);

      cancel->setAccel(Key_Escape);
      ok->setFocus	();
      connect 	(ok	,SIGNAL(clicked()),SLOT (accept()));
      connect 	(cancel	,SIGNAL(clicked()),SLOT (reject()));
      connect 	(load	,SIGNAL(clicked()),SLOT (loadFilter()));
      connect 	(fir	,SIGNAL(clicked()),SLOT (refresh()));
      connect 	(iir	,SIGNAL(clicked()),SLOT (refresh()));
      connect 	(save	,SIGNAL(clicked()),SLOT (saveFilter()));
      connect 	(taps   ,SIGNAL(textChanged(const char *)),SLOT (setTaps(const char *)));
      refresh ();
    }
}
//**********************************************************
void FilterDialog::setMult (int newvalue)
{
  char buf [16];

  for (int i=0;i<filter->num;i++)
    {
      if (mult[i]->value()==newvalue)
	{
	  filter->mult[i]=((double)(-1000+newvalue)*-1)/1000;
	  sprintf (buf,"%3.1f",filter->mult[i]*100);
	  label[i]->setText (buf);
	}
    }
  refresh ();
}
//**********************************************************
void FilterDialog::setOffset (const char *newvalue)
{
  for (int i=0;i<filter->num;i++)
    {
      if (strcmp (offset[i]->text(),newvalue)==0)
	filter->offset[i]=(int)(QString(newvalue)).toLong();
    }
  refresh ();
}
//**********************************************************
void FilterDialog::refresh ()
{
  int max=0;
  for (int i=0;i<filter->num;i++) if (max<filter->offset[i]) max=filter->offset[i]; //get maximum filter offset

  int points=1;
  while (points<max+1) points<<=1; //get number of points for fft...

  if (points<1024) points=1024;

  //  printf ("%d points\n",points);

  gsl_fft_complex_wavetable table;
  gsl_fft_complex_wavetable_alloc (points,&table);
  gsl_fft_complex_init (points,&table);

  complex *data=new complex[points];

    if (data) 
      {
	for (int i=0;i<points;i++)
	  {
	    data [i].real=0;
	    data [i].imag=0;
	  }

      filter->fir=fir->isChecked();
      
	if (filter->fir)
	  {
	    for (int i=0;i<filter->num;i++)
	      if (filter->mult[i])
		data[filter->offset[i]].real=filter->mult[i];
	    //generate impulse response
	  }
	else
	  {
	    data[0].real=filter->mult[0];

	    for (int j=1;j<points;j++)
	      for (int i=1;i<filter->num;i++)
		 if (j-filter->offset[i]>=0) data[j].real+=filter->mult[i]*data[j-filter->offset[i]].real;
	  }

	gsl_fft_complex_forward	(data,points,&table);   //do fft of the impulse response...

	double rea;
	double ima;
	double maxp=0;

	for (int i=0;i<points;i++)
	  {
	    rea=data[i].real;
	    ima=data[i].imag;
	    rea=sqrt(rea*rea+ima*ima);
	    if (rea>maxp) maxp=rea;
	  }

	phasewidget->setPhase (data,points,filter->rate);
	filterwidget->setSignal (data,points,filter->rate);
	filterwidget->refresh();
	phasewidget->refresh();
      }
  gsl_fft_complex_wavetable_free (&table);
}
//**********************************************************
const char *FilterDialog::getCommand ()
{
  if (comstr) free (comstr);

  comstr=catString ("filter (",
		    filter->getCommand (),
		    ")");

  return comstr;
}
//**********************************************************
void FilterDialog::loadFilter ()
{
  QString name=QFileDialog::getOpenFileName (filterDir->path(),"*.filter",this);
  if (!name.isNull())
    {
      filter->load (name.data());

      getNTaps (filter->num);
      repaint (true);
      refresh ();
    }
}
//**********************************************************
void FilterDialog::saveFilter ()
{
  QString name=QFileDialog::getSaveFileName (filterDir->path(),"*.filter",this);
  if (!name.isNull())
      filter->save (name.data());
}

//**********************************************************
void FilterDialog::getNTaps (int newnum)
{
  QSlider **newmult=new QSlider* [newnum];
  KIntegerLine **newoffset=new KIntegerLine*[newnum];
  QLabel **newlabel=new QLabel*[newnum];

  if ((newmult)&&(newoffset)&&(newlabel)&&filter->resize(newnum))
    {
      if (mult)
	{
	  for (int i=0;i<oldnum;i++) delete mult[i];
	  delete mult;
	}
      if (offset)
	{
	  for (int i=0;i<oldnum;i++) delete offset[i];
	  delete offset;
	}
      if (label)
	{
	  for (int i=0;i<oldnum;i++) delete label[i];
	  delete label;
	}

      mult=newmult;
      label=newlabel;
      offset=newoffset;

      for (int i=0;i<newnum;i++)
	{
	  label[i]=new QLabel ("0.00",this);
	  offset[i]=new KIntegerLine (this);
	  mult[i]=new QSlider (0,2000,1,0,QSlider::Vertical,this);
	  label[i]->show();
	  mult[i]->show();
	  offset[i]->show();

	  oldnum=newnum;

	  connect (offset[i],SIGNAL(textChanged(const char *)),this,SLOT(setOffset(const char *)));
	  connect (mult[i],SIGNAL  (valueChanged(int)),        this,SLOT(setMult(int)));
	}
      offset[0]->setEnabled (false);

      int bsize=ok->sizeHint().height();
      int top=h/2;
      int he=h-top-bsize*2;
      for (int i=0;i<filter->num;i++)
	{
	  label[i]->setGeometry   (i*w/filter->num,top,w/filter->num,bsize);
	  mult[i]->setGeometry    (i*w/filter->num,top+bsize,w/filter->num,he-2*bsize);
	  offset[i]->setGeometry  (i*w/filter->num,top+he-bsize,w/filter->num,bsize);
	}
      
      refreshView();

      int width=bsize*(filter->num+4);
      if (width<400) width=400;

      setMinimumSize (width,bsize*20);
      resize	     (width,bsize*20);
    }
  else printf ("error allocating objects, get some memory...\n");
}
//**********************************************************
void FilterDialog::refreshView ()
{
  char buf[16];
  int  ms;
  for (int i=0;i<filter->num;i++)
    {
      mult[i]->setValue (1000-(int)(1000*filter->mult[i]));
      ms=filter->offset[i];
      sprintf (buf,"%d",ms);
      offset[i]->setText (buf); 
    }
  if (filter->fir)
    {
      fir->setChecked (true);
      iir->setChecked (false);
    }
  else
    {
      iir->setChecked (true);
      fir->setChecked (false);
    }
  sprintf (buf,"%d",filter->num);
  taps->setText (buf);
}
//**********************************************************
void FilterDialog::setTaps (const char *n)
{
  int x=1;
  if (n) x=strtol(n,0,0);
  if (x<2) x=2;           //since you need 2... there are other functions for volume...
  if (x>50) x=50;         //feel free to change, if you have a big screen ...
  if (x!=filter->num) getNTaps (x);
  repaint ();
  refresh ();
}
//**********************************************************
void FilterDialog::resizeEvent (QResizeEvent *)
{
 int bsize=ok->sizeHint().height();
 int lsize=bsize;
 w=this->width();
 h=this->height();
 int top=h/2;
 int he=h-top-bsize*2;
 int ty=bsize*2;
 int tapl=w-2*ty-2*lsize;

 int offs=lsize/2+bsize;

 taplabel->setGeometry (tapl+lsize/2,lsize/4,ty,lsize);
 taps->setGeometry     (tapl+lsize+ty,lsize/4,ty,bsize);

 load->setGeometry   (tapl+lsize/2,offs,w-tapl-lsize,bsize);
 offs+=bsize+lsize/4;
 save->setGeometry   (tapl+lsize/2,offs,w-tapl-lsize,bsize);
 offs+=bsize+lsize/4;
 bg->setGeometry     (tapl+lsize/2,offs,w-tapl-lsize,top-offs);  
 filterwidget->setGeometry (bsize,0,tapl-bsize,top/2-bsize);
 phasewidget->setGeometry  (bsize,top/2+lsize/4,tapl-bsize,top/2-bsize);
 phasey->setGeometry       (0,0,top/2-bsize,bsize);
 ampcorner->setGeometry    (0,top/2-bsize,bsize,bsize);
 phasecorner->setGeometry  (0,top-bsize,bsize,bsize);
 ampy->setGeometry         (0,0,bsize,top/2-bsize);
 phasey->setGeometry       (0,top/2,bsize,top/2-bsize);
 ampx->setGeometry         (bsize,top/2-bsize,tapl-bsize,bsize);
 phasex->setGeometry       (bsize,top-bsize,tapl-bsize,bsize);

 top+=lsize/2;

 for (int i=0;i<filter->num;i++)
   {
      label[i]->setGeometry   (i*w/filter->num,top,w/filter->num,bsize);
      mult[i]->setGeometry    (i*w/filter->num,top+bsize,w/filter->num,he-2*bsize);
      offset[i]->setGeometry  (i*w/filter->num,top+he-bsize,w/filter->num,bsize);
   }

 ok->setGeometry	(w/10,h-bsize-lsize/2,w*3/10,bsize);  
 cancel->setGeometry	(w*6/10,h-bsize-lsize/2,w*3/10,bsize);  
}
//**********************************************************
FilterDialog::~FilterDialog ()
{
  if (filter) delete filter;
  if (comstr) free (comstr);
}










