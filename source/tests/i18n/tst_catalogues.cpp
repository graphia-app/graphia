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

#include <QTest>
#include <QDir>
#include <QFile>
#include <QRegularExpression>
#include <QString>
#include <QStringList>
#include <QXmlStreamReader>

#include <algorithm>
#include <map>
#include <set>
#include <tuple>
#include <vector>

using namespace Qt::Literals::StringLiterals;

namespace
{
// Both QString::arg's and Utils.format's
std::set<QString> placeholdersIn(const QString& text)
{
    static const QRegularExpression placeholderRegex(uR"(%L?(\d+)|\{(\d+)\})"_s);

    std::set<QString> placeholders;
    for(const auto& match : placeholderRegex.globalMatch(text))
        placeholders.insert(match.captured());

    return placeholders;
}

QStringList tagsIn(const QString& text)
{
    static const QRegularExpression tagRegex(u"<[^>]+>"_s);

    QStringList tags;
    for(const auto& match : tagRegex.globalMatch(text))
        tags.append(match.captured()); // clazy:exclude=reserve-candidates

    tags.sort();
    return tags;
}

// An & that isn't doubled, before a space, or part of an HTML entity
qsizetype mnemonicsIn(const QString& text)
{
    static const QRegularExpression mnemonicRegex(uR"((?<!&)&(?!&|\s|[a-zA-Z]+;|#\d+;))"_s);
    return text.count(mnemonicRegex);
}
} // namespace

class TestCatalogues : public QObject
{
    Q_OBJECT

    struct Message
    {
        QString _context;
        QString _source;
        QString _comment;
        QString _type;
        bool _numerus = false;
        QStringList _translations;

        QString description() const
        {
            return _comment.isEmpty() ? u"%1: \"%2\""_s.arg(_context, _source) :
                u"%1: \"%2\" (%3)"_s.arg(_context, _source, _comment);
        }
    };

    struct Catalogue
    {
        QString _language;
        std::vector<Message> _messages;
    };

    const QDir _sourceDir{QString::fromUtf8(SOURCE_DIR)}; // clazy:exclude=qstring-allocations

    // Keyed by the catalogue's path within the source tree
    std::map<QString, Catalogue> _catalogues;

    static void addProblem(QStringList& problems, const Message& message, const QString& problem)
    {
        problems.append(u"%1 %2"_s.arg(message.description(), problem));
    }

    void addCatalogueRows()
    {
        QTest::addColumn<QString>("fileName");

        for(const auto& [fileName, catalogue] : _catalogues)
            QTest::newRow(qPrintable(fileName)) << fileName;
    }

    const Catalogue& catalogueForRow()
    {
        QFETCH(const QString, fileName);
        return _catalogues.at(fileName);
    }

    void load(const QString& fileName)
    {
        QFile file(_sourceDir.filePath(fileName));
        QVERIFY2(file.open(QIODevice::ReadOnly), qPrintable(file.fileName()));

        auto& catalogue = _catalogues[fileName];
        QString context;

        QXmlStreamReader reader(&file);
        while(!reader.atEnd())
        {
            if(reader.readNext() != QXmlStreamReader::StartElement)
                continue;

            const auto name = reader.name();
            const auto attributes = reader.attributes();

            if(name == u"TS")
                catalogue._language = attributes.value(u"language").toString();
            else if(name == u"name")
                context = reader.readElementText();
            else if(name == u"message")
            {
                auto& message = catalogue._messages.emplace_back();
                message._context = context;
                message._numerus = attributes.value(u"numerus") == u"yes";
            }
            else if(name == u"source")
                catalogue._messages.back()._source = reader.readElementText();
            else if(name == u"comment")
                catalogue._messages.back()._comment = reader.readElementText();
            else if(name == u"translation")
            {
                auto& message = catalogue._messages.back();
                message._type = attributes.value(u"type").toString();

                // A plural translation is a list of numerusforms instead
                if(!message._numerus)
                    message._translations.append(reader.readElementText());
            }
            else if(name == u"numerusform")
                catalogue._messages.back()._translations.append(reader.readElementText());
        }

        QVERIFY2(!reader.hasError(), qPrintable(u"%1: %2"_s.arg(fileName, reader.errorString())));
        QVERIFY2(!catalogue._language.isEmpty(), qPrintable(fileName));
        QVERIFY2(!catalogue._messages.empty(), qPrintable(fileName));
    }

private slots:
    // The app's catalogues, and each plugin's
    void initTestCase()
    {
        const auto plugins = QDir(_sourceDir.filePath(u"plugins"_s)).entryList(QDir::Dirs | QDir::NoDotAndDotDot);

        QStringList translationsDirs = {u"translations"_s};
        translationsDirs.reserve(plugins.size() + 1);

        for(const auto& plugin : plugins)
            translationsDirs.append(u"plugins/%1/translations"_s.arg(plugin));

        for(const auto& translationsDir : translationsDirs)
        {
            const auto tsFiles = QDir(_sourceDir.filePath(translationsDir)).entryList({u"*.ts"_s}, QDir::Files);
            for(const auto& tsFile : tsFiles)
                load(u"%1/%2"_s.arg(translationsDir, tsFile));
        }

        QVERIFY(_catalogues.contains(u"translations/graphia_en_US.ts"_s));
    }

    void finished_data() { addCatalogueRows(); }
    void finished()
    {
        QStringList problems;

        for(const auto& message : catalogueForRow()._messages)
        {
            if(!message._type.isEmpty())
                addProblem(problems, message, u"is %1"_s.arg(message._type));
            else if(message._translations.isEmpty() || message._translations.contains(QString()))
                addProblem(problems, message, u"has no translation"_s);
        }

        QVERIFY2(problems.isEmpty(), qPrintable(problems.join(u'\n')));
    }

    // %n is left out, as a language may have a plural form that doesn't need the number
    void placeholders_data() { addCatalogueRows(); }
    void placeholders()
    {
        QStringList problems;

        for(const auto& message : catalogueForRow()._messages)
        {
            const auto sourcePlaceholders = placeholdersIn(message._source);

            for(const auto& translation : message._translations)
            {
                if(placeholdersIn(translation) != sourcePlaceholders)
                    addProblem(problems, message, u"has different placeholders in \"%1\""_s.arg(translation));
            }
        }

        QVERIFY2(problems.isEmpty(), qPrintable(problems.join(u'\n')));
    }

    void markup_data() { addCatalogueRows(); }
    void markup()
    {
        QStringList problems;

        for(const auto& message : catalogueForRow()._messages)
        {
            const auto sourceTags = tagsIn(message._source);
            const auto sourceMnemonics = mnemonicsIn(message._source);

            for(const auto& translation : message._translations)
            {
                if(tagsIn(translation) != sourceTags)
                    addProblem(problems, message, u"has different tags in \"%1\""_s.arg(translation));

                if(mnemonicsIn(translation) != sourceMnemonics)
                    addProblem(problems, message, u"has different mnemonics in \"%1\""_s.arg(translation));
            }
        }

        QVERIFY2(problems.isEmpty(), qPrintable(problems.join(u'\n')));
    }

    // Every plural message in a language has the same number of forms
    void pluralForms_data() { addCatalogueRows(); }
    void pluralForms()
    {
        QStringList problems;
        std::map<qsizetype, int> messagesWithFormCount;

        const auto& messages = catalogueForRow()._messages;
        for(const auto& message : messages)
        {
            if(message._numerus)
                messagesWithFormCount[message._translations.size()]++;
            else if(message._translations.size() > 1)
                addProblem(problems, message, u"isn't plural, but has plural forms"_s);
        }

        if(messagesWithFormCount.size() > 1)
        {
            const auto mostCommon = std::ranges::max_element(messagesWithFormCount,
                {}, [](const auto& entry) { return entry.second; })->first;

            for(const auto& message : messages)
            {
                if(message._numerus && message._translations.size() != mostCommon)
                {
                    addProblem(problems, message, u"has %1 plural forms, where most have %2"_s
                        .arg(message._translations.size()).arg(mostCommon));
                }
            }
        }

        QVERIFY2(problems.isEmpty(), qPrintable(problems.join(u'\n')));
    }

    // A message in more than one catalogue is translated the same in each, as the
    // app's takes precedence over a plugin's
    void consistentAcrossCatalogues()
    {
        QStringList problems;
        std::map<std::tuple<QString, QString, QString, QString>,
            std::pair<QString, QStringList>> translationsOf;

        for(const auto& [fileName, catalogue] : _catalogues)
        {
            for(const auto& message : catalogue._messages)
            {
                auto [it, inserted] = translationsOf.try_emplace(
                    {catalogue._language, message._context, message._source, message._comment},
                    fileName, message._translations);

                const auto& [otherFileName, otherTranslations] = it->second;
                if(!inserted && otherTranslations != message._translations)
                {
                    addProblem(problems, message, u"is translated differently in %1 and %2"_s
                        .arg(otherFileName, fileName));
                }
            }
        }

        QVERIFY2(problems.isEmpty(), qPrintable(problems.join(u'\n')));
    }
};

QTEST_GUILESS_MAIN(TestCatalogues)
#include "tst_catalogues.moc"
