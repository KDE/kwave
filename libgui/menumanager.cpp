#include <stdlib.h>
#include <string.h>
#include <qstack.h>
#include "menumanager.h"
#include <libkwave/parser.h>
#include <libkwave/kwavestring.h>
#include <kapp.h>
#include <klocale.h>

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
  KwavePopMenu *tmp=notifymenus.first();

  while (tmp)
    {
      tmp->clear();  //clear menu
      int itemcnt=0;
      int id=tmp->getMemberId();

      const char *entry=entries.first();  //and insert all entries from scratch

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
int parseToKeyCode (char *key)
  //parse the key string into integer qt keycode
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
	if (!strcmp (&key[pos],"PLUS")) keycode+=Key_Plus;
	if (!strcmp (&key[pos],"MINUS")) keycode+=Key_Minus;
	if (!strcmp (&key[pos],"SPACE")) keycode+=Key_Space;
	if (!strcmp (&key[pos],"CTRL")) keycode+=CTRL;
	if (!strcmp (&key[pos],"PAGEUP")) keycode+=Key_PageUp;
	if (!strcmp (&key[pos],"PAGEDOWN")) keycode+=Key_PageDown;
	if (!strcmp (&key[pos],"UP")) keycode+=Key_Up;
	if (!strcmp (&key[pos],"DEL")) keycode+=Key_Delete;
	if (!strcmp (&key[pos],"DOWN")) keycode+=Key_Down;
	if (!strcmp (&key[pos],"LEFT")) keycode+=Key_Left;
	if (!strcmp (&key[pos],"RIGHT")) keycode+=Key_Right;
	if (!strcmp (&key[pos],"SHIFT")) keycode+=SHIFT;

	cnt++;
      }
  return keycode;
}
//*****************************************************************************
void MenuManager::setCommand (const char *command)
{
  if (bar)
    {
      //      printf  ("%s\n",command);
      KwaveParser parser(command);

      const char *tmp;
      char *pos=0;
      char *key=0;
      char *com=0;

      tmp=parser.getFirstParam ();
      if (tmp) com=duplicateString (tmp);

      tmp=parser.getNextParam ();
      if (tmp) pos=duplicateString (tmp);

      tmp=parser.getNextParam ();
      if (tmp) key=duplicateString (tmp);

      if (pos) //if position of menu is given
	{
	  KwavePopMenu *parentmenu=0;
	  KwavePopMenu *newmenu=0;

	  int len=strlen(pos);
	  int cnt=0;

	  //parse the string pos into menu structure and insert accordingly

	  fflush (stdout);

	  while (cnt<len)
	    {
	      int begin=cnt;
	      while ((pos[cnt])&&(pos[cnt]!='/')) cnt++;
	      if (cnt<len) pos[cnt]=0;

	      if (parentmenu)
		{
		    if ((strncmp (&pos[begin],"listmenu",8)==0))
		      {
			//create a numbered Menu and connect it to the parent
			KwaveParser getname(&pos[begin]);
			NumberedMenu *newmenu=
			  addNumberedMenu (getname.getFirstParam());
			if (newmenu)
			  {
			    newmenu->notifyMenu (parentmenu);
			    parentmenu->setCommand (com);
			  }
			else debug ("creation of listmenu failed\n");
		      }
		    else
		    if ((strcmp (&pos[begin],"exclusive")==0))
		      parentmenu->checkable ();
		    else
		    if ((strcmp (&pos[begin],"number")==0))
		      parentmenu->numberable ();
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

			    if (key) keycode=parseToKeyCode (key);
			  
			    // insert the entry into the current parent menu
//			    debug("new entry='%s'", pos+begin); // ###
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
//		      debug("new menu='%s'", pos+begin); // ###
		      if (parentmenu) parentmenu->insertMenu (newmenu);
		      else
			{
			  bar->insertItem (klocale->translate(&pos[begin]),newmenu);
			  toplevelmenus.append (newmenu);
			  //append to known list of top-level menus
			}
		      //		      connect (newmenu,SIGNAL(command(const char *)),
		      //       this,SLOT(deliverCommand(const char *)));
		    }
		  else debug ("creation of menu failed !\n");
		}

	      parentmenu=newmenu;

	      cnt++;
	    }

	}
      else debug ("no position field !\n");

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
void MenuManager::checkMenuEntry (const char *name,bool check)
{

  KwavePopMenu *top=toplevelmenus.first();
  while (top)
    {
//      fprintf(stderr, "toplevel menu name='%s'\n", top->getName()); // ###
      if (strcmp(top->getName(),"&File")==0)
	{
	  debug("menu found!\n");
	}

//      KwavePopMenu *child = top->

      top=toplevelmenus.next();
    }



  //straight forward linear search

  NumberedMenu *tmp=numberedMenus.first();
  while (tmp)
    {
      if (strcmp(tmp->name(),name)==0) break;
      tmp=numberedMenus.next();
    }

}
//*****************************************************************************
void MenuManager::clearNumberedMenu (const char *name)
{

  NumberedMenu *menu=findNumberedMenu (name);

  if (menu) menu->clear();
}
//*****************************************************************************
NumberedMenu* MenuManager::addNumberedMenu (const char *name)
{
  NumberedMenu *newmenu=findNumberedMenu (name);

  if (newmenu) return newmenu;

  newmenu=new NumberedMenu (name);
  numberedMenus.append (newmenu);
  return newmenu; 
}
//*****************************************************************************
void MenuManager::addNumberedMenuEntry (const char *name,const char *entry)
{
  NumberedMenu *menu=findNumberedMenu (name);
  if (menu) menu->addEntry (entry);
  else debug ("could not find numbered Menu %s\n",name);
}
//*****************************************************************************
NumberedMenu *MenuManager::findNumberedMenu (const char *name)
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
//void MenuManager::deliverCommand (const char *c)
//{
//  emit command(c);
//}
//*****************************************************************************
MenuManager::~MenuManager ()
{
}
