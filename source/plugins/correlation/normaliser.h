#ifndef NORMALISER_H
#define NORMALISER_H

#include <cstdlib>

class Normaliser
{
public:
    virtual ~Normaliser() {}
    virtual double value(size_t column, size_t row) = 0;
};

#endif // NORMALISER_H
