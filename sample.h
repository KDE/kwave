#ifndef _SAMPLE_H_
#define _SAMPLE_H_ 1

#include <qapp.h>
#include <kapp.h>
#include "sampleop.h"

struct wavheader
{
	char		riffid[4];
	long		filelength;
	char		wavid[4];
	char		fmtid[4];
	long		fmtlength;
	short int	mode;
	short int	channels;
	long		rate;
	long		AvgBytesPerSec;
	short int	BlockAlign;
	short int	bitspersample;
};

class MSignal : public QObject
{
 Q_OBJECT
 public:
	MSignal		(QWidget *parent,QString *filename);
 	MSignal		(QWidget *parent,int size,int rate);
 	~MSignal	();
 void	play		();
 void	loop		();
 void	stopplay	();
 int 	getRate		();
 int 	*getSample	();
 int 	getLength	();
 int 	getLMarker	();
 int 	getRMarker	();
 int 	getPlayPosition	();
 void	doRangeOp	(int);

 protected:
 void	deleteRange	();
 void	flipRange	();
 void	zeroRange	();
 void	reverseRange	();
 void	cutRange	();
 void	copyRange	();
 void	pasteRange	();
 void	cropRange	();

 void	findDatainFile	(QFile *sigin);
 void	load8Bit	(QFile *sigin,int offset,int interleave);
 void	load16Bit	(QFile *sigin,int offset,int interleave);
 void	save16Bit	(QString *filename);

 signals:

 void	sampleChanged	();

 public slots:

 void setMarkers (int,int);

 private: 

 int		*sample;
 int		length;
 int		rate;
 int		rmarker,lmarker;
 int		*msg;
 int		memid;
 QString 	filename;		
 QWidget	*parent;		//used for displaying requesters
};

#endif  /* sample.h */   

