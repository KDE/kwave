#ifndef _KWAVECLASSES_H_
#define _KWAVECLASSES_H_ 1

#include <qapp.h>
#include <qlayout.h>
#include <qpushbt.h>
#include <qfont.h>
#include <qfile.h>
#include <qscrbar.h>
#include <qstring.h>
#include <qfiledlg.h>
#include <qwidget.h>
#include <qcombo.h>
#include <qpixmap.h>
#include <qtimer.h>
#include <qframe.h>
#include <kapp.h>
#include <kslider.h>
#include <kselect.h>
#include <ktopwidget.h>
#include <kmenubar.h>
#include <kbuttonbox.h>
#include <kstatusbar.h>  
#include <kbutton.h>
#include "sample.h"
#include "multistateimage.h"

class MainWidget;
//***********************************************************
class MarkerType
{
 public:
  MarkerType ();
  ~MarkerType();

  int    named;         //boolean if named
  int    selected;
  QString*name;         //pointer to name
  QColor *color;        //color
};
//***********************************************************
class Marker
{
 public:
  Marker();
  ~Marker();

  int     pos;                         //position
  QString *name;                       //pointer to name
  struct  MarkerType *type;            //pointer to type
};
//***********************************************************
class MarkerList:public QListT<Marker>
{
  //  Q_OBJECT
 public:
  MarkerList::MarkerList();
  MarkerList::~MarkerList();
  int MarkerList::compareItems (GCI,GCI);
};
//***********************************************************
class OverViewWidget : public QWidget
{
 Q_OBJECT
 public:
 	OverViewWidget	(QWidget *parent=0,const char *name=0);
 	OverViewWidget	(MainWidget *parent=0,const char *name=0);
 	~OverViewWidget	();
 void	mousePressEvent		(QMouseEvent * );
 void	mouseReleaseEvent	(QMouseEvent * );  
 void	mouseMoveEvent		(QMouseEvent * );  
 void 	setSignal		(char *);
 void   setValue                (int);
 void   refresh                 ();

 public slots:

 void 	setRange		(int,int,int);
 void increase ();

 signals:

 void valueChanged (int);

 protected:

 void	paintEvent(QPaintEvent *);

 private:

 int        width,height;
 int        grabbed;
 int        max;
 int        len;
 int        act;
 int        dir;        //addup for direction...
 int	    redraw;	//flag for redrawing pixmap
 MainWidget *mparent;
 QWidget    *parent;
 QTimer     *timer;     //to spare user repeated pressing of the widget...
 QPixmap    *pixmap;	//pixmap to be blitted to screen
};
//***********************************************************
class SigWidget : public QWidget
{
 Q_OBJECT
 public:
 	SigWidget	(QWidget *parent=0,const char *name=0);
 	~SigWidget	();


 void 	setSignal		(QString *filename);
 void 	saveSignal		(QString *filename,int,int=false);
 void 	saveBlocks		(int);
 void 	saveSelectedSignal	(QString *filename,int,int=false);
 void 	setSignal		(MSignal *signal);
 void	setZoom			(double);
 void	setRange		(int,int);
 unsigned char   *getOverview   (int);

 public slots:

 void 	refresh		();
 void	setOffset	(int);
 void	setRangeOp	(int);
 void	toggleChannel	(int);
 void	time		();
 void	zoomRange	();
 void	zoomIn		();
 void	zoomOut		();
 void	zoomNormal	();

 void   signalinserted (int,int);
 void   signaldeleted  (int,int);

 signals:

 void channelReset	();
 void playingfinished	();
 void viewInfo		(int,int,int);
 void lengthInfo	(int);
 void rateInfo		(int);
 void timeInfo		(int);
 void selectedtimeInfo	(int);
 void channelInfo       (int);
 void addMarkerType     (struct MarkerType *);

 protected:

 void	mousePressEvent		(QMouseEvent * );
 void	mouseReleaseEvent	(QMouseEvent * );  
 void	mouseMoveEvent		(QMouseEvent * );  
 void	paintEvent	(QPaintEvent *);
 void	deleteLastRange	();
 void	drawRange	();
 void	drawSignal	(int *,int,int);
 void	drawInterpolatedSignal	(int *,int,int);
 void	drawOverviewSignal	(int *,int,int);
 void	calcTimeInfo	();
 void   loadMarks       ();
 void   appendMarks     ();
 void   deleteMarks     ();
 void   saveMarks       ();
 void   addMark         ();
 void   markSignal      ();
 void   markPeriods     ();
 void   savePeriods     ();

 private:
 int	offset;
 int	width,height;		//of widget
 int	down,reset;		//flags for drawing
 double zoomy;
 double	zoom;			//number of samples represented by 1
				//vertical line on the screen
 int	firstx,lastx,nextx;	//markers for mouse mark operation
 int    lasty;			//marker for mouse zoom...      
 int	playpointer,lastplaypointer;	 
 int	playing;		//flag if playing task is running...
 int	redraw;		        //flag for redrawing pixmap
 MSignal	*signal;
 QTimer		*timer;
 QPainter 	p;
 QPixmap	*pixmap;	//pixmap to be blitted to screen
 MarkerList     *markers;       //linked list of markers
 MarkerType     *markertype;    //selected marker type
};
//***********************************************************
class MainWidget : public QWidget
{
 Q_OBJECT
 public:

 	MainWidget	(QWidget *parent=0,const char *name=0,KStatusBar *status=0);
 	~MainWidget	();
 void	setSignal	(QString *filename);
 void	setSignal	(MSignal *);
 void 	saveSignal	(QString *filename,int,int=false);
 void	setRangeOp	(int);
 unsigned char *getOverView (int);
 MSignal *getSignalView	();

 public slots:

 void getChannelInfo	(int);
 void resetChannels	();
 void setRateInfo	(int);
 void setLengthInfo	(int);
 void setTimeInfo	(int);
 void parseKey	        (int);
 void setSelectedTimeInfo(int);
 void selectedZoom	(int);
 void play		();
 void stop		();
 void loop		();

 signals:

 void setOperation	(int);
 void channelInfo       (int);

 protected:

 void resizeEvent	(QResizeEvent *);

 private:

 KButtonBox	*buttons;
 OverViewWidget	*slider;
 SigWidget 	*signalview;
 QPushButton	*plusbutton,*minusbutton;
 QPushButton	*zoombutton,*nozoombutton;
 QPushButton	*playbutton,*loopbutton;
 QComboBox	*zoomselect;
 KStatusBar	*status;
 MultiStateWidget **lamps;
 MultiStateWidget **speakers;
 QWidget        *parent;
 int 		numsignals;
 int            bsize;
};
#endif //_KWAVECLASSES_H_
