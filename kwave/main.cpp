#include "main.h"
#include "about.h"
#include <unistd.h>
#include <qkeycode.h>

MSignal *clipboard=0;
QList<TopWidget>*topwidget=0; 
QList<MarkerType>*markertypes=0;
QStrList *recentFiles=0; 
QDir *configDir;
QDir *filterDir;
QStrList *filterNameList;
KApplication *app;
//*****************************************************************************************
TopWidget::TopWidget (KApplication *a) : KTopLevelWidget ()
{
  app=a;
  bit=24;

  QPopupMenu *file= new QPopupMenu ();
  QPopupMenu *edit=	new QPopupMenu ();
  QPopupMenu *clip=	new QPopupMenu ();
  QPopupMenu *effects=	new QPopupMenu ();
  QPopupMenu *calculate=new QPopupMenu ();
  QPopupMenu *help=     new QPopupMenu ();
  QPopupMenu *select=	new QPopupMenu ();
  QPopupMenu *amplify=	new QPopupMenu ();
  QPopupMenu *filter=	new QPopupMenu ();
  QPopupMenu *view=	new QPopupMenu ();
  QPopupMenu *options=	new QPopupMenu ();
  QPopupMenu *channel=	new QPopupMenu ();
  QPopupMenu *freq=	new QPopupMenu ();
  QPopupMenu *marker=   new QPopupMenu ();
  QPopupMenu *genmenu=   new QPopupMenu ();
  recent=   new QPopupMenu ();
  save=     new QPopupMenu ();
  channels= new QPopupMenu ();
  mtypemenu=new QPopupMenu ();
  mtypemenu->setCheckable (true);

  MarkerType *act;
  for (act=markertypes->first();act;act=markertypes->next())
	mtypemenu->insertItem(act->name->data());  
  if (markertypes->count()!=0) mtypemenu->setItemChecked (mtypemenu->idAt(0),true);

  channels->insertItem (klocale->translate("none"));
  connect( recent, SIGNAL(activated(int)), SLOT(openRecent(int)) ); 
  connect( mtypemenu, SIGNAL(activated(int)), SLOT(setMarkType(int)) ); 
  connect( channels, SIGNAL(activated(int)), SLOT(deleteChannel(int)) ); 

  status=new KStatusBar (this);

  status->insertItem (klocale->translate("Length: 0 ms           "),1);
  status->insertItem (klocale->translate("Rate: 0 kHz         "),2);
  status->insertItem (klocale->translate("Samples: 0             "),3);
  status->insertItem (klocale->translate("selected: 0 ms        "),4);
  status->insertItem (klocale->translate("Clipboard: 0 ms      "),5);

  bar=		new KMenuBar (this);
  mainwidget=	new MainWidget (this,"MainView",status);
  connect( mainwidget, SIGNAL(channelInfo(int)),this, SLOT(getChannelInfo(int)) ); 

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
  file->insertItem	(klocale->translate("Save"),	save);
  
  save->insertItem	(klocale->translate("As ..."),this,SLOT(saveFileas()),CTRL+SHIFT+Key_S);
  save->insertItem	(klocale->translate("Selection ..."),this,SLOT(saveSelection()));
  save->insertItem	(klocale->translate("Blocks ..."),this,SLOT(saveBlocksOp()));
  save->insertSeparator	();
  bit24=save->insertItem (klocale->translate("24 bit"));
  bit16=save->insertItem (klocale->translate("16 bit"));
  bit8=save->insertItem	 (klocale->translate(" 8 bit"));
  save->setCheckable (true);
  save->setItemChecked (bit24,true);
  save->connectItem (bit24,this,SLOT(save24Bit()));
  save->connectItem (bit16,this,SLOT(save16Bit()));
  save->connectItem (bit8,this,SLOT(save8Bit()));
  
  file->insertSeparator	();
  file->insertItem	(klocale->translate("Revert"),	this,SLOT(revert()),CTRL+Key_R);
  file->insertSeparator	();
  file->insertItem	(klocale->translate("&Quit"),	this,SLOT(quitInstance()),CTRL+Key_Q);

  edit->insertItem	(klocale->translate("Cut"),	this,SLOT(cutOp()),CTRL+Key_X);
  edit->insertItem	(klocale->translate("Copy"),	this,SLOT(copyOp()),CTRL+Key_C);
  edit->insertItem	(klocale->translate("Paste"),	this,SLOT(pasteOp()),CTRL+Key_V);
  edit->insertItem	(klocale->translate("Mix & Paste"),this,SLOT(mixpasteOp()),CTRL+SHIFT+Key_V);

  edit->insertItem	(klocale->translate("Trim"),	this,SLOT(cropOp()),CTRL+Key_T);
  edit->insertItem	(klocale->translate("Delete"),	this,SLOT(deleteOp()),Key_Delete);
  edit->insertSeparator	();
  edit->insertItem	(klocale->translate("Flip Phase"), this,SLOT(flipOp()));
  edit->insertItem	(klocale->translate("Center Wave"),this,SLOT(centerOp()),Key_C);
  edit->insertItem	(klocale->translate("Resample"),this,SLOT(resampleOp()),SHIFT+Key_R);
  edit->insertSeparator	();

  edit->insertItem	(klocale->translate("Channel"),	channel);
  channel->insertItem	(klocale->translate("add")   ,this,SLOT(addChannelOp()),SHIFT+Key_A);
  channel->insertItem	(klocale->translate("delete"),channels);
  channel->insertItem	(klocale->translate("select all"),this,SLOT(allChannelOp()),Key_A);
  channel->insertItem	(klocale->translate("invert all"),this,SLOT(toggleChannelOp()),Key_I);

  edit->insertItem	(klocale->translate("Select"),	select);
  select->insertItem	(klocale->translate("&All"),    this,SLOT(selectAllOp()),CTRL+Key_A);
  select->insertItem	(klocale->translate("Range"),	this,SLOT(selectRangeOp()),Key_R);
  select->insertItem	(klocale->translate("Visible area"),	this,SLOT(selectVisibleOp()),Key_V);
  select->insertItem	(klocale->translate("Next"),	this,SLOT(selectNextOp()),Key_Plus);
  select->insertItem	(klocale->translate("Prev"),	this,SLOT(selectPrevOp()),Key_Minus);
  select->insertItem	(klocale->translate("None"),	this,SLOT(selectNoneOp()),Key_N);
  edit->insertSeparator	();
  clip->insertItem	(klocale->translate("Flush"),	this,SLOT(flushClip()));
  clip->insertItem	(klocale->translate("To new window"),this,SLOT(cliptoNew()));
  edit->insertItem	(klocale->translate("Clipboard"),clip);

  view->insertItem	(klocale->translate("Next page"),    this,SLOT(nextPageOp()),Key_PageUp);
  view->insertItem	(klocale->translate("Previous page"),this,SLOT(prevPageOp()),Key_PageDown);
  view->insertItem	(klocale->translate("Scroll right"),this,SLOT(scrollRightOp()),Key_Right);
  view->insertItem	(klocale->translate("Scroll left"), this,SLOT(scrollLeftOp()),Key_Left);
  view->insertSeparator	();
  view->insertItem	(klocale->translate("Zoom In"),this,SLOT(zoomInOp()),CTRL+Key_Plus);
  view->insertItem	(klocale->translate("Zoom Out"), this,SLOT(zoomOutOp()),CTRL+Key_Minus);
  view->insertItem	(klocale->translate("Zoom to Range"), this,SLOT(zoomRangeOp()),CTRL+Key_Space);

  effects->insertItem	(klocale->translate("Amplify"),	amplify);

  amplify->insertItem	(klocale->translate("Distort"),	this,SLOT(distortOp()));
  amplify->insertItem	(klocale->translate("Fade In"), this,SLOT(fadeInOp()));
  amplify->insertItem	(klocale->translate("Fade Out"),this,SLOT(fadeOutOp()));
  amplify->insertItem	(klocale->translate("Free"),	this,SLOT(amplifyOp()));
  amplify->insertItem	(klocale->translate("To maximum"),this,SLOT(amplifyMaxOp()),Key_M);
  amplify->insertItem	(klocale->translate("With clipboard"),this,SLOT(amplifyClipOp()));

  effects->insertSeparator ();
  effects->insertItem	(klocale->translate("Filter"), filter);
  filter->insertItem    (klocale->translate("moving Average"),this,SLOT(mAverageFilterOp()));
  filter->insertItem    (klocale->translate("Create"),this,SLOT(filterCreateOp()));
  
  filter->insertSeparator ();
  
  filterDir=new QDir(configDir->path());

  if (!filterDir->cd ("presets"))
    {
      filterDir->mkdir ("presets");
      filterDir->cd ("presets");
    }
  if (!filterDir->cd ("filters"))
    {
      filterDir->mkdir ("filters");
      filterDir->cd ("filters");
    }

  filterDir->setNameFilter ("*.filter");

  filterNameList=(QStrList *)filterDir->entryList ();

  for (char *tmp=filterNameList->first();tmp!=0;tmp=filterNameList->next())
    {
      char buf[strlen(tmp)-6];
      strncpy (buf,tmp,strlen(tmp)-6);
      buf[strlen(tmp)-7]=0;
      filter->insertItem (buf);
    }

  connect( filter, SIGNAL(activated(int)), SLOT(doFilter(int)) ); 

  effects->insertSeparator	();
  effects->insertItem	(klocale->translate("&Delay"),	this,SLOT(delayOp()));
  effects->insertItem	(klocale->translate("Reverse"),	this,SLOT(reverseOp()));
  effects->insertItem	(klocale->translate("&Periodic Silence"),	this,SLOT(stutterOp()));

  effects->insertSeparator	();
  effects->insertItem	(klocale->translate("&Change Rate"),this,SLOT(rateChangeOp()));
  effects->insertItem	(klocale->translate("Re&quantize"),	this,SLOT(requantizeOp()));

   calculate->insertItem	(klocale->translate("&Silence"),	this,SLOT(zeroOp()));
  calculate->insertItem	(klocale->translate("&Noise"),	this,SLOT(noiseOp()));
  calculate->insertItem	(klocale->translate("&Additive Synthesis"),this,SLOT(addSynthOp()));
  calculate->insertItem	(klocale->translate("&Pulse Series"),this,SLOT(pulseOp()));
  calculate->insertSeparator	();
  calculate->insertItem	(klocale->translate("&Hullcurve"),this,SLOT(hullCurveOp()));
  calculate->insertSeparator	();
  calculate->insertItem	(klocale->translate("Frequencies"),freq);
  freq->insertItem	(klocale->translate("&Spectrum"),	this,SLOT(fftOp()));
  freq->insertItem	(klocale->translate("&Average Spectrum"),	this,SLOT(averageFFTOp()));
  freq->insertItem	(klocale->translate("S&onagram"),	this,SLOT(sonagramOp()));

  marker->insertItem	(klocale->translate("&Add"),	this,SLOT(addMarkOp()),Key_A);
  marker->insertItem	(klocale->translate("&Delete"),	this,SLOT(deleteMarkOp()),Key_D);
  marker->insertSeparator	();
  marker->insertItem	(klocale->translate("&Generate"),genmenu);
  genmenu->insertItem	(klocale->translate("&Signal Markers"),this,SLOT(signalMarkerOp()));
  genmenu->insertItem	(klocale->translate("&Period Markers"),this,SLOT(periodMarkerOp()));
  marker->insertSeparator	();
  marker->insertItem	(klocale->translate("&Load"),	this,SLOT(loadMarkOp()));
  marker->insertItem	(klocale->translate("&Insert"),	this,SLOT(appendMarkOp()));
  marker->insertItem	(klocale->translate("&Save"),	this,SLOT(saveMarkOp()));
  marker->insertItem	(klocale->translate("&Save Label Frequency"),
			 this,SLOT(savePeriodsOp()));
  marker->insertSeparator	();
  marker->insertItem	(klocale->translate("&Change Type"),mtypemenu);
  marker->insertItem	(klocale->translate("Create &Type"),this,SLOT(addMarkType()));

  options->insertItem	(klocale->translate("Playback"),this,SLOT(playBackOp()));

  help->insertItem	(klocale->translate("&Contents"),this,SLOT(getHelp()));
  help->insertSeparator	();
  help->insertItem	(klocale->translate("&About kwave"),	this,SLOT(about()));

  bar->insertItem	(klocale->translate("&File"),	file);
  bar->insertItem	(klocale->translate("&Edit"),	edit);
  bar->insertItem	(klocale->translate("&View"),	view);
  bar->insertItem	(klocale->translate("F&x"),	effects);
  bar->insertItem	(klocale->translate("&Calculate"),calculate);
  bar->insertItem	(klocale->translate("&Labels") ,marker);
  bar->insertItem	(klocale->translate("&Options") ,options);
  bar->insertItem	(klocale->translate("&Help"),	help);

  KDNDDropZone *dropZone = new KDNDDropZone( this , DndURL);
  connect( dropZone, SIGNAL( dropAction( KDNDDropZone *) ),
           this, SLOT( dropEvent( KDNDDropZone *) ) );        

  setView (mainwidget);
  setMenu (bar);
  setStatusBar (status);
}
//*****************************************************************************************
TopWidget::~TopWidget ()
{
  //remove this instance from list of widgets
  topwidget->removeRef(this);	

  if (topwidget->isEmpty()) app->exit (0); //if list is empty -> no more windows there 
}
//*****************************************************************************************
void TopWidget::about ()
{
  AboutDialog dialog (this);
  dialog.exec ();
}
//*****************************************************************************************
void TopWidget::getHelp ()
{
  app->invokeHTMLHelp ("kwave/index.html","");
}
//*****************************************************************************************
void TopWidget::quitInstance ()
{
  delete this;
}
//*****************************************************************************************
void TopWidget::newInstance ()
{
  TopWidget *tnew=new TopWidget(app);
  topwidget->append (tnew);

  tnew->show();
}
//*****************************************************************************************
void TopWidget::cliptoNew ()
{
  if (clipboard)
   {
    TopWidget *tnew=new TopWidget(app);
    topwidget->append (tnew);
    tnew->setSignal (clipboard);

    tnew->show();
    tnew->setCaption (klocale->translate("Clipboard"));
    clipboard=0;
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
void TopWidget::mixpasteOp	()	{mainwidget->setRangeOp	(MIXPASTE);}
void TopWidget::centerOp	()	{mainwidget->setRangeOp (CENTER);}
void TopWidget::reverseOp	()	{mainwidget->setRangeOp (REVERSE);}
void TopWidget::fadeInOp	()	{mainwidget->setRangeOp	(FADEIN);}
void TopWidget::fadeOutOp	()	{mainwidget->setRangeOp	(FADEOUT);}
void TopWidget::selectAllOp	()	{mainwidget->setRangeOp	(SELECTALL);}
void TopWidget::selectRangeOp	()	{mainwidget->setRangeOp	(SELECTRANGE);}
void TopWidget::selectVisibleOp	()	{mainwidget->setRangeOp	(SELECTVISIBLE);}
void TopWidget::selectNoneOp	()	{mainwidget->setRangeOp	(SELECTNONE);}
void TopWidget::selectNextOp	()	{mainwidget->setRangeOp	(SELECTNEXT);}
void TopWidget::selectPrevOp	()	{mainwidget->setRangeOp	(SELECTPREV);}
void TopWidget::amplifyOp	()	{mainwidget->setRangeOp	(AMPLIFY);}
void TopWidget::amplifyMaxOp	()	{mainwidget->setRangeOp	(AMPLIFYMAX);}
void TopWidget::amplifyClipOp	()	{mainwidget->setRangeOp	(AMPWITHCLIP);}
void TopWidget::noiseOp		()	{mainwidget->setRangeOp	(NOISE);}
void TopWidget::addSynthOp	()	{mainwidget->setRangeOp	(ADDSYNTH);}
void TopWidget::pulseOp  	()	{mainwidget->setRangeOp	(PULSE);}
void TopWidget::distortOp	()	{mainwidget->setRangeOp	(DISTORT);}
void TopWidget::hullCurveOp	()	{mainwidget->setRangeOp	(HULLCURVE);}
void TopWidget::delayOp		()	{mainwidget->setRangeOp	(DELAY);}
void TopWidget::rateChangeOp	()	{mainwidget->setRangeOp	(RATECHANGE);}
void TopWidget::fftOp		()	{mainwidget->setRangeOp	(FFT);}
void TopWidget::playBackOp	()	{mainwidget->setRangeOp (PLAYBACKOPTIONS);}
void TopWidget::addChannelOp	()	{mainwidget->setRangeOp (ADDCHANNEL);}
void TopWidget::allChannelOp	()	{mainwidget->setRangeOp (ALLCHANNEL);}
void TopWidget::toggleChannelOp	()	{mainwidget->setRangeOp (INVERTCHANNEL);}
void TopWidget::mAverageFilterOp()	{mainwidget->setRangeOp (MOVINGAVERAGE);}
void TopWidget::sonagramOp      ()	{mainwidget->setRangeOp (SONAGRAM);}
void TopWidget::resampleOp      ()	{mainwidget->setRangeOp (RESAMPLE);}
void TopWidget::addMarkOp       ()	{mainwidget->setRangeOp (ADDMARK);}
void TopWidget::loadMarkOp      ()	{mainwidget->setRangeOp (LOADMARK);}
void TopWidget::appendMarkOp    ()	{mainwidget->setRangeOp (APPENDMARK);}
void TopWidget::saveMarkOp      ()	{mainwidget->setRangeOp (SAVEMARK);}
void TopWidget::deleteMarkOp    ()	{mainwidget->setRangeOp (DELETEMARK);}
void TopWidget::scrollLeftOp    ()	{mainwidget->setRangeOp (SCROLLLEFT);}
void TopWidget::scrollRightOp   ()	{mainwidget->setRangeOp (SCROLLRIGHT);}
void TopWidget::nextPageOp      ()	{mainwidget->setRangeOp (NEXTPAGE);}
void TopWidget::prevPageOp      ()	{mainwidget->setRangeOp (PREVPAGE);}
void TopWidget::zoomInOp        ()	{mainwidget->setRangeOp (ZOOMIN);}
void TopWidget::zoomOutOp       ()	{mainwidget->setRangeOp (ZOOMOUT);}
void TopWidget::zoomRangeOp     ()	{mainwidget->setRangeOp (ZOOMRANGE);}
void TopWidget::filterCreateOp  ()	{mainwidget->setRangeOp (FILTERCREATE);}
void TopWidget::averageFFTOp    ()	{mainwidget->setRangeOp (AVERAGEFFT);}
void TopWidget::stutterOp       ()	{mainwidget->setRangeOp (STUTTER);}
void TopWidget::requantizeOp    ()	{mainwidget->setRangeOp (REQUANTISE);}
void TopWidget::signalMarkerOp  ()	{mainwidget->setRangeOp (MARKSIGNAL);}
void TopWidget::periodMarkerOp  ()	{mainwidget->setRangeOp (MARKPERIOD);}
void TopWidget::saveBlocksOp    ()	{mainwidget->setRangeOp (SAVEBLOCKS+bit);}
void TopWidget::savePeriodsOp   ()	{mainwidget->setRangeOp (SAVEPERIODS);}
//*****************************************************************************************
void TopWidget::doFilter    (int num)
 {
   if (num>=3)
     mainwidget->setRangeOp (FILTER+num-3); //ignore first three items, since they are covered by own signals ...
 }
//*****************************************************************************************
void TopWidget::setMarkType    (int num)
{
  for (unsigned int i=0;i<markertypes->count();i++) mtypemenu->setItemChecked (mtypemenu->idAt(i),false);
  mtypemenu->setItemChecked (mtypemenu->idAt(num),true);
  mainwidget->setRangeOp (SELECTMARK+num);
}
//*****************************************************************************************
void TopWidget::getChannelInfo	(int num)
{
  char buf[8];
  channels->clear();

  for (int i =0 ; i < num; i++)
    {
      sprintf (buf,"%d",i);
      channels->insertItem (buf);
    }
}
//*****************************************************************************************
void TopWidget::deleteChannel	(int num)
{
  mainwidget->setRangeOp (DELETECHANNEL+num);
}
//*****************************************************************************************
void TopWidget::revert ()
{
 if (!name.isNull())
  {
	mainwidget->setSignal (&name);
  }
}
//*****************************************************************************************
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
//*****************************************************************************************
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
//*****************************************************************************************
void TopWidget::updateRecentFiles ()
{
  recent->clear();

  recent->insertItem	(klocale->translate("&New ..."),this,SLOT(openFile()),CTRL+Key_O);
  recent->insertSeparator	();

  for (unsigned int i =0 ; i < recentFiles->count(); i++)
      recent->insertItem (recentFiles->at(i));
}           
//*****************************************************************************************
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
//*****************************************************************************************
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
//*****************************************************************************************
void TopWidget::save24Bit ()
{
  save->setItemChecked (bit24,true);
  save->setItemChecked (bit16,false);
  save->setItemChecked (bit8,false);
  bit=24;
}
//*****************************************************************************************
void TopWidget::save16Bit ()
{
  save->setItemChecked (bit24,false);
  save->setItemChecked (bit16,true);
  save->setItemChecked (bit8,false);
  bit=16;
}
//*****************************************************************************************
void TopWidget::save8Bit ()
{
  save->setItemChecked (bit24,false);
  save->setItemChecked (bit16,false);
  save->setItemChecked (bit8,true);
  bit=8;
}
//*****************************************************************************************
void TopWidget::saveFile ()
{
 if (!name.isEmpty())
  {
	this->name=name;
	mainwidget->saveSignal (&name,bit);
	setCaption (name.data());
  }
}
//*****************************************************************************************
void TopWidget::saveFileas ()
{
  name=QFileDialog::getSaveFileName (0,"*.wav",this);
  if (!name.isNull())
    {
      mainwidget->saveSignal (&name,bit);
      addRecentFile (name.data());
    }
}
//*****************************************************************************************
void TopWidget::saveSelection ()
{
 name=QFileDialog::getSaveFileName (0,"*.wav",this);
 if (!name.isNull())
 mainwidget->saveSignal (&name,bit,true);
}
//*****************************************************************************************
void TopWidget::setSignal (QString name)
{
 this->name=name;
 mainwidget->setSignal (&name);
 setCaption (name.data());
}
//*****************************************************************************************
void TopWidget::addMarkType (struct MarkerType *marker)
{
	  markertypes->append (marker);
	  mtypemenu->insertItem (marker->name->data());
}
//*****************************************************************************************
void TopWidget::addMarkType ()
{
  MarkerTypeDialog dialog (this);

  if (dialog.exec())
    {
      MarkerType *marker=new MarkerType();

      marker->name=new QString (dialog.getName());
      if (!marker->name->isEmpty())
	{
	  marker->named=dialog.getIndividual();
	  marker->color=new QColor(dialog.getColor());

	  addMarkType (marker);
	}
      else delete marker;
    }
}
//*****************************************************************************************
void TopWidget::setSignal (MSignal *signal)
{
 mainwidget->setSignal (signal);
}
//*****************************************************************************************
extern int play16bit;
extern int bufbase;
//*****************************************************************************************
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
//*****************************************************************************************
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
//*****************************************************************************************
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
