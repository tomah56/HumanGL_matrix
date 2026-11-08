#pragma once

#include "Sequence.hpp"

namespace animation {
const Sequence &walkingSequence();
const Sequence &jumpingSequence();
const Sequence &armWaveDanceSequence();
const Sequence &greetingWaveSequence();
void standStill(Humanoid &body);
}
