#pragma once
#define ENGINE_VERSION "0.1.0"
#define VENDOR_USE_DX11
#define ENABLE_PHYSICS 0

#include "StaticVar.h"
#include <iostream>
#include <vector>
#include <queue>
#include <map>
#include <unordered_map>
#include <list>
#include <set>
#include <unordered_set>
#include <thread>
#include <sys/timeb.h>
#include <string>
#include <sstream>
#include <filesystem>
#include <chrono>

#include <windows.h>

#include "../ThirdParty/ImGui/imgui.h"
#include "../ThirdParty/ImGui/ImGuizmo.h"

#include "Brane.h"
#include "MathLibrary.h"

#define VERTEX_MAX_BONE 4

#define MAT_INS_BIND_INDEX 0
#define TRANS_BIND_INDEX 0
#define TRANS_INDEX_BIND_INDEX 1
#define MORPHDATA_BIND_INDEX 2
#define MORPHWEIGHT_BIND_INDEX 3
#define PARTICLE_BIND_INDEX 4
#define CAM_BIND_INDEX 5
#define DIRECT_LIGHT_BIND_INDEX 6
#define POINT_LIGHT_BIND_INDEX 7

// DxMath default matrix is row-major, while HLSL only accept
// matrix with column-major. Thus, engine will apply extra
// transposition operation before upload to shader. However
// BraneEngine used to be a OpenGL based engine with Eigen
// math library (column-major default), so using macro to
// specify matrix upload operation.
#define MATRIX_UPLOAD_OP(mat) (mat).transpose()

using namespace std;
constexpr auto PI = 3.1415926535897932346f;