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
  if (matchCommand (str,"revert")) revert();
  else
  if (matchCommand (str,"importascii")) importAsciiFile();
  else
  if (matchCommand (str,"saveas")) saveFileAs(false);
  else
  if (matchCommand (str,"loadbatch")) loadBatch(str);
  else
  if (matchCommand (str,"bits"))
    {
      KwaveParser parser (str);
      bit=parser.toInt ();
    }
  else
  if (matchCommand (str,"saveselect")) saveFileAs(true);
  else
  if (matchCommand(str,"quit")) globals.app->closeWindow (this);
  else mainwidget->doCommand (str);
}
//*****************************************************************************
void TopWidget::loadBatch (const char *str)
{
  KwaveParser parser(str);
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
TopWidget::TopWidget () : KTMainWindow ()
{
  bit=24;

  saveDir=0;
  loadDir=0;
  name=0;

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
  parseCommands (loader.getMem());

  updateRecentFiles ();

  mainwidget=new MainWidget (this,menumanage,status);
  setView (mainwidget);

  setMenu (bar);
  setStatusBar (status);
}
//*****************************************************************************
void TopWidget::revert ()
{
 if (name) mainwidget->setSignal (name);
}
//*****************************************************************************
void TopWidget::openRecent (const char *str)
{
  KwaveParser parser (str);
  int cnt=parser.toInt();

  if ((cnt>=0)&&(cnt<20))
    {
      QString name=recentFiles.at(cnt);

      if (name)
	{
	  if (this->name) deleteString (this->name);
	  this->name=duplicateString (name.data());
	  mainwidget->setSignal (this->name);
	  setCaption (this->name);
	}
    }
  else debug ("out of range\n");
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
      globals.app->addRecentFile (this->name);
      setCaption (this->name);
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
    }
}
//*****************************************************************************
void TopWidget::saveFile ()
{
  if (name)
    {
      mainwidget->saveSignal (name,bit);
      setCaption (name);
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

	  mainwidget->saveSignal (name,bit,selection);
	  globals.app->addRecentFile (name);
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
  setCaption (name);
  globals.app->addRecentFile (name);
}
//*****************************************************************************
void TopWidget::setSignal (SignalManager *signal)
{
  mainwidget->setSignal (signal);
}
//*****************************************************************************
void TopWidget::updateRecentFiles ()
{
  menumanage->clearNumberedMenu ("recentfiles");
  for (unsigned int i =0 ; i < recentFiles.count(); i++)
    menumanage->addNumberedMenuEntry ("recentfiles",recentFiles.at(i));
}           
//*****************************************************************************
TopWidget::~TopWidget ()
{
  if (saveDir) delete saveDir;
  if (loadDir) delete loadDir;
  if (name)    deleteString (name);
}
//*****************************************************************************
int main( int argc, char **argv )
{
  globals.app=new KwaveApp (argc, argv);

  if (globals.app)
    {
      int result=globals.app->exec();

      return result;
    }
  return -1;
}

