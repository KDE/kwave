#ifndef _KMAINWDIGET_H_
#define _KMAINWIDGET_H_ 1

#include <qlayout.h>
#include <qpushbt.h>
#include <qfont.h>
#include <qfile.h>
#include <qstring.h>
#include <qfiledlg.h>
#include <qwidget.h>
#include <qcombo.h>
#include <qpixmap.h>
#include <qtimer.h>
#include <qframe.h>
#include "../libgui/multistateimage.h"
#include "../libgui/overview.h"
#include "signalview.h"
#include "menumanager.h"
#include <kapp.h>
#include <kselect.h>
#include <ktopwidget.h>
#include <kmenubar.h>
#include <kbuttonbox.h>
#include <kstatusbar.h>  
#include <kbutton.h>
//***********************************************************
class MainWidget : public QWidget
//mainwidget is parent for all widgets in the main window
{
 Q_OBJECT
 public:

 	MainWidget	(QWidget *parent,MenuManager *manage,KStatusBar *status=0);
 	~MainWidget	();
 void	setSignal	(QString *filename,int type=0);
 void	setSignal	(SignalManager *);
 void 	saveSignal	(QString *filename,int,int=false);
 int	doCommand	(const char *);
 unsigned char *getOverView (int);

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
 void halt		();
 void loop		();

 signals:

 void command	        (const char *);
 void setOperation	(int);
 void channelInfo       (int);

 protected:

 void resizeEvent	(QResizeEvent *);

 private:

 KButtonBox	  *buttons;
 OverViewWidget	  *slider;
 SignalWidget 	  *signalview;
 QPushButton	  *plusbutton,*minusbutton;
 QPushButton	  *zoombutton,*nozoombutton;
 QPushButton	  *playbutton,*loopbutton;
 QComboBox	  *zoomselect;
 KStatusBar	  *status;
 MultiStateWidget **lamps;
 MultiStateWidget **speakers;
 MenuManager      *manage;
 QWidget          *parent;
 int 		  numsignals;
 int              bsize;
 bool             menushown;
};
#endif



