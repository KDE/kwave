#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../../src/Signal.h"
#include "../../../src/Interpolation.h"
#include "../../../src/TimeOperation.h"
#include "../../../src/Curve.h"
#include "../../../src/Parser.h"

const char *version = "1.0";
const char *author = "Martin Wilz";
const char *name = "envelope";
//**********************************************************
int operation (TimeOperation *operation) {
    int *sample = operation->getSample();
    int rate = (operation->getSignal())->getRate();
    int len = operation->getLength();
    Parser parser (operation->getCommand());

    double time = parser.toInt();
    const char *type = parser.getNextParam();

    int max = 0;

    int chunksize = (int)(rate * 10 / time);
    Interpolation interpolation (type);
    Curve *points = new Curve;

    if (chunksize < len) {
	for (int i = 0; i < chunksize / 2; i++) {
	    int act = sample[i];
	    if (max < act) max = act;
	    if (max < -act) max = -act;
	}

	points->append (0, (double) max);

	int pos = 0;
	while (pos < len - chunksize) {
	    max = 0;

	    for (int i = 0; i < chunksize; i++) {
		int act = sample[pos++];
		if (max < act) max = act;
		if (max < -act) max = -act;
	    }
	    points->append ((double) (pos - chunksize / 2) / len, (double) max);
	}

	max = 0;
	for (int i = len - chunksize / 2; i < len; i++) {
	    int act = sample[i];
	    if (max < act) max = act;
	    if (max < -act) max = -act;
	}
	points->append (1, (double) max);

	double *y = interpolation.getInterpolation (points, len);

	for (int i = 0; i < len; i++) sample[i] = (int)y[i];

    }

    operation->done();
    return 0;
}
//**********************************************************













