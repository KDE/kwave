#include "commandqueue.h"

CommandQueue::CommandQueue ()
{
  first=0;
  last=0;
}

CommandQueue::~CommandQueue ()
{

}

void CommandQueue::push (const char *c)
{


}

const char *CommandQueue::pop ()
{
  if (first)
    {
      const char *tmp=first->command;
      first=first->next;
      return tmp;
    }
  return 0; 
}

bool CommandQueue::isEmpty ()
{
  return (first==0);
}

