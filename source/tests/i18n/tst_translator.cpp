/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "shared/utils/translator.h"

#include <QTest>
#include <QObject>
#include <QString>
#include <QStringList>

using namespace Qt::Literals::StringLiterals;

class TestTranslator : public QObject
{
    Q_OBJECT

private slots:
    // Each row's uiLanguages are what Qt gives for its LANG and LANGUAGE, and en_US is the
    // one language with a catalogue
    void translationLanguage_data()
    {
        QTest::addColumn<QStringList>("uiLanguages");
        QTest::addColumn<QString>("expected");

        QTest::newRow("LANG=en_US") << QStringList{u"en-Latn-US"_s, u"en-US"_s, u"en-Latn"_s, u"en"_s} << u"en_US"_s;
        QTest::newRow("LANG=en_GB") << QStringList{u"en-Latn-GB"_s, u"en-GB"_s, u"en-Latn"_s, u"en"_s} << QString();
        QTest::newRow("LANG=en_GB LANGUAGE=en_GB:en_US") << QStringList{u"en-Latn-GB"_s, u"en-GB"_s,
            u"en-Latn-US"_s, u"en-US"_s, u"en-Latn"_s, u"en"_s} << QString();
        QTest::newRow("LANG=en_GB LANGUAGE=en_GB:en") << QStringList{u"en-Latn-GB"_s, u"en-GB"_s,
            u"en-Latn-US"_s, u"en-US"_s, u"en-Latn"_s, u"en"_s} << QString();
        QTest::newRow("LANG=en_US LANGUAGE=en_US:en_GB") << QStringList{u"en-Latn-US"_s, u"en-US"_s,
            u"en-Latn"_s, u"en"_s, u"en-Latn-GB"_s, u"en-GB"_s} << u"en_US"_s;
        QTest::newRow("LANG=de_DE LANGUAGE=de:en_US") << QStringList{u"de-Latn-DE"_s, u"de-DE"_s,
            u"de-Latn"_s, u"de"_s, u"en-Latn-US"_s, u"en-US"_s, u"en-Latn"_s, u"en"_s} << u"en_US"_s;
        QTest::newRow("LANG=de_DE LANGUAGE=de") << QStringList{u"de-Latn-DE"_s, u"de-DE"_s,
            u"de-Latn"_s, u"de"_s} << QString();
        QTest::newRow("LANG=en_AU") << QStringList{u"en-Latn-AU"_s, u"en-AU"_s, u"en-Latn"_s, u"en"_s} << QString();
        QTest::newRow("LANG=C") << QStringList{u"C"_s} << QString();
    }

    void translationLanguage()
    {
        QFETCH(const QStringList, uiLanguages);
        QFETCH(const QString, expected);

        QCOMPARE(u::translationLanguage(uiLanguages, [](const QString& name) { return name == u"en_US"; }), expected);
    }
};

QTEST_GUILESS_MAIN(TestTranslator)
#include "tst_translator.moc"
