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
#include <kapp.h>
#include <kslider.h>
#include <kselect.h>
#include <ktopwidget.h>
#include <kmenubar.h>
#include <kbuttonbox.h>
#include <kstatusbar.h>  
#include <qtimer.h>
#include "sample.h"

class SigWidget : public QWidget
{
 Q_OBJECT
 public:
 	SigWidget	(QWidget *parent=0,const char *name=0);
 	~SigWidget	();
 void	mousePressEvent		(QMouseEvent * );
 void	mouseReleaseEvent	(QMouseEvent * );  
 void	mouseMoveEvent		(QMouseEvent * );  
 void 	setSignal		(QString *filename);
 void 	saveSignal		(QString *filename);
 void 	saveSelectedSignal	(QString *filename);
 void 	setSignal		(MSignal *signal);
 void	setFit			(int);
 void	setZoom			(double);

 public slots:

 void 	refresh		();
 void	setOffset	(int);
 void	setRangeOp	(int);
 void	time		();
 signals:

 void playingfinished	();
 void viewInfo		(int,int,int);
 void lengthInfo	(int);
 void rateInfo		(int);
 void timeInfo		(int);
 void selectedtimeInfo	(int);

 protected:

 void	paintEvent	(QPaintEvent *);
 void	deleteLastRange	();
 void	drawRange	();
 void	drawSignal	(MSignal *);

 private:
 int	offset;
 int	width,height;		//of widget
 int	down,reset;		//flags for drawing
 int	fit;			//flag for fitting sample in window
 double	zoom;			//number of samples representen by 1
				//Vline on the screen
 int	firstx,lastx,nextx;	//markers for mouse drag operation
 int	playpointer,lastplaypointer;	 
 MSignal	*signal;
 QTimer		*timer;
 QPainter 	p;
};
//***********************************************************
class MainWidget : public QWidget
{
 Q_OBJECT
 public:

 	MainWidget	(QWidget *parent=0,const char *name=0,KStatusBar *status=0);
 void	setSignal	(QString *filename);
 void	setSignal	(MSignal *);
 void 	saveSignal		(QString *filename);
 void 	saveSelectedSignal	(QString *filename);
 void	setRangeOp	(int);
 MSignal *getSignalView	();

 public slots:

 void getSliderInfo	(int offset,int min,int max);
 void setRateInfo	(int);
 void setLengthInfo	(int);
 void setTimeInfo	( int);
 void setSelectedTimeInfo	( int);
 void selectedZoom	(int);
 void play		();
 void stop		();
 void loop		();

 signals:

 void setOperation	(int);

 protected:

 void resizeEvent	(QResizeEvent *);

 private:

 KButtonBox	*buttons;
 QScrollBar	*slider;
 SigWidget 	*signalview;
 QPushButton	*plusbutton,*minusbutton;
 QPushButton	*playbutton,*loopbutton;
 QComboBox	*zoomselect;
 KStatusBar	*status;
 int 		numsignals;
 QList<SigWidget>	*signalviews;
};
