#include "../src/matrixMath/MatrixStack.hpp"
#include "../src/matrixMath/MatrixTransform.hpp"

#include <cmath>
#include <iostream>
#include <stdexcept>

namespace {
int checks = 0;

void require(bool condition, const char *message) {
    if (!condition) throw std::runtime_error(message);
    ++checks;
}

void expectOrigin(const MatrixStack &stack, GLfloat x, GLfloat y, GLfloat z) {
    const auto matrix = stack.top();
    require(std::fabs(matrix[3] - x) < 1e-5f && std::fabs(matrix[7] - y) < 1e-5f
            && std::fabs(matrix[11] - z) < 1e-5f && matrix[15] == 1.0f,
            "Unexpected joint origin");
}
}

int main() {
    try {
        MatrixStack stack;
        const auto identity = stack.top();
        require(stack.size() == 1, "A stack starts with one entry");
        for (int i = 0; i < 16; ++i) {
            require(identity[i] == (i % 5 == 0 ? 1.0f : 0.0f), "Initial matrix is identity");
        }
        auto input = identity;
        input[3] = 3.0f;
        stack.load(input);
        input[3] = 99.0f;
        expectOrigin(stack, 3.0f, 0.0f, 0.0f);
        auto copy = stack.top();
        copy[3] = -99.0f;
        require(copy[3] == -99.0f && stack.top()[3] == 3.0f, "Top returns an independent value copy");

        const auto parent = stack.top();
        require(stack.push(), "Push left branch");
        stack.translate(0.0f, 2.0f, 0.0f);
        expectOrigin(stack, 3.0f, 2.0f, 0.0f);
        require(stack.pop() && stack.top() == parent, "Pop restores the complete parent");
        require(stack.push(), "Push right branch");
        stack.translate(0.0f, -2.0f, 0.0f);
        expectOrigin(stack, 3.0f, -2.0f, 0.0f);
        require(stack.pop() && stack.top() == parent, "Sibling transforms stay independent");

        stack.rotateZ(90.0f);
        stack.translate(0.0f, 2.0f, 0.0f);
        expectOrigin(stack, 1.0f, 0.0f, 0.0f);
        const auto joint = stack.top();
        require(stack.push(), "Push geometry frame");
        stack.scale(2.0f, 3.0f, 4.0f);
        const auto geometry = stack.top();
        require(std::fabs(geometry[4] - 2.0f) < 1e-5f
                && std::fabs(geometry[1] + 3.0f) < 1e-5f && geometry[10] == 4.0f,
                "Nonuniform scale follows the rotated local axes");
        require(stack.pop() && stack.top() == joint, "Geometry scale does not leak into a joint");
        stack.translate(0.0f, -2.0f, 0.0f);
        expectOrigin(stack, 3.0f, 0.0f, 0.0f);

        stack.reset();
        require(stack.size() == 1 && stack.top() == identity, "Reset restores the base identity");
        stack.rotateX(90.0f);
        stack.translate(0.0f, 1.0f, 0.0f);
        expectOrigin(stack, 0.0f, 0.0f, 1.0f);
        stack.reset();
        stack.scale(2.0f);
        stack.translate(1.0f, 0.0f, 0.0f);
        expectOrigin(stack, 2.0f, 0.0f, 0.0f);

        stack.reset();
        for (std::size_t i = 1; i < MatrixStack::capacity; ++i) {
            require(stack.push(), "Push up to capacity");
            stack.translate(1.0f, 0.0f, 0.0f);
        }
        const auto full = stack.top();
        require(!stack.push() && stack.size() == MatrixStack::capacity && stack.top() == full,
                "Overflow preserves depth and values");
        while (stack.size() > 1) require(stack.pop(), "Pop back to base");
        require(!stack.pop() && stack.size() == 1 && stack.top() == identity,
                "Underflow preserves the base entry");

        const auto before = stack.top();
        int rejected = 0;
        try { stack.load(nullptr); }
        catch (const MatrixTransformException &) { ++rejected; }
        try { stack.multiply(nullptr); }
        catch (const MatrixTransformException &) { ++rejected; }
        require(rejected == 2 && stack.top() == before && stack.size() == 1,
                "Null input leaves the stack unchanged");

        input = identity;
        input[7] = 4.0f;
        stack.multiply(input);
        expectOrigin(stack, 0.0f, 4.0f, 0.0f);
        stack.reset();
        stack.load(input.data());
        expectOrigin(stack, 0.0f, 4.0f, 0.0f);
        std::cout << "Passed " << checks << " matrix-stack assertions.\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << "Stack test failed: " << error.what() << '\n';
        return 1;
    }
}
