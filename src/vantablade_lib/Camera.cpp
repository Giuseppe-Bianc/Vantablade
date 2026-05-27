/*
 * Created by gbian on 27/05/2026.
 * Copyright (c) 2026 All rights reserved.
 */
// clang-format off
// NOLINTBEGIN(*-include-cleaner, *-easily-swappable-parameters, *-uppercase-literal-suffix, *-pro-type-union-access,*-avoid-magic-numbers,*-magic-numbers)
// clang-format on
#include "Vantablade/Camera.hpp"
void Camera::setOrthographicProjection(float left, float right, float bottom, float top, float near, float far) {
    projectionMatrix = glm::orthoLH(left, right, bottom, top, near, far);
}

void Camera::setPerspectiveProjection(float fovy, float aspect, float near, float far) {
    assert(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);
    projectionMatrix = glm::perspectiveLH(fovy, aspect, near, far);
}
// clang-format off
// NOLINTEND(*-include-cleaner, *-easily-swappable-parameters, *-uppercase-literal-suffix, *-pro-type-union-access,*-avoid-magic-numbers,*-magic-numbers)
// clang-format on