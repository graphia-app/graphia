#ifndef MCLTRANSFORM_H
#define MCLTRANSFORM_H

#include "graphtransform.h"
#include "graph/graph.h"
#include "shared/utils/flags.h"

class MCLTransform : public GraphTransform
{
public:
    struct SparseMatrixEntry
    {
        size_t _row; size_t _column; float _value;
        SparseMatrixEntry(size_t row, size_t column, float value)
            : _row(row), _column(column), _value(value){}
    };

    struct MCLRowData
    {
        std::vector<float> values;
        std::vector<char> valid;
        std::vector<size_t> indices;
        explicit MCLRowData(size_t columnCount) : values(columnCount, 0.0f),
            valid(columnCount, 0), indices(columnCount, 0UL) {}
    };
    template<class MatrixType>
    class ColumnsIterator
    {
    public:
        class iterator: public std::iterator<
                std::input_iterator_tag,
                size_t,
                size_t,
                const size_t*,
                size_t
                >{
            size_t _num = 0;
        public:
            explicit iterator(size_t num = 0) : _num(num) {}
            iterator& operator++() { _num = _num + 1; return *this; }
            iterator operator++(int) { iterator retval = *this; ++(*this); return retval; }
            bool operator==(iterator other) const { return _num == other._num; }
            bool operator!=(iterator other) const { return !(*this == other); }
            reference operator*() const { return _num; }
        };
        MatrixType& matrix;
        ColumnsIterator(MatrixType& _matrix) : matrix(_matrix) {}
        iterator begin() { return iterator(0); }
        iterator end() { return iterator(matrix.columns()); }
    };

    explicit MCLTransform(GraphModel* graphModel) : _graphModel(graphModel) {}
    bool apply(TransformedGraph &target) const;

    void enableDebugIteration(){ _debugIteration = true; }
    void enableDebugMatrices(){ _debugMatrices = true; }
    void disableDebugIteration(){ _debugIteration = false; }
    void disableDebugMatrices(){ _debugMatrices = false; }

private:
    const float MCL_PRUNE_LIMIT = 1e-4f;
    const float MCL_CONVERGENCE_LIMIT = 1e-3f;

    bool _debugIteration = false;
    bool _debugMatrices = false;

    void calculateMCL(float inflation, TransformedGraph& target) const;

private:
    GraphModel* _graphModel = nullptr;
};

class MCLTransformFactory : public GraphTransformFactory
{
public:
    explicit MCLTransformFactory(GraphModel* graphModel) :
        GraphTransformFactory(graphModel)
    {}

    QString description() const {
        return R"(<a href="https://micans.org/mcl/">MCL - Markov Clustering</a> )" //
                " creates discrete groups (clusters) of nodes based on a flow simulation model"; }
    ElementType elementType() const { return ElementType::None; }
    GraphTransformParameters parameters() const
    {
        GraphTransformParameters p;
        p.emplace("Granularity",
                  GraphTransformParameter(ValueType::Float,
                                          "Controls the size of the resultant clusters."
                                          " A larger granularity value results in smaller clusters",
                                          1.5, 1.5, 3.5));

        return p;
    }

    std::unique_ptr<GraphTransform> create(const GraphTransformConfig& graphTransformConfig) const;
};

#endif // MCLTRANSFORM_H
