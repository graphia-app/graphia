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
        GenericGraphWidget* graphWidget = new GenericGraphWidget;

        connect(graphWidget, &GraphWidget::progress, this, &MainWindow::on_loadProgress);
        connect(graphWidget, &GraphWidget::complete, this, &MainWindow::on_loadCompletion);

        graphWidget->initFromFile(filename);

        ui->tabs->addTab(graphWidget, QString(tr("%1 0%")).arg(graphWidget->name()));
    }
}

void MainWindow::on_tabs_tabCloseRequested(int index)
{
    GraphWidget* graphWidget = static_cast<GraphWidget*>(ui->tabs->widget(index));

    graphWidget->cancelInitialisation();
    delete graphWidget;
}

void MainWindow::on_loadProgress(int percentage)
{
    GraphWidget* graphWidget = static_cast<GraphWidget*>(sender());
    Q_ASSERT(graphWidget != nullptr);

    int tabIndex = ui->tabs->indexOf(graphWidget);
    ui->tabs->setTabText(tabIndex, QString(tr("%1 %2%")).arg(graphWidget->name()).arg(percentage));
}

void MainWindow::on_loadCompletion(int /*success*/)
{
    GraphWidget* graphWidget = static_cast<GraphWidget*>(sender());
    Q_ASSERT(graphWidget != nullptr);

    int tabIndex = ui->tabs->indexOf(graphWidget);
    ui->tabs->setTabText(tabIndex, graphWidget->name());
}

void MainWindow::on_actionQuit_triggered()
{
    qApp->quit();
}
