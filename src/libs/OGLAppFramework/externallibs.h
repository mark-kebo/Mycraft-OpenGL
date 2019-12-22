#pragma once

#include <iostream>
#include <cstdint>
#include <string>
#include <optional>

#define GLFW_INCLUDE_NONE

#define GLM_FORCE_RADIANS
#define GLM_FORCE_SSE2

#include <glbinding/Version.h>
#include <glbinding/Binding.h>
#include <glbinding/FunctionCall.h>
#include <glbinding/CallbackMask.h>
#include <glbinding/gl/gl.h>
#include <glbinding/gl/extension.h>
#include <glbinding/glbinding.h>

#include <glbinding-aux/types_to_string.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/constants.hpp>
//#include <glm/gtx/vector_angle.hpp>

#include <gli/gli.hpp>
