#include "ControlsPanel.hpp"
#include "../inputHandler/InputData.hpp"
#include "../scene/Humanoid.hpp"
#include "../animation/Sequence.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace {
void keyButton(InputData &input, const char *label, int key, float width, int mods = 0) {
    if (ImGui::Button(label, ImVec2(width, 30))) {
        InputHandler::getInstance().pressKey(input, key, mods);
    }
}

void jointRow(InputData &input, Joint joint, const char *label, const char *shortcut) {
    ImGui::PushID(static_cast<int>(joint));
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextUnformatted(label);
    ImGui::TextDisabled("%s", shortcut);
    for (int direction : {1, -1}) {
        ImGui::TableSetColumnIndex(direction == 1 ? 1 : 2);
        if (ImGui::Button(direction == 1 ? "+5" : "-5", ImVec2(-1, 34))) {
            if (input.player) input.player->stop();
            input.body->moveJoint(joint, direction * 5.0f);
        }
        ImGui::SetItemTooltip("%s: %.0f degrees", label, input.body->jointAngle(joint));
    }
    ImGui::PopID();
}

bool jointTable(const char *id, const char *positive = "Forward", const char *negative = "Back") {
    if (!ImGui::BeginTable(id, 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) return false;
    ImGui::TableSetupColumn("Joint / keys", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupColumn(positive, ImGuiTableColumnFlags_WidthFixed, 65);
    ImGui::TableSetupColumn(negative, ImGuiTableColumnFlags_WidthFixed, 65);
    ImGui::TableHeadersRow();
    return true;
}
}

ControlsPanel::ControlsPanel(GLFWwindow *window) : _window(window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = nullptr;
    ImFontConfig font;
    font.SizePixels = 16.0f;
    io.Fonts->AddFontDefault(&font);
    ImGui::StyleColorsDark();
    auto &style = ImGui::GetStyle();
    style.WindowRounding = 0;
    style.FrameRounding = 5;
    style.WindowPadding = ImVec2(16, 16);
    style.ItemSpacing = ImVec2(8, 8);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.07f, 0.09f, 0.12f, 1);
    style.Colors[ImGuiCol_Button] = ImVec4(0.12f, 0.28f, 0.36f, 1);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.17f, 0.42f, 0.52f, 1);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.17f, 0.55f, 0.60f, 1);
    if (!ImGui_ImplGlfw_InitForOpenGL(window, true)) {
        ImGui::DestroyContext();
        throw std::runtime_error("Could not initialize the controls panel input");
    }
    if (!ImGui_ImplOpenGL3_Init("#version 410")) {
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        throw std::runtime_error("Could not initialize the controls panel renderer");
    }
}

ControlsPanel::~ControlsPanel() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void ControlsPanel::beginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    _panelWidth = std::min(440.0f, ImGui::GetIO().DisplaySize.x * 0.4f);
}

int ControlsPanel::sceneFramebufferWidth() const {
    int width, height;
    glfwGetFramebufferSize(_window, &width, &height);
    const int panelPixels = static_cast<int>(std::lround(_panelWidth * ImGui::GetIO().DisplayFramebufferScale.x));
    return std::max(1, width - panelPixels);
}

void ControlsPanel::draw(InputData &input) {
    const ImVec2 display = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos(ImVec2(display.x - _panelWidth, 0));
    ImGui::SetNextWindowSize(ImVec2(_panelWidth, display.y));
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoNav;
    if (!ImGui::Begin("HumanGL controls", nullptr, flags)) {
        ImGui::End();
        return;
    }
    ImGui::TextUnformatted("HUMANGL / CONTROLS");
    ImGui::TextDisabled("Keyboard shortcuts + clickable controls");
    ImGui::TextColored(input.player->isPlaying() ? ImVec4(0.35f, 0.85f, 0.65f, 1)
                                                : ImVec4(0.65f, 0.72f, 0.8f, 1),
                       "%s", input.player->isPlaying() ? "Sequence playing" : "Manual controls");
    const float half = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) / 2;

    ImGui::SeparatorText("Animations");
    keyButton(input, "Walk [C]", GLFW_KEY_C, half);
    ImGui::SameLine(); keyButton(input, "Jump [V]", GLFW_KEY_V, half);
    keyButton(input, "Arm-wave dance [N]", GLFW_KEY_N, half);
    ImGui::SameLine(); keyButton(input, "Greeting wave [M]", GLFW_KEY_M, half);
    keyButton(input, "Stand still [B]", GLFW_KEY_B, -1);

    ImGui::SeparatorText("Joint movement");
    ImGui::TextWrapped("Hold L/R for a side; L+K or R+T for lower parts. A/D = arm; Q/E = leg.");
    ImGui::TextDisabled("Click or hold a +/- button to move it.");
    ImGui::PushItemFlag(ImGuiItemFlags_ButtonRepeat, true);
    if (jointTable("limb joints")) {
        jointRow(input, Joint::LeftShoulder, "Left upper arm", "L + A / D");
        jointRow(input, Joint::RightShoulder, "Right upper arm", "R + A / D");
        jointRow(input, Joint::LeftElbow, "Left forearm", "L+K + A / D");
        jointRow(input, Joint::RightElbow, "Right forearm", "R+T + A / D");
        jointRow(input, Joint::LeftHip, "Left thigh", "L + Q / E");
        jointRow(input, Joint::RightHip, "Right thigh", "R + Q / E");
        jointRow(input, Joint::LeftKnee, "Left lower leg", "L+K + Q / E");
        jointRow(input, Joint::RightKnee, "Right lower leg", "R+T + Q / E");
        ImGui::EndTable();
    }
    ImGui::PopItemFlag();

    if (ImGui::CollapsingHeader("Sideways arm movement (mouse)")) {
        ImGui::PushItemFlag(ImGuiItemFlags_ButtonRepeat, true);
        if (jointTable("sideways joints", "+ Z", "- Z")) {
            jointRow(input, Joint::LeftShoulderSide, "Left shoulder", "Sideways / Z");
            jointRow(input, Joint::RightShoulderSide, "Right shoulder", "Sideways / Z");
            jointRow(input, Joint::LeftElbowSide, "Left elbow", "Sideways / Z");
            jointRow(input, Joint::RightElbowSide, "Right elbow", "Sideways / Z");
            ImGui::EndTable();
        }
        ImGui::PopItemFlag();
    }

    ImGui::SeparatorText("Body rotation + position");
    const int mods = ImGui::GetIO().KeyShift ? GLFW_MOD_SHIFT : 0;
    ImGui::PushItemFlag(ImGuiItemFlags_ButtonRepeat, true);
    keyButton(input, "Turn left [Left]", GLFW_KEY_LEFT, half, mods);
    ImGui::SameLine(); keyButton(input, "Turn right [Right]", GLFW_KEY_RIGHT, half, mods);
    keyButton(input, "Move up [Up]", GLFW_KEY_UP, half, mods);
    ImGui::SameLine(); keyButton(input, "Move down [Down]", GLFW_KEY_DOWN, half, mods);
    keyButton(input, "Closer [W]", GLFW_KEY_W, half, mods);
    ImGui::SameLine(); keyButton(input, "Farther [S]", GLFW_KEY_S, half, mods);
    ImGui::PopItemFlag();
    ImGui::TextDisabled("Hold Shift for faster turns / movement.");
    keyButton(input, "Auto rotate [Ctrl/Cmd+R]", GLFW_KEY_R, -1, GLFW_MOD_CONTROL);

    ImGui::SeparatorText("Shared limb sizes");
    ImGui::Text("Upper arms / thighs: %.1f", input.body->limbDimensions.upper.length);
    ImGui::PushItemFlag(ImGuiItemFlags_ButtonRepeat, true);
    keyButton(input, "Shorter [1]", GLFW_KEY_1, half);
    ImGui::SameLine(); keyButton(input, "Longer [2]", GLFW_KEY_2, half);
    ImGui::Text("Forearms / lower legs: %.1f", input.body->limbDimensions.forearm.length);
    keyButton(input, "Shorter [3]", GLFW_KEY_3, half);
    ImGui::SameLine(); keyButton(input, "Longer [4]", GLFW_KEY_4, half);
    ImGui::PopItemFlag();
    keyButton(input, "Reset limb poses + sizes [0]", GLFW_KEY_0, -1);
    ImGui::Separator();
    ImGui::TextWrapped("Drag in the 3D view to rotate. Scroll over the view to zoom; scroll over this panel to see more controls.");
    if (ImGui::Button("Close [Esc]", ImVec2(-1, 30))) glfwSetWindowShouldClose(_window, GLFW_TRUE);
    ImGui::End();
}

void ControlsPanel::render() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
