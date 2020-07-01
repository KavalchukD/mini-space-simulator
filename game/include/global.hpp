#pragma once
#include "iengine.hpp"
#include "world_physics.hpp"

class Environement;

class Global
{
public:
    using worldCalcType                          = double;
    static constexpr om::myGlfloat baseScaleXtoY = om::IEngine::baseScaleXtoY;
    static constexpr om::myGlfloat ndcSize =
        om::IEngine::maxCoordinate - om::IEngine::minCoordinate;
    static constexpr om::myGlfloat baseWorldWindowSizeX = 1200.0;
    static constexpr om::myGlfloat baseWorldWindowSizeY =
        baseWorldWindowSizeX / baseScaleXtoY;
    static constexpr om::myGlfloat baseWorldToNdcScaleX =
        ndcSize / baseWorldWindowSizeX;
    static constexpr om::myGlfloat baseWorldToNdcScaleY =
        ndcSize / baseWorldWindowSizeY;
    static constexpr om::myGlfloat initialScale = 1.0;

    static constexpr om::myGlfloat minimalScale = 0.2;

    static constexpr om::myGlfloat maximalScale = 3.0;

    static om::myGlfloat getCurrentScale() { return currentWorldScale; };

    static om::myGlfloat getCurrentWorldScaleX()
    {
        return currentWorldScale * baseWorldToNdcScaleX;
    };

    static om::myGlfloat getCurrentWorldScaleY()
    {
        return currentWorldScale * baseWorldToNdcScaleY;
    };

    static om::myGlfloat getCurrentWorldScaleForRender()
    {
        return getCurrentWorldScaleY();
    };

    static om::Vector<2> getUserNdcPosition() { return userNdcPosition; };

    static std::array<worldCalcType, 2> getUserWorldPosition()
    {
        return userWorldPosition;
    };

    static void setUserPosition(double userPositionInWorldX,
                                double userPositionInWorldY)
    {
        userWorldPosition[0] = userPositionInWorldX;
        userWorldPosition[1] = userPositionInWorldY;
        userNdcPosition.elements[0] =
            userPositionInWorldX * getCurrentWorldScaleForRender();
        userNdcPosition.elements[1] =
            userPositionInWorldY * getCurrentWorldScaleForRender();
    };
    static constexpr std::string_view texturePathPrefix{ "res/texture/" };
    static constexpr std::string_view shaderPathPrefix{ "res/program/" };
    static constexpr std::string_view soundPathPrefix{ "res/sounds/" };

private:
    friend Environement;
    static om::myGlfloat         currentWorldScale;
    static om::Vector<2>         userNdcPosition;
    static std::array<double, 2> userWorldPosition;
};
