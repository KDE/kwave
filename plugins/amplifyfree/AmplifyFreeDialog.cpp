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

#include "config.h"

#include <QPushButton>

#include <KLocalizedString>
#include <KToolInvocation>

#include "libkwave/Parser.h"
#include "libkwave/String.h"

#include "libgui/CurveWidget.h"
#include "libgui/ScaleWidget.h"

#include "AmplifyFreeDialog.h"

//***************************************************************************
Kwave::AmplifyFreeDialog::AmplifyFreeDialog(QWidget *parent)
    :QDialog(parent), Ui::AmplifyFreeDlg()
{
    setupUi(this);
    curveWidget->setMinimumSize(150, 100);

    xScale->setMinimumSize(250,  30);
    xScale->setMinMax(0, 100);
    xScale->setLogMode(false);
    xScale->setUnit(i18n("ms"));

    yScale->setMinimumSize( 30, 150);
    yScale->setMinMax(0, 100);
    yScale->setLogMode(false);
    yScale->setUnit(i18n("%"));

    connect(buttonBox_Help->button(QDialogButtonBox::Help), SIGNAL(clicked()),
            this,   SLOT(invokeHelp()));

    // set the focus onto the "OK" button
    buttonBox->button(QDialogButtonBox::Ok)->setFocus();
}

//***************************************************************************
Kwave::AmplifyFreeDialog::~AmplifyFreeDialog()
{
}

//***************************************************************************
QString Kwave::AmplifyFreeDialog::getCommand()
{
    QString cmd;
    Q_ASSERT(curveWidget);
    Kwave::Parser p(curveWidget->getCommand());

    cmd = _("amplifyfree(");
    if (p.hasParams()) cmd += p.nextParam();
    while (!p.isDone()) {
	cmd += _(",") + p.nextParam();
    }
    cmd += _(")");

    return cmd;
}

//***************************************************************************
void Kwave::AmplifyFreeDialog::setParams(QStringList &params)
{
    QStringList::Iterator it;
    QString cmd = _("curve(");

    it = params.begin();
    if (it != params.end()) cmd += *(it++);

    for (; it != params.end(); ++it)
	cmd += _(",") + *it;
    cmd += _(")");

    if (curveWidget) curveWidget->setCurve(cmd);
}

//***************************************************************************
void Kwave::AmplifyFreeDialog::invokeHelp()
{
    KToolInvocation::invokeHelp(_("plugin_sect_amplifyfree"));
}

//***************************************************************************
//***************************************************************************
//***************************************************************************
