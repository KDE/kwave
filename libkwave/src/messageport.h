#ifndef MESSAGEPORT_H
#define MESSAGEPORT_H 1 
#include <pthread.h>
struct Message
{
  const char *text;
  Message    *next;
};

class MessagePort
//supposed to be a threadsafe message passing class...
{
 public:
  MessagePort::MessagePort ();

  const char *getMessage ();
  void        putMessage (const char *);

 private:
  Message *first;
  Message *last;
  pthread_mutex_t share;
};
#endif















