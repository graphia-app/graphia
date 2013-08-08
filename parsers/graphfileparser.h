#ifndef GRAPHFILEPARSER_H
#define GRAPHFILEPARSER_H

#include <QObject>
#include <QMutex>
#include <QMutexLocker>

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
    GraphFileParser() : _cancelled(false) {}

    virtual bool parse(Graph& graph) = 0;

private:
    QMutex cancelMutex;
    bool _cancelled;

public:
    void cancel()
    {
        QMutexLocker lock(&cancelMutex);
        _cancelled = true;
    }

    bool cancelled()
    {
        QMutexLocker lock(&cancelMutex);
        return _cancelled;
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

#endif // GRAPHFILEPARSER_H
