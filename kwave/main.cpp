//Kwave main file
//This one includes methods of the Topwidget Class.

#include "main.h"
#include "about.h"
#include <unistd.h>
#include <qkeycode.h>
#include "clipboard.h"

ClipBoard *clipboard=0;
QList<TopWidget>*topwidget=0; 
QList<MarkerType>*markertypes=0;
QStrList *recentFiles=0; 
QDir *configDir;
KApplication *app;
//*****************************************************************************
TopWidget::TopWidget (KApplication *a) : KTopLevelWidget ()
{
  app=a;
  bit=24;

  QPopupMenu *file=     new QPopupMenu ();
  QPopupMenu *help=     new QPopupMenu ();
  QPopupMenu *options=	new QPopupMenu ();
  recent=   new QPopupMenu ();
  save=     new QPopupMenu ();

  connect( recent, SIGNAL(activated(int)), SLOT(openRecent(int)) ); 

  status=new KStatusBar (this);

  status->insertItem (klocale->translate("Length: 0 ms           "),1);
  status->insertItem (klocale->translate("Rate: 0 kHz         "),2);
  status->insertItem (klocale->translate("Samples: 0             "),3);
  status->insertItem (klocale->translate("selected: 0 ms        "),4);
  status->insertItem (klocale->translate("Clipboard: 0 ms      "),5);

  bar=		new KMenuBar    (this);
  manage=       new MenuManager (this,bar);

  //this is where the menus are created
  //now  cleaned  up, mom ! just a little bit of dirt left

  file->insertItem	(klocale->translate("&New..."),	this,SLOT(newOp()),CTRL+Key_N);
  file->insertItem	(klocale->translate("New &Window"),this,SLOT(newInstance()),CTRL+Key_W);
  file->insertSeparator	();
  file->insertItem	(klocale->translate("&Open"),recent);
  recent->insertItem	(klocale->translate("&New ..."),this,SLOT(openFile()),CTRL+Key_O);
  recent->insertSeparator	();
  if (recentFiles->count()>0)
    for ( unsigned int i =0 ; i < recentFiles->count(); i++)
	recent->insertItem(recentFiles->at(i));

  file->insertItem	(klocale->translate("&Save"),	this,SLOT(saveFile()),CTRL+Key_S);
  file->insertItem	(klocale->translate("&Save"),	save);
  
  save->insertItem	(klocale->translate("&As ..."),this,SLOT(saveFileas()),CTRL+SHIFT+Key_S);
  save->insertItem	(klocale->translate("&Selection ..."),this,SLOT(saveSelection()));
  save->insertItem	(klocale->translate("&Blocks ..."),this,SLOT(saveBlocksOp()));
  save->insertSeparator	();
  bit24=save->insertItem (klocale->translate("&24 bit"));
  bit16=save->insertItem (klocale->translate("&16 bit"));
  bit8=save->insertItem	 (klocale->translate(" &8 bit"));
  save->setCheckable (true);
  save->setItemChecked (bit24,true);
  save->connectItem (bit24,this,SLOT(save24Bit()));
  save->connectItem (bit16,this,SLOT(save16Bit()));
  save->connectItem (bit8,this,SLOT(save8Bit()));
  
  file->insertSeparator	();
  file->insertItem	(klocale->translate("&Revert"),	this,SLOT(revert()),CTRL+Key_R);
  file->insertSeparator	();
  file->insertItem	(klocale->translate("&Import Ascii"),this,SLOT(importAsciiFile()),CTRL+Key_I);
  file->insertSeparator	();
  file->insertItem	(klocale->translate("&Quit"),	this,SLOT(quitInstance()),CTRL+Key_Q);

  options->insertItem	(klocale->translate("Playback"),this,SLOT(playBackOp()));
  options->insertItem	(klocale->translate("Memory"),this,SLOT(memoryOp()));

  help->insertItem	(klocale->translate("&Contents"),this,SLOT(getHelp()));
  help->insertSeparator	();
  help->insertItem	(klocale->translate("&About kwave"),	this,SLOT(about()));

  bar->insertItem	(klocale->translate("&File"),	file);
  bar->insertItem	(klocale->translate("&Options") ,options);
  bar->insertItem	(klocale->translate("&Help"),	help);

  KDNDDropZone *dropZone = new KDNDDropZone( this , DndURL);
  connect( dropZone, SIGNAL( dropAction( KDNDDropZone *) ),
           this, SLOT( dropEvent( KDNDDropZone *) ) );        

  mainwidget=new MainWidget (this,manage,status);

  //connect clicked menu entrys with main communication channel of kwave
  connect(manage, SIGNAL(id(int)),this, SLOT(setOp(int))); 

  setView (mainwidget);

  ClipBoard ().registerMenu (manage);

  setMenu (bar);
  setStatusBar (status);
}
//*****************************************************************************
TopWidget::~TopWidget ()
{
  //remove this instance from list of widgets
  topwidget->removeRef(this);	

  ClipBoard ().unregisterMenu (manage);

  if (topwidget->isEmpty()) app->exit (0); //if list is empty -> no more windows there 
}
//*****************************************************************************
void TopWidget::about ()
{
  AboutDialog dialog (this);
  dialog.exec ();
}
//*****************************************************************************
void TopWidget::getHelp ()
{
  app->invokeHTMLHelp ("kwave/index.html","");
}
//*****************************************************************************
void TopWidget::quitInstance ()
{
  delete this;
}
//*****************************************************************************
void TopWidget::newInstance ()
{
  TopWidget *tnew=new TopWidget(app);
  topwidget->append (tnew);

  tnew->show();
}
//****************************************************************************
void TopWidget::cliptoNew ()
{
  if (clipboard) clipboard->toWindow ();
  else debug ("clipboard is empty !\n");
}
//*****************************************************************************
void TopWidget::flushClip ()
{
  if (clipboard) delete clipboard;
  clipboard=0;
}
//*****************************************************************************
void TopWidget::newOp()		{mainwidget->setOp (NEW);}
void TopWidget::playBackOp()	{mainwidget->setOp (PLAYBACKOPTIONS);}
void TopWidget::memoryOp()	{mainwidget->setOp (MEMORYOPTIONS);}
void TopWidget::saveBlocksOp()	{mainwidget->setOp (SAVEBLOCKS+bit);}
//*****************************************************************************
void TopWidget::setOp (int id)
{
  mainwidget->setOp (id);
  if (clipboard) clipboard->setOp (id);
}
//*****************************************************************************
void TopWidget::deleteChannel	(int num)
{
  mainwidget->setOp (DELETECHANNEL+num);
}
//*****************************************************************************
void TopWidget::revert ()
{
 if (!name.isNull())
  {
    mainwidget->setSignal (&name);
  }
}
//*****************************************************************************
void TopWidget::openRecent (int num)
{
 if (num-1>0)
   {
     QString name=recentFiles->at(num-2);

     if (!name.isNull())
       {
	 this->name=name;


	 mainwidget->setSignal (&name);
	 setCaption (name.data());
       }
   }
}
//*****************************************************************************
void TopWidget::addRecentFile (char* newfile)
{
  if (recentFiles->find(newfile) != -1) return;

  if (recentFiles->count() < 20) recentFiles->insert(0,newfile);
  else
    {
      recentFiles->remove(19);
      recentFiles->insert(0,newfile);
    }

   TopWidget *tmp;

   for (tmp=topwidget->first();tmp!=0;tmp=topwidget->next())
       tmp->updateRecentFiles(); //update all windows

}           
//*****************************************************************************
void TopWidget::updateRecentFiles ()
{
  recent->clear();

  recent->insertItem	(klocale->translate("&New ..."),this,SLOT(openFile()),CTRL+Key_O);
  recent->insertSeparator	();

  for (unsigned int i =0 ; i < recentFiles->count(); i++)
      recent->insertItem (recentFiles->at(i));
}           
//*****************************************************************************
void TopWidget::dropEvent (KDNDDropZone *drop)
{
  QStrList & list =drop->getURLList();
  char *s=list.getFirst();
  if (s)
    {
      QString name = s;
      if ( name.left(5) == "file:")
	{
	  name=name.right (name.length()-5);
	  this->name=name;
	  mainwidget->setSignal (&name);
	  addRecentFile (name.data());
	  setCaption (name.data());
	}
    }
}
//*****************************************************************************
void TopWidget::openFile ()
{
 QString name=QFileDialog::getOpenFileName (0,"*.wav",this);
 if (!name.isNull())
  {
	this->name=name;
	mainwidget->setSignal (&name);
	addRecentFile (name.data());
	setCaption (name.data());
  }
}
//*****************************************************************************
void TopWidget::importAsciiFile ()
{
 QString name=QFileDialog::getOpenFileName (0,"*.*",this);
 if (!name.isNull())
  {
	this->name=name;
	mainwidget->setSignal (&name,ASCII);
	setCaption (name.data());
  }
}
//*****************************************************************************
void TopWidget::save24Bit ()
{
  save->setItemChecked (bit24,true);
  save->setItemChecked (bit16,false);
  save->setItemChecked (bit8,false);
  bit=24;
}
//*****************************************************************************
void TopWidget::save16Bit ()
{
  save->setItemChecked (bit24,false);
  save->setItemChecked (bit16,true);
  save->setItemChecked (bit8,false);
  bit=16;
}
//*****************************************************************************
void TopWidget::save8Bit ()
{
  save->setItemChecked (bit24,false);
  save->setItemChecked (bit16,false);
  save->setItemChecked (bit8,true);
  bit=8;
}
//*****************************************************************************
void TopWidget::saveFile ()
{
 if (!name.isEmpty())
  {
	this->name=name;
	mainwidget->saveSignal (&name,bit);
	setCaption (name.data());
  }
}
//*****************************************************************************
void TopWidget::saveFileas ()
{
  name=QFileDialog::getSaveFileName (0,"*.wav",this);
  if (!name.isNull())
    {
      mainwidget->saveSignal (&name,bit);
      addRecentFile (name.data());
    }
}
//*****************************************************************************
void TopWidget::saveSelection ()
{
 name=QFileDialog::getSaveFileName (0,"*.wav",this);
 if (!name.isNull())
 mainwidget->saveSignal (&name,bit,true);
}
//****************************************************************************
void TopWidget::setSignal (QString name)
{
 this->name=name;
 mainwidget->setSignal (&name);
 setCaption (name.data());
}
//*****************************************************************************
void TopWidget::setSignal (MSignal *signal)
{
 mainwidget->setSignal (signal);
}
//*****************************************************************************
//global variables needed/changed by read/save config routines
extern int play16bit;      //flag for playing 16 Bit
extern int bufbase;        //log of bufferrsize for playback... 
extern int mmap_threshold; //threshold in MB for using mmapping
extern char *mmap_dir;     //storage of dir name
extern char *mmapallocdir; //really used directory
//*****************************************************************************
void saveConfig(KApplication *app)
{
  char buf[64];
  KConfig *config=app->getConfig();
  config->setGroup ("Recent Files");

  for (unsigned int i =0 ; i < recentFiles->count(); i++)
    {
      sprintf (buf,"%d",i);
      config->writeEntry (buf,recentFiles->at(i));
    }

  config->setGroup ("Sound Settings");
  config->writeEntry ("16Bit",play16bit);
  config->writeEntry ("BufferSize",bufbase);

  config->setGroup ("Memory Settings");
  config->writeEntry ("Mmap threshold",mmap_threshold);
  config->writeEntry ("Mmap dir",mmap_dir);

  config->setGroup ("Labels");

  for (unsigned int i =0 ; i < markertypes->count(); i++)
    {
      sprintf (buf,"%dName",i);
      config->writeEntry (buf,markertypes->at(i)->name->data());

      sprintf (buf,"%dhasName",i);
      config->writeEntry (buf,markertypes->at(i)->named);

      sprintf (buf,"%dColor",i);
      config->writeEntry (buf,*((markertypes->at(i))->color));
    }
  config->sync();
}
//*****************************************************************************
// reads in Configuration via KConfig, sets global variables accordingly
void readConfig(KApplication *app)
{
  QString result;
  char  buf[64];
  
  KConfig *config=app->getConfig();

  config->setGroup ("Recent Files");
 
  for (unsigned int i =0 ; i < 20; i++)
    {
      sprintf (buf,"%d",i);           //generate number 
      result=config->readEntry (buf); //and read coresponding entry

      if (!result.isNull())
	{
	  QFile file(result.data());
	  if (file.exists())          //check if file exists and insert it
	  recentFiles->append (result.data());
	}
    }

  config->setGroup ("Sound Settings");

  result=config->readEntry ("16Bit");
  if (!result.isNull())  play16bit=result.toInt();
  result=config->readEntry ("BufferSize");
  if (!result.isNull())  bufbase=result.toInt();

  config->setGroup   ("Memory Settings");
  result=config->readEntry ("Mmap threshold");
  if (!result.isNull())  mmap_threshold=result.toInt();
  result=config->readEntry ("Mmap dir");
  if (!result.isNull())  mmap_dir=strdup(result.data());
  mmapallocdir=mmap_dir;

  config->setGroup ("Labels");
  for (unsigned int i =0 ; i < 20; i++)
    {
      sprintf (buf,"%dName",i);                
      QString name=config->readEntry (buf);
      if (!name.isEmpty())
	{
	  sprintf (buf,"%dColor",i);                
	  QColor color=config->readColorEntry (buf);

	  sprintf (buf,"%dhasName",i);
	  int hasname=config->readNumEntry (buf,-1);


	  MarkerType *marker=new MarkerType();

	  marker->name=new QString(name);
	  marker->named=hasname;
	  marker->color=new QColor(color);
	  markertypes->append (marker);
	}
    }
}
//*****************************************************************************
int main( int argc, char **argv )
{
  app=new KApplication (argc, argv);

  srandom (0); //just in case the random generator be used anywhere later

  if (app)
    {
      configDir=new QDir((app->localkdedir()).data());
      if (configDir->cd ("share"))
	{
	  if (!configDir->cd ("apps"))
	    {
	      configDir->mkdir ("apps");
	      configDir->cd ("apps");
	    }
	  if (!configDir->cd ("kwave"))
	    {
	      configDir->mkdir ("kwave");
	      configDir->cd ("kwave");
	    }
	}  
      else printf ("no local kdedir found \n");

      TopWidget *tnew;

      markertypes=new QList<MarkerType>();
      markertypes->setAutoDelete (true);

      recentFiles=new QStrList (true);
      recentFiles->setAutoDelete (false);
      readConfig (app);

      tnew=new TopWidget(app);
      topwidget=new QList<TopWidget>();
      topwidget->append (tnew);

      app->setMainWidget (tnew);

      if (argc==2) 
	{
	  QString filename=argv[1];
	  tnew->setSignal (filename);
	  tnew->addRecentFile (filename.data());
	}
      tnew->show();

      int result=app->exec();
      //    int result=0;

      saveConfig (app);

      if (markertypes) delete markertypes;
      if (recentFiles) delete recentFiles;

      return result;
    }
  return -1;
}
