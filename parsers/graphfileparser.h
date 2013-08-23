#ifndef GRAPHFILEPARSER_H
#define GRAPHFILEPARSER_H

#include <QObject>
#include <QThread>
#include <QAtomicInt>

#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/spirit/include/support_istream_iterator.hpp>

#include <iterator>
#include <cstdint>

#include "../graph/graph.h"

class GraphFileParser : public QObject
{
    Q_OBJECT
signals:
    void progress(int percentage) const;
    void complete(bool success) const;

public:
    GraphFileParser() : cancelAtomic(0) {}

    virtual bool parse(Graph& graph) = 0;

private:
    QAtomicInt cancelAtomic;
    void setCancel(bool cancel)
    {
        int expectedValue = static_cast<int>(!cancel);
        int newValue = static_cast<int>(!!cancel);

        cancelAtomic.testAndSetRelaxed(expectedValue, newValue);
    }

public:
    void cancel()
    {
        setCancel(true);
    }

    bool cancelled()
    {
        return cancelAtomic.testAndSetRelaxed(1, 1);
    }

public:
    virtual void onParsePositionIncremented(int64_t /*position*/) {}

    class progress_iterator : public boost::iterator_adaptor<progress_iterator,
            boost::spirit::istream_iterator>
    {
     private:
        GraphFileParser* parser;
        int64_t position;
        const progress_iterator* end;
        struct enabler {};

     public:
        progress_iterator()
            : progress_iterator::iterator_adaptor_() {}

        explicit progress_iterator(const boost::spirit::istream_iterator iterator,
                                  GraphFileParser* parser, const progress_iterator* end)
            : progress_iterator::iterator_adaptor_(iterator),
              parser(parser),
              position(0),
              end(end) {}

     private:
        friend class boost::iterator_core_access;

        bool equal(progress_iterator const& x) const
        {
            // If the parse has been cancelled and x is the end iterator
            if(parser->cancelled() && x.base_reference() == end->base_reference())
                return true;

            return this->base_reference() == x.base_reference();
        }

        void increment()
        {
            parser->onParsePositionIncremented(position++);
            this->base_reference()++;
        }
    };
};

class GraphFileParserThread : public QThread
{
    Q_OBJECT
private:
    QString filename;
    Graph* graph;
    GraphFileParser* graphFileParser;

public:
    GraphFileParserThread(const QString& filename, Graph& graph, GraphFileParser* graphFileParser) :
        filename(filename),
        graph(&graph),
        graphFileParser(graphFileParser)
    {
        // Take ownership of the parser
        graphFileParser->moveToThread(this);
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

#endif // GRAPHFILEPARSER_H
