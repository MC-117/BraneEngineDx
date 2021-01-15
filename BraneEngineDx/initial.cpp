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

Mesh* boxMesh(const Range<Vector3f>& boundary)
{
	MeshPart part = VendorManager::getInstance().getVendor().newMeshPart(8, 36);

	Mesh* mesh = new Mesh();
	mesh->setTotalMeshPart(part);
	mesh->addMeshPart("Materail", part);
}

void loadVehicle()
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
		return;
	if (FLWMesh == NULL)
		return;
	if (FRWMesh == NULL)
		return;
	if (BLWMesh == NULL)
		return;
	if (BRWMesh == NULL)
		return;
	if (bodyAsset == NULL)
		return;
	if (frontWheelAsset == NULL)
		return;
	if (backWheelAsset == NULL)
		return;
	if (tpCamAsset == NULL)
		return;
	if (fpCamAsset == NULL)
		return;
	if (gunAsset == NULL)
		return;
	if (gunFirePSAsset == NULL)
		return;
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

	loadVehicle();

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
	world.getCurrentCamera().cameraRender.renderTarget.setMultisampleLevel(Engine::engineConfig.msaa);
}