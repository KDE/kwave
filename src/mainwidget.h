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
#include "../libgui/menumanager.h"
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
 void	setSignal	(const char *filename,int type=0);
 void	setSignal	(SignalManager *);
 void 	saveSignal	(const char *filename,int,int=false);
 unsigned char *getOverView (int);

 protected:

 void setChannelInfo	(int);

 public slots:

 int  doCommand	        (const char *);
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
 void checkMenu 	(const char*, bool);

 signals:

 void command	        (const char *);
 void setOperation	(int);
 void channelInfo       (int);
 void checkMenuEntry	(const char*, bool);

 protected:
 void updateChannels    (int);
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



