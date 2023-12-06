#include "resource.h"
#include "Core\Engine.h"
#include "Core\Console.h"
#include "Core\ObjectUltility.h"
#include "Core\DirectLight.h"
#include "Core\GUI\EditorWindow.h"
#include "Core\GUI\ESCMenu.h"
#include "Core\GUI\DebugLogWindow.h"
#include "Core\GUI\CMDWindow.h"
#include "Core\GUI\MatBranchModifier.h"
#include "Core\GUI\PhysicsDebug.h"
#include "Core\GUI\ShaderManagerWindow.h"
#include "Core\GUI\UVViewer.h"
#include "Core\GUI\UIViewport.h"
#include "Core\Timeline\TimelineWindow.h"
#include "Core\Timeline\AnimationTrack.h"
#include "Core\Graph\GraphWindow.h"
#include "Cameras\DebugCamera.h"
#include "Actors\SkySphere.h"
#include "Actors\VehicleActor.h"
#include "Actors\GunTowerActor.h"
#include "Live2D\Live2DActor.h"
#include "Actors\ClothActor.h"
#include "Core/Application.h"
#include "Midi/MidiInstrumentWindow.h"

VehicleActor* loadCubeVehicle(float unit = 2)
{
	World& world = *Engine::getCurrentWorld();
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
	World& world = *Engine::getCurrentWorld();
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

class EditorApplication : public WorldApplication
{
public:
	EditorApplication() : WorldApplication(MAKEINTRESOURCE(IDI_ICON1)) {}
	virtual void initializeWorld()
	{
		World& world = *Engine::getCurrentWorld();
		//world.input.setCursorHidden(true);

		//world.physicalWorld.setPause(true);
		world.physicalWorld.setGravity(Vector3f(0, 0, -10));

		//Mesh* clothMesh = getAssetByPath<Mesh>("Content/Cloth/ClothPlane.fbx");
		//Material* clothMaterial = getAssetByPath<Material>("Content/Cloth/ClothM.imat");
		//ClothActor& cloth = *new ClothActor("Cloth");
		//cloth.setMesh(clothMesh);
		//cloth.meshRender.setMaterial(0, *clothMaterial);
		////cloth.addSphereCollider({ 0, 0, -100 }, 10);
		//cloth.addSphereCollider({ 20, 20, -100 }, 10);
		//cloth.addSphereCollider({ -20, 20, -100 }, 10);
		//cloth.addSphereCollider({ 20, -20, -100 }, 10);
		//cloth.addSphereCollider({ -20, -20, -100 }, 10);
		////cloth.addBoxCollider({ 0, 0, -305 }, { 100, 100, 5 });

		//ClothVertexCluster* cluster = cloth.clothBody.addVertexCluster({ 50, 50, 0 }, Quaternionf::Identity());
		//cluster->addSphere({ 0, 0, 0, 5 });
		//cluster = cloth.clothBody.addVertexCluster({ 50, -50, 0 }, Quaternionf::Identity());
		//cluster->addSphere({ 0, 0, 0, 5 });
		//world += cloth;
		//cloth.setPosition(0, 0, 300);

		GUISurface::getMainViewportGUISurface().bindCamera(&world.defaultCamera);

		DebugCamera& debugCamera = *new DebugCamera();
		world += debugCamera;
		world.switchCamera(debugCamera);

		static VehicleActor* vehicles = loadVehicle();
		static VehicleActor* cubeVehicles = loadCubeVehicle();

		/*Live2DModel* live2DModel = getAssetByPath<Live2DModel>("Content/Live2D/Hiyori/Hiyori.model3.live2d");
		if (live2DModel != NULL) {
			Live2DActor& live2DActor = *new Live2DActor("Hiyori");
			live2DActor.setModel(live2DModel);
			world += live2DActor;
		}*/

		/*world.events.registerOnTick([](Object* self, float dt) {
			World* world = (World*)self;
			Input& input = Engine::input;
			if (input.getCursorHidden() || input.getMouseButtonDown(MouseButtonEnum::Right)) {
				if (input.getKeyPress('Q')) {
					if (vehicles->isControled())
						world->switchCamera(cubeVehicles->TPCamera);
					else if (cubeVehicles->isControled())
						world->switchCamera(vehicles->TPCamera);
				}
			}
			if (input.getKeyPress('P')) {
				world->physicalWorld.setPause(!world->physicalWorld.getPause());
			}
		});*/

		SerializationInfo* info = getAssetByPath<SerializationInfo>(Engine::engineConfig.startMapPath);
		if (info == NULL) {
			Console::error("Can not load map '%s'", Engine::engineConfig.startMapPath.c_str());
			return;
		}

		world.loadWorld(*info);
		world.deserialize(*info);

		// if (findFirst(world, *SerializationManager::getSerialization("DirectLight")) == NULL) {
		// 	DirectLight& dirLight = *new DirectLight("DirLight");
		// 	dirLight.setRotation(0, -45, -45);
		// 	world += dirLight;
		// }
	}
	
	virtual void initializeGUILayout(GUI& gui)
	{
		//ESCMenu& escMenu = *new ESCMenu("ESCMenu", true);
		//gui += escMenu;

		UIViewport& mainViewport = *new UIViewport("MainViewport", true);
		gui += mainViewport;

		EditorWindow& editorWindow = *new EditorWindow(*Engine::getCurrentWorld(), Material::nullMaterial, "Editor", true);
		editorWindow.blurBackground = true;
		editorWindow.showCloseButton = false;
		gui += editorWindow;

		DebugLogWindow& debugLogWindow = *new DebugLogWindow("DebugLogWindow", true);
		gui += debugLogWindow;
		debugLogWindow.show = true;

		CMDWindow& cmdWindow = *new CMDWindow("CMDWindow");
		gui += cmdWindow;

		MatBranchModifier& matBranchModifier = *new MatBranchModifier("MatBranchModifier", false);
		gui += matBranchModifier;

		PhysicsDebug& physicsDebug = *new PhysicsDebug("PhysicsDebug");
		gui += physicsDebug;

		TimelineWindow& timelineWindow = *new TimelineWindow("Timeline");
		gui += timelineWindow;

		//GraphWindow& graphWindow = *new GraphWindow("Graph");
		//world += graphWindow;

		ShaderManagerWindow& shaderManagerWindow = *new ShaderManagerWindow("ShaderManagerWindow");
		gui += shaderManagerWindow;

		UVViewer& uvViewer = *new UVViewer("UVViewer");
		gui += uvViewer;

		MidiInstrumentWindow& midiInstrumentWindow = *new MidiInstrumentWindow("MidiInstrumentWindow");
		gui += midiInstrumentWindow;
	}
};

RegisterApplication(EditorApplication);
