#include <qfiledlg.h>
#include <qkeycode.h>
#include "filter.h"
#include <math.h>

extern QDir *filterDir;
extern QStrList *filterNameList;

//**********************************************************
Filter::Filter ()
{
  num=0;
  offset=0;
  mult=0;
}
//**********************************************************
Filter::~Filter ()
{
  if (offset) delete offset;
  if (mult) delete mult;
}
//**********************************************************
int  Filter::resize (int newnum)
{
  if (newnum!=num) //no need to resize arrays ....
    {
      int    *boffset=new int[newnum];
      double *bmult=new double[newnum];

      if (boffset&&bmult)
	{
	  for (int i=0;i<newnum;i++)  //initialize new arrays
	    {
	      bmult[i]=0;
	      boffset[i]=i;
	    }
	  bmult[0]=1;

	  int less=num;
	  if (newnum<num) less=newnum;

	  if (offset) //copy old values....
	    {
	      for (int i=0;i<less;i++) boffset[i]=offset[i];
	      delete offset;
	    }
	  if (mult)
	    {
	      for (int i=0;i<less;i++) bmult[i]=mult[i];
	      delete mult;
	    }

	  offset=boffset;
	  mult=bmult;
	  num=newnum;
	  return num;
	}
      else return false;
    }
  return newnum;
}
//**********************************************************
void Filter::save (QString *name)
{
  char buf[80];
  printf ("%s\n",name->data());
  if (name->find (".filter")==-1) name->append (".filter");
  QFile out(name->data());
  out.open (IO_WriteOnly);

  if (fir) sprintf (buf,"FIR %d\n",num);
  else sprintf (buf,"IIR %d\n",num);
  out.writeBlock (&buf[0],strlen(buf));

  for (int i=0;i<num;i++)
    {
      sprintf (buf,"%d %e\n",offset[i],((double)mult[i]));
      out.writeBlock (&buf[0],strlen(buf));
    }                                         
}
//**********************************************************
void Filter::load (QString *name)
{
  char buf[120];

  QFile *in=new QFile(name->data());
  if ((in)&&(in->open (IO_ReadOnly)))  
    {
      int res;
      res=in->readLine(buf,120);
      while ((res>0)&&((strncmp (buf,"FIR",3)!=0)&&(strncmp(buf,"IIR",3)!=0)))
	res=in->readLine(buf,120);

      if (res>0)
	{
	  if (strncmp (buf,"FIR",3)==0)
	      fir=true;
	  else
	      fir=false;

	  int newnum=strtol (&buf[4],0,0);
	  int x;
	  int cnt=0;
	  float y;

	  if (resize (newnum))
	    {

	      while ((in->readLine(buf,120)>0)&&(cnt<newnum))
		{
		  if ((buf[0]!='/')||(buf[0]!='#'))
		    {
		      sscanf (buf,"%d %f\n",&x,&y);
		      offset[cnt]=x;
		      mult[cnt++]=(int) (y);
		    }
		}
	    }
	}
    }
}
//**********************************************************
FilterDialog::FilterDialog (QWidget *par,int rate): QDialog(par,0,true)
{
  setCaption	(klocale->translate("Choose filter parameters:"));

  filter.rate=rate;

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
//**********************************************************
void FilterDialog::setMult (int newvalue)
{
  char buf [16];

  for (int i=0;i<filter.num;i++)
    {
      if (mult[i]->value()==newvalue)
	{
	  filter.mult[i]=((double)(-1000+newvalue)*-1)/1000;
	  sprintf (buf,"%3.1f",filter.mult[i]*100);
	  label[i]->setText (buf);
	}
    }
  refresh ();
}
//**********************************************************
void FilterDialog::setOffset (const char *newvalue)
{
  for (int i=0;i<filter.num;i++)
    {
      if (strcmp (offset[i]->text(),newvalue)==0)
	filter.offset[i]=(int)(QString(newvalue)).toLong();
    }
  refresh ();
}
//**********************************************************
void FilterDialog::refresh ()
{
  int max=0;
  for (int i=0;i<filter.num;i++) if (max<filter.offset[i]) max=filter.offset[i]; //get maximum filter offset

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

      filter.fir=fir->isChecked();
      
	if (filter.fir)
	  {
	    for (int i=0;i<filter.num;i++)
	      if (filter.mult[i])
		data[filter.offset[i]].real=filter.mult[i];
	    //generate impulse response
	  }
	else
	  {
	    data[0].real=filter.mult[0];

	    for (int j=1;j<points;j++)
	      for (int i=1;i<filter.num;i++)
		 if (j-filter.offset[i]>=0) data[j].real+=filter.mult[i]*data[j-filter.offset[i]].real;
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

	phasewidget->setPhase (data,maxp,points,filter.rate);
	filterwidget->setSignal (data,maxp,points,filter.rate);
	filterwidget->refresh();
	phasewidget->refresh();
      }
  gsl_fft_complex_wavetable_free (&table);
}
//**********************************************************
Filter *FilterDialog::getFilter ()
{
  return &filter;
}
//**********************************************************
void FilterDialog::loadFilter ()
{
  QString name=QFileDialog::getOpenFileName (filterDir->path(),"*.filter",this);
  if (!name.isNull())
    {
      filter.load (&name);

      getNTaps (filter.num);
      repaint (true);
      refresh ();
    }
}
//**********************************************************
void FilterDialog::saveFilter ()
{
  QString name=QFileDialog::getSaveFileName (filterDir->path(),"*.filter",this);
  if (!name.isNull())
      filter.save (&name);
}
//**********************************************************
void FilterDialog::getNTaps (int newnum)
{
  QSlider **newmult=new QSlider* [newnum];
  KIntegerLine **newoffset=new KIntegerLine*[newnum];
  QLabel **newlabel=new QLabel*[newnum];

  if ((newmult)&&(newoffset)&&(newlabel)&&filter.resize(newnum))
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
      for (int i=0;i<filter.num;i++)
	{
	  label[i]->setGeometry   (i*w/filter.num,top,w/filter.num,bsize);
	  mult[i]->setGeometry    (i*w/filter.num,top+bsize,w/filter.num,he-2*bsize);
	  offset[i]->setGeometry  (i*w/filter.num,top+he-bsize,w/filter.num,bsize);
	}
      
      refreshView();

      int width=bsize*(filter.num+4);
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
  for (int i=0;i<filter.num;i++)
    {
      mult[i]->setValue (1000-(int)(1000*filter.mult[i]));
      ms=filter.offset[i];
      sprintf (buf,"%d",ms);
      offset[i]->setText (buf); 
    }
  if (filter.fir)
    {
      fir->setChecked (true);
      iir->setChecked (false);
    }
  else
    {
      iir->setChecked (true);
      fir->setChecked (false);
    }
  sprintf (buf,"%d",filter.num);
  taps->setText (buf);
}
//**********************************************************
void FilterDialog::setTaps (const char *n)
{
  int x=1;
  if (n) x=strtol(n,0,0);
  if (x<2) x=2;           //since you need 2... there are other functions for volume...
  if (x>50) x=50;         //feel free to change, if you have a big screen ...
  if (x!=filter.num) getNTaps (x);
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
 bg->setGeometry    (tapl+lsize/2,offs,w-tapl-lsize,top-offs);  
 filterwidget->setGeometry	(0,0,tapl,top/2);
 phasewidget->setGeometry	(0,top/2+lsize/4,tapl,top/2);
 top+=lsize/2;

 for (int i=0;i<filter.num;i++)
   {
      label[i]->setGeometry   (i*w/filter.num,top,w/filter.num,bsize);
      mult[i]->setGeometry    (i*w/filter.num,top+bsize,w/filter.num,he-2*bsize);
      offset[i]->setGeometry  (i*w/filter.num,top+he-bsize,w/filter.num,bsize);
   }

 ok->setGeometry	(w/10,h-bsize-lsize/2,w*3/10,bsize);  
 cancel->setGeometry	(w*6/10,h-bsize-lsize/2,w*3/10,bsize);  
}
//**********************************************************
FilterDialog::~FilterDialog ()
{
}
//**********************************************************
MovingFilterDialog::MovingFilterDialog (QWidget *par=NULL,int num): QDialog(par, 0,true)
{
  this->num=num;
  setCaption	(klocale->translate("Choose Filter Movement :"));

  ok	 = new QPushButton (klocale->translate("Filter"),this);
  cancel = new QPushButton (klocale->translate("Cancel"),this);

  lowlabel= new QLabel (klocale->translate("Range goes from"),this);
  highlabel= new QLabel (klocale->translate("to"),this);

  tap=new KIntegerLine (this);
  tap->setText ("1");
  usecurve=new QCheckBox (klocale->translate("do filtering with changing coefficient"),this);

  low=new KRestrictedLine (this);
  low->setValidChars ("-0123456789.");
  low->setText ("-100 %");
  high=new KRestrictedLine (this);
  high->setValidChars ("-0123456789.");
  high->setText ("100 %");

  curve= new CurveWidget (this);
  curve->setBackgroundColor	(QColor(black) );

  high->setEnabled (false);
  low->setEnabled (false);
  tap->setEnabled (false);
  curve->setEnabled (false);

  int bsize=ok->sizeHint().height();

  setMinimumSize (320,bsize*9);
  resize	 (320,bsize*9);

  ok->setAccel	(Key_Return);
  cancel->setAccel(Key_Escape);
  ok->setFocus	();
  connect 	(ok	,SIGNAL(clicked()),SLOT (accept()));
  connect 	(cancel	,SIGNAL(clicked()),SLOT (reject()));
  connect 	(usecurve,SIGNAL(clicked()),SLOT (toggleState()));
}
//**********************************************************
void MovingFilterDialog::toggleState ()
{
    if  (usecurve->isChecked())
      {
	  high->setEnabled (true);
	  low->setEnabled (true);
	  tap->setEnabled (true);
	  curve->setEnabled (true);
      }
      else
	{
	  high->setEnabled (false);
	  low->setEnabled (false);
	  tap->setEnabled (false);
	  curve->setEnabled (false);
	}
}
//**********************************************************
void MovingFilterDialog::checkTap (const char *text)
{
  char buf[16];
  int i=strtol (text,0,0);
  if (i<0) i=0;
  if (i>num) i=num;

  sprintf (buf,"%d",num);
  tap->setText (buf);
}
//**********************************************************
int MovingFilterDialog::getTap ()
{
  const char *buf=tap->text();
  return strtol (buf,0,0);
}
//**********************************************************
int MovingFilterDialog::getLow ()
{
  const char *buf=low->text();
  double x=strtod (buf,0);
  return (int) (x*10);
}
//**********************************************************
int MovingFilterDialog::getHigh ()
{
  const char *buf=high->text();
  double x=strtod (buf,0);
  return (int) (x*10);
}
//**********************************************************
int MovingFilterDialog::getState ()
{
  return usecurve->isChecked();
}
//**********************************************************
QList<CPoint> *MovingFilterDialog::getPoints ()
{
  return curve->getPoints();
}
//**********************************************************
int MovingFilterDialog::getType ()
{
  return curve->getType();
}
//**********************************************************
void MovingFilterDialog::resizeEvent (QResizeEvent *)
{
 int bsize=ok->sizeHint().height();
 int offset=0;

 curve->setGeometry	(width()/20,0,width()*18/20,height()-bsize*5);  

 usecurve->setGeometry	(width()/20,height()-bsize*9/2,usecurve->sizeHint().width(),bsize);
 offset+=usecurve->sizeHint().width()+bsize/4;
 tap->setGeometry	(width()/20+offset,height()-bsize*9/2,width()*18/20-offset,bsize);  

 offset=0;
 lowlabel->setGeometry	(width()/20,height()-bsize*3,lowlabel->sizeHint().width(),bsize);  
 offset+=lowlabel->sizeHint().width()+bsize/4;
 low->setGeometry	(width()/20+offset,height()-bsize*3,bsize*2,bsize);  
 offset+=bsize*9/4;
 highlabel->setGeometry	(width()/20+offset,height()-bsize*3,highlabel->sizeHint().width(),bsize);  
 offset+=highlabel->sizeHint().width()+bsize/4;
 high->setGeometry	(width()/20+offset,height()-bsize*3,bsize*2,bsize);  
 offset+=highlabel->sizeHint().width();
 ok->setGeometry	(width()/10,height()-bsize*3/2,width()*3/10,bsize);  
 cancel->setGeometry	(width()*6/10,height()-bsize*3/2,width()*3/10,bsize);  
}
//**********************************************************
MovingFilterDialog::~MovingFilterDialog ()
{
  delete curve ;
}
//**********************************************************












