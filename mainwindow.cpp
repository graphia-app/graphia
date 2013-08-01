#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "graph/qgraph.h"
#include "graph/grapharray.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionOpen_triggered()
{
    QGraph graph;
    NodeArray<QString> nodeArray(graph);
}

void MainWindow::on_actionQuit_triggered()
{
    qApp->quit();
}
