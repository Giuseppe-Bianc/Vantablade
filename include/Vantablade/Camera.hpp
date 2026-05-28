/*
 * Created by gbian on 27/05/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#pragma once

#include "headers.hpp"

class Camera {
public:
    void setOrthographicProjection(float left, float right, float top, float bottom, float near, float far);

    void setPerspectiveProjection(float fovy, float aspect, float near, float far);

    void setViewDirection(const glm::vec3 &position, const glm::vec3 &direction, const glm::vec3 &up = glm::vec3{0.f, -1.f, 0.f});
    void setViewTarget(const glm::vec3 &position, const glm::vec3 &target, const glm::vec3 &up = glm::vec3{0.f, -1.f, 0.f});
    void setViewYXZ(const glm::vec3 &position, const glm::vec3 &rotation);

    [[nodiscard]] const glm::mat4 &getProjection() const noexcept { return projectionMatrix; }
    [[nodiscard]] const glm::mat4 &getView() const noexcept { return viewMatrix; }

private:
    glm::mat4 projectionMatrix{1.f};
    glm::mat4 viewMatrix{1.f};
};
