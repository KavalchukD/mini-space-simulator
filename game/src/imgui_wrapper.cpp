#include "imgui_wrapper.hpp"
#include "matrix.hpp"
#include <imgui.h>

#include <algorithm>

template <typename T>
static T findAbsoluteValue(T value1, T value2)
{
    return std::sqrt(value1 * value1 + value2 * value2);
}

static std::vector<ImVec2> rotateImVec2Array(std::vector<ImVec2> inputImVectors,
                                             float angle, ImVec2 center)
{
    const auto shipOrientationRotationMatrix =
        om::MatrixFunctor::getRotateMatrix(angle, { center.x, center.y });

    std::vector<om::Vector<3>> lineWithArrowNormalizedOmPos;
    lineWithArrowNormalizedOmPos.reserve(inputImVectors.size());

    const auto getOmVecFromImVec = [](ImVec2 imVec) {
        return om::Vector<3>{ imVec.x, imVec.y, 1.f };
    };

    std::transform(inputImVectors.begin(), inputImVectors.end(),
                   std::back_inserter(lineWithArrowNormalizedOmPos),
                   getOmVecFromImVec);

    std::vector<om::Vector<3>> lineWithArrowRotatedOmPos;
    lineWithArrowNormalizedOmPos.reserve(inputImVectors.size());

    const auto rotateOmVec =
        [&shipOrientationRotationMatrix](om::Vector<3> omVec) {
            return shipOrientationRotationMatrix * omVec;
        };

    std::transform(lineWithArrowNormalizedOmPos.begin(),
                   lineWithArrowNormalizedOmPos.end(),
                   std::back_inserter(lineWithArrowRotatedOmPos), rotateOmVec);

    std::vector<ImVec2> lineWithArrowRotatedImPos;
    lineWithArrowRotatedImPos.reserve(inputImVectors.size());

    const auto getImVecFromOmVec = [](om::Vector<3> omVec) {
        return ImVec2{ omVec.elements[0], omVec.elements[1] };
    };

    std::transform(
        lineWithArrowRotatedOmPos.begin(), lineWithArrowRotatedOmPos.end(),
        std::back_inserter(lineWithArrowRotatedImPos), getImVecFromOmVec);

    return lineWithArrowRotatedImPos;
}

static void drawNaviArrow(float length, float angle, ImVec2 center,
                          ImColor color)
{
    constexpr float  lineThickness{ 2 };
    constexpr size_t numberOfElements{ 3 };

    const auto sideLinesLengts{ length * 0.2f };
    const auto sideLinesAngle{ 25.f / 180.f * M_PI };

    const auto sideLineDx{ sideLinesLengts * std::sin(sideLinesAngle) };
    const auto sideLineDy{ sideLinesLengts * std::cos(sideLinesAngle) };

    std::vector<ImVec2> arrowNormalizedPos;
    arrowNormalizedPos.reserve(numberOfElements);
    arrowNormalizedPos.emplace_back(center.x,
                                    center.y - length / 2.f); // upper main

    arrowNormalizedPos.emplace_back(arrowNormalizedPos[0].x - sideLineDx,
                                    arrowNormalizedPos[0].y +
                                        sideLineDy); // left side

    arrowNormalizedPos.emplace_back(arrowNormalizedPos[0].x + sideLineDx,
                                    arrowNormalizedPos[0].y +
                                        sideLineDy); // right side

    const auto arrowRotatedPos{ rotateImVec2Array(arrowNormalizedPos, angle,
                                                  center) };

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    for (size_t i = 1; i < numberOfElements; ++i)
    {
        draw_list->AddLine(arrowRotatedPos[0], arrowRotatedPos[i], color,
                           lineThickness);
    }
}

static void drawNaviLineWithArrow(float length, float angle, ImVec2 center,
                                  ImColor color)
{
    constexpr float  lineThickness{ 2 };
    constexpr size_t numberOfElements{ 2 };

    drawNaviArrow(length, angle, center, color);

    std::vector<ImVec2> lineWithArrowNormalizedPos;
    lineWithArrowNormalizedPos.reserve(numberOfElements);
    lineWithArrowNormalizedPos.emplace_back(center.x,
                                            center.y -
                                                length / 2.f); // upper main

    lineWithArrowNormalizedPos.emplace_back(center.x,
                                            center.y +
                                                length / 2.f); // bottom main

    const auto lineWithArrowRotatedPos{ rotateImVec2Array(
        lineWithArrowNormalizedPos, angle, center) };

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    draw_list->AddLine(lineWithArrowRotatedPos[0], lineWithArrowRotatedPos[1],
                       color, lineThickness);
}

static void drawNaviCircle(ImVec2 windowPos, ImVec2 windowSize,
                           const Model::Rocket& userShip)
{
    const ImVec2 windowCenter{ windowPos.x + windowSize.x / 2,
                               windowPos.y + windowSize.y / 2 };

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImColor     circleColor{ 0.f, 1.f, 0.f, 1.f };
    float       circleThickness{ 3 };
    float       circleSectionsAmount{ 0 };

    const auto circleRadius = windowSize.y / 2.f - 10.f;
    draw_list->AddCircle(windowCenter, circleRadius, circleColor,
                         circleSectionsAmount, circleThickness);

    const auto lineSize{ circleRadius * 2.f - 10.f };

    ImColor lineShipOrientationColor{ 1.f, 1.f, 0.f, 1.f };

    const auto lineShipOrientationAngle{ static_cast<float>(-userShip.angle) };

    drawNaviLineWithArrow(lineSize, lineShipOrientationAngle, windowCenter,
                          lineShipOrientationColor);

    ImColor lineShipSpeedColor{ 0.f, 0.f, 1.f, 1.f };

    const auto speedAbsolute = findAbsoluteValue(userShip.vx, userShip.vy);

    const auto lineShipSpeedCos{
        (speedAbsolute > static_cast<Model::worldCalcType>(0.05))
            ? static_cast<float>(userShip.vy / speedAbsolute)
            : 1.f
    };
    const auto lineShipSpeedAngle = (userShip.vx > 0.0)
                                        ? std::acos(lineShipSpeedCos)
                                        : -std::acos(lineShipSpeedCos);

    drawNaviLineWithArrow(lineSize, lineShipSpeedAngle, windowCenter,
                          lineShipSpeedColor);

    ImColor directionToCenterColor{ 1.f, 0.f, 1.f, 1.f };

    const auto distanceToCenter = findAbsoluteValue(userShip.x, userShip.y);

    const auto posShipCos{
        (distanceToCenter > static_cast<Model::worldCalcType>(0.05))
            ? -static_cast<float>(userShip.y / distanceToCenter)
            : 1.f
    };

    const auto lineShipPosAngle =
        (userShip.x > 0.0) ? -std::acos(posShipCos) : std::acos(posShipCos);

    drawNaviArrow(lineSize, lineShipPosAngle, windowCenter,
                  directionToCenterColor);
}

static void drawSpeedMeter(float speedAbsolute, float angleSpeed)
{
    ImGui::Text("linear speed:");
    ImGui::Text("%.1f m/s", speedAbsolute);
    ImGui::Spacing();
    ImGui::Text("angle speed:");
    ImGui::Text("%.1f rad/s", angleSpeed);
}

static void stabilizationCheckBox(bool& isNeedStabilization1,
                                  bool& isNeedStabilization2)
{
    ImGui::Checkbox(" Stabiliz 1", &isNeedStabilization1);
    ImGui::Checkbox(" Stabiliz 2", &isNeedStabilization2);
}

static void drawEnginePowerHandler(ImVec2 windowSize, Model::Rocket& userShip)
{
    auto mainEnginePowerPercent{ static_cast<float>(
        userShip.mainEngine.enginePercentThrust) };
    auto sideEnginesPowerPercent{ static_cast<float>(
        userShip.sideEngines[0].enginePercentThrust) };

    ImVec2 size{ 1, 1 };

    ImGui::Text("Engines thrust");
    ImGui::Indent(9);
    ImGui::Text("Main");
    ImGui::SameLine();
    ImGui::Indent(50);
    ImGui::Text("Side");
    ImGui::Unindent(59);
    ImGui::Indent(10);
    [[maybe_unused]] const auto isSliderMainMoved =
        ImGui::VSliderFloat("##w_main", { 30, windowSize.y - 55 },
                            &mainEnginePowerPercent, 0.0f, 100.0f, "%.0f", 1.f);
    ImGui::SameLine();
    ImGui::Indent(50);
    [[maybe_unused]] const auto isSliderSideMoved = ImGui::VSliderFloat(
        "##w_side", { 30, windowSize.y - 55 }, &sideEnginesPowerPercent, 0.0f,
        100.0f, "%.0f", 1.f);
    ImGui::Unindent(60);

    userShip.setMainEngineThrust(mainEnginePowerPercent);
    userShip.setSideEnginesThrust(sideEnginesPowerPercent);
}

static void createShipMetersWindow(Model::Rocket& userShip)
{
    const std::string      shipMetersWindowName{ "Ship meters" };
    const ImVec4           backgroundColor{ 0.f, 0.f, 0.f, 0.6f };
    const ImGuiWindowFlags windowCreatingFlags{
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollWithMouse |
        ImGuiWindowFlags_NoDecoration
    };
    ;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, backgroundColor);

    ImGui::Begin(shipMetersWindowName.c_str(), nullptr,
                 windowCreatingFlags); // Create a window called "Hello,

    ImGui::SetWindowFontScale(1.15f);

    // world!" and append into it.

    // SetWindowPos(const ImVec2& pos, ImGuiCond cond = 0); // (not
    // recommended) set current window position - call within Begin()/End().
    // prefer using SetNextWindowPos(), as this may incur tearing and
    // side-effects.

    const auto displayMax = ImGui::GetIO().DisplaySize;

    const ImVec2 displayCenter{ displayMax.x / 2, displayMax.y / 2 };

    const ImVec2 windowDesiredSize{ 400, 150 };

    const ImVec2 windowPos{ displayCenter.x - windowDesiredSize.x / 2,
                            displayMax.y - windowDesiredSize.y };

    ImGui::SetWindowSize(shipMetersWindowName.c_str(), windowDesiredSize);
    ImGui::SetWindowPos(shipMetersWindowName.c_str(), windowPos);

    const auto speedAbsolute = findAbsoluteValue(userShip.vx, userShip.vy);

    ImGui::Columns(3, "mycolumns3", false); // 3-ways, no border
    ImGui::Separator();

    drawSpeedMeter(speedAbsolute, userShip.angleSpeed);
    stabilizationCheckBox(userShip.stabilizationLevel1Enabled,
                          userShip.stabilizationLevel2Enabled);
    ImGui::NextColumn();
    drawNaviCircle(windowPos, windowDesiredSize, userShip);
    ImGui::NextColumn();
    drawEnginePowerHandler(windowDesiredSize, userShip);

    ImGui::End();
    ImGui::PopStyleColor();
}

ImguiWrapper::ImguiWrapper(om::IEngine& engine)
    : m_engine{ engine }
{
}

void ImguiWrapper::createImguiObjects(Model::World& world)
{
    m_engine.uiNewFrame();

    auto& userShip = *world.userShipPtr;

    createShipMetersWindow(userShip);
}
