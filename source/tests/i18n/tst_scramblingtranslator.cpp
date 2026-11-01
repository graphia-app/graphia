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

#include "tests/common/scramblingtranslator.h"

#include <QTest>
#include <QCoreApplication>
#include <QString>

using namespace Qt::Literals::StringLiterals;

class TestScramblingTranslator : public QObject
{
    Q_OBJECT

private slots:
    void installedWhenScrambling()
    {
        const auto sourceText = u"Text"_s;
        const auto expected = qEnvironmentVariableIsSet("GRAPHIA_SCRAMBLE_TRANSLATIONS") ?
            ScramblingTranslator::scrambled(sourceText) : sourceText;

        QCOMPARE(QCoreApplication::translate("TestScramblingTranslator", "Text"), expected);
    }
};

QTEST_GUILESS_MAIN(TestScramblingTranslator)
#include "tst_scramblingtranslator.moc"
