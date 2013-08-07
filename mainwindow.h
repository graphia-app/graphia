#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>

#include "ui/graphwidget.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private slots:
    void on_actionOpen_triggered();
    void on_actionQuit_triggered();

private:
    Ui::MainWindow *ui;
    QList<GraphWidget*> graphWidgets;
};

#endif // MAINWINDOW_H
