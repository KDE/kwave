#include <qlist.h>
#include "classes.h"

class TopWidget : public KTopLevelWidget
{
 Q_OBJECT
 public:

 	TopWidget	(const char *name=0);
 void	setSignal	(QString name);
 void	setSignal	(MSignal *);

 protected:

 private:

 MainWidget	*mainwidget;
 KMenuBar	*bar;
 KStatusBar	*status;
 QString	name;

 public slots:

 void 	quitInstance();
 void	newInstance();
 void	openFile();
 void	revert();
 void	saveFile();
 void	saveFileas();
 void	saveSelection();
 void	about();
 void	cliptoNew();
 void	flushClip();

 void	newOp();
 void	deleteOp();
 void	cutOp	();
 void	copyOp	();
 void	zeroOp	();
 void	pasteOp	();
 void	cropOp	();
 void	flipOp	();
 void	centerOp	();
 void	selectRangeOp	();
 void	selectAllOp	();
 void	selectVisibleOp	();
 void	selectNoneOp	();
 void	reverseOp	();
 void	fadeInlOp	();
 void	fadeOutlOp	();
 void	amplifyMaxOp	();
 void	fadeInLogOp	();
 void	fadeOutLogOp	();
 void	noiseOp	();
 void	delayOp	();
 void	rateChangeOp	();
 void	fftOp	();
 void	playBackOp	();
};

