#pragma once
#define ENGINE_VERSION "0.1.0"
#define VENDOR_USE_DX11
#define VENDOR_USE_DX12
//#define AUDIO_USE_OPENAL
#define ENABLE_PHYSICS 1

#include <mimalloc.h>
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

#define PHYSICS_USE_PHYSX

#if ENABLE_PHYSICS
#include <PxPhysicsApi.h>
using namespace physx;
typedef PxActor CollisionObject;
typedef PxGeometry CollisionShape;
typedef PxRigidBody CollisionRigidBody;
typedef PxJoint PConstraint;
typedef PxVec3 PVec3;
typedef PxQuat PQuat;
typedef PxTransform PTransform;

#include <NvCloth\Factory.h>
#include <NvCloth\Cloth.h>
#include <NvCloth\Fabric.h>
#include <NvCloth\Solver.h>
#include <JobManager.h>

typedef nv::cloth::Fabric PFabric;
typedef nv::cloth::Cloth PCloth;
#endif // ENABLE_PHYSICS

#include "MathLibrary.h"

#define VERTEX_MAX_BONE 4

// DxMath default matrix is row-major, while HLSL only accept
// matrix with column-major. Thus, engine will apply extra
// transposition operation before upload to shader. However
// BraneEngine used to be a OpenGL based engine with Eigen
// math library (column-major default), so using macro to
// specify matrix upload operation.
#define MATRIX_UPLOAD_OP(mat) (mat).transpose()

using namespace std;
constexpr float PI = 3.141592653589793238462643383279502884197169399375105820974944592307816406L;