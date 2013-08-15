#ifndef CONTENTPANEWIDGET_H
#define CONTENTPANEWIDGET_H

#include <QWidget>

#include "../graph/graphmodel.h"

class GraphFileParserThread;

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
    GraphFileParserThread *graphFileParserThread;

public:
    GraphModel* graphModel() { return _graphModel; }

    bool initFromFile(const QString& filename);
};

#endif // CONTENTPANEWIDGET_H
