#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QMap>

#include <memory>

class MainWidget;
class QLabel;
class QProgressBar;
class Graph;
class Command;
class CommandManager;
class SelectionManager;
class QCloseEvent;

namespace Ui {
class MainWindow;
}

struct TabData
{
    TabData() :
        commandVerb(QString()),
        commandProgress(0)
    {}

    QString commandVerb;
    int commandProgress;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent* e);
    
private:
    MainWidget* currentTabWidget();
    TabData* currentTabData();
    TabData* tabDataForWidget(MainWidget* widget);

    MainWidget* createNewTabWidget(const QString& filename);
    void closeTab(int index);
    QString showGeneralFileOpenDialog();
    void configurePauseLayoutAction();
    void configureSelectActions();
    void configureEditActions();
    void configureUndoActions();
    void configureStatusBar();
    void configureUI();

private slots:
    void on_actionOpen_triggered();
    void on_actionQuit_triggered();
    void on_tabs_tabCloseRequested(int index);

    void on_actionPause_Layout_triggered();
    void on_tabs_currentChanged(int index);
    void on_actionOpen_In_New_Tab_triggered();
    void on_actionSelect_All_triggered();
    void on_actionSelect_None_triggered();
    void on_actionInvert_Selection_triggered();
    void on_actionUndo_triggered();
    void on_actionRedo_triggered();
    void on_actionDelete_triggered();

    void onLoadProgress(int percentage);
    void onLoadCompletion(int success);
    void onGraphChanged(const Graph* graph);

    void onCommandWillExecuteAsynchronously(std::shared_ptr<const Command> command, const QString& verb);
    void onCommandProgress(std::shared_ptr<const Command> command, int progress);
    void onCommandCompleted(std::shared_ptr<const Command> command);
    void onSelectionChanged(const SelectionManager* selectionManager);

public:
    bool openFileInNewTab(const QString& filename);

private:
    Ui::MainWindow* _ui;
    QMap<MainWidget*, TabData> _tabData;
    QLabel* _statusBarLabel;
    QLabel* _statusBarProgressLabel;
    QProgressBar* _statusBarProgressBar;
};

#endif // MAINWINDOW_H
