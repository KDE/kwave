#ifndef _SIGNAL_WIDGET_H_
#define _SIGNAL_WIDGET_H_ 1

#include <qfile.h>
#include <qfiledlg.h>
#include <qwidget.h>
#include <qtimer.h>
#include "MouseMark.h"
#include "../libgui/MultiStateWidget.h"
#include "../libgui/MenuManager.h"
#include <libkwave/Label.h>
#include <libkwave/LabelList.h>
#include <kapp.h>
#include <kselect.h>
#include <kstatusbar.h>  
#include <kbutton.h>

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

 int    mstosamples             (double);
 void 	setSignal		(const char *filename,int type=0);
 void 	saveSignal		(const char *filename,int bits,bool selection=false);
 void 	saveBlocks		(int);
 void 	saveSelectedSignal	(const char *filename,int bits,bool selection=true);
 void 	setSignal		(SignalManager *signal);
 void	setZoom			(double);
 unsigned char   *getOverview   (int);
 int    checkPosition	        (int);
 void 	drawSelection		(int,int);

 void   addLabelType            (LabelType *);
 void   addLabelType            (const char *);
 int	doCommand	        (const char *);
 int    getSignalCount          ();
 int    getBitsPerSample        ();

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

 void   signalinserted  (int,int);
 void   signaldeleted   (int,int);
 void	estimateRange   (int,int);
 signals:

 void channelReset	();
 void playingfinished	();
 void viewInfo		(int,int,int);
 void selectedTimeInfo	(int);
 void timeInfo          (int);
 void rateInfo	        (int);
 void lengthInfo	(int);
 void checkMenu 	(const char*, bool);

 protected:
 void	setRange                (int,int,bool=true);
 void	selectRange		();
 void	updateChannels	        ();

 void	mousePressEvent		(QMouseEvent *);
 void	mouseReleaseEvent	(QMouseEvent *);  
 void	mouseMoveEvent		(QMouseEvent *);  
 void	paintEvent	        (QPaintEvent *);

    /**
     * Draws the signal as an overview with multiple samples per
     * pixel.
     * @param channel the index of the channel [0..channels-1]
     * @param middle the y position of the zero line in the drawing
     *               area [pixels]
     * @param height the height of the drawing are [pixels]
     * @param first the index of the first sample
     * @param last the index of the last sample
     */
    void drawOverviewSignal(int channel, int middle, int height,
	int first, int last);
 	
 void	drawInterpolatedSignal	(int,int,int);

 void	calcTimeInfo	();
 void   loadLabel       ();
 void   appendLabel     ();
 void   deleteLabel     ();
 void   saveLabel       (const char *);
 void   addLabel        (const char *);
 void   jumptoLabel     ();
 void   markSignal      (const char *);
 void   markPeriods     (const char *);
 void   savePeriods     ();
 void   createSignal    (const char *);
 void   connectSignal   ();
 void   showDialog      (const char *);

 bool   checkForLabelCommand      (const char *);
 bool   checkForNavigationCommand (const char *);

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
 MouseMark     *select;
 SignalManager *signalmanage;
 QTimer	       *timer;
 QPainter       p;
 QPixmap       *pixmap;	//pixmap to be blitted to screen
 LabelList     *labels;        //linked list of markers
 LabelType     *markertype;    //selected marker type
 MenuManager   *manage;
};

#endif // _SIGNAL_WIDGET_H_
