/***************************************************************************
       MemoryDialog.cpp  -  setup dialog of Kwave's memory management
                             -------------------
    begin                : Sun Aug 05 2001
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
#include <limits.h>
#include <unistd.h>

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qobject.h>
#include <qpushbutton.h>
#include <qslider.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qspinbox.h>

#include <kfiledialog.h>

#include "MemoryDialog.h"
#include "kwave/MemoryManager.h"

//***************************************************************************
MemoryDialog::MemoryDialog(QWidget* parent, bool physical_limited,
    unsigned int physical_limit, bool virtual_enabled, bool virtual_limited,
    unsigned int virtual_limit, const QString &virtual_dir)
    : MemDlg(parent, 0, true)
{
    MemoryManager &mem = MemoryManager::instance();
    unsigned int total_physical = mem.totalPhysical();

    if (!isOK()) return;

    if (physical_limit > total_physical) physical_limit = total_physical;

    // connect the controls
    connect(chkEnableVirtual, SIGNAL(toggled(bool)),
            this, SLOT(virtualMemoryEnabled(bool)));
    connect(btSearch, SIGNAL(clicked()),
            this, SLOT(searchSwapDir()));

    // initialize all controls
    chkLimitPhysical->setChecked(physical_limited);
    slPhysical->setMaxValue(total_physical);
    sbPhysical->setMaxValue(total_physical);
    slPhysical->setValue(physical_limit);
    sbPhysical->setValue(physical_limit);
    chkEnableVirtual->setChecked(virtual_enabled);
    chkLimitVirtual->setChecked(virtual_limited);
    sbVirtual->setValue(virtual_limit);
    edDirectory->setText(virtual_dir);

    virtualMemoryEnabled(virtual_enabled);

    // set fixed size
    setFixedWidth(sizeHint().width());
    setFixedHeight(sizeHint().height());
}

//***************************************************************************
MemoryDialog::~MemoryDialog()
{
}

//***************************************************************************
bool MemoryDialog::isOK()
{
    Q_ASSERT(chkEnableVirtual);
    Q_ASSERT(chkLimitPhysical);
    Q_ASSERT(chkLimitVirtual);
    Q_ASSERT(edDirectory);
    Q_ASSERT(sbPhysical);
    Q_ASSERT(sbVirtual);
    Q_ASSERT(slPhysical);
    Q_ASSERT(slVirtual);

    return chkEnableVirtual && chkLimitPhysical && chkLimitVirtual &&
           edDirectory && sbPhysical && sbVirtual && slPhysical &&
           slVirtual;
}

//***************************************************************************
void MemoryDialog::params(QStringList &par)
{
    par.clear();
    par << QString::number(chkLimitPhysical->isChecked() ? 1 : 0);
    par << QString::number(sbPhysical->value());
    par << QString::number(chkEnableVirtual->isChecked() ? 1 : 0);
    par << QString::number(chkLimitVirtual->isChecked() ? 1 : 0);
    par << QString::number(sbVirtual->value());
    par << edDirectory->text();
}

//***************************************************************************
void MemoryDialog::virtualMemoryEnabled(bool enable)
{
    bool limit = enable && (chkLimitVirtual->isChecked());

    chkLimitVirtual->setEnabled(enable);
    slVirtual->setEnabled(limit);
    sbVirtual->setEnabled(limit);

    txtDirectory->setEnabled(enable);
    edDirectory->setEnabled(enable);
    btSearch->setEnabled(enable);
}

//***************************************************************************
void MemoryDialog::searchSwapDir()
{
    QString dir = KFileDialog::getExistingDirectory(
	edDirectory->text(), this);
    if (dir.length()) edDirectory->setText(dir);
}

//***************************************************************************
//***************************************************************************
