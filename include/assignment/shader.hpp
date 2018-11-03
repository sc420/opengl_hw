#pragma once

#include "common.hpp"

GLchar* LoadShaderSource(const std::string& file);

void FreeShaderSource(GLchar* src);
