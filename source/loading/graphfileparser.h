#ifndef GRAPHFILEPARSER_H
#define GRAPHFILEPARSER_H

#include <QObject>

#ifndef Q_MOC_RUN
#include <iostream>
#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/spirit/include/support_istream_iterator.hpp>
#endif

#include <memory>
#include <iterator>
#include <cstdint>
#include <thread>
#include <atomic>

class MutableGraph;

//FIXME rename this FileParser? files not necessarily graphs
class GraphFileParser : public QObject
{
    Q_OBJECT
signals:
    void progress(int percentage) const;

public:
    virtual bool parse(MutableGraph& graph) = 0;

private:
    std::atomic<bool> _cancelAtomic = false;
    void setCancel(bool cancel)
    {
        _cancelAtomic = cancel;
    }

public:
    void cancel()
    {
        setCancel(true);
    }

    bool cancelled()
    {
        return _cancelAtomic;
    }

public:
    virtual void onParsePositionIncremented(int64_t /*position*/) {}

    class progress_iterator : public boost::iterator_adaptor<progress_iterator,
            boost::spirit::istream_iterator>
    {
     private:
        GraphFileParser* _parser;
        int64_t _position;
        const progress_iterator* _end;
        struct enabler {};

     public:
        progress_iterator() :
            progress_iterator::iterator_adaptor_() {}

        explicit progress_iterator(const boost::spirit::istream_iterator iterator,
                                  GraphFileParser* parser, const progress_iterator* end) :
            progress_iterator::iterator_adaptor_(iterator),
              _parser(parser),
              _position(0),
              _end(end) {}

     private:
        friend class boost::iterator_core_access;

        bool equal(progress_iterator const& x) const
        {
            // If the parse has been cancelled and x is the end iterator
            if(_parser->cancelled() && x.base_reference() == _end->base_reference())
                return true;

            return this->base_reference() == x.base_reference();
        }

        void increment()
        {
            _parser->onParsePositionIncremented(_position++);
            this->base_reference()++;
        }
    };
};

class GraphFileParserThread : public QObject
{
    Q_OBJECT
private:
    MutableGraph& _graph;
    std::unique_ptr<GraphFileParser> _graphFileParser;
    std::thread _thread;

public:
    GraphFileParserThread(MutableGraph& graph, std::unique_ptr<GraphFileParser> graphFileParser);
    virtual ~GraphFileParserThread();

    void start();
    void cancel();

private:
    void run();

signals:
    void progress(int percentage) const;
    void complete(bool success) const;
};

#endif // GRAPHFILEPARSER_H
