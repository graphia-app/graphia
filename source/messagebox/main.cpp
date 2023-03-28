/* Copyright © 2013-2023 Graphia Technologies Ltd.
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

#include "build_defines.h"

#include <QApplication>
#include <QCoreApplication>
#include <QMessageBox>
#include <QAbstractButton>
#include <QCommandLineParser>
#include <QStringList>
#include <QIcon>
#include <QHash>

using namespace Qt::Literals::StringLiterals;

static QMessageBox::Icon parseIcon(const QString& text)
{
    QHash<QString, QMessageBox::Icon> icons =
    {
        {u"Question"_s,    QMessageBox::Question},
        {u"Information"_s, QMessageBox::Information},
        {u"Warning"_s,     QMessageBox::Warning},
        {u"Critical"_s,    QMessageBox::Critical},
    };

    if(icons.contains(text))
        return icons[text];

    return QMessageBox::NoIcon;
}

struct Button
{
    QString _text;
    QMessageBox::ButtonRole _role = QMessageBox::InvalidRole;
};

static Button parseButton(const QString& text)
{
    Button button;
    auto tokens = text.split(':');

    if(tokens.size() != 2)
        return button;

    button._text = tokens.at(0).trimmed();

    QHash<QString, QMessageBox::ButtonRole> roles =
    {
        {u"Accept"_s,      QMessageBox::AcceptRole},
        {u"Reject"_s,      QMessageBox::RejectRole},
        {u"Destructive"_s, QMessageBox::DestructiveRole},
        {u"Action"_s,      QMessageBox::ActionRole},
        {u"Help"_s,        QMessageBox::HelpRole},
        {u"Yes"_s,         QMessageBox::YesRole},
        {u"No"_s,          QMessageBox::NoRole},
        {u"Apply"_s,       QMessageBox::ApplyRole},
        {u"Reset"_s,       QMessageBox::ResetRole},
    };

    auto role = tokens.at(1).trimmed();

    if(roles.contains(role))
        button._role = roles[role];

    return button;
}

int main(int argc, char *argv[])
{
    QStringList arguments;
    arguments.reserve(argc);
    for(int i = 0; i < argc; i++)
        arguments << QString::fromLocal8Bit(argv[i]);

    const QApplication app(argc, argv);

    QCoreApplication::setOrganizationName(u"Graphia"_s);
    QCoreApplication::setOrganizationDomain(u"graphia.app"_s);
    QCoreApplication::setApplicationName(QStringLiteral(PRODUCT_NAME));
    QCoreApplication::setApplicationVersion(QStringLiteral(VERSION));

    QIcon mainIcon;
    mainIcon.addFile(u":/icon/icon.svg"_s);
    QApplication::setWindowIcon(mainIcon);

    QCommandLineParser p;

    p.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    p.addHelpOption();
    p.addOptions(
    {
        {{"t", "title"},            QObject::tr("Window title."),    "title",           u"MessageBox"_s},
        {{"q", "text"},             QObject::tr("Window text."),     "text",
            QStringLiteral("Lorem ipsum dolor sit amet, consectetur adipiscing elit, "
                "sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. "
                "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris "
                "nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in "
                "reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla "
                "pariatur. Excepteur sint occaecat cupidatat non proident, sunt in "
                "culpa qui officia deserunt mollit anim id est laborum.")},
        {{"i", "icon"},             QObject::tr("Window icon."),     "icon",            u"Question"_s},
        {{"b", "button"},           QObject::tr("Window button."),   "button",          u"OK:Accept"_s},
        {{"d", "defaultButton"},    QObject::tr("Default button."),  "defaultButton"},
    });

    p.process(arguments);

    QMessageBox mb;

    mb.setWindowTitle(p.value(u"title"_s));
    mb.setText(p.value(u"text"_s));
    mb.setIcon(parseIcon(p.value(u"icon"_s)));
    auto defaultButtonText = p.value(u"defaultButton"_s);

    auto buttonTexts = p.values(u"button"_s);
    for(const auto& buttonText : buttonTexts)
    {
        auto button = parseButton(buttonText);

        auto* pushButton = mb.addButton(button._text, button._role);

        if(defaultButtonText == buttonText)
            mb.setDefaultButton(pushButton);
    }

    mb.exec();

    return mb.buttonRole(mb.clickedButton());
}
