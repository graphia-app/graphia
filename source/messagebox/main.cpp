/* Copyright © 2013-2022 Graphia Technologies Ltd.
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

#include <QApplication>
#include <QCoreApplication>
#include <QMessageBox>
#include <QAbstractButton>
#include <QCommandLineParser>
#include <QStringList>
#include <QIcon>
#include <QHash>

static QMessageBox::Icon parseIcon(const QString& text)
{
    QHash<QString, QMessageBox::Icon> icons =
    {
        {QStringLiteral("Question"),    QMessageBox::Question},
        {QStringLiteral("Information"), QMessageBox::Information},
        {QStringLiteral("Warning"),     QMessageBox::Warning},
        {QStringLiteral("Critical"),    QMessageBox::Critical},
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
        {QStringLiteral("Accept"),      QMessageBox::AcceptRole},
        {QStringLiteral("Reject"),      QMessageBox::RejectRole},
        {QStringLiteral("Destructive"), QMessageBox::DestructiveRole},
        {QStringLiteral("Action"),      QMessageBox::ActionRole},
        {QStringLiteral("Help"),        QMessageBox::HelpRole},
        {QStringLiteral("Yes"),         QMessageBox::YesRole},
        {QStringLiteral("No"),          QMessageBox::NoRole},
        {QStringLiteral("Apply"),       QMessageBox::ApplyRole},
        {QStringLiteral("Reset"),       QMessageBox::ResetRole},
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

    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName(QStringLiteral("Graphia"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("graphia.app"));
    QCoreApplication::setApplicationName(QStringLiteral(PRODUCT_NAME));
    QCoreApplication::setApplicationVersion(QStringLiteral(VERSION));

    QIcon mainIcon;
    mainIcon.addFile(QStringLiteral(":/icon/icon.svg"));
    QApplication::setWindowIcon(mainIcon);

    QCommandLineParser p;

    p.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    p.addHelpOption();
    p.addOptions(
    {
        {{"t", "title"},            QObject::tr("Window title."),    "title",           QStringLiteral("MessageBox")},
        {{"q", "text"},             QObject::tr("Window text."),     "text",
            QStringLiteral("Lorem ipsum dolor sit amet, consectetur adipiscing elit, "
                "sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. "
                "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris "
                "nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in "
                "reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla "
                "pariatur. Excepteur sint occaecat cupidatat non proident, sunt in "
                "culpa qui officia deserunt mollit anim id est laborum.")},
        {{"i", "icon"},             QObject::tr("Window icon."),     "icon",            QStringLiteral("Question")},
        {{"b", "button"},           QObject::tr("Window button."),   "button",          QStringLiteral("OK:Accept")},
        {{"d", "defaultButton"},    QObject::tr("Default button."),  "defaultButton"},
    });

    p.process(arguments);

    QMessageBox mb;

    mb.setWindowTitle(p.value(QStringLiteral("title")));
    mb.setText(p.value(QStringLiteral("text")));
    mb.setIcon(parseIcon(p.value(QStringLiteral("icon"))));
    auto defaultButtonText = p.value(QStringLiteral("defaultButton"));

    auto buttonTexts = p.values(QStringLiteral("button"));
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
