#ifndef LIMITCONSTANTS_H
#define LIMITCONSTANTS_H

#include <QObject>

class LimitConstants : public QObject
{
    Q_OBJECT

    Q_PROPERTY(float minimumNodeSize READ minimumNodeSize CONSTANT)
    Q_PROPERTY(float maximumNodeSize READ maximumNodeSize CONSTANT)

    Q_PROPERTY(float minimumEdgeSize READ minimumEdgeSize CONSTANT)
    Q_PROPERTY(float maximumEdgeSize READ maximumEdgeSize CONSTANT)

    Q_PROPERTY(float minimumMinimumComponentRadius READ minimumMinimumComponentRadius CONSTANT)
    Q_PROPERTY(float maximumMinimumComponentRadius READ maximumMinimumComponentRadius CONSTANT)

    Q_PROPERTY(float minimumTransitionTime READ minimumTransitionTime CONSTANT)
    Q_PROPERTY(float maximumTransitionTime READ maximumTransitionTime CONSTANT)

public:
    static float minimumNodeSize() { return 0.25f; }
    static float maximumNodeSize() { return 4.0f; }

    static float minimumEdgeSize() { return 0.02f; }
    static float maximumEdgeSize() { return 2.0f; }

    static float minimumMinimumComponentRadius() { return 0.05f; }
    static float maximumMinimumComponentRadius() { return 15.0f; }

    static float minimumTransitionTime() { return 0.1f; }
    static float maximumTransitionTime() { return 5.0f; }
};

#endif // LIMITCONSTANTS_H
