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

#include "gui/LabelView.h"



KWAVE_PLUGIN(LabelerPlugin, "labeler", "0.1",
             I18N_NOOP("Signal labeler"), "Daniel Tihelka");


//***************************************************************************
// initializers of the standard property names
const QString LabelerPlugin::LBRPROP_TYPEVAL("Label[level > 0]");
const QString LabelerPlugin::LBRPROP_NUMLEVELS("LBRPROP_NUMLEVELS");
const QString LabelerPlugin::LBRPROP_LEVEL("LBRPROP_LEVEL");
const QString LabelerPlugin::LBRPROP_START_REFID("LBRPROP_START_REFID");
const QString LabelerPlugin::LBRPROP_START_REFPOS("LBRPROP_START_REFPOS");
const QString LabelerPlugin::LBRPROP_END_REFID("LBRPROP_END_REFID");
const QString LabelerPlugin::LBRPROP_END_REFPOS("LBRPROP_END_REFPOS");
const QString LabelerPlugin::LBRPROP_POS_REFID("LBRPROP_POS_REFID");
const QString LabelerPlugin::LBRPROP_POS_REFPOS("LBRPROP_POS_REFPOS");

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
  float i = 0.0F, s = (this->signalManager().length() -1) / 800.0F;
  for (int j = 1; static_cast<sample_index_t>(i) < this->signalManager().length(); i += s, j++) {
    // level 0
//      Kwave::MetaData dataItem(Kwave::MetaData::Position);
//      // Fill custom data of the item */
//      dataItem.setProperty(Kwave::MetaData::STDPROP_POS, static_cast<sample_index_t>(i));

      Kwave::MetaData dataItem(Kwave::MetaData::Range);
      // Fill custom data of the item */
      dataItem.setProperty(Kwave::MetaData::STDPROP_START, static_cast<sample_index_t>(i));
      dataItem.setProperty(Kwave::MetaData::STDPROP_END, static_cast<sample_index_t>(i +s));
      dataItem.setProperty(Kwave::MetaData::STDPROP_DESCRIPTION, QString("lbl %1").arg(j));
      dataItem.setProperty(Kwave::MetaData::STDPROP_TYPE, "Label"); // TODO: is there a pre-defined constant?
      dataItem.setProperty(Kwave::MetaData::STDPROP_TRACKS, tracks);
      dataItem.setProperty("XLABELPROP_COLOR", QVariant("custom label property"));
      dataItem.setProperty("J", j);  // dummy property just for higher-levels relation establishing
      // Store the item into signal manager
      signalManager().metaData().add(dataItem);

      // level 1
      if (j % 4 == 0) {
        dataItem = Kwave::MetaData(Kwave::MetaData::Range);
        // find the label corresponding to the beginning and the end of this label
        const Kwave::MetaData lbeg = signalManager().metaData().selectByValue("J", j -3).begin().value();
        const Kwave::MetaData lend = signalManager().metaData().selectByValue("J", j).begin().value();
        // Fill custom data of the item */
        dataItem.setProperty(LabelerPlugin::LBRPROP_START_REFPOS, Kwave::MetaData::STDPROP_START);
//        dataItem.setProperty(LabelerPlugin::LBRPROP_START_REFPOS, Kwave::MetaData::STDPROP_POS);
        dataItem.setProperty(LabelerPlugin::LBRPROP_START_REFID, lbeg.id());
        dataItem.setProperty(LabelerPlugin::LBRPROP_END_REFID, lend.id());
        dataItem.setProperty(LabelerPlugin::LBRPROP_END_REFPOS, Kwave::MetaData::STDPROP_END);
//        dataItem.setProperty(LabelerPlugin::LBRPROP_END_REFPOS, Kwave::MetaData::STDPROP_POS);
        dataItem.setProperty(LabelerPlugin::LBRPROP_LEVEL, 1);
        dataItem.setProperty(Kwave::MetaData::STDPROP_DESCRIPTION, QString("lbl %1-%2 - l1").arg(j-3).arg(j));
        dataItem.setProperty(Kwave::MetaData::STDPROP_TYPE, LabelerPlugin::LBRPROP_TYPEVAL);
        dataItem.setProperty(Kwave::MetaData::STDPROP_TRACKS, tracks);
       // Store the item into signal manager
        signalManager().metaData().add(dataItem);
     }

     // level 2
     if (j % 9 == 0) {
        dataItem = Kwave::MetaData(Kwave::MetaData::Range);
        // find the label corresponding to the beginning and the end of this label
        const Kwave::MetaData lbeg = signalManager().metaData().selectByValue("J", j -5).begin().value();
        const Kwave::MetaData lend = signalManager().metaData().selectByValue("J", j).begin().value();
        // Fill custom data of the item */
        dataItem.setProperty(LabelerPlugin::LBRPROP_START_REFPOS, Kwave::MetaData::STDPROP_START);
//        dataItem.setProperty(LabelerPlugin::LBRPROP_START_REFPOS, Kwave::MetaData::STDPROP_POS);
        dataItem.setProperty(LabelerPlugin::LBRPROP_START_REFID, lbeg.id());
        dataItem.setProperty(LabelerPlugin::LBRPROP_END_REFID, lend.id());
//        dataItem.setProperty(LabelerPlugin::LBRPROP_END_REFPOS, Kwave::MetaData::STDPROP_POS);
        dataItem.setProperty(LabelerPlugin::LBRPROP_END_REFPOS, Kwave::MetaData::STDPROP_END);
        dataItem.setProperty(LabelerPlugin::LBRPROP_LEVEL, 2);
        dataItem.setProperty(Kwave::MetaData::STDPROP_DESCRIPTION, QString("lbl %1-%2 - level 2").arg(j-5).arg(j));
        dataItem.setProperty(Kwave::MetaData::STDPROP_TYPE, LabelerPlugin::LBRPROP_TYPEVAL);
        dataItem.setProperty(Kwave::MetaData::STDPROP_TRACKS, tracks);
       // Store the item into signal manager
        signalManager().metaData().add(dataItem);
     }

     // level 3
     if (j % 15 == 0) {
        dataItem = Kwave::MetaData(Kwave::MetaData::Range);
        // find the label corresponding to the beginning and the end of this label
        const Kwave::MetaData lbeg = signalManager().metaData().selectByValue("J", j -3).begin().value();
        const Kwave::MetaData lend = signalManager().metaData().selectByValue("J", j -1).begin().value();
        // Fill custom data of the item */
        dataItem.setProperty(LabelerPlugin::LBRPROP_START_REFPOS, Kwave::MetaData::STDPROP_END);
//        dataItem.setProperty(LabelerPlugin::LBRPROP_START_REFPOS, Kwave::MetaData::STDPROP_POS);
        dataItem.setProperty(LabelerPlugin::LBRPROP_START_REFID, lbeg.id());
        dataItem.setProperty(LabelerPlugin::LBRPROP_END_REFID, lend.id());
//        dataItem.setProperty(LabelerPlugin::LBRPROP_END_REFPOS, Kwave::MetaData::STDPROP_POS);
        dataItem.setProperty(LabelerPlugin::LBRPROP_END_REFPOS, Kwave::MetaData::STDPROP_START);
        dataItem.setProperty(LabelerPlugin::LBRPROP_LEVEL, 3);
        dataItem.setProperty(Kwave::MetaData::STDPROP_DESCRIPTION, QString("lbl %1-%2 - level 3").arg(j-3).arg(j-1));
        dataItem.setProperty(Kwave::MetaData::STDPROP_TYPE, LabelerPlugin::LBRPROP_TYPEVAL);
        dataItem.setProperty(Kwave::MetaData::STDPROP_TRACKS, tracks);
       // Store the item into signal manager
        signalManager().metaData().add(dataItem);
     }

     if (j % 7 == 0) {
        dataItem = Kwave::MetaData(Kwave::MetaData::Position);
        // find the label corresponding to the position of this label
        const Kwave::MetaData label = signalManager().metaData().selectByValue("J", j).begin().value();
        // Fill custom data of the item */
//        dataItem.setProperty(LabelerPlugin::LBRPROP_POS_REFPOS, Kwave::MetaData::STDPROP_POS);
        dataItem.setProperty(LabelerPlugin::LBRPROP_POS_REFPOS, Kwave::MetaData::STDPROP_START);
        dataItem.setProperty(LabelerPlugin::LBRPROP_POS_REFID, label.id());
        dataItem.setProperty(LabelerPlugin::LBRPROP_LEVEL, 4);
        dataItem.setProperty(Kwave::MetaData::STDPROP_DESCRIPTION, QString("lbl %1 - level 4").arg(j));
        dataItem.setProperty(Kwave::MetaData::STDPROP_TYPE, LabelerPlugin::LBRPROP_TYPEVAL);
        dataItem.setProperty(Kwave::MetaData::STDPROP_TRACKS, tracks);
       // Store the item into signal manager
        signalManager().metaData().add(dataItem);
     }

  }

  // Create special MetaData item holding the number of levels in the view
  Kwave::MetaData levels = Kwave::MetaData(Kwave::MetaData::None);
  levels.setProperty(LBRPROP_NUMLEVELS, 5);
  signalManager().metaData().add(levels);

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
                                   & this->signalManager(), Kwave::SignalView::Bottom, index);

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
