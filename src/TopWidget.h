#ifndef _TOP_WIDGET_H_
#define _TOP_WIDGET_H_ 1

#include <ktmainwindow.h>

class MenuManager;
class MainWidget;
class SignalManager;
class KDNDDropZone;
class KStatusBar;

class TopWidget : public KTMainWindow
{
 Q_OBJECT
 public:

 	TopWidget	  ();
	~TopWidget	  ();
 void   closeWindow       ();
 void	setSignal	  (const char *name);
 void	setSignal	  (SignalManager *);
 void   updateRecentFiles ();
 void   parseCommands     (const char *);
 void   loadBatch         (const char *);
 void   updateMenu        ();

 public slots:
 void 	setOp             (const char *);
 void	dropEvent	  (KDNDDropZone *);

 protected:

 void	revert();
 void	openFile();
 void   importAsciiFile();
 void   openRecent (const char *str);
 void	saveFile();
 void	saveFileAs(bool selection=false);
 void	resolution (const char *str);

 private:

 QDir           *saveDir;
 QDir           *loadDir;
 MainWidget	*mainwidget;
 KStatusBar	*status;      //global status bar
 char           *name;        //filename
 MenuManager    *menumanage;  //menu manager object...
 int            bits;         //bit resolution to save with
};

#endif // _TOP_WIDGET_H_
