#include <qobject.h>
// #include <kapp.h>
// #include <qlist.h>
// #include "MainWidget.h"
// #include <drag.h>
// #include "KwaveApp.h"
// #include "../libgui/MenuManager.h"

#include <ktmainwindow.h>

// class KTMainWindow;
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


















