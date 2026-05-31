/*
 * Created by gbian on 27/05/2026.
 * Copyright (c) 2026 All rights reserved.
 */
// clang-format off
// NOLINTBEGIN(*-include-cleaner, *-easily-swappable-parameters, *-uppercase-literal-suffix, *-pro-type-union-access,*-avoid-magic-numbers,*-magic-numbers, *-identifier-length)
// clang-format on
#include "Vantablade/Camera.hpp"
#include "Vantablade/Profiler.hpp"
void Camera::setOrthographicProjection(float left, float right, float top, float bottom, float near, float far) {
    VZ_ZONE_SCOPED;
    // LH = Mano sinistra, ZO = Range profondità 0 a 1 (Vulkan)
    projectionMatrix = glm::orthoLH_ZO(left, right, bottom, top, near, far);
}

void Camera::setPerspectiveProjection(float fovy, float aspect, float near, float far) {
    VZ_ZONE_SCOPED;
    assert(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);
    // LH = Mano sinistra, ZO = Range profondità 0 a 1 (Vulkan)
    projectionMatrix = glm::perspectiveLH_ZO(fovy, aspect, near, far);
}

void Camera::setViewDirection(const glm::vec3 &position, const glm::vec3 &direction, const glm::vec3 &up) {
    VZ_ZONE_SCOPED;
    viewMatrix = glm::lookAtLH(position, position + direction, up);
}

void Camera::setViewTarget(const glm::vec3 &position, const glm::vec3 &target, const glm::vec3 &up) {
    VZ_ZONE_SCOPED;
    setViewDirection(position, target - position, up);
}

void Camera::setViewYXZ(const glm::vec3 &position, const glm::vec3 &rotation) {
    VZ_ZONE_SCOPED;
    const glm::mat4 rotationMatrix = glm::eulerAngleYXZ(rotation.y, rotation.x, rotation.z);

    const glm::mat4 translation = glm::translate(glm::mat4{1.0f}, -position);

    viewMatrix = translation * rotationMatrix;
}
// clang-format off
// NOLINTEND(*-include-cleaner, *-easily-swappable-parameters, *-uppercase-literal-suffix, *-pro-type-union-access,*-avoid-magic-numbers,*-magic-numbers, *-identifier-length)
// clang-format on