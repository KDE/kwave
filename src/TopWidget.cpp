#include <stdio.h>
#include <unistd.h>

#include <kapp.h>
#include <qkeycode.h>
#include <qdir.h>
#include <qfiledlg.h>
#include <drag.h>

#include <libkwave/DynamicLoader.h>
#include <libkwave/DialogOperation.h>
#include <libkwave/Parser.h>
#include <libkwave/LineParser.h>
#include <libkwave/Global.h>
#include <libkwave/FileLoader.h>

#include "libgui/Dialog.h"
#include "libgui/MenuManager.h"

#include "sampleop.h"

#include "KwaveApp.h"
#include "ClipBoard.h"
#include "MainWidget.h"
#include "TopWidget.h"

extern Global globals;
QStrList           recentFiles;

//*****************************************************************************
void TopWidget::setOp (const char *str)
{
  if (matchCommand (str,"menu")) menumanage->setCommand (str);
  else
  if (matchCommand (str,"open")) openFile();
  else
  if (matchCommand (str,"openrecent")) openRecent(str);
  else
  if (matchCommand (str,"save")) saveFile();
  else
  if (matchCommand (str,"resolution")) resolution(str);
  else
  if (matchCommand (str,"revert")) revert();
  else
  if (matchCommand (str,"importascii")) importAsciiFile();
  else
  if (matchCommand (str,"saveas")) saveFileAs(false);
  else
  if (matchCommand (str,"loadbatch")) loadBatch(str);
  else
  if (matchCommand (str,"saveselect")) saveFileAs(true);
  else
  if (matchCommand(str,"quit")) globals.app->closeWindow (this);
  else mainwidget->doCommand (str);
}
//*****************************************************************************
void TopWidget::loadBatch (const char *str)
{
  Parser parser(str);
  FileLoader loader (parser.getFirstParam());  
  parseCommands (loader.getMem());
}
//*****************************************************************************
void TopWidget::parseCommands (const char *str)
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
TopWidget::TopWidget ()
  :KTMainWindow()
{
  bits=16;

  saveDir=0;
  loadDir=0;
  name=0;

  status=new KStatusBar (this);

  status->insertItem (i18n("Length: 0 ms           "),1);
  status->insertItem (i18n("Rate: 0 kHz         "),2);
  status->insertItem (i18n("Samples: 0             "),3);
  status->insertItem (klocale->translate("selected: 0 ms        "),4);
  status->insertItem (klocale->translate("Clipboard: 0 ms      "),5);

  KMenuBar *bar = new KMenuBar(this);
  menumanage = new MenuManager(this, *bar);

  //connect clicked menu entries with main communication channel of kwave
  connect(menumanage, SIGNAL(command(const char *)),
	  this, SLOT(setOp(const char *)));

  //enable drop of local files onto kwave window
  KDNDDropZone *dropZone = new KDNDDropZone( this ,DndURL);
  connect( dropZone, SIGNAL( dropAction( KDNDDropZone *)),
           this, SLOT( dropEvent( KDNDDropZone *)));        

  //read menus and create them...
  QDir configDir (globals.globalconfigDir);

  FileLoader loader (configDir.absFilePath("menus.config"));  
  parseCommands (loader.getMem());

  updateRecentFiles ();

  mainwidget=new MainWidget (this,menumanage,status);
  setView (mainwidget);

  setMenu (bar);
  setStatusBar (status);

  updateMenu();
}

//*****************************************************************************
void TopWidget::revert ()
{
  if (name)
    {
      mainwidget->setSignal(name);
      bits = mainwidget->getBitsPerSample();
      updateMenu();
    }
}
//*****************************************************************************
void TopWidget::resolution (const char *str)
{
  Parser parser (str);
  int bps=parser.toInt();

  if ( (bps >= 0) && (bps <= 24) && (bps % 8 == 0))
    {
      bits=bps;
      debug("bits=%d", bits); // ###
    }
  else debug ("out of range\n");
}
//*****************************************************************************
void TopWidget::openRecent (const char *str)
{
    Parser parser (str);
    const char *filename = parser.getFirstParam();

    if (filename) {
	if (this->name) deleteString (this->name);
	this->name=duplicateString(filename);
	mainwidget->setSignal (this->name);
	globals.app->addRecentFile (this->name);
	setCaption (this->name);
	bits = mainwidget->getBitsPerSample();
	updateMenu();
    } else warning("TopWidget::openRecent(%s) failed, filename==0 !?");
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

	  this->name=duplicateString (name.data());
	  mainwidget->setSignal (name);
	  globals.app->addRecentFile (name.data());
	  setCaption (name.data());
	  updateMenu();
	}
    }
}
//*****************************************************************************
void TopWidget::openFile ()
{
  QString name=QFileDialog::getOpenFileName (0,"*.wav",this);
  if (!name.isNull())
    {
      this->name=duplicateString (name.data());
      mainwidget->setSignal (this->name);
      globals.app->addRecentFile (name.data());
      setCaption (this->name);
      bits = mainwidget->getBitsPerSample();
      updateMenu();
    }
}
//*****************************************************************************
void TopWidget::importAsciiFile ()
{
  QString name=QFileDialog::getOpenFileName (0,"*.*",this);
  if (!name.isNull())
    {
      this->name=duplicateString (name.data());
      mainwidget->setSignal (this->name,ASCII);
      setCaption (this->name);
      bits = mainwidget->getBitsPerSample();
      updateMenu();
    }
}
//*****************************************************************************
void TopWidget::saveFile ()
{
  if (name)
    {
      mainwidget->saveSignal (name,bits);
      setCaption (name);
      updateMenu();
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
      name=duplicateString (dialog->selectedFile());
      if (name)
	{
	  if (saveDir) delete saveDir;
	  saveDir=new QDir (dialog->dirPath());

	  mainwidget->saveSignal (name,bits,selection);
	  globals.app->addRecentFile (name);
	  setCaption (name);
	  updateMenu();
	}
      delete dialog;
    }
}
//*****************************************************************************
void TopWidget::setSignal (const char *newname)
{
  if (name) deleteString (name);
  this->name=duplicateString (newname);
  mainwidget->setSignal (name);
  globals.app->addRecentFile (name);
  setCaption (name);
  updateMenu();
}
//*****************************************************************************
void TopWidget::setSignal (SignalManager *signal)
{
  mainwidget->setSignal (signal);
}
//*****************************************************************************
void TopWidget::updateRecentFiles ()
{
  menumanage->clearNumberedMenu("ID_FILE_OPEN_RECENT");
  for (unsigned int i =0 ; i < recentFiles.count(); i++)
    menumanage->addNumberedMenuEntry ("ID_FILE_OPEN_RECENT",recentFiles.at(i));
}
//*****************************************************************************
void TopWidget::updateMenu ()
{
  char *buffer = new char[64];
  char *format;

  format = "ID_FILE_SAVE_RESOLUTION_%d";
  sprintf(buffer, format, bits);
  menumanage->selectItem("@BITS_PER_SAMPLE", (const char *)buffer);

  delete buffer;
}
//*****************************************************************************
TopWidget::~TopWidget ()
{
  if (saveDir) delete saveDir;
  if (loadDir) delete loadDir;
  if (name)    deleteString (name);
}







