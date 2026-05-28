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

    [[nodiscard]] const glm::mat4 &getProjection() const noexcept { return projectionMatrix; }

private:
    glm::mat4 projectionMatrix{1.f};
};
