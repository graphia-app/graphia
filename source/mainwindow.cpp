#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "ui/mainwidget.h"
#include "ui/commandmanager.h"
#include "ui/selectionmanager.h"

#include "graph/graphmodel.h"

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

MainWidget *MainWindow::currentTabWidget()
{
    MainWidget* widget = static_cast<MainWidget*>(_ui->tabs->currentWidget());
    return widget;
}

MainWidget *MainWindow::createNewTabWidget(const QString& filename)
{
    MainWidget* widget = new MainWidget;

    connect(widget, &MainWidget::progress, this, &MainWindow::on_loadProgress);
    connect(widget, &MainWidget::complete, this, &MainWindow::on_loadCompletion);
    connect(widget, &MainWidget::graphChanged, this, &MainWindow::on_graphChanged);
    connect(widget, &MainWidget::commandStackChanged, this, &MainWindow::on_commandStackChanged);
    connect(widget, &MainWidget::selectionChanged, this, &MainWindow::on_selectionChanged);

    widget->initFromFile(filename);

    return widget;
}

void MainWindow::closeTab(int index)
{
    MainWidget* widget = static_cast<MainWidget*>(_ui->tabs->widget(index));

    _ui->tabs->removeTab(index);
    delete widget;
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

    MainWidget* widget;
    if((widget = currentTabWidget()) != nullptr)
    {
        editable = widget->graphModel()->editable();
        selectionNonEmpty = widget->selectionManager() != nullptr &&
                !widget->selectionManager()->selectedNodes().empty();
    }

    _ui->actionDelete->setEnabled(editable && selectionNonEmpty);
}

void MainWindow::updatePerTabUi()
{
    MainWidget* widget;
    if((widget = currentTabWidget()) != nullptr)
    {
        configureActionPauseLayout(widget->layoutIsPaused());
        _statusBarLabel->setText(QString(tr("%1 nodes, %2 edges, %3 components")).arg(
                                    widget->graphModel()->graph()->numNodes()).arg(
                                    widget->graphModel()->graph()->numEdges()).arg(
                                    widget->graphModel()->graph()->numComponents()));

        _ui->actionUndo->setEnabled(widget->canUndo());
        _ui->actionUndo->setText(widget->nextUndoAction());
        _ui->actionRedo->setEnabled(widget->canRedo());
        _ui->actionRedo->setText(widget->nextRedoAction());

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

        MainWidget* widget = createNewTabWidget(filename);

        _ui->tabs->insertTab(index, widget, QString(tr("%1 0%")).arg(widget->graphModel()->name()));
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
        MainWidget* widget = createNewTabWidget(filename);

        int index = _ui->tabs->addTab(widget, QString(tr("%1 0%")).arg(widget->graphModel()->name()));
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
    MainWidget* widget = static_cast<MainWidget*>(sender());
    Q_ASSERT(widget != nullptr);

    int tabIndex = _ui->tabs->indexOf(widget);
    _ui->tabs->setTabText(tabIndex, QString(tr("%1 %2%")).arg(widget->graphModel()->name()).arg(percentage));
    updatePerTabUi();
}

void MainWindow::on_loadCompletion(int /*success*/)
{
    MainWidget* widget = static_cast<MainWidget*>(sender());
    Q_ASSERT(widget != nullptr);

    int tabIndex = _ui->tabs->indexOf(widget);
    _ui->tabs->setTabText(tabIndex, widget->graphModel()->name());
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
    MainWidget* widget;
    if((widget = currentTabWidget()) != nullptr)
    {
        if(widget->layoutIsPaused())
        {
            widget->resumeLayout();
            configureActionPauseLayout(false);
        }
        else
        {
            widget->pauseLayout();
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
    MainWidget* widget;
    if((widget = currentTabWidget()) != nullptr)
        widget->selectAll();
}

void MainWindow::on_actionSelect_None_triggered()
{
    MainWidget* widget;
    if((widget = currentTabWidget()) != nullptr)
        widget->selectNone();
}

void MainWindow::on_actionInvert_Selection_triggered()
{
    MainWidget* widget;
    if((widget = currentTabWidget()) != nullptr)
        widget->invertSelection();
}

void MainWindow::on_actionUndo_triggered()
{
    MainWidget* widget;
    if((widget = currentTabWidget()) != nullptr)
        widget->undo();
}

void MainWindow::on_actionRedo_triggered()
{
    MainWidget* widget;
    if((widget = currentTabWidget()) != nullptr)
        widget->redo();
}

void MainWindow::on_actionDelete_triggered()
{
    MainWidget* widget;
    if((widget = currentTabWidget()) != nullptr)
        widget->deleteSelectedNodes();
}

void MainWindow::on_commandStackChanged(const CommandManager&)
{
    updatePerTabUi();
}

void MainWindow::on_selectionChanged(const SelectionManager&)
{
    updatePerTabUi();
}
