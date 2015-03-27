#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "ui/mainwidget.h"
#include "ui/selectionmanager.h"

#include "commands/commandmanager.h"

#include "graph/genericgraphmodel.h"
#include "graph/graphmodel.h"

#include "loading/gmlfiletype.h"
#include "loading/gmlfileparser.h"

#include "loading/pairwisetxtfiletype.h"
#include "loading/pairwisetxtfileparser.h"

#include "utils/cpp1x_hacks.h"

#include <QFileDialog>
#include <QIcon>
#include <QLabel>
#include <QProgressBar>
#include <QCloseEvent>
#include <QMessageBox>

#include <tuple>

const char* MainWindow::applicationName = "GraphTool";

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    _ui(new Ui::MainWindow),
    _statusBarProgressLabel(new QLabel),
    _statusBarProgressBar(new QProgressBar)
{
    _ui->setupUi(this);
    setWindowTitle(tr(applicationName));

    _statusBarProgressLabel->setVisible(false);
    _ui->statusBar->addPermanentWidget(_statusBarProgressLabel);
    _statusBarProgressBar->setVisible(false);
    _statusBarProgressBar->setFixedWidth(200);
    _ui->statusBar->addPermanentWidget(_statusBarProgressBar);

    _fileIdentifier.registerFileType(std::make_shared<GmlFileType>());
    _fileIdentifier.registerFileType(std::make_shared<PairwiseTxtFileType>());
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

TabData* MainWindow::tabDataForIndex(int index)
{
    MainWidget* widget = dynamic_cast<MainWidget*>(_ui->tabs->widget(index));

    return tabDataForWidget(widget);
}

TabData* MainWindow::tabDataForSignalSender()
{
    return tabDataForWidget(signalSenderTabWidget());
}

MainWidget* MainWindow::createNewTabWidget(const QString& filename)
{
    FileIdentifier::Type* fileType;
    QString baseFilename;
    std::tie(fileType, baseFilename) = _fileIdentifier.identify(filename);

    // Don't know what type this file is
    if(fileType == nullptr)
    {
        QMessageBox::critical(nullptr, tr("Unknown file type"),
            QString(tr("%1 cannot be loaded as its file type is unknown.")).arg(filename));
        return nullptr;
    }

    std::shared_ptr<GraphModel> graphModel;
    std::unique_ptr<GraphFileParser> graphFileParser;

    if(fileType->name().compare("GML") == 0)
    {
        graphModel = std::make_shared<GenericGraphModel>(baseFilename);
        graphFileParser = std::make_unique<GmlFileParser>(filename);
    }
    else if(fileType->name().compare("PairwiseTXT") == 0)
    {
        graphModel = std::make_shared<GenericGraphModel>(baseFilename);
        graphFileParser = std::make_unique<PairwiseTxtFileParser>(filename);
    }

    //FIXME what we should really be doing:
    // query which plugins can load fileTypeName
    // allow the user to choose which plugin to use if there is more than 1
    // _graphModel = plugin->graphModelForFilename(filename);
    // graphFileParser = plugin->parserForFilename(filename);

    // Couldn't find a way to interpret this file
    if(graphModel == nullptr || graphFileParser == nullptr)
    {
        QMessageBox::critical(nullptr, tr("Cannot interpret file"),
            QString(tr("%1 cannot be parsed.")).arg(filename));
        return nullptr;
    }

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

    widget->initFromFile(filename, graphModel, std::move(graphFileParser));

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
    return QFileDialog::getOpenFileName(this, tr("Open File..."), QString(), _fileIdentifier.filter());
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

void MainWindow::configureResetViewAction()
{
    MainWidget* widget;
    if((widget = currentTabWidget()) != nullptr)
        _ui->actionReset_View->setEnabled(!widget->busy() && !widget->viewIsReset());
}

void MainWindow::configureOverviewModeAction()
{
    MainWidget* widget;
    if((widget = currentTabWidget()) != nullptr)
    {
        _ui->actionOverview_Mode->setEnabled(!widget->busy() &&
                                             !widget->interacting() &&
                                             widget->mode() != GraphWidget::Mode::Overview &&
                                             widget->graphModel()->graph().numComponents() > 1);
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
    configureResetViewAction();
    configureOverviewModeAction();
    configureStatusBar();

    MainWidget* widget;
    if((widget = currentTabWidget()) != nullptr)
        setWindowTitle(QString("%1 - %2").arg(widget->graphModel()->name()).arg(tr(applicationName)));
    else
        setWindowTitle(tr(applicationName));
}

void MainWindow::configureUIForLoadingOnTabIndex(int index)
{
    auto* tb = tabDataForIndex(index);
    tb->commandVerb = tr("Loading");
    tb->commandProgress = 0;

    _ui->tabs->setCurrentIndex(index);

    configureUI();
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
    if(!filename.isEmpty())
    {
        MainWidget* widget = createNewTabWidget(filename);

        if(widget != nullptr)
        {
            _ui->tabs->setUpdatesEnabled(false);
            int index = _ui->tabs->currentIndex();
            closeTab(index);

            _ui->tabs->insertTab(index, widget, widget->graphModel()->name());
            configureUIForLoadingOnTabIndex(index);

            _ui->tabs->setUpdatesEnabled(true);
        }
    }
}

void MainWindow::on_actionOpen_In_New_Tab_triggered()
{
    QString filename = showGeneralFileOpenDialog();
    if (!filename.isEmpty())
        openFileInNewTab(filename);
}

void MainWindow::openFileInNewTab(const QString& filename)
{
   MainWidget* widget = createNewTabWidget(filename);

   if(widget != nullptr)
   {
        int index = _ui->tabs->addTab(widget, widget->graphModel()->name());
        configureUIForLoadingOnTabIndex(index);
   }
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

void MainWindow::onCommandWillExecuteAsynchronously(const Command*, const QString& verb)
{
    TabData* tb;
    if((tb = tabDataForSignalSender()) != nullptr)
    {
        tb->commandVerb = verb;
        tb->commandProgress = -1;
    }

    configureUI();
}

void MainWindow::onCommandProgress(const Command*, int progress)
{
    TabData* tb;
    if((tb = tabDataForSignalSender()) != nullptr)
        tb->commandProgress = progress;

    configureUI();
}

void MainWindow::onCommandCompleted(const Command* command, const QString& pastParticiple)
{
    TabData* tb;
    if((tb = tabDataForSignalSender()) != nullptr)
    {
        tb->commandProgress = 100;

        if(command != nullptr)
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

void MainWindow::on_actionReset_View_triggered()
{
    MainWidget* widget;
    if((widget = currentTabWidget()) != nullptr)
        widget->resetView();

    configureUI();
}

void MainWindow::on_actionOverview_Mode_triggered()
{
    MainWidget* widget;
    if((widget = currentTabWidget()) != nullptr)
        widget->switchToOverviewMode();

    configureUI();
}
