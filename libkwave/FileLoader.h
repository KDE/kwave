#ifndef _FILELOADER_H_
#define _FILELOADER_H_ 1

class FileLoader
{
 public:
  FileLoader::FileLoader   (const char *);
  FileLoader::~FileLoader  ();

  inline const char *getMem () {return buf;};
 private:
  char *buf;
};
#endif
