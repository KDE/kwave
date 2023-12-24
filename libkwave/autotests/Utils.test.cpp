// SPDX-FileCopyrightText: 2023 Mark Penner <mrpenner@mailbox.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "../Utils.h"
#include <QtTest>

class TestUtils : public QObject
{
    Q_OBJECT

private:
private Q_SLOTS:
    void test_string2date();
};

void TestUtils::test_string2date()
{
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
    QCOMPARE(Kwave::string2date(QStringLiteral("2023-12-23")), QStringLiteral("2023-12-23"));
    QCOMPARE(Kwave::string2date(QStringLiteral("2023-12-23T01:23:45")), QStringLiteral("2023-12-23T01:23:45"));
    QCOMPARE(Kwave::string2date(QStringLiteral("Wed May 20 03:40:13 1998")), QStringLiteral("1998-05-20T03:40:13"));
    QCOMPARE(Kwave::string2date(QStringLiteral("12/23/99 1:23 PM")), QStringLiteral("1999-12-23T13:23:00"));
    QCOMPARE(Kwave::string2date(QStringLiteral("Saturday, December 23, 2023 1:23:45 AM CET")), QStringLiteral("2023-12-23T01:23:45"));
}
QTEST_MAIN(TestUtils)
#include "Utils.test.moc"
