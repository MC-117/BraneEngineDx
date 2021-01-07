#include "DebugCamera.h"
#include "../Core/Engine.h"

DebugCamera::DebugCamera(string name) : PostProcessingCamera(name)
{
	events.registerFunc("SetCameraSpeed", [](void* obj, float v) {
		((DebugCamera*)obj)->cameraSpeed = v;
	});
}

void DebugCamera::tick(float deltaTime)
{
	World* world = Engine::getCurrentWorld();
	if (world == NULL)
		return;
	Input& input = world->input;
	if (&world->getCurrentCamera() != this)
		return;
	if ((input.getCursorHidden() || input.getMouseButtonDown(MouseButtonEnum::Right))) {
		Vector3f v(0, 0, 0);
		v.x() = (int)input.getKeyDown('W') - (int)input.getKeyDown('S');
		v.x() += input.getMouseWheelValue();
		v.y() = (int)input.getKeyDown('A') - (int)input.getKeyDown('D');
		v.z() = (int)input.getKeyDown('E') - (int)input.getKeyDown('Q');
		v *= deltaTime * 30 * cameraSpeed;
		translate(v.x(), v.y(), 0);
		translate(0, 0, v.z(), WORLD);

		Unit2Di m = input.getMouseMove();
		rotate(0, 0, -m.x * deltaTime * 5, WORLD);
		rotate(0, m.y * deltaTime * 5, 0);
	}
}
