#include "main.h"
#include "about.h"
#include "qkeycode.h"

MSignal		*clipboard=0; 
QList<TopWidget>*topwidget=0;
//*****************************************************************************************
TopWidget::TopWidget (const char *name) : KTopLevelWidget (name)
{
 QPopupMenu *file=	new QPopupMenu ();
 QPopupMenu *edit=	new QPopupMenu ();
 QPopupMenu *clip=	new QPopupMenu ();
 QPopupMenu *effects=	new QPopupMenu ();
 QPopupMenu *calculate=	new QPopupMenu ();
 QPopupMenu *help=	new QPopupMenu ();
 QPopupMenu *select=	new QPopupMenu ();
 QPopupMenu *fadein=	new QPopupMenu ();
 QPopupMenu *fadeout=	new QPopupMenu ();
 QPopupMenu *amplify=	new QPopupMenu ();
 QPopupMenu *options=	new QPopupMenu ();

 status=new KStatusBar (this,"Main Status Bar");
 status->insertItem ("Length: 0 ms          ",1);
 status->insertItem ("Rate: 0 kHz       ",2);
 status->insertItem ("Samples: 0           ",3);
 status->insertItem ("selected: 0 ms     ",4);
 status->insertItem ("Clipboard: 0 ms      ",5);

 bar=		new KMenuBar (this,"MainMenu");
 mainwidget=	new MainWidget (this,"MainView",status);

 file->insertItem	(klocale->translate("&New..."),	this,SLOT(newOp()),CTRL+Key_N);
 file->insertItem	(klocale->translate("New &Window"),this,SLOT(newInstance()),CTRL+Key_W);
 file->insertSeparator	();
 file->insertItem	(klocale->translate("&Open"),	this,SLOT(openFile()),CTRL+Key_O);
 file->insertItem	(klocale->translate("&Save"),	this,SLOT(saveFile()),CTRL+Key_S);
 file->insertItem	(klocale->translate("Save &As"),this,SLOT(saveFileas()),CTRL+Key_A);
 file->insertItem	(klocale->translate("Save Se&lection"),this,SLOT(saveSelection()),CTRL+Key_L);
 file->insertSeparator	();
 file->insertItem	(klocale->translate("Revert"),	this,SLOT(revert()),CTRL+Key_Q);
 file->insertSeparator	();
 file->insertItem	(klocale->translate("&Quit"),	this,SLOT(quitInstance()),CTRL+Key_Q);

 edit->insertItem	(klocale->translate("Cut"),	this,SLOT(cutOp()),CTRL+Key_X);
 edit->insertItem	(klocale->translate("Copy"),	this,SLOT(copyOp()),CTRL+Key_C);
 edit->insertItem	(klocale->translate("Paste"),	this,SLOT(pasteOp()),CTRL+Key_V);
 edit->insertItem	(klocale->translate("Crop"),	this,SLOT(cropOp()));
 edit->insertItem	(klocale->translate("Delete"),	this,SLOT(deleteOp()),Key_Delete);
 edit->insertItem	(klocale->translate("Flip"),	this,SLOT(flipOp()));
 edit->insertItem	(klocale->translate("Center"),	this,SLOT(centerOp()));
 edit->insertSeparator	();
 edit->insertItem	(klocale->translate("Select"),	select);
 select->insertItem	(klocale->translate("All"),	this,SLOT(selectAllOp()));
 select->insertItem	(klocale->translate("Range"),	this,SLOT(selectRangeOp()));
 select->insertItem	(klocale->translate("Visible Area"),	this,SLOT(selectVisibleOp()));
 select->insertItem	(klocale->translate("None"),	this,SLOT(selectNoneOp()));
 edit->insertSeparator	();
 clip->insertItem	(klocale->translate("Flush"),	this,SLOT(flushClip()));
 clip->insertItem	(klocale->translate("to new Window"),this,SLOT(cliptoNew()));
 edit->insertItem	(klocale->translate("Clipboard"),clip);

 effects->insertItem	(klocale->translate("Reverse"),	this,SLOT(reverseOp()));
 effects->insertItem	(klocale->translate("Fade In"),	fadein);
 effects->insertItem	(klocale->translate("Fade Out"),fadeout);
 effects->insertItem	(klocale->translate("Amplify"),	amplify);

 fadein->insertItem	(klocale->translate("Linear"),	this,SLOT(fadeInlOp()));
 fadein->insertItem	(klocale->translate("Logarithmic"),	this,SLOT(fadeInLogOp()));
 fadeout->insertItem	(klocale->translate("Linear"),	this,SLOT(fadeOutlOp()));
 fadeout->insertItem	(klocale->translate("Logarithmic"),	this,SLOT(fadeOutLogOp()));
 amplify->insertItem	(klocale->translate("to	Maximum"),this,SLOT(amplifyMaxOp()));
 effects->insertSeparator	();
 effects->insertItem	(klocale->translate("Delay"),	this,SLOT(delayOp()));
 effects->insertSeparator	();
 effects->insertItem	(klocale->translate("Change Rate"),this,SLOT(rateChangeOp()));
 
 calculate->insertItem	(klocale->translate("Silence"),	this,SLOT(zeroOp()));
 calculate->insertItem	(klocale->translate("Noise"),	this,SLOT(noiseOp()));
 calculate->insertSeparator	();
 calculate->insertItem	(klocale->translate("Frequencies"),	this,SLOT(fftOp()));

 options->insertItem	(klocale->translate("Playback"),this,SLOT(playBackOp()));

 help->insertItem	(klocale->translate("About"),	this,SLOT(about()));

 bar->insertItem	(klocale->translate("&File"),	file);
 bar->insertItem	(klocale->translate("&Edit"),	edit);
 bar->insertItem	(klocale->translate("F&x"),	effects);
 bar->insertItem	(klocale->translate("&Calculate"),calculate);
 bar->insertItem	(klocale->translate("&Options") ,options);
 bar->insertItem	(klocale->translate("&Help"),	help);

 setView (mainwidget);
 setMenu (bar);
 setStatusBar (status);
}
//*****************************************************************************************
void TopWidget::about ()
{
	AboutDialog dialog (this);
	dialog.exec ();
}
//*****************************************************************************************
void TopWidget::quitInstance ()
{
 int index;
	this->hide();

	index=topwidget->find (this);			//remove this instance 
	if (index!=-1) topwidget->remove(index);	//from list of widgets

	if (!topwidget->isEmpty())
	 {
	    qApp->setMainWidget(topwidget->last());
	 }
	else
	 {
		qApp->exit (0);
	 }
}
//*****************************************************************************************
void TopWidget::newInstance ()
{
    TopWidget *tnew=new TopWidget();
    topwidget->append (tnew);

    tnew->show();
}
//*****************************************************************************************
void TopWidget::cliptoNew ()
{
  if (clipboard)
   {
    TopWidget *tnew=new TopWidget();
    topwidget->append (tnew);
    tnew->setSignal (clipboard);

    tnew->show();
    tnew->setCaption ("Copy of Clipboard");
   }
}
//*****************************************************************************************
void TopWidget::flushClip ()
{
	if (clipboard) delete clipboard;
	clipboard=0;
}
//*****************************************************************************************
void TopWidget::newOp()		{mainwidget->setRangeOp (NEW);}
void TopWidget::deleteOp()	{mainwidget->setRangeOp (DELETE);}
void TopWidget::cutOp	()	{mainwidget->setRangeOp (CUT);}
void TopWidget::copyOp	()	{mainwidget->setRangeOp (COPY);}
void TopWidget::pasteOp	()	{mainwidget->setRangeOp (PASTE);}
void TopWidget::cropOp	()	{mainwidget->setRangeOp (CROP);}
void TopWidget::zeroOp	()	{mainwidget->setRangeOp (ZERO);}
void TopWidget::flipOp	()	{mainwidget->setRangeOp (FLIP);}
void TopWidget::centerOp	()	{mainwidget->setRangeOp (CENTER);}
void TopWidget::reverseOp	()	{mainwidget->setRangeOp (REVERSE);}
void TopWidget::fadeInlOp	()	{mainwidget->setRangeOp	(FADEINLINEAR);}
void TopWidget::fadeInLogOp	()	{mainwidget->setRangeOp	(FADEINLOG);}
void TopWidget::fadeOutLogOp	()	{mainwidget->setRangeOp	(FADEOUTLOG);}
void TopWidget::fadeOutlOp	()	{mainwidget->setRangeOp (FADEOUTLINEAR);}
void TopWidget::selectAllOp	()	{mainwidget->setRangeOp	(SELECTALL);}
void TopWidget::selectRangeOp	()	{mainwidget->setRangeOp	(SELECTRANGE);}
void TopWidget::selectVisibleOp	()	{mainwidget->setRangeOp	(SELECTVISIBLE);}
void TopWidget::selectNoneOp	()	{mainwidget->setRangeOp	(SELECTNONE);}
void TopWidget::amplifyMaxOp	()	{mainwidget->setRangeOp	(AMPLIFYMAX);}
void TopWidget::noiseOp		()	{mainwidget->setRangeOp	(NOISE);}
void TopWidget::delayOp		()	{mainwidget->setRangeOp	(DELAY);}
void TopWidget::rateChangeOp	()	{mainwidget->setRangeOp	(RATECHANGE);}
void TopWidget::fftOp		()	{mainwidget->setRangeOp	(FFT);}
void TopWidget::playBackOp	()	{mainwidget->setRangeOp (PLAYBACKOPTIONS);}
//*****************************************************************************************
void TopWidget::revert ()
{
 if (!name.isNull())
  {
	mainwidget->setSignal (&name);
  }
}
//*****************************************************************************************
void TopWidget::openFile ()
{
 QString name=QFileDialog::getOpenFileName ();
 if (!name.isNull())
  {
	this->name=name;
	mainwidget->setSignal (&name);
	setCaption (name.data());
  }
}
//*****************************************************************************************
void TopWidget::saveFile ()
{
 if (!name.isEmpty())
  {
	this->name=name;
	mainwidget->saveSignal (&name);
	setCaption (name.data());
  }
}
//*****************************************************************************************
void TopWidget::saveFileas ()
{
 name=QFileDialog::getSaveFileName ();
 if (!name.isNull())
 mainwidget->saveSignal (&name);
}
//*****************************************************************************************
void TopWidget::saveSelection ()
{
 name=QFileDialog::getSaveFileName ();
 if (!name.isNull())
 mainwidget->saveSelectedSignal (&name);
}
//*****************************************************************************************
void TopWidget::setSignal (QString name)
{
 this->name=name;
 mainwidget->setSignal (&name);
 setCaption (name.data());
}
//*****************************************************************************************
void TopWidget::setSignal (MSignal *signal)
{
 mainwidget->setSignal (signal);
}
//*****************************************************************************************
int main( int argc, char **argv )
{
    KApplication a( argc, argv);
    TopWidget *tnew;

    tnew=new TopWidget();
    topwidget=new QList<TopWidget>();
    topwidget->append (tnew);

    if (argc==2) 
     {
	QString filename=argv[1];
	tnew->setSignal (filename);
     }

    a.setMainWidget( tnew );
    tnew->show();
    return a.exec();
}
