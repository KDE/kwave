#ifndef _KWAVEPARSER_H_
#define _KWAVEPARSER_H_ 1

bool   matchCommand     (const char *,const char *);

class KwaveParser
{
 public:
  KwaveParser::KwaveParser   (const char *);
  KwaveParser::~KwaveParser  ();

  inline bool        hasParams       ();
  inline bool        isDone          () {return done;};
         int         countParams     ();
         const char *getCommand      ();
         const char *getFirstParam   ();
         const char *getNextParam    ();
	 void        skipParam       ();
         int         toInt           ();
         bool        toBool          (const char *match="true");
         double      toDouble        ();

 private:
  int    ptr;
  int    len;  //length of string to be parsed
  char   *str; //points to string that is parsed
  char   *ret; //substring returned to callers, kept for deleting...
  bool   done;
};
//*****************************************************************************
class LineParser
{
 public:
  LineParser::LineParser   (const char *);
  LineParser::~LineParser  ();

         const char *getLine         ();

 private:
  int    ptr;
  int    len;
  char   *str; //points to string that is parsed
  char   *ret; //substring returned to callers, kept for deleting in destructor
  bool   done;
};
#endif



