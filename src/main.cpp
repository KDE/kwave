//Kwave main file
//This one includes methods of the Topwidget Class.

#include <unistd.h>
#include <qkeycode.h>
#include "main.h"
#include "clipboard.h"
#include <libkwave/dynamicloader.h>
#include <libkwave/dialogoperation.h>
#include <libkwave/parser.h>
#include <libkwave/globals.h>
#include <libkwave/fileloader.h>
#include "../libgui/kwavedialog.h"

struct Global      globals;
QList<TopWidget>   topwidgetlist; 
QStrList           recentFiles; 
//*****************************************************************************
void TopWidget::setOp (const char *str)
{
  if (matchCommand (str,"menu")) menumanage->setCommand (str);
  else
  if (matchCommand (str,"open")) openFile();
  else
  if (matchCommand (str,"save")) saveFile();
  else
  if (matchCommand (str,"revert")) revert();
  else
  if (matchCommand (str,"importascii")) importAsciiFile();
  else
  if (matchCommand (str,"saveas")) saveFileAs(false);
  else
  if (matchCommand (str,"saveselect")) saveFileAs(true);
  else
  if (matchCommand(str,"help")) globals.app->invokeHTMLHelp ("kwave/index.html","");
  else
  if (matchCommand(str,"newwindow")) newInstance ();
  else
  if (matchCommand(str,"quit")) delete this;
  else mainwidget->doCommand (str);
}
//*****************************************************************************
void TopWidget::parseBatch (const char *str)
  //parses a list a of commands separated by newlines
{
  LineParser lineparser(str);
  const char *line=lineparser.getLine ();

  while (line)
    {
      setOp (line);
      line=lineparser.getLine ();
    }
}
//*****************************************************************************
TopWidget::TopWidget () : KTopLevelWidget ()
{
  bit=24;

  saveDir=0;
  loadDir=0;

  status=new KStatusBar (this);

  status->insertItem (klocale->translate("Length: 0 ms           "),1);
  status->insertItem (klocale->translate("Rate: 0 kHz         "),2);
  status->insertItem (klocale->translate("Samples: 0             "),3);
  status->insertItem (klocale->translate("selected: 0 ms        "),4);
  status->insertItem (klocale->translate("Clipboard: 0 ms      "),5);

  bar=		new KMenuBar    (this);
  menumanage=   new MenuManager (this,bar);

  //connect clicked menu entrys with main communication channel of kwave
  connect(menumanage, SIGNAL(command(const char *)),
	  this, SLOT(setOp(const char *))); 

  //enable drop of local files onto kwave window
  KDNDDropZone *dropZone = new KDNDDropZone( this ,DndURL);
  connect( dropZone, SIGNAL( dropAction( KDNDDropZone *)),
           this, SLOT( dropEvent( KDNDDropZone *)));        

  //read menus and create them...
  QDir configDir (globals.globalconfigDir);

  FileLoader loader (configDir.absFilePath("menus.config"));  
  parseBatch (loader.getMem());

  mainwidget=new MainWidget (this,menumanage,status);
  setView (mainwidget);

  setMenu (bar);
  setStatusBar (status);
  topwidgetlist.append (this);
}
//*****************************************************************************
TopWidget::~TopWidget ()
{
  //remove this instance from list of widgets
  topwidgetlist.removeRef(this);	

  if (saveDir) delete saveDir;
  if (loadDir) delete loadDir;

  ClipBoard ().unregisterMenu (menumanage);

  //if list is empty -> no more windows there -> exit application
  if (topwidgetlist.isEmpty()) globals.app->exit (0);
}
//*****************************************************************************
void TopWidget::newInstance ()
{
  TopWidget *tnew=new TopWidget();

  tnew->show();
}
//****************************************************************************
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
  QString name=recentFiles.at(num);

  if (!name.isNull())
    {
      this->name=name;
      mainwidget->setSignal (&name);
      setCaption (name.data());
    }
}
//*****************************************************************************
void TopWidget::addRecentFile (char* newfile)
{
  if (recentFiles.find(newfile) != -1) return;

  if (recentFiles.count() < 20) recentFiles.insert(0,newfile);
  else
    {
      recentFiles.remove(19);
      recentFiles.insert(0,newfile);
    }

   TopWidget *tmp;

   for (tmp=topwidgetlist.first();tmp;tmp=topwidgetlist.next())
       tmp->updateRecentFiles(); //update all windows
}           
//*****************************************************************************
void TopWidget::updateRecentFiles ()
{
  menumanage->clearNumberedMenu ("RecentFiles");
  for (unsigned int i =0 ; i < recentFiles.count(); i++)
    menumanage->addNumberedMenuEntry ("RecentFiles",recentFiles.at(i));
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
void TopWidget::saveFile ()
{
  if (!name.isEmpty())
    {
      this->name=name;
      mainwidget->saveSignal (&name,bit);
      setCaption (name.data());
    }
  else saveFileAs (false);
}
//*****************************************************************************
void TopWidget::saveFileAs (bool selection)
{
  QFileDialog *dialog;

  if (saveDir)
    dialog=new QFileDialog (saveDir->absPath().data(),"*.wav",this,0,true);
  else
    dialog=new QFileDialog (this,0,true);

  if (dialog)
    {
      dialog->exec();
      name=dialog->selectedFile();
      if (!name.isNull())
	{
	  if (saveDir) delete saveDir;
	  saveDir=new QDir (dialog->dirPath());

	  mainwidget->saveSignal (&name,bit,selection);
	  addRecentFile (name.data());
	}
      delete dialog;
    }
}
//*****************************************************************************
void TopWidget::setSignal (QString name)
{
  this->name=name;
  mainwidget->setSignal (&name);
  setCaption (name.data());
}
//*****************************************************************************
void TopWidget::setSignal (SignalManager *signal)
{
  mainwidget->setSignal (signal);
}
//*****************************************************************************
//definitions of global variables needed/changed by read/save config routines
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

  for (unsigned int i =0 ; i < recentFiles.count(); i++)
    {
      sprintf (buf,"%d",i);
      config->writeEntry (buf,recentFiles.at(i));
    }

  config->setGroup ("Sound Settings");
  config->writeEntry ("16Bit",play16bit);
  config->writeEntry ("BufferSize",bufbase);

  config->setGroup ("Memory Settings");
  config->writeEntry ("Mmap threshold",mmap_threshold);
  config->writeEntry ("Mmap dir",mmap_dir);

  config->setGroup ("Labels");

  for (unsigned int i =0 ; i < globals.markertypes.count(); i++)
    {
      sprintf (buf,"%dName",i);
      config->writeEntry (buf,globals.markertypes.at(i)->name->data());

      sprintf (buf,"%dhasName",i);
      config->writeEntry (buf,globals.markertypes.at(i)->named);

      sprintf (buf,"%dColor",i);
      config->writeEntry (buf,*((globals.markertypes.at(i))->color));
    }
  config->sync();
}
//*****************************************************************************
// reads user config via KConfig, sets global variables accordingly
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
	  recentFiles.append (result.data());
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
  if (!result.isNull())  mmap_dir=duplicateString(result.data());
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
	  globals.markertypes.append (marker);
	}
    }
}
//*****************************************************************************
void findDirectories ()
  //finds/creates configuration/plugin directories and stores them in 
  //the globals struct
{
  QDir localconfig((globals.app->localkdedir()).data());
  if (localconfig.cd ("share"))
    {
      if (!localconfig.cd ("apps"))
	{
	  localconfig.mkdir ("apps");
	  localconfig.cd ("apps");
	}
      if (!localconfig.cd ("kwave"))
	{
	  localconfig.mkdir ("kwave");
	  localconfig.cd ("kwave");
	}
      globals.localconfigDir=duplicateString (localconfig.absPath());
    }
  else debug ("no local user kdedir found !\n");

  QDir globalconfig((globals.app->kde_datadir()).data());
  if (!globalconfig.cd ("kwave"))
    debug ("no global kwave config dir found !\n");
  globals.globalconfigDir=duplicateString (globalconfig.absPath());

  QDir timepluginDir ("/usr/local/lib/kwave/");
  //temporary solution:fixed path
  if (timepluginDir.cd ("modules"))
    if (timepluginDir.cd ("time"))
      globals.timeplugins=DynamicLoader::getPlugins (timepluginDir.absPath().data());

  QDir pluginDir (globals.globalconfigDir);
  if (pluginDir.cd ("modules"))
    if (pluginDir.cd ("dialogs"))
      globals.dialogplugins=DynamicLoader::getPlugins (pluginDir.absPath().data());

  QDir filter(globals.localconfigDir);

  if (!filter.cd ("presets"))
    {
      filter.mkdir ("presets");
      filter.cd ("presets");
    }

  if (!filter.cd ("filters"))
    {
      filter.mkdir ("filters");
      filter.cd ("filters");
    }
  globals.filterDir=duplicateString (filter.absPath());
}
//*****************************************************************************
int main( int argc, char **argv )
{
  globals.app=new KApplication (argc, argv);

  if (globals.app)
    {
      TopWidget *tnew;

      findDirectories (); //puts directory pointers into Global structure

      globals.markertypes.setAutoDelete (true);

      recentFiles=new QStrList (true);
      recentFiles.setAutoDelete (false);
      readConfig (globals.app);

      tnew=new TopWidget();

      globals.app->setMainWidget (tnew);

      if (argc==2) 
	{
	  QString filename=argv[1];
	  tnew->setSignal     (filename);
	  tnew->addRecentFile (filename.data());
	}
      tnew->show();

      int result=globals.app->exec();

      saveConfig (globals.app);

      return result;
    }
  return -1;
}

