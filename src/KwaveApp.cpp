#include "KwaveApp.h"
#include <libkwave/Global.h>
#include <libkwave/String.h>
#include <libkwave/Parser.h>
#include <libkwave/MessagePort.h>
#include <libkwave/DynamicLoader.h>
#include <qdir.h>
#include "TopWidget.h"

extern struct Global globals;     
extern QStrList      recentFiles; 
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
KwaveApp::KwaveApp (int argc, char **argv): KApplication (argc,argv)
{
  findDirectories (); //puts directory pointers into Global structure

  globals.markertypes.setAutoDelete (true);
  globals.port=new MessagePort ();

  recentFiles=new QStrList (true);
  recentFiles.setAutoDelete (false);

  readConfig ();

  newWindow ();

  //      globals.app->setMainWidget (tnew);

  if (argc==2) 
    {
      QString filename=argv[1];
      TopWidget *tmp=topwidgetlist.first();
      if (tmp) tmp->setSignal     (filename);
    }
  check=new QTimer ();

  if (check)
    {
      connect (check,SIGNAL(timeout()), this,SLOT (timer()));
      check->start (10); //check every ms for a new command
    }
  else printf ("Error: Main Timer not started !!!\n");
}
//****************************************************************************
void KwaveApp::setOp (const char* str)
{
  if (matchCommand(str,"newwindow")) newWindow ();
  else
  if (matchCommand(str,"help")) invokeHTMLHelp ("kwave/index.html","");
  else
    {
      for (TopWidget *tmp=topwidgetlist.first();tmp;tmp=topwidgetlist.next())
	tmp->setOp (str);
    }
}
//****************************************************************************
void KwaveApp::addRecentFile (char* newfile)
{
  if (recentFiles.find(newfile) != -1) return;

  if (recentFiles.count() < 20) recentFiles.insert(0,newfile);
  else
    {
      recentFiles.remove(19);
      recentFiles.insert(0,newfile);
    }

  // save the list of recent files
  saveRecentFiles();

  TopWidget *tmp;
  for (tmp=topwidgetlist.first();tmp;tmp=topwidgetlist.next())
       tmp->updateRecentFiles(); //update all windows
}
//*****************************************************************************
void KwaveApp::newWindow ()
{
  TopWidget *tnew=new TopWidget();

  if (tnew)
    {
      topwidgetlist.append (tnew);
      tnew->show();
    }

}
//*****************************************************************************
void KwaveApp::closeWindow (TopWidget *todel)
{
  topwidgetlist.removeRef(todel);

  // save the configuration, including the list of recent files
  saveConfig();

  //if list is empty -> no more windows there -> exit application
  if (topwidgetlist.isEmpty()) exit (0);
}
//*****************************************************************************
//definitions of global variables needed/changed by read/save config routines
extern int play16bit;      //flag for playing 16 Bit
extern int bufbase;        //log of bufferrsize for playback... 
extern int mmap_threshold; //threshold in MB for using mmapping
extern char *mmap_dir;     //storage of dir name
extern char *mmapallocdir; //really used directory
//*****************************************************************************
/**
 * Saves the list of recent files to the kwave configuration file
 * @see KConfig
 */
void KwaveApp::saveRecentFiles()
{
  char buf[64];
  KConfig *config=getConfig();
  config->setGroup ("Recent Files");

  for (unsigned int i =0 ; i < recentFiles.count(); i++)
    {
      sprintf (buf,"%d",i);
      config->writeEntry (buf,recentFiles.at(i));
    }

  config->sync();
}

//*****************************************************************************
/**
 * Saves the current configuration of kwave to the configuration file.
 * This also includes saving the list of recent files.
 * @see KwaveApp::saveRecentFiles()
 */
void KwaveApp::saveConfig()
{
  char buf[64];
  KConfig *config=getConfig();

  config->setGroup ("Sound Settings");
  config->writeEntry ("16Bit",play16bit);
  config->writeEntry ("BufferSize",bufbase);

  config->setGroup ("Memory Settings");
  config->writeEntry ("Mmap threshold",mmap_threshold);
  config->writeEntry ("Mmap dir",mmap_dir);

  config->setGroup ("Labels");

  for (unsigned int i =0 ; i < globals.markertypes.count(); i++)
    {
      sprintf (buf,"%dCommand",i);
      config->writeEntry (buf,globals.markertypes.at(i)->getCommand());
    }

  config->sync();

  // also save the list of recent files
  saveRecentFiles();
}
//*****************************************************************************
// reads user config via KConfig, sets global variables accordingly
void KwaveApp::readConfig()
{
  QString result;
  char  buf[64];
  
  KConfig *config=getConfig();

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
      sprintf (buf,"%dCommand",i);                
      QString name=config->readEntry (buf);
      if (!name.isEmpty())
	{
	  LabelType *marker=new LabelType(name.data());
	  globals.markertypes.append (marker);
	}
    }
}
//*****************************************************************************
void KwaveApp::timer ()
{
  const char *message=0;
  if (globals.port) message=globals.port->getMessage ();
  if (message)
    {
      printf ("got Message:%s\n",message);
      setOp (message);
    }
}
//*****************************************************************************
KwaveApp::~KwaveApp	  ()
{
  saveConfig ();
  if (check) delete check;
}











