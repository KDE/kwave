#include <qlist.h>
#include "mainwidget.h"
#include <drag.h>
#include "kwaveapp.h"
#include "../libgui/menumanager.h"

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

 public slots:
 void 	setOp             (const char *);
 void	dropEvent	  (KDNDDropZone *);
 void	checkMenuEntry	  (const char *name, bool check);

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
 KMenuBar	*bar;
 KStatusBar	*status;      //global status bar
 char           *name;        //filename
 MenuManager    *menumanage;  //menu manager object...
 int            bits;         //bit resolution to save with
};


















