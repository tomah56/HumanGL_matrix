#include "Sequence.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace animation {
JointTarget to(Joint joint, GLfloat angle) { return {joint, angle}; }

SequenceStep step(double seconds, std::initializer_list<JointTarget> targets, GLfloat height) {
    return {seconds, targets, height};
}

Sequence repeat(std::initializer_list<SequenceStep> steps) { return {true, steps}; }
Sequence once(std::initializer_list<SequenceStep> steps) { return {false, steps}; }

void SequencePlayer::play(const Sequence &sequence, const Humanoid &body) {
    if (sequence.steps.empty()) throw std::invalid_argument("A sequence needs at least one step");
    double duration = 0.0;
    for (const auto &entry : sequence.steps) {
        if (!std::isfinite(entry.duration) || entry.duration <= 0.0 || !std::isfinite(entry.height)) {
            throw std::invalid_argument("Step duration must be positive and step values finite");
        }
        duration += entry.duration;
        std::array<bool, static_cast<std::size_t>(Joint::Count)> seen{};
        for (const auto &target : entry.targets) {
            const auto index = static_cast<std::size_t>(target.joint);
            if (index >= seen.size() || seen[index] || !std::isfinite(target.angle)) {
                throw std::invalid_argument("Step targets need unique valid joints and finite angles");
            }
            seen[index] = true;
        }
    }
    if (!std::isfinite(duration)) throw std::invalid_argument("Sequence duration must be finite");
    // Validate the current pose before replacing a running sequence.
    for (std::size_t joint = 0; joint < _startAngles.size(); ++joint) {
        if (!std::isfinite(body.jointAngle(static_cast<Joint>(joint)))) {
            throw std::invalid_argument("Starting pose must be finite");
        }
    }
    _sequence = sequence;
    _duration = duration;
    _step = 0;
    _height = 0.0f;
    _playing = true;
    beginStep(body);
}

void SequencePlayer::beginStep(const Humanoid &body) {
    _elapsed = 0.0;
    _startHeight = _height;
    for (std::size_t joint = 0; joint < _startAngles.size(); ++joint) {
        _startAngles[joint] = body.jointAngle(static_cast<Joint>(joint));
    }
}

void SequencePlayer::update(double deltaTime, Humanoid &body) {
    if (!std::isfinite(deltaTime) || deltaTime < 0.0) {
        throw std::invalid_argument("Animation delta time must be finite and nonnegative");
    }
    while (_playing && deltaTime > 0.0) {
        const auto &entry = _sequence.steps[_step];
        const double remaining = entry.duration - _elapsed;
        const bool complete = deltaTime >= remaining;
        const double advance = std::min(deltaTime, remaining);
        _elapsed += advance;
        deltaTime -= advance;
        const double fraction = complete ? 1.0 : _elapsed / entry.duration;
        for (const auto &target : entry.targets) {
            const double start = _startAngles[static_cast<std::size_t>(target.joint)];
            const GLfloat angle = complete ? target.angle
                : static_cast<GLfloat>(start + (static_cast<double>(target.angle) - start) * fraction);
            body.setJointAngle(target.joint, angle);
        }
        _height = complete ? entry.height : static_cast<GLfloat>(
            _startHeight + (static_cast<double>(entry.height) - _startHeight) * fraction);
        if (!complete) return;
        if (++_step == _sequence.steps.size()) {
            if (!_sequence.loop) {
                _playing = false;
                return;
            }
            _step = 0;
            // Absolute targets have the same endpoint after every complete cycle.
            // Skip whole extra cycles, then evaluate only the remaining fraction.
            if (deltaTime >= _duration) deltaTime = std::fmod(deltaTime, _duration);
        }
        beginStep(body);
    }
}

void SequencePlayer::stop() { _playing = false; _height = 0.0f; }
bool SequencePlayer::isPlaying() const { return _playing; }
GLfloat SequencePlayer::heightOffset() const { return _height; }
}
