/***************************************************************************
    NewSignalDialog.cpp  -  dialog for the "newsignal" plugin
                             -------------------
    begin                : Wed Jul 18 2001
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

#include <limits.h>
#include <math.h>
#include <stdlib.h>

#include <qcombobox.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qstring.h>

#include <klocale.h>

#include "NewSignalDialog.h"

//***************************************************************************
NewSignalDialog::NewSignalDialog(QWidget *parent)
    :NewSigDlg(parent, 0, true)
{

    sbSamples->setMaxValue(INT_MAX);

    // connect the dialog elements
    connect(rbSamples, SIGNAL(toggled(bool)),
            this, SLOT(rbSamplesToggled(bool)));
    connect(rbTime, SIGNAL(toggled(bool)),
            this, SLOT(rbTimeToggled(bool)));
    connect(sbSeconds, SIGNAL(valueChanged(int)),
            this, SLOT(timeChanged(int)));
    connect(sbMinutes, SIGNAL(valueChanged(int)),
            this, SLOT(timeChanged(int)));
    connect(sbHours, SIGNAL(valueChanged(int)),
            this, SLOT(timeChanged(int)));

    connect(cbSampleRate, SIGNAL(textChanged(const QString&)),
            this, SLOT(sampleRateChanged(const QString&)));
    connect(sbTracks, SIGNAL(valueChanged(int)),
            this, SLOT(tracksChanged(int)));
    connect(cbResolution, SIGNAL(activated(int)),
            this, SLOT(timeChanged(int)));

    // initialize the controls
    tracksChanged(0);
    timeChanged(0);

    // that dialog is big enough, limit it to it's minimum size
    setFixedSize(minimumSize());
}

//***************************************************************************
double NewSignalDialog::rate()
{
    ASSERT(cbSampleRate);
    if (!cbSampleRate) return 0;
    bool ok;
    double r = cbSampleRate->currentText().toDouble(&ok);
    if (!ok) r = 0;
    return r;
}

//***************************************************************************
unsigned int NewSignalDialog::tracks()
{
    ASSERT(sbTracks);
    return (sbTracks) ? sbTracks->value() : 0;
}

//***************************************************************************
unsigned int NewSignalDialog::bitsPerSample()
{
    ASSERT(cbResolution);
    if (!cbResolution) return 0;
    bool ok;
    unsigned int res = cbResolution->currentText().toUInt(&ok);
    if (!ok) res = 0;
    return res;
}

//***************************************************************************
void NewSignalDialog::rbSamplesToggled(bool)
{
    ASSERT(rbSamples && rbTime);
    if (!rbSamples || !rbTime) return;

    if (rbSamples->isChecked() == rbTime->isChecked())
	rbTime->setChecked(!rbSamples->isChecked());
}

//***************************************************************************
void NewSignalDialog::rbTimeToggled(bool)
{
    ASSERT(rbSamples && rbTime);
    if (!rbSamples || !rbTime) return;

    if (rbTime->isChecked() == rbSamples->isChecked())
	rbSamples->setChecked(!rbTime->isChecked());
}

//***************************************************************************
void NewSignalDialog::timeChanged(int)
{
    static bool recursive = false;
    if (recursive) return; // don't do recursive processing

    if (!sbHours || !sbMinutes || !sbSeconds) return;
    if (!rbTime || !rbTime->isChecked()) return;
    if (!rate() || !tracks() || (bitsPerSample() < 8)) return;

    // calculate number of samples from current time
    recursive = true;

    // get current time and correct wrap-overs
    int seconds = sbSeconds->value();
    int minutes = sbMinutes->value();
    int hours = sbHours->value();
    if ((seconds < 0) && (minutes > 0) && (hours > 0)) {
	sbSeconds->setValue(59);
	sbMinutes->stepDown();
    } else if (seconds < 0) {
	sbSeconds->setValue(0);
    } else if (seconds > 59) {
	sbSeconds->setValue(0);
	sbMinutes->stepUp();
    }

    if ((minutes < 0) && (hours > 0)) {
	sbMinutes->setValue(59);
	sbHours->stepDown();
    } else if (minutes < 0) {
	sbMinutes->setValue(0);
    } else if (minutes > 59) {
	sbMinutes->setValue(0);
	sbHours->stepUp();
    }
    seconds = sbSeconds->value();
    minutes = sbMinutes->value();
    hours = sbHours->value();
    minutes += 60 * hours;
    seconds += 60 * minutes;

    // limit the current number of samples

    unsigned int bytes_per_sample = bitsPerSample() >> 3;

    double max_samples = floor(UINT_MAX / tracks() / bytes_per_sample);
    double samples = ceil((double)seconds * rate());

    if (samples > max_samples) {
	// wrap down to the maximum allowed number of samples
	samples =  max_samples;
	
	unsigned int max_seconds = (unsigned int)floor(samples / rate());
	hours   = max_seconds / (60*60);
	minutes = (max_seconds / 60) % 60;
	seconds = max_seconds % 60;
	sbHours->setValue(hours);
	sbMinutes->setValue(minutes);
	sbSeconds->setValue(seconds);
    }

    QString suffix = "";
    if (samples <= 1000.0) {
	sbSamples->setValue((int)samples);
    } else {
	int quot = static_cast<int>(floor(samples / 1000.0));
	int rem  = static_cast<int>(fmod(samples, 1000.0));
	suffix.sprintf("%03d", rem);
	sbSamples->setValue(quot);
    }
    sbSamples->setSuffix(suffix);

    updateFileSize();

    // from now on it's no recursion
    recursive = false;
}

//***************************************************************************
void NewSignalDialog::sampleRateChanged(const QString&)
{
    timeChanged(0);
}

//***************************************************************************
void NewSignalDialog::tracksChanged(int)
{
    timeChanged(0);

    ASSERT(lblTracksVerbose);
    if (!lblTracksVerbose) return;
    switch (tracks()) {
	case 1:
	    lblTracksVerbose->setText(i18n("(Mono)"));
	    break;
	case 2:
	    lblTracksVerbose->setText(i18n("(Stereo)"));
	    break;
	case 4:
	    lblTracksVerbose->setText(i18n("(Quadro)"));
	    break;
	default:
	    lblTracksVerbose->setText("");
	    break;
    }
}

//***************************************************************************
void NewSignalDialog::updateFileSize()
{
    ASSERT(sbSamples);
    if (!sbSamples) return;

    double samples = sbSamples->text().toDouble();
    double mbytes = samples * tracks() * (bitsPerSample() >> 3)
           / (1024.0*1024.0);

    QString str_bytes = "";
    if (mbytes > 2.0) {
	// show whole megabytes
	str_bytes.setNum((int)rint(mbytes));
    } else {
	str_bytes.sprintf("%0.3f",mbytes);
    }

    lblFileSize->setText(i18n("(Resulting file size: %1 MB)").arg(str_bytes));
}

//***************************************************************************
//***************************************************************************
