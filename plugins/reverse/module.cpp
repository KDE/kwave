#include <stdio.h>
#include <stdlib.h>
#include "../../../src/TimeOperation.h"

const char *version = "1.0";
const char *author = "Martin Wilz";
const char *name = "reverse";

//**********************************************************
int operation (TimeOperation *operation) {
    int *sample = operation->getSample();
    int len = operation->getLength();

    if (len > 0) {
	int x;

	for (int i = 0; i < len / 2; i++) {
	    x = sample[i];
	    sample[i] = sample[len - 1 - i];
	    sample[len - 1 - i] = x;
	}
    }

    operation->done();
    return 0;
}
//**********************************************************













