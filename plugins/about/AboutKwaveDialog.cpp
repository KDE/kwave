/***************************************************************************
   AboutKwaveDialog.cpp  -  dialog for Kwave's "Help-About"
                             -------------------
    begin                : Sun Feb 10 2002
    copyright            : (C) 2002 by Ralf Waspe
    email                : rwaspe@web.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kapp.h>
#include <kstddirs.h>

#include "AboutKwaveDialog.h"
#include "logo.xpm"

//***************************************************************************
AboutKwaveDialog::AboutKwaveDialog(QWidget *parent)
    :KAboutApplication(parent)
{
    QPixmap logo(xpm_aboutlogo);
    setLogo(logo);
}

//***************************************************************************
AboutKwaveDialog::~AboutKwaveDialog()
{
}

//***************************************************************************
//***************************************************************************
