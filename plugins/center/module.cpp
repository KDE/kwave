#include <stdio.h>
#include <stdlib.h>
#include "../../../src/TimeOperation.h"

const char *version = "1.0";
const char *author = "Martin Wilz";
const char *name = "center";

//**********************************************************
int operation (TimeOperation *operation) {
    int *sample = operation->getSample();
    int len = operation->getLength();

    long long addup = 0;
    int dif = 0;

    for (int i = 0; i < len; addup += sample[i++]) dif = addup / len;    //generate mean value

    operation->setCounter(len / 2);

    if (dif ) for (int i = 0; i < len; sample[i++] -= dif );
    //and subtract it from all samples ...

    operation->done();
    return 0;
}
//**********************************************************













