#include <qlist.h>
#include "classes.h"
#include <drag.h>

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
 void   saveConfig ();
 void   readConfig ();

 protected:

 private:

 KApplication   *app;
 MainWidget	*mainwidget;
 KMenuBar	*bar;
 KStatusBar	*status;
 QString	name;
 QPopupMenu     *recent;
 QPopupMenu     *save;
 QPopupMenu     *channels;
 QPopupMenu     *mtypemenu;
 int            numchannels;
 int            bit24,bit16,bit8;
 int            bit;

 public slots:

 void	dropEvent	(KDNDDropZone *);
 void	deleteChannel	(int);
 void	getChannelInfo	(int);

 void 	quitInstance();
 void	newInstance();
 void	openFile();
 void   openRecent (int num);
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
 void	mixpasteOp	();
 void	cropOp	();
 void	flipOp	();
 void	centerOp	();
 void	selectRangeOp	();
 void	selectAllOp	();
 void	selectVisibleOp	();
 void	selectNoneOp	();
 void	selectNextOp	();
 void	selectPrevOp	();
 void	reverseOp	();
 void	fadeInOp	();
 void	fadeOutOp	();
 void	amplifyOp	();
 void	amplifyMaxOp	();
 void	amplifyClipOp	();
 void	noiseOp	();
 void	hullCurveOp	();
 void	delayOp	();
 void	rateChangeOp	();
 void	fftOp	();
 void	playBackOp	();
 void	addSynthOp	();
 void	distortOp	();
 void	addChannelOp	();
 void	allChannelOp	();
 void	toggleChannelOp	();
 void   save24Bit	();
 void   save16Bit	();
 void   save8Bit	();
 void   mAverageFilterOp();
 void   sonagramOp();
 void   resampleOp();
 void   addMarkOp();
 void   loadMarkOp();
 void   appendMarkOp();
 void   saveMarkOp();
 void   deleteMarkOp();
 void   setMarkType(int);
 void   addMarkType();
 void   addMarkType (struct MarkerType *marker);
 void   scrollLeftOp();
 void   scrollRightOp();
 void   nextPageOp();
 void   prevPageOp();
 void   zoomInOp();
 void   zoomOutOp();
 void   zoomRangeOp();
 void   filterCreateOp();
 void   averageFFTOp();
 void   stutterOp ();
 void   requantizeOp();
 void   doFilter (int);
 void   signalMarkerOp  ();
 void   saveBlocksOp  ();
};







