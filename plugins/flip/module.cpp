#include <stdio.h>
#include <stdlib.h>
#include "kwave/TimeOperation.h"

const char *version = "1.0";
const char *author = "Martin Wilz";
const char *name = "flip";

//**********************************************************
int operation (TimeOperation *operation) {
    int *sample = operation->getSample();
    int len = operation->getLength();

    for (int i = 0; i < len; i++) sample[i] = -sample[i];

    operation->done();
    return 0;
}
//**********************************************************













