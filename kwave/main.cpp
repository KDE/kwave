#include "main.h"
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
 QPopupMenu *help=	new QPopupMenu ();

 status=new KStatusBar (this,"Main Status Bar");
 status->insertItem ("Length: 0 ms       ",1);
 status->insertItem ("Rate: 0 kHz       ",2);
 status->insertItem ("Samples: 0           ",3);
 status->insertItem ("selected: 0 ms     ",4);
 status->insertItem ("Clipboard: 0 ms      ",5);

 bar=		new KMenuBar (this,"MainMenu");
 mainwidget=	new MainWidget (this,"MainView",status);

 file->insertItem	(klocale->translate("&New..."),	this,SLOT(file_new()),CTRL+Key_N);
 file->insertItem	(klocale->translate("New &Instance"),this,SLOT(inst_new()),CTRL+Key_I);
 file->insertSeparator	();
 file->insertItem	(klocale->translate("&Open"),	this,SLOT(openFile()),CTRL+Key_O);
 file->insertItem	(klocale->translate("&Save"),	this,SLOT(saveFile()),CTRL+Key_S);
 file->insertItem	(klocale->translate("Save &As"),this,SLOT(saveFileas()),CTRL+Key_A);
 file->insertItem	(klocale->translate("Save Se&lection"),this,SLOT(saveSelection()),CTRL+Key_L);
 file->insertSeparator	();
 file->insertItem	(klocale->translate("&Quit"),	this,SLOT(inst_quit()),CTRL+Key_Q);
 edit->insertItem	(klocale->translate("Cut"),	this,SLOT(cutOp()),CTRL+Key_X);
 edit->insertItem	(klocale->translate("Copy"),	this,SLOT(copyOp()),CTRL+Key_C);
 edit->insertItem	(klocale->translate("Paste"),	this,SLOT(pasteOp()),CTRL+Key_V);
 edit->insertItem	(klocale->translate("Crop"),	this,SLOT(cropOp()));
 edit->insertItem	(klocale->translate("Delete"),	this,SLOT(deleteOp()),Key_Delete);
 edit->insertItem	(klocale->translate("Flip"),	this,SLOT(flipOp()));
 edit->insertSeparator	();
 clip->insertItem	(klocale->translate("Flush"),	this,SLOT(flushClip()));
 clip->insertItem	(klocale->translate("to new Window"),this,SLOT(cliptoNew()));
 edit->insertItem	(klocale->translate("Clipboard"),clip);
 effects->insertItem	(klocale->translate("Zero"),	this,SLOT(zeroOp()));
 effects->insertItem	(klocale->translate("Reverse"),	this,SLOT(reverseOp()));
 help->insertItem	(klocale->translate("About"), this,SLOT(about()));

 bar->insertItem	(klocale->translate("File"),	file);
 bar->insertItem	(klocale->translate("Edit"),	edit);
 bar->insertItem	(klocale->translate("Effects"),	effects);
 bar->insertItem	(klocale->translate("Help"),	help);

 setView (mainwidget);
 setMenu (bar);
 setStatusBar (status);
}
//*****************************************************************************************
void TopWidget::file_new ()
{
 QDialog *select=new QDialog (this,"ParamSelector",true);
 QPushButton *ok=new QPushButton ("Ok",select);
 QObject::connect (ok,SIGNAL(clicked()),select,SLOT (accept()));
 select->exec();

 delete select;
}
//*****************************************************************************************
void TopWidget::about ()
{
 printf ("by Martin Wilz\n");
}
//*****************************************************************************************
void TopWidget::inst_quit ()
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
	  printf ("last Instance !\n");
		qApp->exit (0);
	 }
}
//*****************************************************************************************
void TopWidget::inst_new ()
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
void TopWidget::deleteOp()	{mainwidget->setRangeOp (DELETE);}
void TopWidget::cutOp	()	{mainwidget->setRangeOp (CUT);}
void TopWidget::copyOp	()	{mainwidget->setRangeOp (COPY);}
void TopWidget::pasteOp	()	{mainwidget->setRangeOp (PASTE);}
void TopWidget::cropOp	()	{mainwidget->setRangeOp (CROP);}
void TopWidget::zeroOp	()	{mainwidget->setRangeOp (ZERO);}
void TopWidget::flipOp()	{mainwidget->setRangeOp (FLIP);}
void TopWidget::reverseOp()	{mainwidget->setRangeOp (REVERSE);}
//*****************************************************************************************
void TopWidget::openFile ()
{
 name=QFileDialog::getOpenFileName ();
 if (!name.isNull())
 mainwidget->setSignal (&name);
 setCaption (name.data());
}
//*****************************************************************************************
void TopWidget::saveFile ()
{
 if (!name.isEmpty())
  {
	mainwidget->saveSignal (&name);
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

    printf ("%d\n",argc);
    if (argc==2) 
     {
	QString filename=argv[1];
	tnew->setSignal (filename);
     }

    a.setMainWidget( tnew );
    tnew->show();
    return a.exec();
}
