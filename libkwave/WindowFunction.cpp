#include <math.h>
#include "WindowFunction.h"

#define INT_COUNT 5
const char *FUNCTIONNAMES[] = {"None", "Hamming", "Hanning", "Blackman", "Triangular", 0
			      };

WindowFunction::WindowFunction (int type) {
    out = 0;
    count = 0;
    this->type = type;
    usagecount = 0;
}
//****************************************************************************
void WindowFunction::incUsage () {
    usagecount++;
}
//****************************************************************************
void WindowFunction::decUsage () {
    usagecount--;
}
//****************************************************************************
int WindowFunction::getUsage () {
    return usagecount;
}
//****************************************************************************
WindowFunction::~WindowFunction () {}
//****************************************************************************


const char** WindowFunction::getTypes () {
    return FUNCTIONNAMES;
}
//****************************************************************************
int WindowFunction::getCount () {
    return INT_COUNT;
}
//****************************************************************************
double *WindowFunction::getFunction (int len) {
    if (out) delete out;
    out = new double[len];

    switch (type) {
	    //Hanning, Hamming, Blackman window functions as proposed in Oppenheim, Schafer, P.241 ff

	    case FUNC_NONE:   //rectangular window


	    for (int i = 0; i < len; i++) out[i] = 1;
	    break;
	    case FUNC_HANNING:
	    for (int i = 0; i < len; i++) out[i] = 0.5 * (1 - cos(i * 2 * M_PI / (len - 1)));
	    break;
	    case FUNC_HAMMING:
	    for (int i = 0; i < len; i++) out[i] = 0.54-(0.46 * cos(((double)i) * 2 * M_PI / (len - 1)));
	    break;
	    case FUNC_BLACKMAN:
	    for (int i = 0; i < len; i++) out[i] = 0.42-(0.5 * cos(((double)i) * 2 * M_PI / (len - 1))) + (0.08 * cos(((double)i) * 4 * M_PI / (len - 1)));
	    break;
	    case FUNC_TRIANGULAR:
	    for (int i = 0; i < len / 2; i++) out[i] = ((double)i) / (len / 2 - 1);
	    for (int i = len / 2; i < len; i++) out[i] = 1 - ((double)i - len / 2) / (len / 2 - 1);
	    break;
    }

    return out;
}










