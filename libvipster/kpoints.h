#ifndef KPOINTS_H
#define KPOINTS_H

#include "vec.h"
#include <vector>

namespace Vipster {
enum class KPointFmt{Gamma, MPG, Discrete};
//TODO: move to KPoints-struct
struct DiscreteKPoint{
    Vec pos;
    float weight;
};

struct KPoints{
    KPointFmt active = KPointFmt::Gamma;
    struct MPG{
        int x{1},y{1},z{1};
        float sx{},sy{},sz{};
    } mpg;
    struct Discrete{
        enum Properties{none=0x0,crystal=0x1,band=0x2,contour=0x4};
        Properties properties{};
        std::vector<DiscreteKPoint> kpoints{};
    } discrete;
};
}

#endif // KPOINTS_H
