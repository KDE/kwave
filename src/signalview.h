#ifndef _KWAVESIGNALVIEW_H_
#define _KWAVESIGNALVIEW_H_ 1

#include <qfile.h>
#include <qstring.h>
#include <qfiledlg.h>
#include <qwidget.h>
#include <qtimer.h>
#include "mousemark.h"
#include "../libgui/multistateimage.h"
#include "menumanager.h"
#include "../lib/markers.h"
#include <kapp.h>
#include <kselect.h>
#include <kstatusbar.h>  
#include <kbutton.h>

#define SELECTALL	17
#define SELECTVISIBLE	18
#define SELECTRANGE	19
#define SELECTNONE	20
#define SELECTNEXT	21
#define SELECTPREV	22
#define JUMPTOLABEL	23

#define ADDMARK 	 7000
#define DELETEMARK    	 7001  
#define EDITMARK         7002
#define LOADMARK         7003
#define SAVEMARK         7004
#define APPENDMARK       7005
#define MARKSIGNAL       7006
#define MARKPERIOD       7007
#define SAVEPERIODS	 7008
#define TOPITCH 	 7009
#define ADDMARKTYPE 	 7010
#define SELECTMARK	 7100 //leave #MAXMENU Items space behind !

class SignalManager;
//***********************************************************
class SignalWidget : public QWidget
//this class is mainly responsible for displaying signals in the time-domain
{
 Q_OBJECT
 public:
 	SignalWidget	(QWidget *parent,MenuManager *manage);
 // 	SignalWidget	(QWidget *parent,MenuManager *manage,const char *name=0);
 	~SignalWidget	();

 void 	setSignal		(QString *filename,int type=0);
 void 	saveSignal		(QString *filename,int,int=false);
 void 	saveBlocks		(int);
 void 	saveSelectedSignal	(QString *filename,int,int=false);
 void 	setSignal		(SignalManager *signal);
 void	setZoom			(double);
 unsigned char   *getOverview   (int);
 int    checkPosition	        (int);
 void 	drawSelection		(int,int);
 void   setMarkType             (int);
 void   addMarkType             ();
 void   addMarkType             (struct MarkerType *marker);
 int	doCommand	        (const char *);

 public slots:

 void 	refresh		();
 void	setOffset	(int);
 void	setOp	        (int);
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
 void addMarkerType     (struct MarkerType *);

 protected:


 void	setRange		(int,int);
 void	selectRange		();

 void	mousePressEvent		(QMouseEvent * );
 void	mouseReleaseEvent	(QMouseEvent * );  
 void	mouseMoveEvent		(QMouseEvent * );  
 void	paintEvent	        (QPaintEvent *);
 void	drawInterpolatedSignal	(int,int,int);
 void	drawOverviewSignal	(int,int,int);

 void	calcTimeInfo	();
 void   loadMarks       ();
 void   appendMarks     ();
 void   deleteMarks     ();
 void   convertMarkstoPitch ();
 void   saveMarks       ();
 void   addMark         ();
 void   jumptoLabel     ();
 void   markSignal      ();
 void   markPeriods     ();
 void   savePeriods     ();
 void   createSignal    ();
 void   connectSignal   ();

 private:
 int	offset;                 //offset from which signal is beeing displayed
 int	width,height;		//of this widget
 int	down;       		//flags if mouse is pressed
 double lasty; 
 double zoomy;
 double	zoom;			//number of samples represented by 1
				//vertical line on the screen
 int	playpointer,lastplaypointer;	 
 int	playing;		//flag if playing task is running...
 int	redraw;		        //flag for redrawing pixmap
 MouseMark      *select;
 SignalManager	*signalmanage;
 QTimer		*timer;
 QPainter       p;
 QPixmap	*pixmap;	//pixmap to be blitted to screen
 MarkerList     *markers;       //linked list of markers
 MarkerType     *markertype;    //selected marker type
 MenuManager    *manage;
};

#endif //_KWAVESIGNALVIEW_H_
