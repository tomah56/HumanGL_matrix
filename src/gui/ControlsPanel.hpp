#pragma once

struct GLFWwindow;
struct InputData;

class ControlsPanel {
public:
    explicit ControlsPanel(GLFWwindow *window);
    ~ControlsPanel();
    ControlsPanel(const ControlsPanel &) = delete;
    ControlsPanel &operator=(const ControlsPanel &) = delete;

    void beginFrame();
    void draw(InputData &input);
    void render();
    int sceneFramebufferWidth() const;

private:
    GLFWwindow *_window;
    float _panelWidth = 0.0f;
};
