#include <qlist.h>
#include "mainwidget.h"
#include <drag.h>
#include "menumanager.h"

class TopWidget : public KTopLevelWidget
{
 Q_OBJECT
 public:

 	TopWidget	(KApplication   *app);
	~TopWidget	();
 void   closeWindow     ();
 void	setSignal	(QString name);
 void	setSignal	(MSignal *);
 void   addRecentFile   (char *);
 void   updateRecentFiles ();

 public slots:

 void	dropEvent	(KDNDDropZone *);
 void 	setOp           (int);

 protected:

 void	newInstance();
 void 	getHelp();
 void	about();
 void	revert();
 void 	quitInstance();
 void	openFile();
 void   importAsciiFile();
 void   openRecent (int num);
 void	saveFile();
 void	saveFileAs();
 void	saveSelection();

 private:

 KApplication   *app;
 MainWidget	*mainwidget;
 KMenuBar	*bar;
 KStatusBar	*status;      //global status bar
 QString	name;         //filename
 MenuManager    *manage;      //menu manager object...
 int            bit;          //bit resolution to save with
};







