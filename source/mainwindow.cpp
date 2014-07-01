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
    _statusBarLabel(new QLabel),
    _disableUI(false)
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

    connect(widget, &MainWidget::progress, this, &MainWindow::onLoadProgress);
    connect(widget, &MainWidget::complete, this, &MainWindow::onLoadCompletion);
    connect(widget, &MainWidget::graphChanged, this, &MainWindow::onGraphChanged);
    connect(widget, &MainWidget::commandWillExecuteAsynchronously, this, &MainWindow::onCommandWillExecuteAsynchronously);
    connect(widget, &MainWidget::commandCompleted, this, &MainWindow::onCommandCompleted);
    connect(widget, &MainWidget::selectionChanged, this, &MainWindow::onSelectionChanged);

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

void MainWindow::configurePauseLayoutAction()
{
    bool pause = false;
    MainWidget* widget;
    if((widget = currentTabWidget()) != nullptr)
        pause = widget->layoutIsPaused();

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

    _ui->actionPause_Layout->setEnabled(widget != nullptr && !_disableUI);
}

void MainWindow::configureSelectActions()
{
    bool enabled = currentTabWidget() != nullptr && !_disableUI;

    _ui->actionSelect_All->setEnabled(enabled);
    _ui->actionSelect_None->setEnabled(enabled);
    _ui->actionInvert_Selection->setEnabled(enabled);
}

void MainWindow::configureEditActions()
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

    _ui->actionDelete->setEnabled(editable && selectionNonEmpty && !_disableUI);
}

void MainWindow::configureUndoActions()
{
    MainWidget* widget;
    if((widget = currentTabWidget()) != nullptr)
    {
        _ui->actionUndo->setEnabled(widget->canUndo() && !_disableUI);
        _ui->actionUndo->setText(widget->nextUndoAction());
        _ui->actionRedo->setEnabled(widget->canRedo() && !_disableUI);
        _ui->actionRedo->setText(widget->nextRedoAction());
    }
    else
    {
        _ui->actionUndo->setEnabled(false);
        _ui->actionUndo->setText(tr("Undo"));
        _ui->actionRedo->setEnabled(false);
        _ui->actionRedo->setText(tr("Redo"));
    }
}

void MainWindow::configureStatusBar()
{
    MainWidget* widget;
    if((widget = currentTabWidget()) != nullptr)
    {
        _statusBarLabel->setText(QString(tr("%1 nodes, %2 edges, %3 components")).arg(
                                    widget->graphModel()->graph().numNodes()).arg(
                                    widget->graphModel()->graph().numEdges()).arg(
                                    widget->graphModel()->graph().numComponents()));
    }
    else
    {
        _statusBarLabel->setText("");
    }
}

void MainWindow::configureUI()
{
    configurePauseLayoutAction();
    configureSelectActions();
    configureEditActions();
    configureUndoActions();
    configureStatusBar();
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

void MainWindow::onLoadProgress(int percentage)
{
    MainWidget* widget = static_cast<MainWidget*>(sender());
    Q_ASSERT(widget != nullptr);

    int tabIndex = _ui->tabs->indexOf(widget);
    _ui->tabs->setTabText(tabIndex, QString(tr("%1 %2%")).arg(widget->graphModel()->name()).arg(percentage));
    configureUI();
}

void MainWindow::onLoadCompletion(int /*success*/)
{
    MainWidget* widget = static_cast<MainWidget*>(sender());
    Q_ASSERT(widget != nullptr);

    int tabIndex = _ui->tabs->indexOf(widget);
    _ui->tabs->setTabText(tabIndex, widget->graphModel()->name());
    configureUI();
}

void MainWindow::onGraphChanged(const Graph*)
{
    configureUI();
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
            widget->resumeLayout();
        else
            widget->pauseLayout();

        configurePauseLayoutAction();
    }
}

void MainWindow::on_tabs_currentChanged(int)
{
    configureUI();
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

void MainWindow::onCommandWillExecuteAsynchronously(const CommandManager*, const Command*)
{
    _disableUI = true;
    configureUI();
}

void MainWindow::onCommandCompleted(const CommandManager*, const Command*)
{
    _disableUI = false;
    configureUI();
}

void MainWindow::onSelectionChanged(const SelectionManager*)
{
    configureUI();
}
