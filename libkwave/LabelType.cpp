//#include "String.h"
//#include "LabelType.h"
//#include "Parser.h"
//#include "Color.h"
//
////****************************************************************************
//LabelType::LabelType()
//{
//    name = 0;
//    color = 0;
//}
//
////****************************************************************************
//LabelType::LabelType(const char *command)
//{
//    Parser parser (command);
//
//    name = duplicateString (parser.getFirstParam());
//    named = (strcmp(parser.getNextParam(), "true") == 0);
//    color = new Color (parser.getNextParam());
//    comstr = 0;
//}
//
////****************************************************************************
//const char *LabelType::getCommand()
//{
//    if (comstr) delete[] comstr;
//    comstr = catString ("newlabeltype (",
//			name,
//			",",
//			named ? "true" : "false",
//			",",
//			color->getCommand(),
//			")"
//		       );
//
//    return comstr;
//}
//
////****************************************************************************
//LabelType::~LabelType()
//{
//    if (name) delete[] name;
//    if (color) delete color;
//    if (comstr) delete[] comstr;
//}

//****************************************************************************
//****************************************************************************
