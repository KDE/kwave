#include <qdir.h>
#include "sonagram.h"
#include <qpainter.h>
#include <math.h>
#include <limits.h>
#include <qcursor.h>
#include <kmsgbox.h>
#include "dialogs.h"
#include "sample.h"
#include "main.h"

extern KApplication *app;
extern char*   mstotimec (int ms); 
ImageView::ImageView	(QWidget *parent) : QWidget (parent)
{
  this->parent=parent;
  image=0;
  x=0;
  y=0;
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
  if ((x<width())&&(x>=0))
    {
      int y=e->pos().y();

      if ((y>=0)&&(y<height()))
	emit info ((double)x/width(),(double)(height()-y)/height());
    }
}
//****************************************************************************
void ImageView::setImage (QImage *image)
{
  this->image=image;
  repaint ();
}
//****************************************************************************
void ImageView::paintEvent (QPaintEvent *)
{
    int height=rect().height();
    int width=rect().width();
  if (image)
    {
      QPixmap map;
      QWMatrix matrix;
      matrix.scale ((float)width/image->width(),(float)height/image->height());
      if (map.convertFromImage (*image,0))
      bitBlt (this,0,0,&(map.xForm (matrix)));
    }
}
//****************************************************************************
SonagramWindow::SonagramWindow () : KTopLevelWidget ()
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

  view=new ImageView (this);
  setView (view);

  connect (view,SIGNAL(info (double,double)),this,SLOT(setInfo(double,double)));

  setStatusBar (status);
  setMenu (bar);

  setCaption ("Sonagram :"); 
  resize (320,200);
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
void SonagramWindow::setSignal (double *input,int size, int points,int rate)
{
  double rea,ima;
  int half=points/2;

  this->length=size;
  this->x=(2*size/points)-1;
  this->points=points;
  this->rate=rate;

  data= new complex *[x];
  image=new QImage (x,half,8,256);
  if ((data)&&(image))
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
		      output[j].real=input[i*half+j]; //copy data into complex array
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
  int half=points/2;
  gsl_fft_complex_wavetable table;
      
  gsl_fft_complex_wavetable_alloc (points,&table);
  gsl_fft_complex_init (points,&table);

  TopWidget *win=new TopWidget (app);

  if (win)
    {
      MSignal *newsig=new MSignal (win,length,rate);
      win->show();

      int *output=newsig->getSample();     //sample data
      complex *tmp;                       //used for swapping
      complex *w1= new complex [points];  //this two windows hold the data for ifft and after that part of the signal
      complex *w2= new complex [points];

      if (output&&w1&&w2&&data)
	{
	  if (data[0]) memcpy (w1,data[0],sizeof(complex)*points);

	  gsl_fft_complex_inverse	(w1,points,&table);
	  //	  for (int j=0;j<half;j++) printf ("%e ",w1[j].real);
	  //for (int j=0;j<half;j++) printf ("%e\n",w1[j].imag);

	  //	  for (int j=0;j<half;j++) output[j]=(int)(w1[j].real*((1<<23)-1)); //do not fade first half of the window
	  for (int j=0;j<points;j++) output[j]=(int)(w1[j].real*((1<<23)-1)); //do not fade first half of the window

	  //	  for (int i=1;i<x;i++)
	  for (int i=2;i<x;i+=2)
	    {
	      if (data[i]) memcpy (w2,data[i],sizeof(complex)*points);
	      gsl_fft_complex_inverse	(w2,points,&table);

	      for (int j=0;j<points;j++) 
		output[i*half+j]=(int)(w2[j].real*((1<<23)-1)); //do not fade first half of the window
	      //	      for (int j=0;j<half;j++)
	      //	output[(i*half)+j]=(int)((((w1[j+half].real*(half-j)/half)
	      //			  +w2[j].real*j/half)*((1<<23)-1))); //crossfading the to window to avoid peaks

	      tmp=w1;
	      w1=w2;
	      w2=w1;
	    }
	  win->setSignal (newsig);
	}
      else 
	{
	  if (newsig) delete newsig;
	  if (win) delete win;
	  KMsgBox::message (this,"Info","Out of memory !",2);
	}
      //      if (w1) delete w1;
      //if (w2) delete w2;
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
	      rea=sqrt(rea*rea+ima*ima)/max;	        //get amplitude
	      *(image->scanLine((points/2-1)-j) + i)=255-(int)(rea*255);
	    }
    }
}
//****************************************************************************
SonagramWindow::~SonagramWindow (QWidget *parent,const char *name)
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

  sprintf (buf,"Time: %s",mstotimec ((int) (x*length*10000/rate)));
  status->changeItem (buf,1);
  sprintf (buf,"Frequency: %d Hz",(int)(y*rate/2));
  status->changeItem (buf,2);
  if (data [(int)(x*this->x)])
    {
      double rea= data [(int)(x*this->x)][(int)(y*points/2)].real;
      double ima= data [(int)(x*this->x)][(int)(y*points/2)].imag;

      sprintf (buf,"Amplitude: %d %%",(int)(sqrt(rea*rea+ima*ima)/max*100));
    }
  else sprintf (buf,"Memory Leak !");
  status->changeItem (buf,3);
  if (data [(int)(x*this->x)])
    {
      double rea= data [(int)(x*this->x)][(int)(y*points/2)].real;
      double ima= data [(int)(x*this->x)][(int)(y*points/2)].imag;
      sprintf (buf,"Phase: %d degree",(int) (atan(ima/rea)*360/M_PI));
    }
  else sprintf (buf,"Memory Leak !");

  status->changeItem (buf,4);
}




