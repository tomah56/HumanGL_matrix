#pragma once

#include "../inputHandler/InputHandler.hpp"
#include "../scene/Transformation.hpp"
#include "../scene/Material.hpp"
#include "../scene/Camera.hpp"

class Transformation;
class Material;
class Camera;
class InputHandler;

struct InputData {
    Transformation* transformation;
    Material* material;
    Camera* camera;
    InputHandler* inputHandler;

    InputData(Transformation* t, Material* m, Camera* c)
        : transformation(t), material(m), camera(c) {}
};
