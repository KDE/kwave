#ifndef _MENU_COMMAND_H_
#define _MENU_COMMAND_H_ 1

class MenuCommand
{
 public:
  MenuCommand::MenuCommand (const char *,int);
  MenuCommand::~MenuCommand ();
  inline int           getId       () const {return id;};  
  inline const char   *getCommand  () const {return command;};
 private:
  char *command;
  int  id;
};

#endif // _MENU_COMMAND_H_
