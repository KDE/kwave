#ifndef KWAVE_TIME_OPERATION
#define KWAVE_TIME_OPERATION 1
#include "kwavestring.h"

class KwaveSignal;
class TimeOperation
{
 public:

 TimeOperation::TimeOperation  (KwaveSignal *signal,const char *operation,int begin,int len);
 TimeOperation::~TimeOperation ();

 int*                getSample  ();
 inline int          getCounter ()      {return counter;};
 inline void         setCounter (int x) {counter=x;};
 inline KwaveSignal *getSignal  ()      {return signal;};
 inline const char  *getCommand ()      {return command;};
 
 inline int          getLength  ()      {return len;};

 //the following function sets counter to a negative value
 //when read by the ProgressClass, the Progressclass will
 //delete itself and its TimeOperation-Object, so this 
 //is the way to get rid of a object of this class in the kwave
 //environment, not that sober, but it works...
 inline void         done       ()      {counter=-1;};

 private:
  char        *command;
  KwaveSignal *signal;
  int         begin;
  int         len;
  int         counter;
};
#endif
