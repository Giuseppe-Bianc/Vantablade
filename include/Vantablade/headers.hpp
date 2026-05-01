/*
 * Created by gbian on 01/05/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#pragma once
// clang-format off
#include "VantabladeCore/vantabladeCore.hpp"
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
// clang-format on

[[nodiscard]] static constexpr auto calcolaCentro(const int &width, const int &w) noexcept { return (width - w) / 2; }
#define CALC_CENTRO(width, w) calcolaCentro(width, w)

inline fs::path calculateRelativePathToSrc(const fs::path &executablePath, const fs::path &targetFile, const std::string &subDir) {
    fs::path parentDir = executablePath.parent_path();
    while(!fs::exists(parentDir / "src")) {
        parentDir = parentDir.parent_path();
        if(parentDir == parentDir.root_path()) {
            LERROR("Error: 'src' directory not found in the path.");
            return {};
        }
    }
    parentDir = parentDir.parent_path();
    const auto resp = fs::path(subDir);
    const auto relativePathToTarget = parentDir / resp / targetFile;
    const auto relativePath = fs::relative(relativePathToTarget, executablePath);
    return relativePath.lexically_normal();
}

inline fs::path calculateRelativePathToShaders(const fs::path &executablePath, const fs::path &targetFile) {
    return calculateRelativePathToSrc(executablePath, targetFile, "shaders");
}
inline fs::path calculateRelativePathToTextures(const fs::path &executablePath, const fs::path &targetFile) {
    return calculateRelativePathToSrc(executablePath, targetFile, "textures");
}
inline fs::path calculateRelativePathToModels(const fs::path &executablePath, const fs::path &targetFile) {
    return calculateRelativePathToSrc(executablePath, targetFile, "models");
}