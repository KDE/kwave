#ifndef _WINDOW_FUNCTION_H_
#define _WINDOW_FUNCTION_H_ 1

#define FUNC_NONE       0
#define FUNC_HAMMING    1
#define FUNC_HANNING    2
#define FUNC_BLACKMAN   3
#define FUNC_TRIANGULAR 4

class WindowFunction
{
 public:

  WindowFunction  (int type);
  ~WindowFunction ();
  void   incUsage ();
  void   decUsage ();
  int    getUsage ();
  //returns array of doubles with size
  double *getFunction (int size);
  //returns array of char* which contain function names
  const char** getTypes ();
  //returns number of available functions
  int    getCount ();

 private:

  double *out;           // arrays of ouput data
  int type;              // type of interpolation
  int count;             // number of points
  int usagecount;        // number of threads using this object
};
#endif /*windowfunction.h*/

