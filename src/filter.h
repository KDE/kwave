#ifndef _KWAVE_FILTER_H
#define _KWAVE_FILTER_H 1

//*****************************************************************************
class Filter
{
 public:
  Filter      (int rate);
  Filter      (const char *);
  ~Filter     ();

  const char *getCommand ();

  int  resize (int);
  void save   (const char *);
  void load   (const char *);

  int      num;                //number of taps
  bool     fir;                //boolean if filter is fir or iir
  double * mult;               //array of coefficients, used according to
  int*     offset;             //array of delay times in samples

 private:
  char *   comstr;
  int      rate;
};
#endif
