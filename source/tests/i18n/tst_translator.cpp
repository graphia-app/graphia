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
#include <QCoreApplication>
#include <QLocale>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QTranslator>

using namespace Qt::Literals::StringLiterals;

namespace
{
QString translated(const char* sourceText)
{
    return QCoreApplication::translate("Test", sourceText);
}

// As main() does, for a user whose only language is that of locale
void installAppTranslator(const QLocale& locale)
{
    QLocale::setDefault(locale);
    u::installTranslator();
}
} // namespace

// The test catalogues are graphia_fr_FR, graphia_es_ES, and testplugin_ in fr_FR, de_DE and en_GB
class TestTranslator : public QObject
{
    Q_OBJECT

    // As it is before any catalogue is installed
    QString _pluginOnly;

private slots:
    void initTestCase() { _pluginOnly = translated("Plugin Only"); }

    void cleanup()
    {
        // A translator removes itself when it's destroyed
        qDeleteAll(QCoreApplication::instance()->findChildren<Translator*>(
            Qt::FindDirectChildrenOnly));

        QLocale::setDefault(QLocale::system());
    }

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

    // The test's own catalogues load as the app's would
    void appCatalogue()
    {
        installAppTranslator(QLocale(QLocale::French, QLocale::France));
        QCOMPARE(translated("Shared"), u"Partagé (application)"_s);
    }

    void pluginInAppsLanguage()
    {
        installAppTranslator(QLocale(QLocale::French, QLocale::France));

        const QObject plugin;
        u::installPluginTranslator(u"TestPlugin"_s, &plugin);
        QCOMPARE(translated("Plugin Only"), u"Extension seulement"_s);
    }

    void noPluginCatalogueInAppsLanguage()
    {
        installAppTranslator(QLocale(QLocale::Spanish, QLocale::Spain));

        const QObject plugin;
        u::installPluginTranslator(u"TestPlugin"_s, &plugin);
        QCOMPARE(translated("Plugin Only"), _pluginOnly);
    }

    // There's no app catalogue in the source language, even where a plugin has one
    void noPluginCatalogueInSourceLanguage()
    {
        installAppTranslator(QLocale(QLocale::English, QLocale::UnitedKingdom));

        const QObject plugin;
        u::installPluginTranslator(u"TestPlugin"_s, &plugin);
        QCOMPARE(translated("Plugin Only"), _pluginOnly);
    }

    void appTakesPrecedence()
    {
        installAppTranslator(QLocale(QLocale::French, QLocale::France));

        const QObject plugin;
        u::installPluginTranslator(u"TestPlugin"_s, &plugin);
        QCOMPARE(translated("Shared"), u"Partagé (application)"_s);
        QCOMPARE(translated("Identical"), u"Identical"_s);
    }

    void pluginCatalogueGoesWithPlugin()
    {
        installAppTranslator(QLocale(QLocale::French, QLocale::France));

        {
            const QObject plugin;
            u::installPluginTranslator(u"TestPlugin"_s, &plugin);
            QCOMPARE(translated("Plugin Only"), u"Extension seulement"_s);
        }

        QCOMPARE(translated("Plugin Only"), _pluginOnly);
        QCOMPARE(translated("Shared"), u"Partagé (application)"_s);
    }
};

QTEST_GUILESS_MAIN(TestTranslator)
#include "tst_translator.moc"
