#pragma once

class Humanoid;
namespace animation { class SequencePlayer; }

class LimbControls {
public:
    // Handles side/segment selectors, joint movement, resizing, and reset.
    bool handleKey(Humanoid &body, int key, int action, int mods,
                   animation::SequencePlayer *player = nullptr);

private:
    bool _leftHeld = false;
    bool _rightHeld = false;
    bool _kHeld = false;
    bool _tHeld = false;
};
