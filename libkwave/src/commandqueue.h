#ifndef _COMMANDQ_H_
#define _COMMANDQ_H_ 1

struct Command
{
  const char *command;
  struct Command *next;
};

class CommandQueue
//Lifo Class that should be threadsafe....
{
 public:
  CommandQueue::CommandQueue ();
  CommandQueue::~CommandQueue ();

  void         push    (const char *);
  const char * pop     ();
  bool         isEmpty ();

 private:
  volatile Command *first;
  volatile Command *last;
};
#endif
