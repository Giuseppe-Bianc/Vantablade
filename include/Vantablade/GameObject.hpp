/*
 * Created by gbian on 22/05/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#pragma once

#include "Model.hpp"
#include "headers.hpp"

struct TransformComponent {
    glm::vec3 translation{};  // (position offset)
    glm::vec3 scale{1.f, 1.f, 1.f};
    glm::vec3 rotation;

    [[nodiscard]] glm::mat4 mat4() const {
        /*glm::mat4 t = glm::translate(glm::mat4(1.f), translation);
        glm::mat4 r = glm::yawPitchRoll(rotation.y, rotation.x, rotation.z);
        glm::mat4 s = glm::scale(glm::mat4(1.f), scale);
        return t * r * s;*/
        glm::mat4 transform = glm::translate(glm::mat4(1.f), translation);
        transform *= glm::eulerAngleYXZ(rotation.x, rotation.y, rotation.z);
        transform = glm::scale(transform, scale);
        return transform;
    }
};

class GameObject {
public:
    using id_t = unsigned int;

    static GameObject createGameObject() {
        static id_t currentId = 0;
        return GameObject{currentId++};
    }

    GameObject(const GameObject &) = delete;
    GameObject &operator=(const GameObject &) = delete;
    GameObject(GameObject &&) = default;
    GameObject &operator=(GameObject &&) = default;

    id_t getId() { return id; }

    std::shared_ptr<Model> model{};
    glm::vec3 color{};
    TransformComponent transform{};

private:
    GameObject(id_t objId) : id{objId} {}

    id_t id;
};
