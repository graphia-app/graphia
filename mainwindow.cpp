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

ContentPaneWidget *MainWindow::currentTabWidget()
{
    ContentPaneWidget* contentPaneWidget = static_cast<ContentPaneWidget*>(ui->tabs->currentWidget());
    Q_ASSERT(contentPaneWidget != nullptr);
    return contentPaneWidget;
}

ContentPaneWidget *MainWindow::createNewTabWidget(const QString& filename)
{
    ContentPaneWidget* contentPaneWidget = new ContentPaneWidget;

    connect(contentPaneWidget, &ContentPaneWidget::progress, this, &MainWindow::on_loadProgress);
    connect(contentPaneWidget, &ContentPaneWidget::complete, this, &MainWindow::on_loadCompletion);

    contentPaneWidget->initFromFile(filename);

    return contentPaneWidget;
}

QString MainWindow::showGeneralFileOpenDialog()
{
    return QFileDialog::getOpenFileName(this, tr("Open File..."), QString(), tr("GML Files (*.gml)"));
}

void MainWindow::configureActionPauseLayout(bool pause)
{
    if(pause)
    {
        ui->actionPause_Layout->setText(tr("Resume Layout"));
    }
    else
    {
        ui->actionPause_Layout->setText(tr("Pause Layout"));
    }
}

void MainWindow::on_actionOpen_triggered()
{
    if(ui->tabs->count() == 0)
    {
        // No tab to replace, open a new one
        on_actionOpen_In_New_Tab_triggered();
        return;
    }

    QString filename = showGeneralFileOpenDialog();
    if (!filename.isEmpty())
    {
        ui->tabs->setUpdatesEnabled(false);
        int index = ui->tabs->currentIndex();
        ui->tabs->removeTab(index);

        ContentPaneWidget* contentPaneWidget = createNewTabWidget(filename);

        ui->tabs->insertTab(index, contentPaneWidget, QString(tr("%1 0%")).arg(contentPaneWidget->graphModel()->name()));
        ui->tabs->setCurrentIndex(index);
        ui->tabs->setUpdatesEnabled(true);
    }
}

void MainWindow::on_actionOpen_In_New_Tab_triggered()
{
    QString filename = showGeneralFileOpenDialog();
    if (!filename.isEmpty())
    {
        ContentPaneWidget* contentPaneWidget = createNewTabWidget(filename);

        int index = ui->tabs->addTab(contentPaneWidget, QString(tr("%1 0%")).arg(contentPaneWidget->graphModel()->name()));
        ui->tabs->setCurrentIndex(index);
    }
}

void MainWindow::on_tabs_tabCloseRequested(int index)
{
    /*ContentPaneWidget* contentPaneWidget = static_cast<ContentPaneWidget*>(ui->tabs->widget(index));
    delete contentPaneWidget;*/
    ui->tabs->removeTab(index);
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

void MainWindow::on_actionPause_Layout_triggered()
{
    ContentPaneWidget* contentPaneWidget = static_cast<ContentPaneWidget*>(ui->tabs->currentWidget());

    if(contentPaneWidget->layoutIsPaused())
    {
        contentPaneWidget->resumeLayout();
        configureActionPauseLayout(false);
    }
    else
    {
        contentPaneWidget->pauseLayout();
        configureActionPauseLayout(true);
    }
}

void MainWindow::on_tabs_currentChanged(int)
{
    ContentPaneWidget* contentPaneWidget = static_cast<ContentPaneWidget*>(ui->tabs->currentWidget());

    if (contentPaneWidget != nullptr)
        configureActionPauseLayout(contentPaneWidget->layoutIsPaused());
}
