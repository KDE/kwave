//Kwave main file
//This one includes methods of the Topwidget Class.

#include "main.h"
#include "about.h"
#include <unistd.h>
#include <qkeycode.h>
#include "clipboard.h"

QList<TopWidget>   topwidget; 
QList<MarkerType>  markertypes;
QStrList           recentFiles; 
ClipBoard          *clipboard=0;
QDir               *configDir;
KApplication       *app;

#define KLOC 1000

#define NEWWINDOW      (KLOC+0)
#define OPENFILE       (KLOC+1)
#define SAVEFILE       (KLOC+3)
#define SAVEFILEAS     (KLOC+4)
#define SAVESELECTION  (KLOC+5)
#define REVERT         (KLOC+6)
#define IMPORTASCII    (KLOC+7)
#define QUIT           (KLOC+8)
#define ABOUT          (KLOC+9)
#define HELP           (KLOC+10)
#define PLAYBACK       (KLOC+11)
#define MEMORY         (KLOC+12)
#define BITRES         (KLOC+15)

#define BIT24       (BITRES+0)
#define BIT16       (BITRES+1)
#define BIT8        (BITRES+2)

#define OPENRECENT (KLOC+21)

KWaveMenuItem file_menus[]=
{
  //internalID    ,name                 ,type  ,id  ,shortcut
  {0              ,"&File"              ,KMENU ,-1   ,KEXCLUSIVE},
  {NEW            ,"&New..."            ,KITEM ,-1   ,CTRL+Key_N},
  {NEWWINDOW      ,"New &window"        ,KITEM ,-1   ,CTRL+Key_W},
  {0              ,0                    ,KSEP  ,KSEP ,-1},  
  {OPENFILE       ,"&Open"              ,KITEM ,-1   ,CTRL+Key_O},
  {0              ,"O&pen recent"       ,KMENU ,-1   ,KEXCLUSIVE},
  {OPENRECENT     ,"RecentFiles"        ,KREF  ,-1   ,-1},
  {0              ,0                    ,KEND  ,KEND ,-1},
  {IMPORTASCII    ,"&Import Ascii"      ,KITEM ,-1   ,CTRL+Key_I},
  {0              ,0                    ,KSEP  ,KSEP ,-1},  
  {SAVEFILE       ,"S&ave"              ,KITEM ,-1   ,CTRL+Key_S},
  {0              ,"&Save"              ,KMENU ,-1   ,KEXCLUSIVE},
  {SAVEFILEAS     ,"&As ..."            ,KITEM ,-1   ,CTRL+SHIFT+Key_S},
  {SAVESELECTION  ,"&Selection ..."     ,KITEM ,-1   ,-1},
  {SAVEBLOCKS     ,"&Blocks ..."        ,KITEM ,-1   ,-1},
  {EXPORTASCII    ,"&Export to Ascii"   ,KITEM ,-1   ,CTRL+Key_E},
  {0              ,0                    ,KSEP  ,KSEP ,-1},  
  {0               ,"Save &resolution"   ,KCHECK,-1   ,KEXCLUSIVE},
  {BITRES          ,"Bits"              ,KREF  ,-1   ,-1},
  {0              ,0                    ,KEND  ,KEND ,-1},
  {0              ,0                    ,KEND  ,KEND ,-1},
  {0              ,0                    ,KSEP  ,KSEP ,-1},  
  {REVERT         ,"&Revert"            ,KITEM ,-1   ,CTRL+Key_R},

  {0              ,0                    ,KSEP  ,KSEP ,-1},  
  {QUIT           ,"&Quit"              ,KITEM ,-1   ,CTRL+Key_Q},
  {0              ,0                    ,KEND  ,KEND ,-1},

  {0              ,"&Options"           ,KMENU ,-1   ,KEXCLUSIVE},
  {PLAYBACK       ,"&Playback"          ,KITEM ,-1   ,-1},
  {MEMORY         ,"&Memory"            ,KITEM ,-1   ,-1},
  {0              ,0                    ,KEND  ,KEND ,-1},

  {0              ,"&Help"              ,KMENU ,-1   ,KEXCLUSIVE},
  {HELP           ,"&Contents"          ,KITEM ,-1   ,Key_F1},
  {0              ,0                    ,KSEP  ,KSEP ,-1},  
  {ABOUT          ,"&About"             ,KITEM ,-1   ,-1},
  {0              ,0                    ,KEND  ,KEND ,-1},
  {0,0,0,0,0} //Terminates
};
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
  manage=       new MenuManager (this,bar);

  //this is where the menus are created
  //now  cleaned  up even more, mom ! just a little bit of dirt left

  manage->addNumberedMenu("RecentFiles");
  if (recentFiles.count()>0)
  for ( unsigned int i =0 ; i < recentFiles.count(); i++)
  manage->addNumberedMenuEntry ("RecentFiles",recentFiles.at(i));

  if (manage->addNumberedMenu("Bits"))
    {
      manage->addNumberedMenuEntry ("Bits","&24 Bit");
      manage->addNumberedMenuEntry ("Bits","&16 Bit");
      manage->addNumberedMenuEntry ("Bits"," &8 Bit");
    }


  KDNDDropZone *dropZone = new KDNDDropZone( this , DndURL);
  connect( dropZone, SIGNAL( dropAction( KDNDDropZone *) ),
           this, SLOT( dropEvent( KDNDDropZone *) ) );        

  manage->appendMenus (file_menus);

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
  topwidget.removeRef(this);	

  if (saveDir) delete saveDir;
  if (loadDir) delete loadDir;

  ClipBoard ().unregisterMenu (manage);

  if (topwidget.isEmpty()) app->exit (0); //if list is empty -> no more windows there 
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
void TopWidget::newInstance ()
{
  TopWidget *tnew=new TopWidget();
  topwidget.append (tnew);

  tnew->show();
}
//****************************************************************************
void TopWidget::setOp (int id)
{
  int oldid=id;
  if (manage) id=manage->translateId (file_menus,id);

  if ((id>=OPENRECENT)&&(id<(OPENRECENT+MENUMAX))) openRecent (id-OPENRECENT);

  switch (id)
    {
    case PLAYBACK:
      mainwidget->setOp (PLAYBACKOPTIONS);
      break;
    case MEMORY:
      mainwidget->setOp (MEMORYOPTIONS);
      break;
    case ABOUT:
      about();
      break;
    case HELP:
      getHelp();
      break;
    case BIT24:
      bit=24;
      break;
    case BIT16:
      bit=16;
      break;
    case BIT8:
      bit=8;
      break;
    case NEWWINDOW:
      newInstance ();
      break;
    case REVERT:
      revert ();
      break;
    case OPENFILE:
      openFile ();
      break;
    case SAVEFILE:
      saveFile ();
      break;
    case SAVEFILEAS:
      saveFileAs ();
      break;
    case SAVESELECTION:
      saveFileAs (true);
      break;
    case IMPORTASCII:
      importAsciiFile ();
      break;
    case SAVEBLOCKS:
      id=SAVEBLOCKS+bit; //change id for further use in children
      break;
    }
  
  if ((oldid==id)||(id<KLOC)) //id must be unchanged or global
    {
      mainwidget->setOp (id);
      if (clipboard) clipboard->setOp (id);
    }

  if (id==QUIT) delete this;
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
  // printf ("num is %d\n",num);

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

   for (tmp=topwidget.first();tmp!=0;tmp=topwidget.next())
       tmp->updateRecentFiles(); //update all windows
}           
//*****************************************************************************
void TopWidget::updateRecentFiles ()
{
  manage->clearNumberedMenu ("RecentFiles");
  for (unsigned int i =0 ; i < recentFiles.count(); i++)
    manage->addNumberedMenuEntry ("RecentFiles",recentFiles.at(i));
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
void TopWidget::setSignal (MSignal *signal)
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

  for (unsigned int i =0 ; i < markertypes.count(); i++)
    {
      sprintf (buf,"%dName",i);
      config->writeEntry (buf,markertypes.at(i)->name->data());

      sprintf (buf,"%dhasName",i);
      config->writeEntry (buf,markertypes.at(i)->named);

      sprintf (buf,"%dColor",i);
      config->writeEntry (buf,*((markertypes.at(i))->color));
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
	  markertypes.append (marker);
	}
    }
}
//*****************************************************************************
int main( int argc, char **argv )
{
  app=new KApplication (argc, argv);

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
      else debug ("no local kdedir found \n");

      TopWidget *tnew;

      markertypes.setAutoDelete (true);

      recentFiles=new QStrList (true);
      recentFiles.setAutoDelete (false);
      readConfig (app);

      tnew=new TopWidget();
      topwidget.append (tnew);

      app->setMainWidget (tnew);

      if (argc==2) 
	{
	  QString filename=argv[1];
	  tnew->setSignal     (filename);
	  tnew->addRecentFile (filename.data());
	}
      tnew->show();

      int result=app->exec();

      saveConfig (app);

      return result;
    }
  return -1;
}

