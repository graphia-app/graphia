#ifndef FINDOPTIONS_H
#define FINDOPTIONS_H

#include "shared/utils/qmlenum.h"

DEFINE_QML_ENUM(
        Q_GADGET,   FindOptions,
                    MatchCase           = 0x1,
                    MatchWholeWords     = 0x2,
                    MatchUsingRegex     = 0x4);

#endif // FINDOPTIONS_H
