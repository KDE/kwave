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

#include <QtGui/QCheckBox>
#include <QtGui/QLabel>
#include <QtCore/QObject>
#include <QtGui/QSlider>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtGui/QSpinBox>

#include <kfiledialog.h>
#include <klineedit.h>
#include <kpushbutton.h>
#include <ktoolinvocation.h>

#include "libkwave/MemoryManager.h"
#include "libkwave/String.h"

#include "MemoryDialog.h"

//***************************************************************************
Kwave::MemoryDialog::MemoryDialog(QWidget* parent, bool physical_limited,
    unsigned int physical_limit, bool virtual_enabled, bool virtual_limited,
    unsigned int virtual_limit, const QString &virtual_dir,
    unsigned int undo_limit)
    :QDialog(parent), Ui::MemDlg()
{
    setupUi(this);
    setModal(true);

    Kwave::MemoryManager &mem = Kwave::MemoryManager::instance();
    unsigned int total_physical = mem.totalPhysical();

    if (!isOK()) return;

    if (physical_limit > total_physical) physical_limit = total_physical;

    // connect the controls
    connect(chkEnableVirtual, SIGNAL(toggled(bool)),
            this, SLOT(virtualMemoryEnabled(bool)));
    connect(btSearch, SIGNAL(clicked()),
            this, SLOT(searchSwapDir()));
    connect(btHelp, SIGNAL(clicked()),
            this,   SLOT(invokeHelp()));

    // initialize all controls
    chkLimitPhysical->setChecked(physical_limited);
    slPhysical->setMaximum(total_physical);
    sbPhysical->setMaximum(total_physical);
    slPhysical->setValue(physical_limit);
    sbPhysical->setValue(physical_limit);
    chkEnableVirtual->setChecked(virtual_enabled);
    chkLimitVirtual->setChecked(virtual_limited);
    sbVirtual->setValue(virtual_limit);
    edDirectory->setText(virtual_dir);
    slUndo->setMaximum(total_physical / 2);
    sbUndo->setMaximum(slUndo->maximum());
    sbUndo->setValue(undo_limit);

    virtualMemoryEnabled(virtual_enabled);

    // set fixed size
    setFixedWidth(sizeHint().width());
    setFixedHeight(sizeHint().height());
}

//***************************************************************************
Kwave::MemoryDialog::~MemoryDialog()
{
}

//***************************************************************************
bool Kwave::MemoryDialog::isOK()
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
void Kwave::MemoryDialog::params(QStringList &par)
{
    par.clear();
    par << QString::number(chkLimitPhysical->isChecked() ? 1 : 0);
    par << QString::number(sbPhysical->value());
    par << QString::number(chkEnableVirtual->isChecked() ? 1 : 0);
    par << QString::number(chkLimitVirtual->isChecked() ? 1 : 0);
    par << QString::number(sbVirtual->value());
    par << edDirectory->text();
    par << QString::number(sbUndo->value());
}

//***************************************************************************
void Kwave::MemoryDialog::virtualMemoryEnabled(bool enable)
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
void Kwave::MemoryDialog::searchSwapDir()
{
    QString dir = KFileDialog::getExistingDirectory(
	edDirectory->text(), this);
    if (dir.length()) edDirectory->setText(dir);
}

//***************************************************************************
void Kwave::MemoryDialog::invokeHelp()
{
    KToolInvocation::invokeHelp(_("memory-setup"));
}

//***************************************************************************
#include "MemoryDialog.moc"
//***************************************************************************
//***************************************************************************
