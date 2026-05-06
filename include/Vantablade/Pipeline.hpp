/*
* Created by gbian on 06/05/2026.
* Copyright (c) 2026 All rights reserved.
*/

#pragma once

#include "headers.hpp"

class Pipeline {
public:
    Pipeline(const std::string& vertFilepath, const std::string& fragFilepath);

private:
    static std::vector<char> readFile(const std::string& filename);

    void createGraphicsPipeline(const std::string& vertFilepath, const std::string& fragFilepath);
};
