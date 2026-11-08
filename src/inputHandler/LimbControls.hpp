#pragma once

class Humanoid;

class LimbControls {
public:
    // Handles side/segment selectors, joint movement, resizing, and reset.
    bool handleKey(Humanoid &body, int key, int action, int mods);

private:
    bool _leftHeld = false;
    bool _rightHeld = false;
    bool _kHeld = false;
    bool _tHeld = false;
};
