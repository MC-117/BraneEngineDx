#pragma once

#include "../Core/Config.h"
#include "../Core/Console.h"
#include "../Core/MeshData.h"
#include "../Core/Mesh.h"
#include "../Core/GPUBuffer.h"

#include <Live2DCubism/CubismFramework.hpp>
#include <Live2DCubism/CubismDefaultParameterId.hpp>
#include <Live2DCubism/Model/CubismModelUserDataJson.hpp>
#include <Live2DCubism/ICubismModelSetting.hpp>
#include <Live2DCubism/CubismModelSettingJson.hpp>
#include <Live2DCubism/Model/CubismModelUserData.hpp>
#include <Live2DCubism/Model/CubismMoc.hpp>
#include <Live2DCubism/Type/csmVector.hpp>
#include <Live2DCubism/Type/csmRectF.hpp>
#include <Live2DCubism/Math/CubismVector2.hpp>
#include <Live2DCubism/Type/csmMap.hpp>
#include <Live2DCubism/Math/CubismMatrix44.hpp>
#include <Live2DCubism/Model/CubismModel.hpp>
#include <Live2DCubism/Motion/CubismMotion.hpp>

#include <Live2DCubism/Effect/CubismPose.hpp>
#include <Live2DCubism/Effect/CubismEyeBlink.hpp>
#include <Live2DCubism/Effect/CubismBreath.hpp>
#include <Live2DCubism/Math/CubismModelMatrix.hpp>
#include <Live2DCubism/Math/CubismTargetPoint.hpp>
#include <Live2DCubism/Model/CubismMoc.hpp>
#include <Live2DCubism/Model/CubismModel.hpp>
#include <Live2DCubism/Motion/CubismMotionManager.hpp>
#include <Live2DCubism/Motion/CubismExpressionMotion.hpp>
#include <Live2DCubism/Physics/CubismPhysics.hpp>
#include <Live2DCubism/Model/CubismModelUserData.hpp>

#include "../Core/InitializationManager.h"

class Live2DInitialization : public Initialization
{
protected:
	static Live2DInitialization instance;
	Live2DInitialization();

	virtual bool initialize();
	virtual bool finalize();
};