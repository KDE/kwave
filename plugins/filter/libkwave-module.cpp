#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../../src/TimeOperation.h"
#include "../../../src/Parser.h"
#include "../../../src/Filter.h"

const char *version = "1.0";
const char *author = "Martin Wilz";
const char *name = "filter";
//**********************************************************
int operation (TimeOperation *operation) {
    int *sample = operation->getSample();
    int len = operation->getLength();
    Parser parser (operation->getCommand());
    Filter *filter = new Filter (parser.getFirstParam());

    if (filter) {
	double val;
	double addup = 0;
	int max = 0;
	for (int j = 0; j < filter->num; j++) {
	    addup += fabs(filter->mult[j]);
	    if (max < filter->offset[j]) max = filter->offset[j];   //find maximum offset
	}



	if (filter->fir) {
	    for (int i = len - 1; i >= max; i--) {
		val = filter->mult[0] * sample[i];
		for (int j = 1; j < filter->num; j++)
		    val += filter->mult[j] * sample[i - filter->offset[j]];
		sample[i] = (int)(val / addup);      //renormalize
	    }


	    for (int i = max - 1; i >= 0; i--)  //slower routine because of check, needed only in this range...
	    {
		val = filter->mult[0] * sample[i];
		for (int j = 1; j < filter->num; j++)
		    if (i - filter->offset[j] > 0) val += filter->mult[j] * sample[i - filter->offset[j]];
		sample[i] = (int)(val / addup);      //renormalize
	    }


	} else //basically the same,but the loops go viceversa
	{
	    for (int i = 0; i < max; i++)  //slower routine because of check, needed only in this range...
	    {
		val = filter->mult[0] * sample[i];
		for (int j = 1; j < filter->num; j++)
		    if (i - filter->offset[j] > 0) val += filter->mult[j] * sample[i - filter->offset[j]];
		sample[i] = (int)(val / addup);      //renormalize
	    }


	    for (int i = max; i < len; i++) {
		val = filter->mult[0] * sample[i];
		for (int j = 1; j < filter->num; j++)
		    val += filter->mult[j] * sample[i - filter->offset[j]];
		sample[i] = (int)(val / addup);      //renormalize
	    }


	}
    }
    operation->done();
    return 0;
}
//**********************************************************













