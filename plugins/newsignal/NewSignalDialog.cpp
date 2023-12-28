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

#include "config.h"

#include <math.h>
#include <stdlib.h>

#include <limits>

#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QRadioButton>
#include <QSlider>
#include <QSpinBox>
#include <QString>
#include <QTimer>

#include <KComboBox>
#include <KHelpClient>
#include <KLocalizedString>

#include "libkwave/String.h"
#include "libkwave/Utils.h"

#include "NewSignalDialog.h"

//***************************************************************************
Kwave::NewSignalDialog::NewSignalDialog(QWidget *parent, sample_index_t samples,
	unsigned int rate, unsigned int bits, unsigned int tracks,
	bool by_time)
    :QDialog(parent), Ui::NewSigDlg(), m_timer(this), m_recursive(false)
{

    setupUi(this);
    setModal(true);

    edSamples->setRange(0, std::numeric_limits<int>::max());
    edSamples->setSingleStep(1);

    // connect the timer for the sample edit
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(checkNewSampleEdit()));
    connect(rbTime, SIGNAL(toggled(bool)),
            this, SLOT(rbTimeToggled(bool)));

    // connect the file format controls
    connect(cbSampleRate, SIGNAL(editTextChanged(QString)),
            this, SLOT(sampleRateChanged(QString)));
    connect(sbChannels, SIGNAL(valueChanged(int)),
            this, SLOT(tracksChanged(int)));
    connect(sbResolution, SIGNAL(valueChanged(int)),
            this, SLOT(checkTimeAndLengthInfo(int)));

    // connect the time controls
    connect(sbSeconds, SIGNAL(valueChanged(int)),
            this, SLOT(timeChanged(int)));
    connect(sbMinutes, SIGNAL(valueChanged(int)),
            this, SLOT(timeChanged(int)));
    connect(sbHours, SIGNAL(valueChanged(int)),
            this, SLOT(timeChanged(int)));

    // selection by number of samples
    connect(slideLength,SIGNAL(valueChanged(int)),
            this, SLOT(setLengthPercentage(int)));

    // selection by percentage of maximum possible length
    connect(edSamples, SIGNAL(valueChanged(int)),
            this, SLOT(samplesChanged(int)));

    // help button
    connect(buttonBox->button(QDialogButtonBox::Help), SIGNAL(clicked()),
            this,   SLOT(invokeHelp()));

    // pre-initialize the size
    setMaximumHeight(sizeHint().height());
    setMaximumWidth(sizeHint().width());

    // initialize the controls
    cbSampleRate->setEditText(QString::number(rate));
    sbResolution->setValue(bits);
    sbChannels->setValue(tracks);
    if (by_time) {
	rbSamples->setChecked(false);
	rbTime->setChecked(true);
	setHMS(samples);
	edSamples->setEnabled(false);
	sbHours->setEnabled(true);
	sbMinutes->setEnabled(true);
	sbSeconds->setEnabled(true);
    } else {
	// by samples
	rbTime->setChecked(false);
	rbSamples->setChecked(true);
	edSamples->setValue(Kwave::toInt(samples));
	edSamples->setEnabled(true);
	sbHours->setEnabled(false);
	sbMinutes->setEnabled(false);
	sbSeconds->setEnabled(false);
    }

    tracksChanged(0);
    checkTimeAndLengthInfo(0);

    // that dialog is big enough, limit it to it's optimal size
    setFixedHeight(sizeHint().height());
    setFixedWidth(sizeHint().width());

    // set the focus onto the "OK" button
    buttonBox->button(QDialogButtonBox::Ok)->setFocus();
}

//***************************************************************************
sample_index_t Kwave::NewSignalDialog::samples()
{
    return static_cast<sample_index_t>(edSamples->value());
}

//***************************************************************************
double Kwave::NewSignalDialog::rate()
{
    bool ok;
    double r = cbSampleRate->currentText().toDouble(&ok);
    if (!ok) r = 0;
    return r;
}

//***************************************************************************
void Kwave::NewSignalDialog::checkNewSampleEdit()
{
    static int last_samples = -1;
    if (edSamples->value() != last_samples) {
	last_samples = edSamples->value();
	samplesChanged(last_samples);
    }
}

//***************************************************************************
unsigned int Kwave::NewSignalDialog::tracks()
{
    return sbChannels->value();
}

//***************************************************************************
unsigned int Kwave::NewSignalDialog::bitsPerSample()
{
    int res = sbResolution->value();
    if (res < 8) res = 8;
    return res;
}

//***************************************************************************
bool Kwave::NewSignalDialog::byTime()
{
    return rbTime->isChecked();
}

//***************************************************************************
sample_index_t Kwave::NewSignalDialog::maxSamples()
{
    unsigned int bytes_per_sample = bitsPerSample() >> 3;

    /*
     * NOTE: this limitation to INT_MAX instead of UINT_MAX is
     *       only needed because some gui elements like
     *       QSpinBox cannot handle more :-(
     */
    const sample_index_t max_file_size = std::numeric_limits<int>::max();

    return (max_file_size / tracks() / bytes_per_sample);
}

//***************************************************************************
void Kwave::NewSignalDialog::rbTimeToggled(bool)
{
    if (rbTime->isChecked()) {
	m_timer.stop();
    } else {
	// activate the sample edit timer
	m_timer.setSingleShot(false);
	m_timer.start(100);
    }
}

//***************************************************************************
void Kwave::NewSignalDialog::checkTimeAndLengthInfo(int)
{
    (rbTime->isChecked()) ? timeChanged(0) : samplesChanged(0);
}

//***************************************************************************
void Kwave::NewSignalDialog::timeChanged(int)
{
    if (m_recursive) return; // don't do recursive processing
    if (!rbTime->isChecked()) return;
    if ((rate() <= 0) || !tracks() || (bitsPerSample() < 8)) return;
    m_recursive = true;

    // get current time and correct wrap-overs
    int seconds = sbSeconds->value();
    int minutes = sbMinutes->value();
    int hours   = sbHours->value();
    if ((seconds < 0) && ((minutes > 0) || (hours > 0)) ) {
	sbSeconds->setValue(59);
	sbMinutes->stepDown();
	minutes--;
    } else if (seconds < 0) {
	sbSeconds->setValue(0);
    } else if (seconds > 59) {
	sbSeconds->setValue(0);
	sbMinutes->stepUp();
	minutes++;
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
    hours   = sbHours->value();
    minutes += 60 * hours;
    seconds += 60 * minutes;

    // limit the current number of samples
    sample_index_t max_samples = maxSamples();
    sample_index_t nr_samples  = static_cast<sample_index_t>(ceil(
	static_cast<double>(seconds) * rate()));

    if (nr_samples > max_samples) {
	// wrap down to the maximum allowed number of samples
	nr_samples =  max_samples;
	setHMS(nr_samples);
    }

    // update the other controls
    edSamples->setValue(Kwave::toInt(nr_samples));
    slideLength->setValue(Kwave::toInt(100.0 *
	static_cast<double>(nr_samples) / static_cast<double>(max_samples)));
    updateFileSize();
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(nr_samples > 0);

    m_recursive = false;
}

//***************************************************************************
void Kwave::NewSignalDialog::samplesChanged(int)
{
    if (m_recursive) return; // don't do recursive processing
    if (!rbSamples->isChecked()) return;
    m_recursive = true;

    sample_index_t nr_samples  = edSamples->value();
    sample_index_t max_samples = maxSamples();

    if (nr_samples > max_samples) {
	nr_samples = max_samples;
	edSamples->setValue(Kwave::toInt(nr_samples));
    }

    // update the other controls
    setHMS(nr_samples);
    slideLength->setValue(Kwave::toInt(100.0 *
	static_cast<double>(nr_samples) / static_cast<double>(max_samples)));
    updateFileSize();
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(nr_samples > 0);

    m_recursive = false;
}

//***************************************************************************
void Kwave::NewSignalDialog::sampleRateChanged(const QString&)
{
    checkTimeAndLengthInfo(0);
}

//***************************************************************************
void Kwave::NewSignalDialog::tracksChanged(int)
{
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
	    lblTracksVerbose->setText(_(""));
	    break;
    }
    checkTimeAndLengthInfo(0);
}

//***************************************************************************
void Kwave::NewSignalDialog::updateFileSize()
{
    double nr_samples = static_cast<double>(edSamples->value());
    double mbytes     = nr_samples * static_cast<double>(tracks()) *
                        static_cast<double>(bitsPerSample() >> 3);
    mbytes /= 1024.0; // to kilobytes
    mbytes /= 1024.0; // to megabytes

    QString str_bytes;
    str_bytes.setNum(mbytes, 'f', (mbytes >= 10.0) ? 1 : 3);
    lblFileSize->setText(i18n("(Resulting file size: %1 MB)", str_bytes));
}

//***************************************************************************
void Kwave::NewSignalDialog::setLengthPercentage(int percent)
{
    if (m_recursive) return; // don't do recursive processing
    if (rate() <= 0) return;
    m_recursive = true;

    sample_index_t nr_samples = static_cast<sample_index_t>(
	static_cast<double>(maxSamples()) *
	static_cast<double>(percent) / 100.0);

    // update the other controls
    setHMS(nr_samples);
    edSamples->setValue(Kwave::toInt(nr_samples));
    updateFileSize();
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(nr_samples > 0);

    m_recursive = false;
}

//***************************************************************************
void Kwave::NewSignalDialog::setHMS(sample_index_t &samples)
{
    double r = this->rate();
    if (r <= 0.0) return;

    // TODO: support for 64 bit
    if (samples > maxSamples()) samples = maxSamples();

    int total_sec = Kwave::toInt(ceil(static_cast<double>(samples) / r));
    int hours   = total_sec / (60*60);
    int minutes = (total_sec / 60) % 60;
    int seconds = total_sec % 60;

    sbHours->setValue(hours);
    sbMinutes->setValue(minutes);
    sbSeconds->setValue(seconds);
}

//***************************************************************************
void Kwave::NewSignalDialog::invokeHelp()
{
    KHelpClient::invokeHelp(_("newsignal"));
}

//***************************************************************************
//***************************************************************************
