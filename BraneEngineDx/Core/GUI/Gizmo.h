#pragma once

#include "../Texture2D.h"

class Camera;
class Object;
class Gizmo;

struct GizmoInfo
{
	Gizmo* gizmo;
	Camera* camera;
};

struct GizmoCameraInfo
{
	bool canControlledByEditor = true;
	float speedInEditor = 1;
};

class Gizmo
{
public:
	struct PointDraw
	{
		Vector3f p;
		float size;
		Color color;
	};

	struct LineDraw
	{
		Vector3f p0, p1;
		Color color;
	};

	struct TextDraw
	{
		string text;
		Vector3f position;
		Color color;
	};

	struct IconDraw
	{
		Texture2D* icon;
		int size;
		Vector3f position;
		Color color;
	};

	enum struct HandleType
	{
		Transition, Rotation, Scaling
	};

	enum struct CameraControlMode
	{
		None, Free, Turn
	};

	vector<PointDraw> points;
	vector<LineDraw> lines;
	vector<TextDraw> texts;
	vector<IconDraw> icons;

	Gizmo();
	Gizmo(const string& name);
	virtual ~Gizmo();

	void drawAABB(const Range<Vector3f>& bound, const Matrix4f& transformMat, Color color);
	void drawPoint(const Vector3f& p, float size, Color color);
	void drawLine(const Vector3f& p0, const Vector3f& p1, Color color);
	void drawCircleX(const Vector3f& p, float radius, const Matrix4f& transformMat, Color color, float segment = 12);
	void drawCircleY(const Vector3f& p, float radius, const Matrix4f& transformMat, Color color, float segment = 12);
	void drawCircleZ(const Vector3f& p, float radius, const Matrix4f& transformMat, Color color, float segment = 12);
	void drawSphere(const Vector3f& p, float radius, const Matrix4f& transformMat, Color color, float segment = 12);

	void drawCapsuleX(const Vector3f& p, float radius, float halfLength, const Matrix4f& transformMat, Color color, float segment = 12);
	void drawCapsuleY(const Vector3f& p, float radius, float halfLength, const Matrix4f& transformMat, Color color, float segment = 12);
	void drawCapsuleZ(const Vector3f& p, float radius, float halfLength, const Matrix4f& transformMat, Color color, float segment = 12);

	void drawPyramidX(const Vector3f& p, float radius, float length, float segment, const Matrix4f& transformMat, Color color);
	void drawPyramidY(const Vector3f& p, float radius, float length, float segment, const Matrix4f& transformMat, Color color);
	void drawPyramidZ(const Vector3f& p, float radius, float length, float segment, const Matrix4f& transformMat, Color color);

	bool pickSphere(const Vector3f& p, float radius, const Vector2f& screenOffset, const Matrix4f& transformMat = Matrix4f::Identity());
	bool pickCapsuleX(const Vector3f& p, float radius, float halfLength, const Vector2f& screenOffset, const Matrix4f& transformMat = Matrix4f::Identity());
	bool pickCapsuleY(const Vector3f& p, float radius, float halfLength, const Vector2f& screenOffset, const Matrix4f& transformMat = Matrix4f::Identity());
	bool pickCapsuleZ(const Vector3f& p, float radius, float halfLength, const Vector2f& screenOffset, const Matrix4f& transformMat = Matrix4f::Identity());

	bool pickIcon(const Vector3f& position, const Vector2f& screenOffset, const Vector2f& iconSize, float size = 20);

	void drawText(const string& text, const Vector3f& position, const Color& color = Color(1.0f, 1.0f, 1.0f, 1.0f));

	void drawIcon(Texture2D& icon, const Vector3f& position, int size = 20, const Color& color = Color(1.0f, 1.0f, 1.0f, 1.0f));
	bool drawHandle(void* id, HandleType type, TransformSpace space, Matrix4f& matrix, const Vector3f& snapVector = Vector3f());

	void setCameraControl(CameraControlMode mode, float transitionSensitivity = 30, float rotationSensitivity = 0.1, float distanceSensitivity = 500);
	
	void reset();

	void beginFrame(ImDrawList* drawList);
	void endFrame();
	void begineWindow();
	void endWindow();

	virtual void onGUI(Object* root);

	virtual void onUpdate(Camera& camera);

	virtual void onRender();

protected:
	string name;
	ImDrawList* drawList = NULL;
	ImVec2 windowPos = { 0, 0 };
	Matrix4f projMatrix;
	Matrix4f viewMatrix;
	Camera* camera = NULL;
	bool isFocused = true;
	bool isLastUsing = false;
	bool isUsing = false;
	bool picked = false;
	CameraControlMode cameraControlMode = CameraControlMode::None;
	float transitionSensitivity = 30;
	float rotationSensitivity = (0.1 / 180.0 * PI);
	float distanceSensitivity = 500;
	Vector3f cameraDeltaTransition = Vector3f::Zero();
	Vector3f cameraDeltaRollPitchYaw = Vector3f::Zero();
	float cameraDeltaDistance = 0;

	float cullDistance = 10000;

	void controlFreeCamera(float transitionSensitivity = 30, float rotationSensitivity = 0.1);
	void controlTurnCamera(float transitionSensitivity = 30, float distanceSensitivity = 500, float rotationSensitivity = 0.1);

};