#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>

class ContentPaneWidget;
class QLabel;
class Graph;

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
    ContentPaneWidget* currentTabWidget();
    ContentPaneWidget* createNewTabWidget(const QString &filename);
    void closeTab(int index);
    QString showGeneralFileOpenDialog();
    void configureActionPauseLayout(bool pause);
    void updatePerTabUi();

private slots:
    void on_actionOpen_triggered();
    void on_actionQuit_triggered();
    void on_tabs_tabCloseRequested(int index);

    void on_loadProgress(int percentage);
    void on_loadCompletion(int success);
    void on_graphChanged(const Graph* graph);

    void on_actionPause_Layout_triggered();

    void on_tabs_currentChanged(int index);

    void on_actionOpen_In_New_Tab_triggered();

private:
    Ui::MainWindow *ui;
    QLabel* statusBarLabel;
};

#endif // MAINWINDOW_H
