// SPDX-FileCopyrightText: 2024 Mark Penner <mrp@markpenner.space>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "Track.h"
#include <QtTest>

class TestTrack : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void deleteRange_data();
    void deleteRange();
};

void TestTrack::deleteRange_data()
{
    QTest::addColumn<sample_index_t>("trackLen");
    QTest::addColumn<sample_index_t>("offset");
    QTest::addColumn<sample_index_t>("deleteLen");

    QTest::newRow("delete all one stripe")   << 16384ull <<     0ull << 16384ull;
    QTest::newRow("delete all stripes")      << 32768ull <<     0ull << 32768ull;
    QTest::newRow("delete first of stripes") << 65536ull <<     0ull << 16384ull;
    QTest::newRow("delete interior stripe")  << 65536ull << 16384ull << 16384ull;
}

void TestTrack::deleteRange()
{
    QFETCH(sample_index_t, trackLen);
    QFETCH(sample_index_t, offset);
    QFETCH(sample_index_t, deleteLen);

    auto uuid{QUuid::createUuid()};
    auto t = Kwave::Track{trackLen, &uuid};
    t.deleteRange(offset, deleteLen);
    QCOMPARE(t.length(), trackLen - deleteLen);
}

QTEST_MAIN(TestTrack)
#include "test_Track.moc"
