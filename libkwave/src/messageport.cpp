#include "messageport.h"
#include "kwavestring.h"

MessagePort::MessagePort ()
{
  pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
  first=0;
  last=0;
  memcpy (&share,&mutex,sizeof (pthread_mutex_t));
}

const char *MessagePort::getMessage ()
{
  pthread_mutex_lock (&share);
  if (first)
    {
      const char *text=duplicateString (first->text);
      printf ("getting message: %s\n",text);

      Message *tmp=first;
      first=first->next;
      delete tmp;
      if (first==0) last=0;

      pthread_mutex_unlock (&share);
      return text;                    //memory leak ! later i will do something about it...
    }
    pthread_mutex_unlock (&share);
  return 0;
}

void MessagePort::putMessage (const char *text)
{
  printf ("putting message: %s\n",text);
  Message *message=new Message;

  if (message)
    {
      message->text=duplicateString (text);

      if (message->text)
	{
	  pthread_mutex_lock (&share);
	  message->next=0;

	  if (last)
	    {
	      last->next=message;
	      last=message;
	    }
	  else
	    {
	      first=message;
	      last=message;
	    }
	  pthread_mutex_unlock (&share);
	  printf ("message stored\n");
	}
    }
}


