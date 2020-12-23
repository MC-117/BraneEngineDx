#pragma once
#define ENGINE_VERSION "0.1.0"
#define VENDOR_USE_DX11
#define ENABLE_PHYSICS 0

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

#include "../ThirdParty/ImGui/imgui.h"
#include "../ThirdParty/ImGui/ImGuizmo.h"

#ifdef VENDOR_USE_DX11
#include <d3d11.h>
#include <DirectXMath.h>
using namespace DirectX;
#endif // VENDOR_USE_DX11

#include "Brane.h"
#include "MathLibrary.h"

#define VERTEX_MAX_BONE 4

#define TRANS_BIND_INDEX 0
#define MORPHDATA_BIND_INDEX 1
#define MORPHWEIGHT_BIND_INDEX 2
#define PARTICLE_BIND_INDEX 3
#define MODIFIER_BIND_INDEX 4
#define TRANS_INDEX_BIND_INDEX 5
#define LIGHT_BIND_INDEX 6
#define CAM_BIND_INDEX 7

using namespace std;
constexpr auto PI = 3.1415926535897932346f;