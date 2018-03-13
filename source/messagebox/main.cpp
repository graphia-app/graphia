#include <QApplication>
#include <QCoreApplication>
#include <QMessageBox>
#include <QAbstractButton>
#include <QCommandLineParser>
#include <QStringList>
#include <QIcon>
#include <QSocketNotifier>
#include <QDebug>

static QMessageBox::Icon parseIcon(const QString& text)
{
    if(text == "Question")
        return QMessageBox::Question;
    else if(text == "Information")
        return QMessageBox::Information;
    else if(text == "Warning")
        return QMessageBox::Warning;
    else if(text == "Critical")
        return QMessageBox::Critical;

    return QMessageBox::NoIcon;
}

struct Button
{
    QString _text;
    QMessageBox::ButtonRole _role;
};

static Button parseButton(const QString& text)
{
    Button button;
    auto tokens = text.split(':');

    if(tokens.size() != 2)
        return button;

    button._text = tokens.at(0).trimmed();

    auto role = tokens.at(1).trimmed();
    if(role == "Accept")
        button._role = QMessageBox::AcceptRole;
    else if(role == "Reject")
        button._role = QMessageBox::RejectRole;
    else if(role == "Destructive")
        button._role = QMessageBox::DestructiveRole;
    else if(role == "Action")
        button._role = QMessageBox::ActionRole;
    else if(role == "Help")
        button._role = QMessageBox::HelpRole;
    else if(role == "Yes")
        button._role = QMessageBox::YesRole;
    else if(role == "No")
        button._role = QMessageBox::NoRole;
    else if(role == "Apply")
        button._role = QMessageBox::ApplyRole;
    else if(role == "Reset")
        button._role = QMessageBox::ResetRole;

    return button;
}

int main(int argc, char *argv[])
{
    QStringList arguments;
    for(int i = 0; i < argc; i++)
        arguments << QString::fromLocal8Bit(argv[i]);

    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName(QStringLiteral("Kajeka"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("kajeka.com"));
    QCoreApplication::setApplicationName(QStringLiteral(PRODUCT_NAME));
    QCoreApplication::setApplicationVersion(QStringLiteral(VERSION));

    QIcon mainIcon;
    mainIcon.addFile(QStringLiteral(":/icon/icon.svg"));
    app.setWindowIcon(mainIcon);

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

    mb.setWindowTitle(p.value("title"));
    mb.setText(p.value("text"));
    mb.setIcon(parseIcon(p.value("icon")));
    auto defaultButtonText = p.value("defaultButton");

    for(const auto& buttonText : p.values("button"))
    {
        auto button = parseButton(buttonText);

        auto pushButton = mb.addButton(button._text, button._role);

        if(defaultButtonText == buttonText)
            mb.setDefaultButton(pushButton);
    }

    mb.exec();

    return mb.buttonRole(mb.clickedButton());
}
