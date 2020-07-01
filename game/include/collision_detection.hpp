#pragma once
#include "world_physics.hpp"
#include <array>
#include <math.h>

namespace Model
{

class CollisionDetection
{
    struct InternalObject
    {
        std::array<worldCalcType, 2> pos;
        std::array<worldCalcType, 2> size;
        std::array<worldCalcType, 2> angle;
    };

public:
    struct InputObject
    {
        std::array<worldCalcType, 2> pos;
        std::array<worldCalcType, 2> size;
    };
    static bool isCollidedAABB(const InputObject& obj1, const InputObject& obj2)
    {
        {
            const auto minX1 = obj1.pos[0] - obj1.size[0] / 2;
            const auto maxX1 = obj1.pos[0] + obj1.size[0] / 2;
            const auto minX2 = obj2.pos[0] - obj2.size[0] / 2;
            const auto maxX2 = obj2.pos[0] + obj2.size[0] / 2;
            if ((maxX1 < minX2) || (minX1 > maxX2))
                return false;
        }
        {
            const auto minY1 = obj1.pos[1] - obj1.size[1] / 2;
            const auto maxY1 = obj1.pos[1] + obj1.size[1] / 2;
            const auto minY2 = obj2.pos[1] - obj2.size[1] / 2;
            const auto maxY2 = obj2.pos[1] + obj2.size[1] / 2;
            if ((maxY1 < minY2) || (minY1 > maxY2))
                return false;
        }

        return true;
    }
};
} // namespace Model
