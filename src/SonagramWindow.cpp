#include <qdir.h>
#include <qpainter.h>
#include <math.h>
#include <limits.h>
#include <qcursor.h>

#include "dialog_progress.h"
#include "main.h"
#include "sonagram.h"
#include "signalmanager.h"

#include <libkwave/kwavesignal.h>
#include <libkwave/windowfunction.h>
#include <kmsgbox.h>

extern KApplication *app;
extern char *mstotimec (int ms); 
//****************************************************************************
ImageView::ImageView	(QWidget *parent) : QWidget (parent)
{
  image=0;
  lh=-1;
  lw=-1;
  offset=0;
  setCursor (crossCursor);
}
//****************************************************************************
ImageView::~ImageView ()
{
}
//****************************************************************************
void ImageView::mouseMoveEvent	(QMouseEvent *e)
{
  int x=e->pos().x();
  if ((x<width)&&(x>=0))
    {
      int y=e->pos().y();

      if ((y>=0)&&(y<height))
	emit info ((double)x/width,(double)(height-y)/height);
    }
}
//****************************************************************************
int  ImageView::getOffset () {return offset;}
int  ImageView::getWidth () {return width;}
void ImageView::setImage (QImage *image)
{
  this->image=image;
  repaint ();
}
//****************************************************************************
void ImageView::setOffset (int offset)
{
  if (this->offset!=offset)
    {
      this->offset=offset;
      repaint ();
    }
}
//****************************************************************************
void ImageView::paintEvent (QPaintEvent *)
{
  height=rect().height();
  width=rect().width();

  if (image)
    {
      if ((lh!=height)||((lw!=width)&&(image->width()<width)))
	{
	  if (offset>image->width()-width) offset=image->width()-width;
	  QWMatrix matrix;
	  QPixmap newmap;
	  newmap.convertFromImage (*image,0);

	  if (image->width()<width)
	    {
	      offset=0; 
	      matrix.scale (((float)width)/image->width(),((float)height)/image->height());
	    }
	  else
	    matrix.scale (1,((float)height)/image->height());

	  map=(newmap.xForm (matrix));
	  lh=height;
	  lw=width;
	}
      emit viewInfo	(offset,width,image->width());
      bitBlt (this,0,0,&map,offset,0,width,height);
    }
}
//****************************************************************************
SonagramContainer::SonagramContainer (QWidget *parent): QWidget (parent)
{
  this->view=0;
}
//****************************************************************************
void SonagramContainer::setObjects (ImageView *view,ScaleWidget *x,ScaleWidget *y,CornerPatchWidget *corner,OverViewWidget *overview)
{
  this->view=view;
  this->xscale=x;
  this->yscale=y;
  this->corner=corner;
  this->overview=overview;
}
//****************************************************************************
SonagramContainer::~SonagramContainer ()
{
}
//****************************************************************************
void SonagramContainer::resizeEvent	(QResizeEvent *)
{
  if (view)
    {
      int bsize=(QPushButton("test",this).sizeHint()).height();
      view->setGeometry	(bsize,0,width()-bsize,height()-bsize*2);  
      xscale->setGeometry	(bsize,height()-bsize*2,width()-bsize,bsize);  
      overview->setGeometry	(bsize,height()-bsize,width()-bsize,bsize);  
      yscale->setGeometry	(0,0,bsize,height()-bsize*2);
      corner->setGeometry	(0,height()-bsize*2,bsize,bsize);  
    }
}
//****************************************************************************
SonagramWindow::SonagramWindow (QString *name) : KTopLevelWidget ()
{
  KMenuBar *bar=    new KMenuBar (this); 
  QPopupMenu *spectral= new QPopupMenu ();
  QPopupMenu *file= new QPopupMenu ();

  bar->insertItem	(klocale->translate("&File"),file);
  bar->insertItem	(klocale->translate("&Spectral Data"),spectral);

  file->insertItem	(klocale->translate("&Import from Bitmap ..."),this,SLOT(load()));
  file->insertItem	(klocale->translate("&Export to Bitmap ..."),this,SLOT(save()));
  spectral->insertItem	(klocale->translate("&reTransform to signal"),this,SLOT(toSignal()));

  status=new KStatusBar (this,"Frequencies Status Bar");
  status->insertItem ("Time:          0 ms     ",1);
  status->insertItem ("Frequency:          0 Hz     ",2);
  status->insertItem ("Amplitude:    0 %      ",3);
  status->insertItem ("Phase:    0        ",4);  

  mainwidget=new SonagramContainer (this);

  view=new ImageView (mainwidget);
  connect (view,SIGNAL(info (double,double)),this,SLOT(setInfo(double,double)));
  xscale=new ScaleWidget (mainwidget,0,100,"ms");
  yscale=new ScaleWidget (mainwidget,100,0,"Hz");
  corner=new CornerPatchWidget (mainwidget);
  overview=new OverViewWidget (mainwidget);

  QObject::connect      (overview,SIGNAL(valueChanged(int)),
			 view,SLOT(setOffset(int)));
  QObject::connect	(view,SIGNAL(viewInfo(int,int,int)),
			 this,SLOT(setRange(int,int,int)));
  QObject::connect	(view,SIGNAL(viewInfo(int,int,int)),
			 overview,SLOT(setRange(int,int,int)));

  mainwidget->setObjects (view,xscale,yscale,corner,overview);
  setView (mainwidget);

  setStatusBar (status);
  setMenu (bar);
  QString *windowname=new QString (QString ("Sonagram of ")+QString(name->data()));
  setCaption (windowname->data());

  resize (480,300);
}
//****************************************************************************
void SonagramWindow::save ()
{
  if (image)
    {
      QString filename = QFileDialog::getSaveFileName("","*.bmp",this);
      if ( !filename.isEmpty() )  image->save( filename, "BMP" ); 
    }
}
//****************************************************************************
void SonagramWindow::load ()
{
  if (image)
    {
      QString filename = QFileDialog::getOpenFileName("","*.bmp",this);
      printf ("loading %s\n",filename.data());
      if (!filename.isNull())
	{
	  printf ("loading %s\n",filename.data());
	  QImage *newimage=new QImage (filename);

	  if (newimage)
	    {
	      if ((image->height()==newimage->height())
		&&(image->width()==newimage->width()))
		{

		  for (int i=0;i<x;i++)
		    {
		      for (int j=0;j<points/2;j++)
			{
			  if (data[i])
			    {
			      //			      data[i][j].real;
			    }
			}
		    }

		  delete image;
		  image=newimage;
		  view->setImage (image);
		}
	      else
		{
		  char buf[64];
		  delete newimage;
		  sprintf (buf,"Bitmap must be %dx%d",image->width(),image->height());
		  KMsgBox::message (this,"Info",buf,2);		  	       
		}
	    }
	  else
	    KMsgBox::message (this,"Error","Could not open Bitmap",2);
	}
    }
}
//****************************************************************************
void SonagramWindow::setSignal (double *input,int size, int points,int windowtype,int rate)
{
  double rea,ima;
  //  printf ("size %d points %d windowtype %d\n",size,points,windowtype);
  this->length=size;
  this->x=(size/points);
  this->points=points;
  this->rate=rate;

  yscale ->setMaxMin (0,rate/2);
  xscale ->setMaxMin ((int)(((double)size)/rate*1000),0);

  data= new complex *[x];

  WindowFunction func (windowtype);
  image=new QImage (x,points/2,8,256);
  double* windowfunction=func.getFunction(points);

  if ((data)&&(image)&&windowfunction)
    {
      char buf [48];
      sprintf (buf,"doing %d %d-point mixed radix fft\'s\n",x,points);
      ProgressDialog *dialog=new ProgressDialog (x,buf);

      if (dialog)
	{
	  dialog->show();

	  gsl_fft_complex_wavetable table;
      
	  gsl_fft_complex_wavetable_alloc (points,&table);
	  gsl_fft_complex_init (points,&table);

     	  for (int i=0;i<x;i++)
	    {
	      complex *output=new complex[points];
	      if (output)
		{
		  for (int j=0;j<points;j++)
		    {
		      output[j].real=windowfunction[j]*input[i*points+j]; //copy data into complex array
		      output[j].imag=0;
		    }

		  gsl_fft_complex_forward	(output,points,&table);

		  for (int k=0;k<points;k++)
		    {
		      rea=output[k].real;
		      ima=output[k].imag;
		      rea=sqrt(rea*rea+ima*ima);	        //get amplitude
		      if (max<rea) max=rea;                     //and set maximum for display..
		    }
		  dialog->setProgress (i);
		}
	      data[i]=output;                          //put single spectrum into array of spectra
	    }
	  gsl_fft_complex_wavetable_free	(&table);
	}
      delete dialog;

      createPalette ();
      createImage ();
      view->setImage (image);
    }
  else 
    {
      KMsgBox::message (this,"Info","Out of memory !",2);
      if (data) delete data;
      if (image) delete image;
    }
}
//****************************************************************************
void  SonagramWindow::toSignal ()
{
  gsl_fft_complex_wavetable table;
      
  gsl_fft_complex_wavetable_alloc (points,&table);
  gsl_fft_complex_init (points,&table);

  TopWidget *win=new TopWidget ();

  if (win)
    {
      KwaveSignal *newsig=new KwaveSignal (length,rate);
      //assure 10 Hz for correction signal, this should not be audible
      int slopesize=rate/10;

      double *slope=new double [slopesize];
      
      if (slope&&newsig)
	{
	  for(int i=0;i<slopesize;i++)
	    slope[i]=0.5+0.5*cos( ((double) i) * M_PI / slopesize);

	  win->show();

	  int *output=newsig->getSample();     //sample data
	  complex *tmp= new complex [points];  //this window holds the data for ifft and after that part of the signal

	  if (output&&tmp&&data)
	    {
	      for (int i=0;i<x;i++)
		{
		  if (data[i]) memcpy (tmp,data[i],sizeof(complex)*points);
		  gsl_fft_complex_inverse	(tmp,points,&table);

		  for (int j=0;j<points;j++) 
		    output[i*points+j]=(int)(tmp[j].real*((1<<23)-1));
		}
	      int dif;
	      int max;
	      for (int i=1;i<x;i++) //remove gaps between windows
		{
		  max=slopesize;
		  if (max>length-i*points) max=length-i*points;
		  dif=output[i*points]-output[i*points-1];
		  if (dif<2)
		  for (int j=0;j<max;j++) output[i*points+j]+=(int) (slope[j]*dif);		}

	      win->setSignal (new SignalManager (newsig));

	      if (tmp) delete tmp;
	    }
	  else
	    {
	    if (newsig) delete newsig;
	    if (win) delete win;
	    KMsgBox::message (this,"Info","Out of memory !",2);
	    }
	}
      if (slope) delete slope;
    }  
}
//****************************************************************************
void SonagramWindow::createPalette ()
{
  for (int i=0;i<256;i++) image->setColor(i, qRgb(i,i,i) );   //create grayscale palette
}
//****************************************************************************
void SonagramWindow::createImage ()
{
  double rea,ima;
  if (image&&data)
    {
      for (int i=0;i<x;i++)
	if (data[i])
	  for (int j=0;j<points/2;j++)
	    {
	      rea=data[i][j].real;
	      ima=data[i][j].imag;
	      rea=sqrt(rea*rea+ima*ima)/max;	        //get amplitude and scale to 1
	      rea=1-((1-rea)*(1-rea));
	      *(image->scanLine((points/2-1)-j) + i)=255-(int)(rea*255);
	    }
    }
}
//****************************************************************************
SonagramWindow::~SonagramWindow ()
{
  if (data)
    {
      for (int i=0;i<x;i++) if (data[i]) delete data[i];
      delete data;
    }
  if (image) delete image;
}
//****************************************************************************
void SonagramWindow::setInfo  (double x,double y)
{
  char buf[64];
  int col;

  if (view->getWidth()>this->x)
    col=(int)(x*(this->x-1));
  else
    col=(int)(view->getOffset()+x*view->getWidth());

  sprintf (buf,"Time: %s",mstotimec ((int)(((double) col)*points*10000/rate)));
  status->changeItem (buf,1);
  sprintf (buf,"Frequency: %d Hz",(int)(y*rate/2));
  status->changeItem (buf,2);
  if (data [(int)(x*this->x)])
    {
      double rea= data [col][(int)(y*points/2)].real;
      double ima= data [col][(int)(y*points/2)].imag;

      sprintf (buf,"Amplitude: %d %%",(int)(sqrt(rea*rea+ima*ima)/max*100));
    }
  else sprintf (buf,"Memory Leak !");
  status->changeItem (buf,3);
  if (data [(int)(x*this->x)])
    {
      double rea= data [col][(int)(y*points/2)].real;
      double ima= data [col][(int)(y*points/2)].imag;
      sprintf (buf,"Phase: %d degree",(int) (atan(ima/rea)*360/M_PI));
    }
  else sprintf (buf,"Memory Leak !");

  status->changeItem (buf,4);
}
//****************************************************************************
void SonagramWindow::setRange (int offset,int width,int)
{
  xscale->setMaxMin ((int)(((double)offset+width)*points*1000/rate),
		     (int)(((double)offset)*points*1000/rate)); 
}




