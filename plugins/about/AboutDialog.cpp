/***************************************************************************
              AboutDialog.h  -  dialog for Kwave's "Help-About"
                             -------------------
    begin                : Sun Oct 29 2000
    copyright            : (C) 2000 by Thomas Eschenbacher
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

#include <qaccel.h>
#include <qdict.h>
#include <qframe.h>
#include <qkeycode.h>
#include <qlayout.h>
#include <qmlined.h>
// #include <qmultilineedit.h> seems to be broken ?
#include <qpushbutton.h>

#include <klocale.h>

#include "LogoWidget.h"
#include "AboutDialog.h"

// PLEASE DO NOT CHANGE THE FOLLOWING TWO LINES, THEY WILL BE
// AUTOMATICALLY UPDATED BY THE VERSION MANAGEMENT SCRIPTS !
#define KWAVE_VERSION "0.6.2"
#define KWAVE_VERSION_DATE "Dez 24, 2001"

//***************************************************************************
static const char about_text[] = "\nKwave Version "KWAVE_VERSION"\n\
"KWAVE_VERSION_DATE"\n\
(c) 2000-2001 by Thomas Eschenbacher (thomas.eschenbacher@gmx.de)\n\
(c) 1998-2000 by Martin Wilz (mwilz@ernie.mi.uni-koeln.de)\n\
\nFFT - Code by GNU gsl - Project, library version 0.3 beta\n\
(GSL - Library may be retrieved from ftp://alpha.gnu.org/gnu/)\n\n\
\n\
Thanks go to: \n\
\
Sven-Steffen Arndt <ssa29@gmx.de>\n\
Carsten Jacobi\n\
Frank Christian Stoffel\n\
Achim Dahlhaus\n\
Klaus Hendrik Lorenz\n\n\
People, who provided valuable feedback (in no particular order)\n\
\
Stephan Loescher <loescher@leo.org>\n\
Gerhard Zintel\n\
Gael Duval\n\
Aaron Johnson\n\
Uwe Steinmann\n\
Juhana Kouhia\n\
Dave Phillips\n\
Martin Petriska\n\
Winfried Truemper\n\
Bruce Garlock\n\
Christoph Raab\n\
tOpHEr lAfaTA\n\
Nemosoft\n\
Guido\n\
Eero\n\
\
This program is free software; you can redistribute it and / or\n\
modify it under the terms of the GNU General Public License\n\
as published by the Free Software Foundation; either version 2\n\
of the License, or (at your option) any later version.\n\n\
This program is distributed in the hope that it will be useful, \n\
but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n\
See the GNU General Public License for more details.\n\n\
You should have received a copy of the GNU General Public License\n\
along with this program; if not, write to the Free Software\n\
Foundation, Inc., 59 Temple Place - Suite 330,\n\
Boston, MA 02111-1307, USA.\n\
";

//***************************************************************************
AboutDialog::AboutDialog(QWidget *parent)
    :QDialog(parent, "about-dialog", true)
{
    m_abouttext = 0;
    m_logo = 0;
    m_ok = 0;

    // toplevel layout is a vbox
    QVBoxLayout *top_layout = new QVBoxLayout(this, 0);
    ASSERT(top_layout);
    if (!top_layout) return;

    // hbox for the upper part with text and logo
    QHBoxLayout *upper_hbox = new QHBoxLayout();
    ASSERT(upper_hbox);
    if (!upper_hbox) return;
    top_layout->addLayout(upper_hbox, 1);

    setCaption(i18n("About Kwave"));

    // animated logo on the left side
    m_logo = new LogoWidget(this);
    ASSERT(m_logo);
    if (!m_logo) return;
    m_logo->setMinimumHeight(150);
    m_logo->setMinimumWidth(100);
    upper_hbox->addWidget(m_logo, 20);

    // text area on the right
    m_abouttext = new QMultiLineEdit(this);
    ASSERT(m_abouttext);
    if (!m_abouttext) return;
    m_abouttext->setText(about_text);
    m_abouttext->setReadOnly(true);
    m_abouttext->setCursorPosition(0,0);
    m_abouttext->setMinimumWidth(m_abouttext->maxLineWidth()+20);
    m_abouttext->setMinimumHeight(150);
    upper_hbox->addWidget(m_abouttext, 80);

    // separator before the OK button
    QFrame *separator = new QFrame(this, "separator line");
    ASSERT(separator);
    if (!separator) return;
    separator->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    separator->setFixedHeight(separator->sizeHint().height());
    top_layout->addSpacing(10);
    top_layout->addWidget(separator);
    top_layout->addSpacing(10);

    // hbox for the ok button
    QHBoxLayout *lower_hbox = new QHBoxLayout();
    ASSERT(lower_hbox);
    if (!lower_hbox) return;
    top_layout->addLayout(lower_hbox);
    top_layout->addSpacing(10);

    // the OK button itself
    m_ok = new QPushButton (i18n("Ok"), this);
    ASSERT(m_ok);
    if (!m_ok) return;
    m_ok->setFixedWidth(2*m_ok->sizeHint().width());
    m_ok->setFixedHeight(m_ok->sizeHint().height());
    m_ok->setAccel(Key_Return);
    m_ok->setFocus();
    connect(m_ok, SIGNAL(clicked()), this, SLOT (accept()));
    lower_hbox->addStretch(1);
    lower_hbox->addWidget(m_ok);
    lower_hbox->addStretch(1);

    top_layout->activate();

    resize(minimumSize());
    int w = width();
    int h = height();
    h = w*4/5;
    resize(w, h);
}

//***************************************************************************
AboutDialog::~AboutDialog()
{
    if (m_logo) delete m_logo;
    if (m_ok)   delete m_ok;
}

//***************************************************************************
//***************************************************************************
/* end of AboutDialog.cpp */
