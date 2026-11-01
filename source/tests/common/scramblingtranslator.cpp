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

#include "scramblingtranslator.h"

#include <QCoreApplication>
#include <QRegularExpression>
#include <QStringList>

#include <algorithm>
#include <iterator>
#include <ranges>

using namespace Qt::Literals::StringLiterals;

QString ScramblingTranslator::scrambled(const QString& sourceText)
{
    // Placeholders, rich text tags and entities, which are left as they are
    static const QRegularExpression markupRegex(uR"(%L?(\d{1,2}|n)|<[^>]*>|&[a-zA-Z]+;|&#\d+;)"_s);

    const QString lower = u"áƀçďéƒĝĥíĵķĺɱñóƥʠŕšťúṽŵẋýž"_s;
    const QString upper = u"ÁƁÇĎÉƑĜĤÍĴĶĹṀÑÓƤɊŔŠŤÚṼŴẊÝŽ"_s;

    auto accented = [&](const QString& text)
    {
        QString result;
        result.reserve(text.size());

        for(const auto c : text)
        {
            if(c >= u'a' && c <= u'z')
                result.append(lower.at(c.unicode() - u'a'));
            else if(c >= u'A' && c <= u'Z')
                result.append(upper.at(c.unicode() - u'A'));
            else
                result.append(c);
        }

        return result;
    };

    auto isNumberedPlaceholder = [](const QString& markup)
        { return markup.startsWith(u'%') && !markup.endsWith(u'n'); };

    QStringList texts;
    QStringList markups;
    qsizetype position = 0;

    for(const auto& match : markupRegex.globalMatch(sourceText))
    {
        texts.append(sourceText.mid(position, match.capturedStart() - position)); // clazy:exclude=reserve-candidates
        markups.append(match.captured()); // clazy:exclude=reserve-candidates
        position = match.capturedEnd();
    }

    texts.append(sourceText.mid(position));

    // Numbered placeholders swap places, as a translation's word order would
    QStringList reordered;
    std::ranges::copy_if(std::views::reverse(markups), std::back_inserter(reordered), isNumberedPlaceholder);

    QString result;
    for(qsizetype i = 0; i < markups.size(); i++)
    {
        result.append(accented(texts.at(i)));

        const auto& markup = markups.at(i);
        result.append(isNumberedPlaceholder(markup) ? reordered.takeFirst() : markup);
    }

    result.append(accented(texts.last()));

    return result;
}

QString ScramblingTranslator::translate(const char*, const char* sourceText, const char*, int) const
{
    return scrambled(QString::fromUtf8(sourceText));
}

static void installScramblingTranslator()
{
    if(qEnvironmentVariableIsSet("GRAPHIA_SCRAMBLE_TRANSLATIONS"))
    {
        static ScramblingTranslator translator;
        QCoreApplication::installTranslator(&translator);
    }
}

Q_COREAPP_STARTUP_FUNCTION(installScramblingTranslator)
