#ifndef _KMAIN_WDIGET_H_
#define _KMAIN_WIDGET_H_ 1

#include <qwidget.h>

class MenuManager;
class KStatusBar;
class MultiStateWidget;
class SignalManager;
class KButtonBox;
class OverViewWidget;
class SignalWidget;
class QPushButton;
class QComboBox;

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
 void 	saveSignal	(const char *filename,int bits, int type, bool selection);
 unsigned char *getOverView (int);
 int    getBitsPerSample();
 bool   hasSignal       () { return (numsignals != 0); };

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

 signals:

 void command	        (const char *);
 void setOperation	(int);
 void channelInfo       (int);

 protected:
 void updateMenu        ();
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
#endif // _KMAIN_WDIGET_H_
