
#include "config.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <libkwave/String.h>
#include "FloatLine.h"

extern QString mstotime (int ms);
extern char* mstotimec (int ms);

//**********************************************************
FloatLine::FloatLine(QWidget *parent, double value)
    :KRestrictedLine(parent)
{
    digits = 1;
    setValue (value);
    setValidChars ("-0123456789.E");
}

//**********************************************************
void FloatLine::setValue (double value) 
{
    char buf[256];
    char conv[256];

    snprintf(conv, sizeof(conv), "%%.%df", digits);
    snprintf(buf, sizeof(buf), conv, value);
    setText(buf);
}

//**********************************************************
double FloatLine::value () 
{
    return strtod (text(), 0);
}

//**********************************************************
FloatLine::~FloatLine () 
{
}

//****************************************************************************
//****************************************************************************
