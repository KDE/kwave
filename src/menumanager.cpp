#include <stdlib.h>
#include <string.h>
#include <qstack.h>
#include "menumanager.h"
#include "../lib/parser.h"
#include <kapp.h>
#include <klocale.h>

//#define BUG

NumberedMenu::NumberedMenu (const char *name)
{
  objname=duplicateString (name);
}
//*****************************************************************************
void NumberedMenu::clear ()
{
  entries.clear ();
  refresh ();
}
//*****************************************************************************
void NumberedMenu::notifyMenu (KwavePopMenu *menu)
{
  notifymenus.append (menu);
}
//*****************************************************************************
void NumberedMenu::addEntry (const char *entry)
{
  entries.append (duplicateString (entry));
  refresh ();
}
//*****************************************************************************
void NumberedMenu::refresh ()
{
  //refresh of menus already in menu tree
  //printf ("refreshing numberedMenu %s\n",objname->data());

  KwavePopMenu *tmp=notifymenus.first();

  while (tmp)
    {
      tmp->clear();  //clear menu
      int itemcnt=0;
      int id=tmp->getMemberId();

      const char *entry=entries.first();  //and append all entries from scratch

      while ((entry)&&(itemcnt<MENUMAX))
	{
	  tmp->insertItem (entry,id+itemcnt);
	  entry=entries.next();
	  itemcnt++;
	}

      tmp->check(); //call just in case an update of the checkmark is needed

      tmp=notifymenus.next();
    }
}
//*****************************************************************************
const char *NumberedMenu::name ()
{
  return objname;
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
  toplevelmenus.clear ();
  numberedMenus.clear ();
}
//*****************************************************************************
KwavePopMenu  *MenuManager::findMenu (const char *name)
{
  KwavePopMenu *tmp=toplevelmenus.first();
  while (tmp)
    {
      if (strcmp(tmp->getName(),name)==0) return tmp;
      tmp=toplevelmenus.next();
    }
  return 0;
}
//*****************************************************************************
void MenuManager::deleteMenus (KwaveMenuItem *newmenus)
{
}
//*****************************************************************************
int parseToCode (char *key)
  //parse the key string into qt keycode
{
  int keycode=0;
  int len=strlen(key);
  int cnt=0;
  if (len==1) keycode=Key_A+key[0]-'A';
  else
    while (cnt<len)
      {
	int pos=cnt;
	while ((key[cnt])&&(key[cnt]!='+')) cnt++;		    
	if (cnt<len) key[cnt]=0;
	if (strlen(&key[pos])==1)
	  {
	    if ((key[pos]>='A')&&(key[pos]<='Z'))	keycode+=Key_A+key[pos]-'A';
	  }			  
	if (strcmp (&key[pos],"PLUS")==0) keycode+=Key_Plus;
	if (strcmp (&key[pos],"MINUS")==0) keycode+=Key_Minus;
	if (strcmp (&key[pos],"SPACE")==0) keycode+=Key_Space;
	if (strcmp (&key[pos],"CTRL")==0) keycode+=CTRL;
	if (strcmp (&key[pos],"PAGEUP")==0) keycode+=Key_PageUp;
	if (strcmp (&key[pos],"PAGEDOWN")==0) keycode+=Key_PageDown;
	if (strcmp (&key[pos],"UP")==0) keycode+=Key_Up;
	if (strcmp (&key[pos],"DEL")==0) keycode+=Key_Delete;
	if (strcmp (&key[pos],"DOWN")==0) keycode+=Key_Down;
	if (strcmp (&key[pos],"LEFT")==0) keycode+=Key_Left;
	if (strcmp (&key[pos],"RIGHT")==0) keycode+=Key_Right;
	if (strcmp (&key[pos],"SHIFT")==0) keycode+=SHIFT;

	cnt++;
      }
  return keycode;
}
//*****************************************************************************
void MenuManager::setCommand (const char *command)
{
  if (bar)
    {
      printf  ("%s\n",command);
      KwaveParser parser(command);

      const char *tmp;
      char *pos=0;
      char *key=0;
      char *com=0;
#ifndef BUG
      tmp=parser.getFirstParam ();
      if (tmp) com=duplicateString (tmp);

      tmp=parser.getNextParam ();
      if (tmp) pos=duplicateString (tmp);
      tmp=parser.getNextParam ();
      if (tmp) key=duplicateString (tmp);

      printf ("%p\n",pos);

      if (pos) //if position of menu is given
	{


	  KwavePopMenu *parentmenu=0;
	  KwavePopMenu *newmenu=0;

	  int len=strlen(pos);
	  int cnt=0;

	  //parse the string pos into menu structure and insert accordingly

	  while (cnt<len)
	    {
	      int begin=cnt;
	      while ((pos[cnt])&&(pos[cnt]!='/')) cnt++;
	      if (cnt<len) pos[cnt]=0;

	      if (parentmenu)
		{
		  if ((strcmp (&pos[begin],"exclusive")==0))
		    parentmenu->checkable ();
		  else
		    if (strcmp (&pos[begin],"separator")==0)
		      {
			parentmenu->insertSeparator ();
			cnt=len;
		      }
		    else
		      if (cnt+1>=len)
			{
			  int keycode=0;

			  if (key) keycode=parseToCode (key);
			  
			  printf ("code is %d\n",keycode);

			  // insert the entry into the current parent menu
			  if (parentmenu)
			    parentmenu->insertEntry (&pos[begin],com,keycode);
			}
		      else
			newmenu=parentmenu->findMenu (&pos[begin]);
		}
	      else //is a top-level window ?
		newmenu=this->findMenu (&pos[begin]);

	      if (!newmenu) //if menu is not already known, create a new menu
		{
		  newmenu=new KwavePopMenu (&pos[begin],KwavePopMenu::getUniqueId());
		  if (newmenu)
		    {
		      if (parentmenu) parentmenu->insertMenu (newmenu);
		      else
			{
			  bar->insertItem (klocale->translate(&pos[begin]),newmenu);
			  connect (newmenu,SIGNAL(command(const char *)),
				   this,SLOT(deliverCommand(const char *)));

			  toplevelmenus.append (newmenu);
			  //append to known list of top-level menus
			}
		    }
		  else debug ("creation of menu failed !\n");
		}

	      parentmenu=newmenu;

	      cnt++;
	    }

	}
      else debug ("no position field !\n");
#endif
      if (com) deleteString (com);
      if (pos) deleteString (pos);
      if (key) deleteString (key);

    }
  else debug ("Menumanager:no menu bar, somthing went wrong !\n");
}
//*****************************************************************************
void MenuManager::appendMenus (KwaveMenuItem *newmenus)
{
  if (bar)
    {
      int cnt=0;

      KwavePopMenu *menu=0;
      while (newmenus[cnt].type!=0)
	{
	  switch (newmenus[cnt].type)
	    {
	    case KREF:
	      {
		if (menu)
		  {
		    if (newmenus[cnt].id==-1) newmenus[cnt].id=KwavePopMenu::getIdRange (MENUMAX);
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
	    case KITEM:
	      {
		if (menu)
		  {
		    if (newmenus[cnt].id==-1) newmenus[cnt].id=KwavePopMenu::getUniqueId ();

		    menu->insertItem ("My guess is there is a bug in qtlib/setAccel !",newmenus[cnt].id);
		    if (newmenus[cnt].shortcut!=-1) //check for existence qt shortcut code
		    menu->setAccel (newmenus[cnt].shortcut,newmenus[cnt].id);

		    menu->changeItem (klocale->translate(newmenus[cnt].name),newmenus[cnt].id);

		    //debug ("menu is %p %s has code %d",menu,newmenus[cnt].name,newmenus[cnt].shortcut);
		    //debug ("really: %d",menu->accel(newmenus[cnt].id));
		    //this replies the value given to setaccel. 
		  }
		else debug ("Menu Structure Error : Items may not be top level\n");
	      }
	      break;
	    }
	  cnt++; //increase index to get next menu item
	}
    }
  else debug ("no menu bar!\n");
}
//*****************************************************************************
void MenuManager::clearNumberedMenu (char *name)
{

  NumberedMenu *menu=findNumberedMenu (name);

  if (menu) menu->clear();
}
//*****************************************************************************
bool MenuManager::addNumberedMenu (char *name)
//returns true, if menu is created from scratch,
//        false if menu already exists
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
  //straight forward linear search

  NumberedMenu *tmp=numberedMenus.first();
  while (tmp)
    {
      if (strcmp(tmp->name(),name)==0) return tmp;
      tmp=numberedMenus.next();
    }

    return 0;
}
//*****************************************************************************
void MenuManager::deliverCommand (const char *c)
{
  emit command(c);
}
//*****************************************************************************
MenuManager::~MenuManager ()
{
}







