/*
 * Created by gbian on 22/05/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#include "Vantablade/GameObject.hpp"

glm::mat4 TransformComponent::mat4() const noexcept {
    glm::mat4 transform = glm::translate(glm::mat4(1.f), translation);
    transform *= glm::eulerAngleYXZ(rotation.x, rotation.y, rotation.z);
    transform = glm::scale(transform, scale);
    return transform;
}

glm::mat3 TransformComponent::normalMatrix() const noexcept { return glm::transpose(glm::inverse(glm::mat3(mat4()))); }