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

    [[nodiscard]] glm::mat4 mat4() const noexcept;

    [[nodiscard]] glm::mat3 normalMatrix() const noexcept;
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

    [[nodiscard]] id_t getId() const noexcept { return id; }

    std::shared_ptr<Model> model{};
    glm::vec3 color{};
    TransformComponent transform{};

private:
    explicit GameObject(id_t objId) noexcept : id{objId} {}

    id_t id;
};
