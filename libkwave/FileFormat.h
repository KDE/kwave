struct wavheader
// header format for reading wav files
{
  char          riffid[4];
  unsigned int  filelength;
  char          wavid[4];
  char          fmtid[4];
  unsigned int  fmtlength;
  short int     mode;
  short int     channels;
  unsigned int  rate;
  unsigned int  AvgBytesPerSec;
  short int     BlockAlign;
  short int     bitspersample;
};
