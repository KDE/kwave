#include <qstack.h>
#include "menumanager.h"
#include <kapp.h>
#include <klocale.h>

int unique_menu_id=1<<16; //this should be greater than any id sampleop.h and other internal ids

KWavePopMenu::KWavePopMenu (const char *name,int id,bool checkItems):QPopupMenu()
{
  this->name=new QString (name);
  this->id=id;
  this->memberId=0;
  this->checkItems=checkItems;

  if (checkItems)
    {
      QObject::connect (this,SIGNAL(activated(int)),this,SLOT(check(int)));
      setCheckable (true);
      checked=-1; 
    }
}
//*****************************************************************************
const char *KWavePopMenu::getName ()
{
  return name->data();
}
//*****************************************************************************
void KWavePopMenu::check ()
{
  if (checkItems)
    {
      if (checked<0) checked=idAt (0);
      setItemChecked (checked,true);  
    }
}
//*****************************************************************************
void KWavePopMenu::check (int num)
{
  if (checkItems)
    {
      setItemChecked (checked,false);
      checked=num;
      check ();
    }
}
//*****************************************************************************
int KWavePopMenu::getId ()
{
  return id;
}
//*****************************************************************************
void KWavePopMenu::insertMenu (KWavePopMenu *entry)
{
  children.append (entry);
  this->insertItem (klocale->translate(entry->getName()),entry);
}
//*****************************************************************************
void KWavePopMenu::setMemberId (int id)
{
  memberId=id;
}
//*****************************************************************************
int KWavePopMenu::getMemberId ()
{
  return memberId;
}
//*****************************************************************************
void KWavePopMenu::removeMenu (const char *name)
{
  KWavePopMenu *menu=findMenu(name);
  if (menu)
    {
      int index=menu->getId();
      removeItem(index);
      children.removeRef (menu);
      delete menu;
    }
}
//*****************************************************************************
KWavePopMenu *KWavePopMenu::findMenu (const char *name)
{
  KWavePopMenu *tmp=children.first();

  while (tmp)
    {
      if (strcmp (tmp->getName(),name)==0) return tmp;
      tmp=children.next();
    }
  return 0;
}
//*****************************************************************************
KWavePopMenu::~KWavePopMenu ()
{
  delete name;
}
//*****************************************************************************
NumberedMenu::NumberedMenu (char *name)
{
  objname=new QString (name);
}
//*****************************************************************************
void NumberedMenu::clear ()
{
  entries.clear ();
  refresh ();
}
//*****************************************************************************
void NumberedMenu::notifyMenu (KWavePopMenu *menu)
{
  notifymenus.append (menu);
}
//*****************************************************************************
void NumberedMenu::addEntry (char *entry)
{
  entries.append (new QString (entry));
  refresh ();
}
//*****************************************************************************
void NumberedMenu::refresh ()
{
  //refresh of menus already in menu tree
  //printf ("refreshing numberedMenu %s\n",objname->data());

  KWavePopMenu *tmp=notifymenus.first();

  while (tmp)
    {
      tmp->clear();  //clear menu
      int itemcnt=0;
      int id=tmp->getMemberId();

      QString *entry=entries.first();  //and append all entries from scratch

      while ((entry)&&(itemcnt<MENUMAX))
	{
	  tmp->insertItem (entry->data(),id+itemcnt);
	  entry=entries.next();
	  itemcnt++;
	}

      tmp->check(); //call just in case an update of the checkmark is needed

      tmp=notifymenus.next();
    }
}
//*****************************************************************************
char *NumberedMenu::name ()
{
  return objname->data();
}
//*****************************************************************************
NumberedMenu::~NumberedMenu ()
{
  entries.setAutoDelete (true);
  entries.clear();
  notifymenus.clear();
  delete objname;
}
//*****************************************************************************
MenuManager::MenuManager (QWidget *parent,KMenuBar *bar):QObject (parent)
{
  this->bar=bar;
}
//*****************************************************************************
int MenuManager::getUniqueId ()
{
  int x=unique_menu_id;
  unique_menu_id++;

  return x;
}
//*****************************************************************************
KWavePopMenu  *MenuManager::findMenu (const char *name)
{
  KWavePopMenu *tmp=menus.first();
  while (tmp)
    {
      if (strcmp(tmp->getName(),name)==0) return tmp;
      tmp=menus.next();
    }
  return 0;
}
//*****************************************************************************
void MenuManager::deleteMenus (KWaveMenuItem *newmenus)
{
  int cnt=0;

  KWavePopMenu *menu=0;
  QStack <KWavePopMenu> *stack=new QStack<KWavePopMenu>();

  while (newmenus[cnt].type!=0)
    {
      switch (newmenus[cnt].type)
	{
	case KCHECK:
	case KMENU:
	  {
	    //deleting is only possible if exclusive rights are available
	    if (newmenus[cnt].shortcut==KEXCLUSIVE)
	      {
		if (menu) menu->removeMenu (newmenus[cnt].name);
		else
		  {
		    bar->removeItem(newmenus[cnt].id);

		    KWavePopMenu *tmp=findMenu (newmenus[cnt].name);

		    if (tmp)
		      {
			menus.removeRef (tmp);
			delete tmp;	
		      }
		  }
		//skip all submenus in KWaveMenuItems structure that belong to the deleted menu entry
		    int dep=1;
		    while (dep>0)
		      {
			cnt++;
			if (newmenus[cnt].type==KMENU) dep++;
			if (newmenus[cnt].type==KEND) dep--;
			if (newmenus[cnt].type==0) break;
		      }
		    if (newmenus[cnt].type!=0) cnt--;
	      }
	    else
	      {
		//menu can't be deleted, but perhaps submenus need to be ->push menu.
		KWavePopMenu *newmenu;

		if (menu) newmenu =menu->findMenu (newmenus[cnt].name);
		else newmenu=this->findMenu (newmenus[cnt].name);

		if (newmenu)
		  {
		    stack->push(menu);
		    menu=newmenu;
		  }
		else debug ("menu %s not found for deleting !\n",newmenus[cnt].name);
	      }
	  }
	  break;
	case KITEM:
	  {
	    if (menu) menu->removeItem(newmenus[cnt].id);	    
	    else debug ("error : deleting Item %s at top level\n",newmenus[cnt].name);
	  }
	  break;
	case KSEP:
	  if (menu) //no separators at top level
	    {
	      int index=menu->indexOf (newmenus[cnt+1].id); 
	      if (index>1) menu->removeItemAt (index-1);  //remove separator...
	      //does there exist another way of deleting ?
	      //I guess not, since separators may not have ID's
	      //but as long as it works...
	    }
	  break;
	case KEND:
	  {
	    if (stack->isEmpty()) menu=0;
	    else menu=stack->pop();
	  }
	  break;
	}
      cnt++;
    }
}
//*****************************************************************************
void MenuManager::appendMenus (KWaveMenuItem *newmenus)
{
  int num=bar->count();
  int cnt=0;

  KWavePopMenu *menu=0;
  QStack <KWavePopMenu> *stack=new QStack<KWavePopMenu>();

  while (newmenus[cnt].type!=0)
    {
      switch (newmenus[cnt].type)
	{
	case KCHECK:
	case KMENU:
	  {
	    KWavePopMenu *newmenu=0;

	    if (menu) newmenu=menu->findMenu (newmenus[cnt].name);
	    else newmenu=this->findMenu (newmenus[cnt].name);

	    if (newmenu) if (newmenus[cnt].id==-1) newmenus[cnt].id=newmenu->getId();

	    if (!newmenu) //if menu is not already known, create a new menu
	      {
		newmenu=new KWavePopMenu (newmenus[cnt].name,getUniqueId(),newmenus[cnt].type==KCHECK);
		if (newmenu)
		  {
		    if (newmenus[cnt].id==-1) newmenus[cnt].id=newmenu->getId();
		    if (menu) menu->insertMenu (newmenu);	
		    else
		      {
			bar->insertItem (klocale->translate(newmenus[cnt].name),newmenu,newmenus[cnt].id,num-2);

			menus.append (newmenu); //append to known list of top-level menus
		      }
		    connect (newmenu,SIGNAL(activated(int)),this,SLOT(map(int)));	      
		  }
	      }
	    stack->push(menu);
	    menu=newmenu;
	  }
	  break;
	case KREF:
	  {
	    if (menu)
	      {
		if (newmenus[cnt].id==-1) newmenus[cnt].id=getIdRange (MENUMAX);
		//get known numberedMenu
		NumberedMenu *nummenu=findNumberedMenu ((char *)newmenus[cnt].name);

		if (nummenu)
		  {
		    //insert current Menu to list of menus to be notified, when changes occur...
		    nummenu->notifyMenu (menu);
		    //remember starting id for this menu
		    menu->setMemberId (newmenus[cnt].id);
		    nummenu->refresh ();
		    menu->check ();
		  }
		else debug ("menu structure error : unknown numbered menu !:%s\n",newmenus[cnt].name);
	      }
	    else debug ("Menu structure error : References may not be top level\n");
	  }
	  break;
	case KSEP:
	  if (menu) menu->insertSeparator ();	
	  break;

	case KITEM:
	  {
	    if (menu)
	      {
		if (newmenus[cnt].id==-1) newmenus[cnt].id=getUniqueId ();
		menu->insertItem (klocale->translate(newmenus[cnt].name),newmenus[cnt].id);
		if (newmenus[cnt].shortcut!=-1)
		  menu->setAccel (newmenus[cnt].shortcut,newmenus[cnt].id);
	      }
	    else debug ("Menu Structure Error : Items may not be top level\n");
	  }
	  break;

	case KEND:
	  {
	    if (stack->isEmpty()) menu=0;
	    else menu=stack->pop();
	  }
	  break;
	}
      cnt++; //increase index to get next menu item
    }
}
//*****************************************************************************
void MenuManager::clearNumberedMenu (char *name)
{

  NumberedMenu *menu=findNumberedMenu (name);

  if (menu) menu->clear();
}
//*****************************************************************************
bool MenuManager::addNumberedMenu (char *name)
//returns true, if menu is created from scratch, and false if menu already exists
{
  NumberedMenu *newmenu=findNumberedMenu (name);
  if (newmenu) return false;

  newmenu=new NumberedMenu (name);
  numberedMenus.append (newmenu);
  return true; 
}
//*****************************************************************************
void MenuManager::addNumberedMenuEntry (char *name,char *entry)
{
  NumberedMenu *menu=findNumberedMenu (name);
  if (menu) menu->addEntry (entry);
  else debug ("could not find numbered Menu %s\n",name);
}
//*****************************************************************************
NumberedMenu *MenuManager::findNumberedMenu (char *name)
{
  NumberedMenu *tmp=numberedMenus.first();
  while (tmp)
    {
      if (strcmp(tmp->name(),name)==0) return tmp;
      tmp=numberedMenus.next();
    }

    return 0;
}
//*****************************************************************************
void MenuManager::map (int op)
{
  emit id(op);
}
//*****************************************************************************
int MenuManager::translateId (KWaveMenuItem *menus,int id)
{
  int cnt=0;

  while (menus[cnt].type!=0)
    {
      switch  (menus[cnt].type)
	{
	case KITEM:
	  if (menus[cnt].id==id)
	    {
	      id=menus[cnt].internalID;
	      break;
	    }
	  break;
	case KREF:
	  if ((menus[cnt].id<=id)&&(menus[cnt].id+MENUMAX>id))
	    {
	      id=menus[cnt].internalID+id-menus[cnt].id;
	      break;
	    }
	  break;
	}
      cnt++;
    }
  return id;
}
//*****************************************************************************
int MenuManager::getIdRange (int num)
{
  int x=unique_menu_id;
  unique_menu_id+=num;

  return x;
}
//*****************************************************************************
MenuManager::~MenuManager ()
{
}

