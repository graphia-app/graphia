#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "ui/contentpanewidget.h"
#include "ui/commandmanager.h"
#include "ui/selectionmanager.h"

#include <QFileDialog>
#include <QIcon>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    _ui(new Ui::MainWindow),
    _statusBarLabel(new QLabel)
{
    _ui->setupUi(this);
    _ui->statusBar->addWidget(_statusBarLabel);
}

MainWindow::~MainWindow()
{
    delete _ui;
}

ContentPaneWidget *MainWindow::currentTabWidget()
{
    ContentPaneWidget* contentPaneWidget = static_cast<ContentPaneWidget*>(_ui->tabs->currentWidget());
    return contentPaneWidget;
}

ContentPaneWidget *MainWindow::createNewTabWidget(const QString& filename)
{
    ContentPaneWidget* contentPaneWidget = new ContentPaneWidget;

    connect(contentPaneWidget, &ContentPaneWidget::progress, this, &MainWindow::on_loadProgress);
    connect(contentPaneWidget, &ContentPaneWidget::complete, this, &MainWindow::on_loadCompletion);
    connect(contentPaneWidget, &ContentPaneWidget::graphChanged, this, &MainWindow::on_graphChanged);
    connect(contentPaneWidget, &ContentPaneWidget::commandStackChanged, this, &MainWindow::on_commandStackChanged);
    connect(contentPaneWidget, &ContentPaneWidget::selectionChanged, this, &MainWindow::on_selectionChanged);

    contentPaneWidget->initFromFile(filename);

    return contentPaneWidget;
}

void MainWindow::closeTab(int index)
{
    ContentPaneWidget* contentPaneWidget = static_cast<ContentPaneWidget*>(_ui->tabs->widget(index));

    _ui->tabs->removeTab(index);
    delete contentPaneWidget;
}

QString MainWindow::showGeneralFileOpenDialog()
{
    return QFileDialog::getOpenFileName(this, tr("Open File..."), QString(), tr("GML Files (*.gml)"));
}

void MainWindow::configureActionPauseLayout(bool pause)
{
    if(pause)
    {
        _ui->actionPause_Layout->setText(tr("Resume Layout"));
        _ui->actionPause_Layout->setIcon(QIcon::fromTheme("media-playback-start"));
    }
    else
    {
        _ui->actionPause_Layout->setText(tr("Pause Layout"));
        _ui->actionPause_Layout->setIcon(QIcon::fromTheme("media-playback-pause"));
    }
}

void MainWindow::setEditActionAvailability()
{
    bool editable = false;
    bool selectionNonEmpty = false;

    ContentPaneWidget* contentPaneWidget;
    if((contentPaneWidget = currentTabWidget()) != nullptr)
    {
        editable = contentPaneWidget->graphModel()->editable();
        selectionNonEmpty = contentPaneWidget->selectionManager() != nullptr &&
                !contentPaneWidget->selectionManager()->selectedNodes().isEmpty();
    }

    _ui->actionDelete->setEnabled(editable && selectionNonEmpty);
}

void MainWindow::updatePerTabUi()
{
    ContentPaneWidget* contentPaneWidget;
    if((contentPaneWidget = currentTabWidget()) != nullptr)
    {
        configureActionPauseLayout(contentPaneWidget->layoutIsPaused());
        _statusBarLabel->setText(QString(tr("%1 nodes, %2 edges, %3 components")).arg(
                                    contentPaneWidget->graphModel()->graph().numNodes()).arg(
                                    contentPaneWidget->graphModel()->graph().numEdges()).arg(
                                    contentPaneWidget->graphModel()->graph().numComponents()));

        _ui->actionUndo->setEnabled(contentPaneWidget->canUndo());
        _ui->actionUndo->setText(contentPaneWidget->nextUndoAction());
        _ui->actionRedo->setEnabled(contentPaneWidget->canRedo());
        _ui->actionRedo->setText(contentPaneWidget->nextRedoAction());

        setEditActionAvailability();
    }
    else
    {
        _statusBarLabel->setText("");

        _ui->actionUndo->setEnabled(false);
        _ui->actionUndo->setText(tr("Undo"));
        _ui->actionRedo->setEnabled(false);
        _ui->actionRedo->setText(tr("Redo"));

        setEditActionAvailability();
    }
}

void MainWindow::on_actionOpen_triggered()
{
    if(_ui->tabs->count() == 0)
    {
        // No tab to replace, open a new one
        on_actionOpen_In_New_Tab_triggered();
        return;
    }

    QString filename = showGeneralFileOpenDialog();
    if (!filename.isEmpty())
    {
        _ui->tabs->setUpdatesEnabled(false);
        int index = _ui->tabs->currentIndex();
        closeTab(index);

        ContentPaneWidget* contentPaneWidget = createNewTabWidget(filename);

        _ui->tabs->insertTab(index, contentPaneWidget, QString(tr("%1 0%")).arg(contentPaneWidget->graphModel()->name()));
        _ui->tabs->setCurrentIndex(index);
        _ui->tabs->setUpdatesEnabled(true);
    }
}

void MainWindow::on_actionOpen_In_New_Tab_triggered()
{
    QString filename = showGeneralFileOpenDialog();
    if (!filename.isEmpty())
        openFileInNewTab(filename);
}

bool MainWindow::openFileInNewTab(const QString& filename)
{
    QFile file(filename);

    if(file.exists())
    {
        ContentPaneWidget* contentPaneWidget = createNewTabWidget(filename);

        int index = _ui->tabs->addTab(contentPaneWidget, QString(tr("%1 0%")).arg(contentPaneWidget->graphModel()->name()));
        _ui->tabs->setCurrentIndex(index);

        return true;
    }

    return false;
}

void MainWindow::on_tabs_tabCloseRequested(int index)
{
    closeTab(index);
}

void MainWindow::on_loadProgress(int percentage)
{
    ContentPaneWidget* contentPaneWidget = static_cast<ContentPaneWidget*>(sender());
    Q_ASSERT(contentPaneWidget != nullptr);

    int tabIndex = _ui->tabs->indexOf(contentPaneWidget);
    _ui->tabs->setTabText(tabIndex, QString(tr("%1 %2%")).arg(contentPaneWidget->graphModel()->name()).arg(percentage));
    updatePerTabUi();
}

void MainWindow::on_loadCompletion(int /*success*/)
{
    ContentPaneWidget* contentPaneWidget = static_cast<ContentPaneWidget*>(sender());
    Q_ASSERT(contentPaneWidget != nullptr);

    int tabIndex = _ui->tabs->indexOf(contentPaneWidget);
    _ui->tabs->setTabText(tabIndex, contentPaneWidget->graphModel()->name());
    updatePerTabUi();
}

void MainWindow::on_graphChanged(const Graph*)
{
    updatePerTabUi();
}

void MainWindow::on_actionQuit_triggered()
{
    qApp->quit();
}

void MainWindow::on_actionPause_Layout_triggered()
{
    ContentPaneWidget* contentPaneWidget;
    if((contentPaneWidget = currentTabWidget()) != nullptr)
    {
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
}

void MainWindow::on_tabs_currentChanged(int)
{
    updatePerTabUi();
}

void MainWindow::on_actionSelect_All_triggered()
{
    ContentPaneWidget* contentPaneWidget;
    if((contentPaneWidget = currentTabWidget()) != nullptr)
        contentPaneWidget->selectAll();
}

void MainWindow::on_actionSelect_None_triggered()
{
    ContentPaneWidget* contentPaneWidget;
    if((contentPaneWidget = currentTabWidget()) != nullptr)
        contentPaneWidget->selectNone();
}

void MainWindow::on_actionInvert_Selection_triggered()
{
    ContentPaneWidget* contentPaneWidget;
    if((contentPaneWidget = currentTabWidget()) != nullptr)
        contentPaneWidget->invertSelection();
}

void MainWindow::on_actionUndo_triggered()
{
    ContentPaneWidget* contentPaneWidget;
    if((contentPaneWidget = currentTabWidget()) != nullptr)
        contentPaneWidget->undo();
}

void MainWindow::on_actionRedo_triggered()
{
    ContentPaneWidget* contentPaneWidget;
    if((contentPaneWidget = currentTabWidget()) != nullptr)
        contentPaneWidget->redo();
}

void MainWindow::on_actionDelete_triggered()
{
    ContentPaneWidget* contentPaneWidget;
    if((contentPaneWidget = currentTabWidget()) != nullptr)
        contentPaneWidget->deleteSelectedNodes();
}

void MainWindow::on_commandStackChanged(const CommandManager&)
{
    updatePerTabUi();
}

void MainWindow::on_selectionChanged(const SelectionManager&)
{
    updatePerTabUi();
}
