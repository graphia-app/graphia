#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class MainWidget;
class QLabel;
class Graph;
class CommandManager;
class SelectionManager;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private:
    MainWidget* currentTabWidget();
    MainWidget* createNewTabWidget(const QString& filename);
    void closeTab(int index);
    QString showGeneralFileOpenDialog();
    void configureActionPauseLayout(bool pause);
    void setEditActionAvailability();
    void updatePerTabUi();

private slots:
    void on_actionOpen_triggered();
    void on_actionQuit_triggered();
    void on_tabs_tabCloseRequested(int index);

    void on_loadProgress(int percentage);
    void on_loadCompletion(int success);
    void on_graphChanged(const Graph& graph);

    void on_actionPause_Layout_triggered();
    void on_tabs_currentChanged(int index);
    void on_actionOpen_In_New_Tab_triggered();
    void on_actionSelect_All_triggered();
    void on_actionSelect_None_triggered();
    void on_actionInvert_Selection_triggered();
    void on_actionUndo_triggered();
    void on_actionRedo_triggered();
    void on_actionDelete_triggered();

    void on_commandStackChanged(const CommandManager& commandManager);
    void on_selectionChanged(const SelectionManager& selectionManager);

public:
    bool openFileInNewTab(const QString& filename);

private:
    Ui::MainWindow* _ui;
    QLabel* _statusBarLabel;
};

#endif // MAINWINDOW_H
