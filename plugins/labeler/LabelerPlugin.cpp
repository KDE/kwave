/***************************************************************************
    LabelerPlugin.cpp    - Plugin for enhanced signal labeling
                             -------------------
    begin                : Jan 02 2010
    copyright            : (C) 2010 by Daniel Tihelka
    email                : dtihelka@kky.zcu.cz
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

#include <QStringList>
#include <QTextCodec>

#include <klocale.h> // for i18n macro

/////////////////////
////////////////// Temporary!!!
#include "libkwave/MetaData.h"
#include "libkwave/SignalManager.h"
//////////////////
/////////////////////

#include "libkwave/PluginManager.h"


#include "LabelerPlugin.h"
#include "Labels.h"

#include "gui/LabelView.h"



KWAVE_PLUGIN(LabelerPlugin, "labeler", "0.1",
             I18N_NOOP("Signal labeler"), "Daniel Tihelka");

//***************************************************************************
LabelerPlugin::LabelerPlugin(const PluginContext &context)
    :Kwave::Plugin(context)
{
  connect(& this->signalManager(), SIGNAL(sigTrackInserted(unsigned int, Track *)),
            this,                  SLOT( slotTrackInserted(unsigned int, Track *)));
}

//***************************************************************************
LabelerPlugin::~LabelerPlugin()
{
}

//***************************************************************************
QStringList *LabelerPlugin::setup(QStringList &previous_params)
{
//  qDebug("volano LabelerPlugin::setup()");
  return Plugin::setup(previous_params);
}


//***************************************************************************
// QString LabelerPlugin::progressText()
// {
// /*    return m_action_name.length() ?
//   i18n(m_action_name.toLocal8Bit()) : i18n("Amplify Free");*/
// }

// //***************************************************************************
// int LabelerPlugin::start(QStringList &params)
// {
//     return Kwave::Plugin::start(params);
// }

//***************************************************************************
void LabelerPlugin::run(QStringList params)
{
  QVariantList tracks;
  // Span the label through all the channels
  for (unsigned int i = 0; i < this->signalManager().tracks(); i++) {
      tracks.append(QVariant(i));
  }

  // Just adds some labels to test how they are visualized
  float i = 0.0F, s = (this->signalManager().length() -1) / 10.0F;
  for (int j = 0; static_cast<sample_index_t>(i) < this->signalManager().length(); i += s, j++) {
      LabelInstant dataItem(static_cast<sample_index_t>(i), QString("lbl %1").arg(QString::number(j)));
      // Fill custom data of the item */
      dataItem.setProperty("XLABELPROP_COLOR", QVariant("custom label property"));
      dataItem.setProperty(Kwave::MetaData::STDPROP_TRACKS, tracks);
      // Store the item into signal manager
      signalManager().metaData().add(dataItem);
  }

////////////
/////////
  qDebug("LabelerPlugin::run() - dumping:");
  signalManager().metaData().dump();
  qDebug("/* LabelerPlugin::run() */");
/////////
////////////

  // invalidate???
//  this->manager().sig
}



//***************************************************************************
void LabelerPlugin::slotTrackInserted(unsigned int index, Track *track)
{
//    qDebug("LabelerPlugin::slotTrackInserted(%d, ", index);
//    track->dump();
//    qDebug(") /* LabelerPlugin::slotTrackInserted */");

    // TODO: test if the label is needed (if it is not already created ...)

    // Create the label view
    LabelView * view = new LabelView(this->parentWidget(),  NULL,
                                   & this->signalManager(), Kwave::SignalView::LowerDockTop, index);
    Q_ASSERT(view);
    view->setMinimumHeight(50);
    view->setMaximumHeight(100);
    // And add it to the manager (no controls currently)
    this->manager().insertView(view, NULL);
}


//***************************************************************************
#include "LabelerPlugin.moc"
//***************************************************************************
//***************************************************************************
