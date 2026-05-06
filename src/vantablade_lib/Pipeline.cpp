/*
* Created by gbian on 06/05/2026.
* Copyright (c) 2026 All rights reserved.
*/

#include "Vantablade/Pipeline.hpp"

Pipeline::Pipeline(const std::string& vertFilepath, const std::string& fragFilepath) {
    createGraphicsPipeline(vertFilepath, fragFilepath);
}

std::vector<char> Pipeline::readFile(const std::string& filepath) {
    auto sCode = vnd::readFromFile(filepath);
    return std::vector<char>(std::ranges::begin(sCode), std::ranges::end(sCode));
}

void Pipeline::createGraphicsPipeline(
    const std::string& vertFilepath, const std::string& fragFilepath) {
    auto vertCode = readFile(vertFilepath);
    auto fragCode = readFile(fragFilepath);

    LINFO("Vertex Shader code size: {}", vertCode.size());
    LINFO("Fragment Shader code size: {}", fragCode.size());
}
