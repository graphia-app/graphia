#ifndef CONTENTPANEWIDGET_H
#define CONTENTPANEWIDGET_H

#include <QWidget>
#include <QThread>

#include "../graph/graphmodel.h"

class LoaderThread;

class ContentPaneWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ContentPaneWidget(QWidget* parent = 0);
    virtual ~ContentPaneWidget();

signals:
    void progress(int percentage) const;
    void complete(int success) const;

public slots:
    void onProgress(int percentage) const { emit progress(percentage); }
    void onCompletion(int success);

private:
    GraphModel* _graphModel;
    bool _initialised;
    LoaderThread *loaderThread;

public:
    GraphModel* graphModel() { return _graphModel; }

    bool initFromFile(const QString& filename);
};

#include "../parsers/graphfileparser.h"

class LoaderThread : public QThread
{
    Q_OBJECT
private:
    QString filename;
    Graph* graph;
    GraphFileParser* graphFileParser;
    ContentPaneWidget* contentPaneWidget;

public:
    LoaderThread(const QString& filename, Graph& graph,
                 GraphFileParser* graphFileParser, ContentPaneWidget* contentPaneWidget) :
        filename(filename),
        graph(&graph),
        graphFileParser(graphFileParser),
        contentPaneWidget(contentPaneWidget)
    {
        // Take ownership of the parser
        graphFileParser->moveToThread(this);

        connect(graphFileParser, &GraphFileParser::progress,
                contentPaneWidget, &ContentPaneWidget::onProgress);
        connect(graphFileParser, &GraphFileParser::complete,
                contentPaneWidget, &ContentPaneWidget::onCompletion);
    }

    void cancel()
    {
        if(this->isRunning())
            graphFileParser->cancel();
    }

private:
    void run() Q_DECL_OVERRIDE
    {
        graphFileParser->parse(*graph);
        delete graphFileParser;
    }
};

#endif // CONTENTPANEWIDGET_H
