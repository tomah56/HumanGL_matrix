#pragma once

#include "Sequence.hpp"

namespace animation {
const Sequence &walkingSequence();
const Sequence &jumpingSequence();
void standStill(Humanoid &body);
}
