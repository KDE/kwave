/***************************************************************************
  AmplifyFreeDialog.cpp  -  dialog for the "amplifyfree" plugin
                             -------------------
    begin                : Sun Sep 02 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
    email                : Thomas.Eschenbacher@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <klocale.h>
#include "libkwave/Parser.h"
#include "libgui/CurveWidget.h"
#include "libgui/ScaleWidget.h"
#include "AmplifyFreeDialog.h"

//***************************************************************************
AmplifyFreeDialog::AmplifyFreeDialog(QWidget *parent)
    :AmplifyFreeDlg(parent, 0, true)
{
    curveWidget->setMinimumSize(150, 100);

    xScale->setMinimumSize(250,  30);
    xScale->setMinMax(0, 100);
    xScale->setLogMode(false);
    xScale->setUnit(i18n("ms"));

    yScale->setMinimumSize( 30, 150);
    yScale->setMinMax(100, 0);
    yScale->setLogMode(false);
    yScale->setUnit(i18n("%"));

}

//***************************************************************************
AmplifyFreeDialog::~AmplifyFreeDialog()
{
}

//***************************************************************************
QString AmplifyFreeDialog::getCommand()
{
    QString cmd;
    ASSERT(curveWidget);
    Parser p(curveWidget->getCommand());

    cmd = "amplifyfree(";
    if (p.hasParams()) cmd += p.nextParam();
    while (!p.isDone()) {
	cmd += (QString)"," + p.nextParam();
    }
    cmd += ")";
    debug("AmplifyFreeDialog::getCommand(): '"+cmd+"'");
    return cmd;
}

//***************************************************************************
void AmplifyFreeDialog::setParams(QStringList &params)
{
    QStringList::Iterator it;
    QString cmd = "curve(";

    it = params.begin();
    if (it != params.end()) cmd += *(it++);

    for (; it != params.end(); ++it)
	cmd += "," + *it;
    cmd += ")";

    if (curveWidget) curveWidget->setCurve(cmd);
}

//***************************************************************************
//***************************************************************************
