#include "MenuCommand.h"
#include <libkwave/String.h>

MenuCommand::MenuCommand (const char *command,int id)
{
  this->command=duplicateString(command);
  this->id=id;
};
//*****************************************************************************
MenuCommand::~MenuCommand ()
{
  if (command) deleteString (command);
}
//*****************************************************************************
