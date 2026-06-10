/*
 * Created by gbian on 31/05/2026.
 * Copyright (c) 2026 All rights reserved.
 */
// clang-format off
// NOLINTBEGIN(*-include-cleaner, *-uppercase-literal-suffix, *-pro-type-union-access, *-avoid-magic-numbers,*-magic-numbers,*-make-member-function-const)
// clang-format on
#include "Vantablade/KeyboardMovementController.hpp"

// NOLINTNEXTLINE(*-identifier-length)
void KeyboardMovementController::moveInPlaneXZ(GLFWwindow *window, float dt, GameObject &gameObject) {
    glm::vec3 rotate{0};
    if(glfwGetKey(window, keys.lookRight) == GLFW_PRESS) { rotate.y += 1.f; }
    if(glfwGetKey(window, keys.lookLeft) == GLFW_PRESS) { rotate.y -= 1.f; }
    if(glfwGetKey(window, keys.lookUp) == GLFW_PRESS) { rotate.x += 1.f; }
    if(glfwGetKey(window, keys.lookDown) == GLFW_PRESS) { rotate.x -= 1.f; }

    if(glm::length2(rotate) > fepsilon) { gameObject.transform.rotation += lookSpeed * dt * glm::normalize(rotate); }

    // limit pitch values between about +/- 85ish degrees
    gameObject.transform.rotation.x = glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);
    gameObject.transform.rotation.y = glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());

    const float yaw = gameObject.transform.rotation.y;
    const glm::vec3 forwardDir{C_F(std::sin(yaw)), 0.f, C_F(std::cos(yaw))};
    const glm::vec3 rightDir{forwardDir.z, 0.f, -forwardDir.x};
    const glm::vec3 upDir{0.f, -1.f, 0.f};

    glm::vec3 moveDir{0.f};
    if(glfwGetKey(window, keys.moveForward) == GLFW_PRESS) { moveDir += forwardDir; }
    if(glfwGetKey(window, keys.moveBackward) == GLFW_PRESS) { moveDir -= forwardDir; }
    if(glfwGetKey(window, keys.moveRight) == GLFW_PRESS) { moveDir += rightDir; }
    if(glfwGetKey(window, keys.moveLeft) == GLFW_PRESS) { moveDir -= rightDir; }
    if(glfwGetKey(window, keys.moveUp) == GLFW_PRESS) { moveDir += upDir; }
    if(glfwGetKey(window, keys.moveDown) == GLFW_PRESS) { moveDir -= upDir; }

    if(glm::length2(moveDir) > fepsilon) { gameObject.transform.translation += moveSpeed * dt * glm::normalize(moveDir); }
}

// clang-format off
// NOLINTEND(*-include-cleaner, *-uppercase-literal-suffix, *-pro-type-union-access, *-avoid-magic-numbers,*-magic-numbers,*-make-member-function-const)
// clang-format on