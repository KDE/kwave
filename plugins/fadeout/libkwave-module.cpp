#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../../src/TimeOperation.h"
#include "../../../src/Parser.h"

const char *version = "1.0";
const char *author = "Martin Wilz";
const char *name = "fadeout";
//**********************************************************
int operation (TimeOperation *operation) {
    int *sample = operation->getSample();
    int len = operation->getLength();
    Parser parser(operation->getCommand());
    double curve = parser.toDouble();
    if (curve) curve *= 10;
    int i = 0;

    if (curve == 0)
	for (; i < len; i++) {
	    sample[i] =
		(int)(((long long) (sample[i])) * (len - i) / len);
	    operation->setCounter(i);
	}
    else if (curve < 0)
	for (; i < len; i++) {
	    sample[i] =
		(int)((double)sample[i] *
		      log10(1 + ( -curve * ((double)len - i) / len)) / log10(1 - curve));
	    operation->setCounter(i);
	}
    else
	for (; i < len; i++) {
	    sample[i] =
		(int)((double)sample[i] *
		      (1 - log10(1 + (curve * ((double)i) / len)) / log10(1 + curve)));
	    operation->setCounter(i);
	}

    operation->done();
    return 0;
}
//**********************************************************













