#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "ui/mainwidget.h"
#include "ui/commandmanager.h"
#include "ui/selectionmanager.h"

#include "graph/graphmodel.h"

#include <QFileDialog>
#include <QIcon>
#include <QLabel>
#include <QProgressBar>
#include <QCloseEvent>
#include <QMessageBox>

const QString MainWindow::applicationName = tr("GraphTool");

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    _ui(new Ui::MainWindow),
    _statusBarProgressLabel(new QLabel),
    _statusBarProgressBar(new QProgressBar)
{
    _ui->setupUi(this);
    setWindowTitle(applicationName);

    _statusBarProgressLabel->setVisible(false);
    _ui->statusBar->addPermanentWidget(_statusBarProgressLabel);
    _statusBarProgressBar->setVisible(false);
    _statusBarProgressBar->setFixedWidth(200);
    _ui->statusBar->addPermanentWidget(_statusBarProgressBar);
}

MainWindow::~MainWindow()
{
    delete _ui;
}

void MainWindow::closeEvent(QCloseEvent* e)
{
    //FIXME: this should be handled more elegantly
    //FIXME: also, ask to save here
    for(int i = 0; i < _ui->tabs->count(); i++)
    {
        MainWidget* widget = static_cast<MainWidget*>(_ui->tabs->widget(i));
        if(widget->busy())
        {
            QMessageBox::information(nullptr, tr("Cannot close"),
                                     tr("Please wait until all operations have completed before closing."));
            e->ignore();
            return;
        }
    }

    e->accept();
}

MainWidget *MainWindow::currentTabWidget()
{
    MainWidget* widget = static_cast<MainWidget*>(_ui->tabs->currentWidget());
    return widget;
}

MainWidget* MainWindow::signalSenderTabWidget()
{
    MainWidget* widget = dynamic_cast<MainWidget*>(QObject::sender());
    return widget;
}

TabData* MainWindow::currentTabData()
{
    return tabDataForWidget(currentTabWidget());
}

TabData* MainWindow::tabDataForWidget(MainWidget* widget)
{
    if(widget != nullptr && _tabData.contains(widget))
        return &_tabData[widget];

    return nullptr;
}

TabData* MainWindow::tabDataForSignalSender()
{
    return tabDataForWidget(signalSenderTabWidget());
}

MainWidget *MainWindow::createNewTabWidget(const QString& filename)
{
    MainWidget* widget = new MainWidget;
    TabData initialTabData;
    _tabData.insert(widget, initialTabData);

    connect(widget, &MainWidget::progress, this, &MainWindow::onLoadProgress);
    connect(widget, &MainWidget::complete, this, &MainWindow::onLoadCompletion);
    connect(widget, &MainWidget::graphChanged, this, &MainWindow::onGraphChanged);
    connect(widget, &MainWidget::commandWillExecuteAsynchronously, this, &MainWindow::onCommandWillExecuteAsynchronously);
    connect(widget, &MainWidget::commandProgress, this, &MainWindow::onCommandProgress);
    connect(widget, &MainWidget::commandCompleted, this, &MainWindow::onCommandCompleted);
    connect(widget, &MainWidget::selectionChanged, this, &MainWindow::onSelectionChanged);
    connect(widget, &MainWidget::userInteractionStarted, this, &MainWindow::onUserInteractionStarted);
    connect(widget, &MainWidget::userInteractionFinished, this, &MainWindow::onUserInteractionFinished);

    widget->initFromFile(filename);

    return widget;
}

void MainWindow::closeTab(int index)
{
    MainWidget* widget = static_cast<MainWidget*>(_ui->tabs->widget(index));

    _tabData.remove(widget);
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

    _ui->actionPause_Layout->setEnabled(widget != nullptr && !widget->busy() && !widget->interacting());
}

void MainWindow::configureSelectActions()
{
    MainWidget* widget = currentTabWidget();

    bool enabled = widget != nullptr && !widget->busy() && !widget->interacting();

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

        _ui->actionDelete->setEnabled(editable && selectionNonEmpty &&
                                      !widget->busy() && !widget->interacting());
    }
    else
        _ui->actionDelete->setEnabled(false);
}

void MainWindow::configureUndoActions()
{
    MainWidget* widget;
    if((widget = currentTabWidget()) != nullptr)
    {
        _ui->actionUndo->setEnabled(widget->canUndo() && !widget->busy() && !widget->interacting());
        _ui->actionUndo->setText(widget->nextUndoAction());
        _ui->actionRedo->setEnabled(widget->canRedo() && !widget->busy() && !widget->interacting());
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
        const auto* tb = currentTabData();

        if(widget->busy())
        {
            _statusBarProgressLabel->setText(tb->commandVerb);
            _statusBarProgressLabel->setVisible(!tb->commandVerb.isEmpty());

            if(tb->commandProgress >= 0)
            {
                _statusBarProgressBar->setRange(0, 100);
                _statusBarProgressBar->setValue(tb->commandProgress);
            }
            else
            {
                // Indeterminate
                _statusBarProgressBar->setRange(0, 0);
            }

            _statusBarProgressBar->setVisible(true);
        }
        else
        {
            _statusBarProgressLabel->setVisible(false);
            _statusBarProgressBar->setVisible(false);
        }

        if(!tb->statusBarMessage.isEmpty())
            _ui->statusBar->showMessage(tb->statusBarMessage);
        else
            _ui->statusBar->clearMessage();
    }
    else
    {
        _ui->statusBar->clearMessage();
        _statusBarProgressLabel->setVisible(false);
        _statusBarProgressBar->setVisible(false);
    }
}

void MainWindow::configureUI()
{
    configurePauseLayoutAction();
    configureSelectActions();
    configureEditActions();
    configureUndoActions();
    configureStatusBar();

    MainWidget* widget;
    if((widget = currentTabWidget()) != nullptr)
        setWindowTitle(QString("%1 - %2").arg(widget->graphModel()->name()).arg(applicationName));
    else
        setWindowTitle(applicationName);
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

        int index = _ui->tabs->addTab(widget, widget->graphModel()->name());

        auto* tb = tabDataForWidget(widget);
        tb->commandVerb = tr("Loading");
        tb->commandProgress = 0;

        _ui->tabs->setCurrentIndex(index);

        configureUI();

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
    TabData* tb;
    if((tb = tabDataForSignalSender()) != nullptr)
        tb->commandProgress = percentage;

    configureUI();
}

void MainWindow::onLoadCompletion(int /*success*/)
{
    MainWidget* widget = signalSenderTabWidget();
    TabData* tb;
    if((tb = tabDataForWidget(widget)) != nullptr)
    {
        tb->commandProgress = 100;
        tb->statusBarMessage = QString(tr("Loaded %1 (%2 nodes, %3 edges, %4 components)")).arg(
                    widget->graphModel()->name()).arg(
                    widget->graphModel()->graph().numNodes()).arg(
                    widget->graphModel()->graph().numEdges()).arg(
                    widget->graphModel()->graph().numComponents());
    }

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

void MainWindow::onCommandWillExecuteAsynchronously(std::shared_ptr<const Command>, const QString& verb)
{
    TabData* tb;
    if((tb = tabDataForSignalSender()) != nullptr)
    {
        tb->commandVerb = verb;
        tb->commandProgress = -1;
    }

    configureUI();
}

void MainWindow::onCommandProgress(std::shared_ptr<const Command>, int progress)
{
    TabData* tb;
    if((tb = tabDataForSignalSender()) != nullptr)
        tb->commandProgress = progress;

    configureUI();
}

void MainWindow::onCommandCompleted(std::shared_ptr<const Command>, const QString& pastParticiple)
{
    TabData* tb;
    if((tb = tabDataForSignalSender()) != nullptr)
    {
        tb->commandProgress = 100;
        tb->statusBarMessage = pastParticiple;
    }

    configureUI();
}

void MainWindow::onSelectionChanged(const SelectionManager*)
{
    configureUI();
}

void MainWindow::onUserInteractionStarted()
{
    configureUI();
}

void MainWindow::onUserInteractionFinished()
{
    configureUI();
}
