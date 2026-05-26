/*
 * Created by gbian on 01/05/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#pragma once
#include "glm_prety_string_cast.hpp"
// clang-format off
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
// clang-format on

static inline constexpr std::string_view wtile = Vantablade::cmake::project_name;
static inline constexpr std::size_t factor = 80;
static inline constexpr auto wfactor = 16;
static inline constexpr auto hfactor = 9;
// static inline constexpr std::size_t factor = 200;
// static inline constexpr auto wfactor = 4;
// static inline constexpr auto hfactor = 3;
static inline constexpr auto wwidth = wfactor * factor;
static inline constexpr auto wheight = hfactor * factor;
static inline constexpr auto aspectRatio = C_D(wfactor) / C_D(hfactor);
static inline constexpr auto aspectRatiof = C_F(wfactor) / C_F(hfactor);
static inline constexpr auto ui32tmax = std::numeric_limits<uint32_t>::max();
static inline constexpr auto uint64Max = std::numeric_limits<uint64_t>::max();
static inline constexpr auto fepsilon = std::numeric_limits<long double>::epsilon();
// static inline const auto currentP = fs::current_path();

[[nodiscard]] static constexpr auto calcolaCentro(int width, int w) noexcept { return (width - w) / 2; }
#define CALC_CENTRO(width, w) calcolaCentro(width, w)

[[nodiscard]] inline std::optional<fs::path> findProjectRoot(const fs::path &startPath) noexcept {
    std::error_code ec;

    // Se startPath è un file, parte dal suo parent; se è già una directory, parte da lì.
    fs::path current = fs::is_directory(startPath, ec) ? startPath : startPath.parent_path();

    current = fs::weakly_canonical(current, ec);

    while(!current.empty()) {
        if(fs::is_directory(current / "src", ec)) { return current; }
        const fs::path parent = current.parent_path();
        if(parent == current) { break; }  // filesystem root raggiunto
        current = parent;
    }
    return std::nullopt;
}

[[nodiscard]] inline fs::path calculateRelativePathToSrc(const fs::path &startPath, const fs::path &targetFile, std::string_view subDir) {
    const auto projectRoot = findProjectRoot(startPath);
    if(!projectRoot.has_value()) {
        LERROR("project root not found: 'src' directory missing in hierarchy of '{}'", startPath.string());
        return {};
    }
    std::error_code ec;
    const auto result = fs::weakly_canonical(*projectRoot / subDir / targetFile, ec);
    if(ec) {
        LERROR("path resolution failed for '{}/{}': {}", subDir, targetFile.string(), ec.message());
        return {};
    }
    return result;
}

[[nodiscard]] inline fs::path calculateRelativePathToShaders(const fs::path &startPath, const fs::path &targetFile) {
    return calculateRelativePathToSrc(startPath, targetFile, "shaders");
}

[[nodiscard]] inline fs::path calculateRelativePathToTextures(const fs::path &startPath, const fs::path &targetFile) {
    return calculateRelativePathToSrc(startPath, targetFile, "textures");
}

[[nodiscard]] inline fs::path calculateRelativePathToModels(const fs::path &startPath, const fs::path &targetFile) {
    return calculateRelativePathToSrc(startPath, targetFile, "models");
}

[[nodiscard]] inline fs::path calculateRelativePathToAssets(const fs::path &startPath, const fs::path &targetFile) {
    return calculateRelativePathToSrc(startPath, targetFile, "asssets");
}