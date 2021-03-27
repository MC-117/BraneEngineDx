#include "Core\Engine.h"
#include "Core\Console.h"
#include "Core\GUI\EditorWindow.h"
#include "Core\GUI\ESCMenu.h"
#include "Core\GUI\DebugLogWindow.h"
#include "Core\GUI\MatBranchModifier.h"
#include "Cameras\DebugCamera.h"
#include "Actors\SkySphere.h"
#include "Actors\VehicleActor.h"
#include "Actors\GunTowerActor.h"

VehicleActor* loadCubeVehicle(float unit = 2)
{
	Material* gridM = getAssetByPath<Material>("Content/GridM.imat");
	SerializationInfo* tpCamAsset = getAssetByPath<SerializationInfo>("Content/CubeVehicle/CubeVehicleTPCameraPack.asset");
	SerializationInfo* fpCamAsset = getAssetByPath<SerializationInfo>("Content/CubeVehicle/CubeVehicleFPCameraPack.asset");
	SerializationInfo* bodyAsset = getAssetByPath<SerializationInfo>("Content/CubeVehicle/CubeVehiclePack.asset");
	if (gridM == NULL)
		return NULL;
	if (tpCamAsset == NULL)
		return NULL;
	if (fpCamAsset == NULL)
		return NULL;
	if (bodyAsset == NULL)
		return NULL;
	Vector3f cubeScale = { 6, 3, 1 };
	Vector3f columnScale = { 2, 2, 1 };
	cubeScale *= unit;
	columnScale *= unit;
	Mesh& cube = *Box(-cubeScale, cubeScale).toMesh();
	Mesh& column = *Column(-columnScale, columnScale).toMesh(60, Vector3f::UnitY());
	VehicleActor& vehicle = *new VehicleActor(cube, *gridM, "CubeVehicle");
	vehicle.deserialize(*bodyAsset);
	vehicle.TPCamera.deserialize(*tpCamAsset);
	vehicle.FPCamera.deserialize(*fpCamAsset);

	WheelDesc desc;

	Vector3f offset = cubeScale;
	offset.x() -= columnScale.x();
	offset.z() *= -1;

	desc.offset = { -offset.x(), offset.y(), offset.z() };
	vehicle.addWheel(desc, column, *gridM);
	desc.offset = { -offset.x(), -offset.y(), offset.z() };
	vehicle.addWheel(desc, column, *gridM);

	desc.steerAngle = 30 / 180.f * PI;
	desc.offset = { offset.x(), offset.y(), offset.z() };
	vehicle.addWheel(desc, column, *gridM);
	desc.offset = { offset.x(), -offset.y(), offset.z() };
	vehicle.addWheel(desc, column, *gridM);

	world += vehicle;
	world.switchCamera(vehicle.TPCamera);

	return &vehicle;
}

VehicleActor* loadVehicle()
{
	Mesh* bodyMesh = getAssetByPath<Mesh>("Content/Vehicle/VehicleBody.fbx");
	Mesh* FLWMesh = getAssetByPath<Mesh>("Content/Vehicle/VehicleFLW.fbx");
	Mesh* FRWMesh = getAssetByPath<Mesh>("Content/Vehicle/VehicleFRW.fbx");
	Mesh* BLWMesh = getAssetByPath<Mesh>("Content/Vehicle/VehicleBLW.fbx");
	Mesh* BRWMesh = getAssetByPath<Mesh>("Content/Vehicle/VehicleBRW.fbx");
	SkeletonMesh* gunMesh = getAssetByPath<SkeletonMesh>("Content/GunTower/GunTower.fbx");
	SerializationInfo* bodyAsset = getAssetByPath<SerializationInfo>("Content/Vehicle/VehicleBodyPack.asset");
	SerializationInfo* frontWheelAsset = getAssetByPath<SerializationInfo>("Content/Vehicle/VehicleFrontWheelPack.asset");
	SerializationInfo* backWheelAsset = getAssetByPath<SerializationInfo>("Content/Vehicle/VehicleBackWheelPack.asset");
	SerializationInfo* tpCamAsset = getAssetByPath<SerializationInfo>("Content/Vehicle/VehicleTPCameraPack.asset");
	SerializationInfo* fpCamAsset = getAssetByPath<SerializationInfo>("Content/Vehicle/VehicleFPCameraPack.asset");
	SerializationInfo* gunAsset = getAssetByPath<SerializationInfo>("Content/GunTower/GunPack.asset");
	SerializationInfo* gunFirePSAsset = getAssetByPath<SerializationInfo>("Content/GunTower/GunFirePSPack.asset");
	if (bodyMesh == NULL)
		return NULL;
	if (FLWMesh == NULL)
		return NULL;
	if (FRWMesh == NULL)
		return NULL;
	if (BLWMesh == NULL)
		return NULL;
	if (BRWMesh == NULL)
		return NULL;
	if (bodyAsset == NULL)
		return NULL;
	if (frontWheelAsset == NULL)
		return NULL;
	if (backWheelAsset == NULL)
		return NULL;
	if (tpCamAsset == NULL)
		return NULL;
	if (fpCamAsset == NULL)
		return NULL;
	if (gunAsset == NULL)
		return NULL;
	if (gunFirePSAsset == NULL)
		return NULL;
	VehicleActor& vehicle = *new VehicleActor(*bodyMesh, Material::nullMaterial, "Vehicle");
	vehicle.deserialize(*bodyAsset);
	vehicle.TPCamera.deserialize(*tpCamAsset);
	vehicle.FPCamera.deserialize(*fpCamAsset);
	WheelDesc desc;
	desc.offset = { -10.762, 8.439, -1.117 };
	vehicle.addWheel(desc, *BLWMesh, Material::nullMaterial).deserialize(*backWheelAsset);
	desc.offset = { -10.762, -8.439, -1.117 };
	vehicle.addWheel(desc, *BRWMesh, Material::nullMaterial).deserialize(*backWheelAsset);

	desc.steerAngle = 30 / 180.f * PI;
	desc.offset = { 9.021, 7.886, -1.088 };
	vehicle.addWheel(desc, *FLWMesh, Material::nullMaterial).deserialize(*frontWheelAsset);
	desc.offset = { 9.021, -7.886, -1.088 };
	vehicle.addWheel(desc, *FRWMesh, Material::nullMaterial).deserialize(*frontWheelAsset);

	GunTowerActor& gun = *new GunTowerActor(*gunMesh, Material::nullMaterial, "Gun");
	gun.deserialize(*gunAsset);
	gun.setYawBone("BasinBone");
	gun.setPitchBone("BodyBone");
	gun.setRollBone("TubeBone");
	gun.particleSystem.deserialize(*gunFirePSAsset);

	vehicle.addChild(gun);

	world += vehicle;

	world.switchCamera(vehicle.TPCamera);

	return &vehicle;
}

void InitialWorld()
{
	//world.input.setCursorHidden(true);

	DirectLight& dirLight = *new DirectLight("DirLight");
	dirLight.setRotation(0, -45, -45);
	world += dirLight;

	SkySphere& sky = *new SkySphere();
	sky.loadDefaultTexture();
	sky.scaling(50, 50, 50);
	world += sky;

	DebugCamera& debugCamera = *new DebugCamera();
	world += debugCamera;
	world.switchCamera(debugCamera);

	static VehicleActor* vehicles = loadVehicle();
	static VehicleActor* cubeVehicles = loadCubeVehicle();

	world.events.registerOnTick([](Object* self, float dt) {
		World* world = (World*)self;
		Input& input = world->input;
		if (input.getCursorHidden() || input.getMouseButtonDown(MouseButtonEnum::Right)) {
			if (input.getKeyPress('Q')) {
				Console::log("Q");
				if (vehicles->isControled())
					world->switchCamera(cubeVehicles->TPCamera);
				else if (cubeVehicles->isControled())
					world->switchCamera(vehicles->TPCamera);
			}
		}
	});

	ESCMenu& escMenu = *new ESCMenu("ESCMenu", true);
	world += escMenu;

	EditorWindow& editorWindow = *new EditorWindow(world, Material::nullMaterial, "Editor", false);
	editorWindow.blurBackground = true;
	editorWindow.showCloseButton = false;
	world += editorWindow;

	DebugLogWindow& debugLogWindow = *new DebugLogWindow("DebugLogWindow", true);
	world += debugLogWindow;
	debugLogWindow.show = true;

	MatBranchModifier& matBranchModifier = *new MatBranchModifier("MatBranchModifier", false);
	world += matBranchModifier;

	SerializationInfo* info = getAssetByPath<SerializationInfo>(Engine::engineConfig.startMapPath);
	if (info == NULL) {
		Console::error("Can not load map '%s'", Engine::engineConfig.startMapPath.c_str());
		return;
	}

	world.loadWorld(*info);
	world.deserialize(*info);
}