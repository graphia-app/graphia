#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "ui/contentpanewidget.h"

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
        ContentPaneWidget* contentPaneWidget = new ContentPaneWidget;

        connect(contentPaneWidget, &ContentPaneWidget::progress, this, &MainWindow::on_loadProgress);
        connect(contentPaneWidget, &ContentPaneWidget::complete, this, &MainWindow::on_loadCompletion);

        contentPaneWidget->initFromFile(filename);

        ui->tabs->addTab(contentPaneWidget, QString(tr("%1 0%")).arg(contentPaneWidget->graphModel()->name()));
    }
}

void MainWindow::on_tabs_tabCloseRequested(int index)
{
    ContentPaneWidget* contentPaneWidget = static_cast<ContentPaneWidget*>(ui->tabs->widget(index));
    delete contentPaneWidget;
}

void MainWindow::on_loadProgress(int percentage)
{
    ContentPaneWidget* contentPaneWidget = static_cast<ContentPaneWidget*>(sender());
    Q_ASSERT(contentPaneWidget != nullptr);

    int tabIndex = ui->tabs->indexOf(contentPaneWidget);
    ui->tabs->setTabText(tabIndex, QString(tr("%1 %2%")).arg(contentPaneWidget->graphModel()->name()).arg(percentage));
}

void MainWindow::on_loadCompletion(int /*success*/)
{
    ContentPaneWidget* contentPaneWidget = static_cast<ContentPaneWidget*>(sender());
    Q_ASSERT(contentPaneWidget != nullptr);

    int tabIndex = ui->tabs->indexOf(contentPaneWidget);
    ui->tabs->setTabText(tabIndex, contentPaneWidget->graphModel()->name());
}

void MainWindow::on_actionQuit_triggered()
{
    qApp->quit();
}
