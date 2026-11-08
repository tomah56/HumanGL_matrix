#pragma once

#include "../scene/Humanoid.hpp"

#include <initializer_list>
#include <vector>

namespace animation {
struct JointTarget {
    Joint joint;
    GLfloat angle;
};

struct SequenceStep {
    double duration;
    std::vector<JointTarget> targets;
    GLfloat height = 0.0f; // World-space Y offset from the body's current root.
};

struct Sequence {
    bool loop;
    std::vector<SequenceStep> steps;
};

JointTarget to(Joint joint, GLfloat angle);
SequenceStep step(double seconds, std::initializer_list<JointTarget> targets, GLfloat height = 0.0f);
Sequence repeat(std::initializer_list<SequenceStep> steps);
Sequence once(std::initializer_list<SequenceStep> steps);

class SequencePlayer {
public:
    void play(const Sequence &sequence, const Humanoid &body);
    void update(double deltaTime, Humanoid &body);
    void stop();
    bool isPlaying() const;
    GLfloat heightOffset() const;

private:
    Sequence _sequence{};
    bool _playing = false;
    std::size_t _step = 0;
    double _elapsed = 0.0;
    double _duration = 0.0;
    std::array<GLfloat, static_cast<std::size_t>(Joint::Count)> _startAngles{};
    GLfloat _height = 0.0f;
    GLfloat _startHeight = 0.0f;

    void beginStep(const Humanoid &body);
};
}
