
//#include <stdio.h>
//#include <math.h>
//#include "Label.h"
//#include "Parser.h"
//#include "String.h"
//#include "Color.h"
//#include "Global.h"
//
//extern Global globals;
////****************************************************************************
//Label::Label (const char *params) {
//    Parser parser (params);
//
//    this->pos = parser.toDouble ();
//    const char *type = parser.getNextParam ();
//
//    LabelType *act;
//    this->type = globals.markertypes.first();
//    for (act = globals.markertypes.first(); act; act = globals.markertypes.next()) {
//	if (strcmp (act->name, type) == 0) {
//	    this->type = act;
//	    break;
//	}
//    }
//
//    if (this->type->named) {
//	const char *name = parser.getNextParam ();
//	this->name = duplicateString(name);
//    }
//    comstr = 0;
//}
////****************************************************************************
//Label::Label (double pos, LabelType *type, const char *name) {
//    this->pos = pos;
//    this->type = type;
//
//    if (name)
//	this->name = duplicateString (name);
//    else
//	this->name = 0;
//    comstr = 0;
//}
////****************************************************************************
//const char * Label::getCommand () {
//    if (comstr) deleteString (comstr);
//    char buf[64];
//
//    snprintf(buf, sizeof(buf), "%f", pos);
//
//    if (type->named)
//	comstr = catString ("label (", buf, ",", type->name, ",", name, ")");
//    else
//	comstr = catString ("label (", buf, ",", type->name, ")");
//    return comstr;
//}
////****************************************************************************
//void Label::setName (const char *name) {
//    this->name = duplicateString (name);
//}
////****************************************************************************
//Label::~Label () {
//    if (name) deleteString (name);
//    if (comstr) deleteString (comstr);
//}
////****************************************************************************
