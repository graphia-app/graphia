#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "parsers/gmlfileparser.h"
#include "graph/graph.h"
#include "graph/grapharray.h"
#include "layout/randomlayout.h"
#include "layout/eadeslayout.h"

#include <QFileDialog>

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
    QString filename = QFileDialog::getOpenFileName(this, tr("Open GML File..."),
                                              QString(), tr("GML Files (*.gml)"));
    if (!filename.isEmpty())
    {
        GmlFileParser gmlFileParser(filename);
        Graph graph;
        gmlFileParser.parse(graph);
        graph.dumpToQDebug(1);

        NodeArray<QVector3D> positions(graph);

        RandomLayout randomLayout(positions);
        randomLayout.execute();
        positions.dumpToQDebug(1);

        EadesLayout eadesLayout(positions);

        for(int i = 0; i < 100; i++)
        {
            qDebug() << i;
            eadesLayout.execute();
        }
        positions.dumpToQDebug(1);
    }
}

void MainWindow::on_actionQuit_triggered()
{
    qApp->quit();
}
