#ifndef GRAPHFILEPARSER_H
#define GRAPHFILEPARSER_H

#include <QObject>

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
    virtual bool parse(Graph& graph) = 0;

public:
    virtual void onParsePositionIncremented(int64_t /*position*/) {}

    class progess_iterator : public boost::iterator_adaptor<progess_iterator,
            boost::spirit::istream_iterator>
    {
     private:
        GraphFileParser* parser;
        int64_t position;
        struct enabler {};

     public:
        progess_iterator()
            : progess_iterator::iterator_adaptor_() {}

        explicit progess_iterator(const boost::spirit::istream_iterator iterator,
                                  GraphFileParser* parser)
            : progess_iterator::iterator_adaptor_(iterator),
              parser(parser),
              position(0) {}

     private:
        friend class boost::iterator_core_access;
        void increment()
        {
            parser->onParsePositionIncremented(position++);
            this->base_reference()++;
        }
    };
};

#endif // GRAPHFILEPARSER_H
