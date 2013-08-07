#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "ui/genericgraphwidget.h"

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
        GenericGraphWidget* ggw = new GenericGraphWidget;

        connect(ggw, &GenericGraphWidget::progress,
            [](int percentage)
            {
                qDebug() << "Parse %" << percentage;
            }
        );

        connect(ggw, &GenericGraphWidget::complete,
            [](int success)
            {
                qDebug() << "Parsing complete" << success;
            }
        );

        ggw->initFromFile(filename);
    }
}

void MainWindow::on_actionQuit_triggered()
{
    qApp->quit();
}
