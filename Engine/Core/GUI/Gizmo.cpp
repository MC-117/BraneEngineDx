#include "Gizmo.h"
#include "../Camera.h"
#include "../Engine.h"
#include "../Editor/Editor.h"
#include "../Editor/CameraEditor.h"
#include "../Mesh.h"
#include "../RenderCore/RenderCore.h"
#include "../RenderCore/RenderCoreUtility.h"
#include "../MeshRender.h"

Gizmo::Gizmo()
{
}

Gizmo::Gizmo(const string& name) : name(name)
{
}

Gizmo::~Gizmo()
{
}

void Gizmo::setEnableGUI(bool enable)
{
	enableGUI = enable;
}

bool Gizmo::getEnableGUI() const
{
	return enableGUI;
}

void Gizmo::drawAABB(const BoundBox& bound, const Matrix4f& transformMat, Color color)
{
	Vector3f extent = bound.getExtent();
	Vector4f center = Vector4f(bound.getCenter(), 0.0f);
	Vector4f points[8] =
	{
		center + Vector4f(extent, 1.0f),
		center + Vector4f(-extent.x(), extent.y(), extent.z(), 1.0f),
		center + Vector4f(-extent.x(), -extent.y(), extent.z(), 1.0f),
		center + Vector4f(extent.x(), -extent.y(), extent.z(), 1.0f),

		center + Vector4f(extent.x(), extent.y(), -extent.z(), 1.0f),
		center + Vector4f(-extent.x(), extent.y(), -extent.z(), 1.0f),
		center + Vector4f(-extent, 1.0f),
		center + Vector4f(extent.x(), -extent.y(), -extent.z(), 1.0f)
	};

	for (int i = 0; i < 8; i++) {
		Vector4f& point = points[i];
		point = transformMat * Vector4f(point.x(), point.y(), point.z(), 1);
	}

	lines.emplace_back(LineDrawData{ points[0], color, points[1], color });
	lines.emplace_back(LineDrawData{ points[1], color, points[2], color });
	lines.emplace_back(LineDrawData{ points[2], color, points[3], color });
	lines.emplace_back(LineDrawData{ points[3], color, points[0], color });

	lines.emplace_back(LineDrawData{ points[0], color, points[4], color });
	lines.emplace_back(LineDrawData{ points[1], color, points[5], color });
	lines.emplace_back(LineDrawData{ points[2], color, points[6], color });
	lines.emplace_back(LineDrawData{ points[3], color, points[7], color });

	lines.emplace_back(LineDrawData{ points[4], color, points[5], color });
	lines.emplace_back(LineDrawData{ points[5], color, points[6], color });
	lines.emplace_back(LineDrawData{ points[6], color, points[7], color });
	lines.emplace_back(LineDrawData{ points[7], color, points[4], color });
}

void Gizmo::drawPoint(const Vector3f& p, float size, Color color)
{
	points.emplace_back(PointDraw{ p, size, color });
}

void Gizmo::drawLine(const Vector3f& p0, const Vector3f& p1, Color color)
{
	lines.emplace_back(LineDrawData{ Vector4f(p0, 1.0f), color, Vector4f(p1, 1.0f), color });
}

void Gizmo::drawCircleX(const Vector3f& p, float radius, const Matrix4f& transformMat, Color color, float segment)
{
	Vector4f pointA = transformMat * Vector4f(p.x(), p.y() + radius, p.z(), 1);
	for (int i = 0; i < segment; i++) {
		float radian = (i + 1) / (float)segment * PI * 2;
		Vector4f pointB = transformMat * Vector4f(p.x(), p.y() + radius * cosf(radian), p.z() + radius * sinf(radian), 1);
		lines.emplace_back(LineDrawData{ pointA, color, pointB, color });
		pointA = pointB;
	}
}

void Gizmo::drawCircleY(const Vector3f& p, float radius, const Matrix4f& transformMat, Color color, float segment)
{
	Vector4f pointA = transformMat * Vector4f(p.x() + radius, p.y(), p.z(), 1);
	for (int i = 0; i < segment; i++) {
		float radian = (i + 1) / (float)segment * PI * 2;
		Vector4f pointB = transformMat * Vector4f(p.x() + radius * cosf(radian), p.y(), p.z() + radius * sinf(radian), 1);
		lines.emplace_back(LineDrawData{ pointA, color, pointB, color });
		pointA = pointB;
	}
}

void Gizmo::drawCircleZ(const Vector3f& p, float radius, const Matrix4f& transformMat, Color color, float segment)
{
	Vector4f pointA = transformMat * Vector4f(p.x() + radius, p.y(), p.z(), 1);
	for (int i = 0; i < segment; i++) {
		float radian = (i + 1) / (float)segment * PI * 2;
		Vector4f pointB = transformMat * Vector4f(p.x() + radius * cosf(radian), p.y() + radius * sinf(radian), p.z(), 1);
		lines.emplace_back(LineDrawData{ pointA, color, pointB, color });
		pointA = pointB;
	}
}

void Gizmo::drawSphere(const Vector3f& p, float radius, const Matrix4f& transformMat, Color color, float segment)
{
	if (camera != NULL) {
		Vector3f camPos = camera->getPosition(WORLD);
		Vector3f camForward = camera->getForward(WORLD);
		Vector3f rVec = (transformMat * Vector4f(radius, 0, 0, 0)).block(0, 0, 3, 1);
		Vector3f pos = (transformMat * Vector4f(p.x(), p.y(), p.z(), 1)).block(0, 0, 3, 1);
		float r = rVec.norm();
		Vector3f vec = pos - camPos;
		float vecLen = vec.norm();
		vec /= vecLen;
		float dis = abs((vec * (vecLen - r)).dot(camForward));
		float tanHalfFovy = tanf(camera->fov * PI / 360.0f);
		float pixelSize = r / (dis * tanHalfFovy) * camera->size.y;

		int seg = segment * pixelSize * 0.02;

		segment = std::max(segment, (float)seg);
		segment = std::min(segment, 60.0f);
	}
	drawCircleX(p, radius, transformMat, color, segment);
	drawCircleY(p, radius, transformMat, color, segment);
	drawCircleZ(p, radius, transformMat, color, segment);
}

void Gizmo::drawCapsuleX(const Vector3f& p, float radius, float halfLength, const Matrix4f& transformMat, Color color, float segment)
{
	drawSphere(p + Vector3f(halfLength, 0, 0), radius, transformMat, color, segment);
	drawSphere(p + Vector3f(-halfLength, 0, 0), radius, transformMat, color, segment);
	Vector4f linePointAs[4] =
	{
		Vector4f(halfLength, radius, 0, 1),
		Vector4f(halfLength, -radius, 0, 1),
		Vector4f(halfLength, 0, radius, 1),
		Vector4f(halfLength, 0, -radius, 1)
	};
	Vector4f linePointBs[4] =
	{
		Vector4f(-halfLength, radius, 0, 1),
		Vector4f(-halfLength, -radius, 0, 1),
		Vector4f(-halfLength, 0, radius, 1),
		Vector4f(-halfLength, 0, -radius, 1)
	};
	Vector4f offset = Vector4f(p.x(), p.y(), p.z(), 0);
	for (int i = 0; i < 4; i++) {
		Vector4f linePointA = transformMat * (linePointAs[i] + offset);
		Vector4f linePointB = transformMat * (linePointBs[i] + offset);
		lines.emplace_back(LineDrawData{ linePointA, color, linePointB, color });
	}
}

void Gizmo::drawCapsuleY(const Vector3f& p, float radius, float halfLength, const Matrix4f& transformMat, Color color, float segment)
{
	drawSphere(p + Vector3f(0, halfLength, 0), radius, transformMat, color, segment);
	drawSphere(p + Vector3f(0, -halfLength, 0), radius, transformMat, color, segment);
	Vector4f linePointAs[4] =
	{
		Vector4f(radius, halfLength, 0, 1),
		Vector4f(-radius, halfLength, 0, 1),
		Vector4f(0, halfLength, radius, 1),
		Vector4f(0, halfLength, -radius, 1)
	};
	Vector4f linePointBs[4] =
	{
		Vector4f(radius, -halfLength, 0, 1),
		Vector4f(-radius, -halfLength, 0, 1),
		Vector4f(0, -halfLength, radius, 1),
		Vector4f(0, -halfLength, -radius, 1)
	};
	Vector4f offset = Vector4f(p.x(), p.y(), p.z(), 0);
	for (int i = 0; i < 4; i++) {
		Vector4f linePointA = transformMat * (linePointAs[i] + offset);
		Vector4f linePointB = transformMat * (linePointBs[i] + offset);
		lines.emplace_back(LineDrawData{ linePointA, color, linePointB, color });
	}
}

void Gizmo::drawCapsuleZ(const Vector3f& p, float radius, float halfLength, const Matrix4f& transformMat, Color color, float segment)
{
	drawSphere(p + Vector3f(0, 0, halfLength), radius, transformMat, color, segment);
	drawSphere(p + Vector3f(0, 0, -halfLength), radius, transformMat, color, segment);
	Vector4f linePointAs[4] =
	{
		Vector4f(radius, 0, halfLength, 1),
		Vector4f(-radius, 0, halfLength, 1),
		Vector4f(0, radius, halfLength, 1),
		Vector4f(0, -radius, halfLength, 1)
	};
	Vector4f linePointBs[4] =
	{
		Vector4f(radius, 0, -halfLength, 1),
		Vector4f(-radius, 0, -halfLength, 1),
		Vector4f(0, radius, -halfLength, 1),
		Vector4f(0, -radius, -halfLength, 1)
	};
	Vector4f offset = Vector4f(p.x(), p.y(), p.z(), 0);
	for (int i = 0; i < 4; i++) {
		Vector4f linePointA = transformMat * (linePointAs[i] + offset);
		Vector4f linePointB = transformMat * (linePointBs[i] + offset);
		lines.emplace_back(LineDrawData{ linePointA, color, linePointB, color });
	}
}

void Gizmo::drawPyramidX(const Vector3f& p, float radius, float length, float segment, const Matrix4f& transformMat, Color color)
{
	Vector4f pos = Vector4f(p.x(), p.y(), p.z(), 1);
	Vector4f highPoint = transformMat * (pos + Vector4f(length, 0, 0, 0));
	Vector4f lastPoint = transformMat * (pos + Vector4f(0, radius, 0, 0));
	for (int i = 0; i < segment; i++) {
		float radian = (i + 1) / (float)segment * PI * 2;
		Vector4f point = transformMat * (pos + Vector4f(0, radius * cosf(radian), radius * sinf(radian), 0));
		lines.emplace_back(LineDrawData{ lastPoint, color, highPoint, color });
		lines.emplace_back(LineDrawData{ point, color, lastPoint, color });
		lastPoint = point;
	}
}

void Gizmo::drawPyramidY(const Vector3f& p, float radius, float length, float segment, const Matrix4f& transformMat, Color color)
{
	Vector4f pos = Vector4f(p.x(), p.y(), p.z(), 1);
	Vector4f highPoint = transformMat * (pos + Vector4f(0, length, 0, 0));
	Vector4f lastPoint = transformMat * (pos + Vector4f(radius, 0, 0, 0));
	for (int i = 0; i < segment; i++) {
		float radian = (i + 1) / (float)segment * PI * 2;
		Vector4f point = transformMat * (pos + Vector4f(radius * cosf(radian), 0, radius * sinf(radian), 0));
		lines.emplace_back(LineDrawData{ lastPoint, color, highPoint, color });
		lines.emplace_back(LineDrawData{ point, color, lastPoint, color });
		lastPoint = point;
	}
}

void Gizmo::drawPyramidZ(const Vector3f& p, float radius, float length, float segment, const Matrix4f& transformMat, Color color)
{
	Vector4f pos = Vector4f(p.x(), p.y(), p.z(), 1);
	Vector4f highPoint = transformMat * (pos + Vector4f(0, 0, length, 0));
	Vector4f lastPoint = transformMat * (pos + Vector4f(radius, 0, 0, 0));
	for (int i = 0; i < segment; i++) {
		float radian = (i + 1) / (float)segment * PI * 2;
		Vector4f point = transformMat * (pos + Vector4f(radius * cosf(radian), radius * sinf(radian), 0, 0));
		lines.emplace_back(LineDrawData{ lastPoint, color, highPoint, color });
		lines.emplace_back(LineDrawData{ point, color, lastPoint, color });
		lastPoint = point;
	}
}

bool Gizmo::pickSphere(const Vector3f& p, float radius, const Vector2f& screenOffset, const Matrix4f& transformMat)
{
	if (!ImGui::IsWindowFocused() ||
		!ImGui::IsMouseReleased(ImGuiMouseButton_Left) ||
		camera == NULL || picked) {
		return false;
	}

	Vector3f _pos, _sca;
	Quaternionf _rot;
	transformMat.decompose(_pos, _rot, _sca);

	auto _mpos = ImGui::GetMousePos();
	Vector4f mpos = { _mpos.x - windowPos.x + screenOffset.x(), _mpos.y - windowPos.y + screenOffset.y(), 1, 1 };
	mpos.x() /= camera->size.x;
	mpos.y() /= camera->size.y;
	mpos.x() = mpos.x() * 2 - 1;
	mpos.y() = 1 - mpos.y() * 2;

	mpos = projMatrix.inverse() * mpos;
	mpos /= mpos.w();
	mpos.w() = 0;
	mpos = viewMatrix.inverse() * mpos;

	Vector3f ray = mpos.block(0, 0, 3, 1);
	ray.normalize();

	Vector3f p3d = (transformMat * Vector4f(p.x(), p.y(), p.z(), 1)).block(0, 0, 3, 1);
	Vector3f v0 = p3d - (camera->getPosition(WORLD) - camera->getForward(WORLD) * camera->distance);

	float dis0 = v0.dot(ray);

	if (dis0 <= 0)
		return false;

	bool _picked = sqrtf(v0.squaredNorm() - dis0 * dis0) <= (radius * _sca.norm());

	if (_picked)
		picked = _picked;

	return _picked;
}

bool Gizmo::pickCapsuleX(const Vector3f& p, float radius, float halfLength, const Vector2f& screenOffset, const Matrix4f& transformMat)
{
	if (!ImGui::IsWindowFocused() ||
		!ImGui::IsMouseReleased(ImGuiMouseButton_Left) ||
		camera == NULL || picked) {
		return false;
	}

	bool _picked = false;

	Vector3f _pos, _sca;
	Quaternionf _rot;
	transformMat.decompose(_pos, _rot, _sca);

	radius *= _sca.norm();

	auto _mpos = ImGui::GetMousePos();
	Vector4f mpos = { _mpos.x - windowPos.x + screenOffset.x(), _mpos.y - windowPos.y + screenOffset.y(), 1, 1 };
	mpos.x() /= camera->size.x;
	mpos.y() /= camera->size.y;
	mpos.x() = mpos.x() * 2 - 1;
	mpos.y() = 1 - mpos.y() * 2;

	mpos = projMatrix.inverse() * mpos;
	mpos /= mpos.w();
	mpos.w() = 0;
	mpos = viewMatrix.inverse() * mpos;

	Vector3f d = mpos.block(0, 0, 3, 1);
	d.normalize();

	Vector3f O = camera->getPosition(WORLD) - camera->getForward(WORLD) * camera->distance;

	Vector3f A = (transformMat * Vector4f(p.x() + halfLength, p.y(), p.z(), 1)).block(0, 0, 3, 1);
	Vector3f B = (transformMat * Vector4f(p.x() - halfLength, p.y(), p.z(), 1)).block(0, 0, 3, 1);

	Vector3f AB = B - A;
	Vector3f AO = O - A;

	float AB_dot_d = AB.dot(d);
	float AB_dot_AO = AB.dot(AO);
	float AB_dot_AB = AB.dot(AB);

	float m = AB_dot_d / AB_dot_AB;
	float n = AB_dot_AO / AB_dot_AB;

	Vector3f Q = d - (AB * m);
	Vector3f R = AO - (AB * n);

	float a = Q.dot(Q);
	float b = 2.0f * Q.dot(R);
	float c = R.dot(R) - (radius * radius);

	Vector3f OA = A - O;
	Vector3f OB = B - O;
	float disA = OA.dot(d);
	float disB = OB.dot(d);

	bool intersectSphereA = disA >= 0 && sqrtf(OA.squaredNorm() - disA * disA) <= radius;
	bool intersectSphereB = disB >= 0 && sqrtf(OB.squaredNorm() - disB * disB) <= radius;

	if (intersectSphereA || intersectSphereB)
		_picked = true;
	else if (a == 0.0f)
	{
		_picked = intersectSphereA || intersectSphereB;
	}
	else {
		float discriminant = b * b - 4.0f * a * c;
		if (discriminant < 0.0f)
		{
			// The ray doesn't hit the infinite cylinder defined by (A, B).
			// No intersection.
			_picked = false;
		}
		else {
			float tmin = (-b - sqrtf(discriminant)) / (2.0f * a);
			float tmax = (-b + sqrtf(discriminant)) / (2.0f * a);
			if (tmin > tmax)
			{
				float temp = tmin;
				tmin = tmax;
				tmax = temp;
			}

			// Now check to see if K1 and K2 are inside the line segment defined by A,B
			float t_k1 = tmin * m + n;
			float t_k2 = tmax * m + n;
			_picked = !((t_k1 < 0.0f && !intersectSphereA) ||
				(t_k1 > 1.0f && !intersectSphereB) ||
				(t_k2 < 0.0f && !intersectSphereA) ||
				(t_k2 > 1.0f && !intersectSphereB));
		}
	}

	if (_picked)
		picked = _picked;
	return _picked;
}

bool Gizmo::pickCapsuleY(const Vector3f& p, float radius, float halfLength, const Vector2f& screenOffset, const Matrix4f& transformMat)
{
	if (!ImGui::IsWindowFocused() ||
		!ImGui::IsMouseReleased(ImGuiMouseButton_Left) ||
		camera == NULL || picked) {
		return false;
	}

	bool _picked = false;

	Vector3f _pos, _sca;
	Quaternionf _rot;
	transformMat.decompose(_pos, _rot, _sca);

	radius *= _sca.norm();

	auto _mpos = ImGui::GetMousePos();
	Vector4f mpos = { _mpos.x - windowPos.x + screenOffset.x(), _mpos.y - windowPos.y + screenOffset.y(), 1, 1 };
	mpos.x() /= camera->size.x;
	mpos.y() /= camera->size.y;
	mpos.x() = mpos.x() * 2 - 1;
	mpos.y() = 1 - mpos.y() * 2;

	mpos = projMatrix.inverse() * mpos;
	mpos /= mpos.w();
	mpos.w() = 0;
	mpos = viewMatrix.inverse() * mpos;

	Vector3f d = mpos.block(0, 0, 3, 1);
	d.normalize();

	Vector3f O = camera->getPosition(WORLD) - camera->getForward(WORLD) * camera->distance;

	Vector3f A = (transformMat * Vector4f(p.x(), p.y() + halfLength, p.z(), 1)).block(0, 0, 3, 1);
	Vector3f B = (transformMat * Vector4f(p.x(), p.y() - halfLength, p.z(), 1)).block(0, 0, 3, 1);

	Vector3f AB = B - A;
	Vector3f AO = O - A;

	float AB_dot_d = AB.dot(d);
	float AB_dot_AO = AB.dot(AO);
	float AB_dot_AB = AB.dot(AB);

	float m = AB_dot_d / AB_dot_AB;
	float n = AB_dot_AO / AB_dot_AB;

	Vector3f Q = d - (AB * m);
	Vector3f R = AO - (AB * n);

	float a = Q.dot(Q);
	float b = 2.0f * Q.dot(R);
	float c = R.dot(R) - (radius * radius);

	Vector3f OA = A - O;
	Vector3f OB = B - O;
	float disA = OA.dot(d);
	float disB = OB.dot(d);

	bool intersectSphereA = disA >= 0 && sqrtf(OA.squaredNorm() - disA * disA) <= radius;
	bool intersectSphereB = disB >= 0 && sqrtf(OB.squaredNorm() - disB * disB) <= radius;

	if (intersectSphereA || intersectSphereB)
		_picked = true;
	else if (a == 0.0f)
	{
		_picked = intersectSphereA || intersectSphereB;
	}
	else {
		float discriminant = b * b - 4.0f * a * c;
		if (discriminant < 0.0f)
		{
			// The ray doesn't hit the infinite cylinder defined by (A, B).
			// No intersection.
			_picked = false;
		}
		else {
			float tmin = (-b - sqrtf(discriminant)) / (2.0f * a);
			float tmax = (-b + sqrtf(discriminant)) / (2.0f * a);
			if (tmin > tmax)
			{
				float temp = tmin;
				tmin = tmax;
				tmax = temp;
			}

			// Now check to see if K1 and K2 are inside the line segment defined by A,B
			float t_k1 = tmin * m + n;
			float t_k2 = tmax * m + n;
			_picked = !((t_k1 < 0.0f && !intersectSphereA) ||
				(t_k1 > 1.0f && !intersectSphereB) ||
				(t_k2 < 0.0f && !intersectSphereA) ||
				(t_k2 > 1.0f && !intersectSphereB));
		}
	}

	if (_picked)
		picked = _picked;
	return _picked;
}

bool Gizmo::pickCapsuleZ(const Vector3f& p, float radius, float halfLength, const Vector2f& screenOffset, const Matrix4f& transformMat)
{
	if (!ImGui::IsWindowFocused() ||
		!ImGui::IsMouseReleased(ImGuiMouseButton_Left) ||
		camera == NULL || picked) {
		return false;
	}

	bool _picked = false;

	Vector3f _pos, _sca;
	Quaternionf _rot;
	transformMat.decompose(_pos, _rot, _sca);

	radius *= _sca.norm();

	auto _mpos = ImGui::GetMousePos();
	Vector4f mpos = { _mpos.x - windowPos.x + screenOffset.x(), _mpos.y - windowPos.y + screenOffset.y(), 1, 1 };
	mpos.x() /= camera->size.x;
	mpos.y() /= camera->size.y;
	mpos.x() = mpos.x() * 2 - 1;
	mpos.y() = 1 - mpos.y() * 2;

	mpos = projMatrix.inverse() * mpos;
	mpos /= mpos.w();
	mpos.w() = 0;
	mpos = viewMatrix.inverse() * mpos;

	Vector3f d = mpos.block(0, 0, 3, 1);
	d.normalize();

	Vector3f O = camera->getPosition(WORLD) - camera->getForward(WORLD) * camera->distance;

	Vector3f A = (transformMat * Vector4f(p.x(), p.y(), p.z() + halfLength, 1)).block(0, 0, 3, 1);
	Vector3f B = (transformMat * Vector4f(p.x(), p.y(), p.z() - halfLength, 1)).block(0, 0, 3, 1);

	Vector3f AB = B - A;
	Vector3f AO = O - A;

	float AB_dot_d = AB.dot(d);
	float AB_dot_AO = AB.dot(AO);
	float AB_dot_AB = AB.dot(AB);

	float m = AB_dot_d / AB_dot_AB;
	float n = AB_dot_AO / AB_dot_AB;

	Vector3f Q = d - (AB * m);
	Vector3f R = AO - (AB * n);

	float a = Q.dot(Q);
	float b = 2.0f * Q.dot(R);
	float c = R.dot(R) - (radius * radius);

	Vector3f OA = A - O;
	Vector3f OB = B - O;
	float disA = OA.dot(d);
	float disB = OB.dot(d);

	bool intersectSphereA = disA >= 0 && sqrtf(OA.squaredNorm() - disA * disA) <= radius;
	bool intersectSphereB = disB >= 0 && sqrtf(OB.squaredNorm() - disB * disB) <= radius;

	if (intersectSphereA || intersectSphereB)
		_picked = true;
	else if (a == 0.0f)
	{
		_picked = intersectSphereA || intersectSphereB;
	}
	else {
		float discriminant = b * b - 4.0f * a * c;
		if (discriminant < 0.0f)
		{
			// The ray doesn't hit the infinite cylinder defined by (A, B).
			// No intersection.
			_picked = false;
		}
		else {
			float tmin = (-b - sqrtf(discriminant)) / (2.0f * a);
			float tmax = (-b + sqrtf(discriminant)) / (2.0f * a);
			if (tmin > tmax)
			{
				float temp = tmin;
				tmin = tmax;
				tmax = temp;
			}

			// Now check to see if K1 and K2 are inside the line segment defined by A,B
			float t_k1 = tmin * m + n;
			float t_k2 = tmax * m + n;
			_picked = !((t_k1 < 0.0f && !intersectSphereA) ||
				(t_k1 > 1.0f && !intersectSphereB) ||
				(t_k2 < 0.0f && !intersectSphereA) ||
				(t_k2 > 1.0f && !intersectSphereB));
		}
	}

	if (_picked)
		picked = _picked;
	return _picked;
}

bool Gizmo::pickIcon(const Vector3f& position, const Vector2f& screenOffset, const Vector2f& iconSize, float size)
{
	if (!ImGui::IsWindowFocused() ||
		!ImGui::IsMouseReleased(ImGuiMouseButton_Left) ||
		camera == NULL || picked) {
		return false;
	}

	auto _mpos = ImGui::GetMousePos();
	Vector4f mpos = { _mpos.x - windowPos.x + screenOffset.x(), _mpos.y - windowPos.y + screenOffset.y(), 1, 1 };

	Vector3f camPos = camera->getPosition(WORLD);
	Matrix4f pvm = camera->getProjectionMatrix() * camera->getViewMatrix();
	
	float dis = (position - camPos).norm();

	if (dis > cullDistance)
		return false;

	Vector4f p = pvm * Vector4f(position.x(), position.y(), position.z(), 1);

	if ((p.z() < camera->zNear || p.z() > camera->zFar))
		return false;

	p /= p.w();
	p.x() = (p.x() + 1.0f) * 0.5f * camera->size.x;
	p.y() = (1.0f - p.y()) * 0.5f * camera->size.y;

	float w = iconSize.x();
	float h = iconSize.y();

	float sizeX;
	float sizeY;

	if (w > h) {
		sizeX = size * 0.5f;
		sizeY = sizeX * h / w;
	}
	else {
		sizeY = size * 0.5f;
		sizeX = sizeY * w / h;
	}

	if (ImGui::IsMouseHoveringRect(
		{ p.x() + windowPos.x - screenOffset.x() - sizeX,
		p.y() + windowPos.y - screenOffset.y() - sizeY },
		{ p.x() + windowPos.x - screenOffset.x() + sizeX,
		p.y() + windowPos.y - screenOffset.y() + sizeY }))
		picked = true;

	return picked;
}

void Gizmo::drawText(const string& text, const Vector3f& position, const Color& color)
{
	texts.emplace_back(TextDraw { text, position, color });
}

void Gizmo::drawIcon(Texture2D& icon, const Vector3f& position, int size, const Color& color)
{
	icons.emplace_back(IconDraw { &icon, size, position, color });
}

void Gizmo::controlFreeCamera(float transitionSensitivity, float rotationSensitivity)
{
	Input& input = Engine::getInput();
	if (!isFocused || !ImGui::IsMouseDown(ImGuiMouseButton_Right))
		return;
	double deltaTime = Time::delta().toSecond();
	Vector3f v(0, 0, 0);
	v.x() = (int)ImGui::IsKeyDown(ImGuiKey_W) - (int)ImGui::IsKeyDown(ImGuiKey_S);
	v.x() += input.getMouseWheelValue();
	v.y() = (int)ImGui::IsKeyDown(ImGuiKey_A) - (int)ImGui::IsKeyDown(ImGuiKey_D);
	v.z() = (int)ImGui::IsKeyDown(ImGuiKey_E) - (int)ImGui::IsKeyDown(ImGuiKey_Q);
	v *= deltaTime * transitionSensitivity;
	cameraDeltaTransition += v;

	ImVec2 m = ImGui::GetIO().MouseDelta;
	cameraDeltaRollPitchYaw.z() += m.x * -rotationSensitivity;
	cameraDeltaRollPitchYaw.y() += m.y * rotationSensitivity;

	isUsing = true;
}

void Gizmo::controlTurnCamera(float transitionSensitivity, float distanceSensitivity, float rotationSensitivity)
{
	if (!isFocused)
		return;

	double deltaTime = Time::delta().toSecond();
	float up = (int)ImGui::IsKeyDown(ImGuiKey_E) - (int)ImGui::IsKeyDown(ImGuiKey_Q);
	up *= deltaTime * transitionSensitivity;
	cameraDeltaTransition.z() += up;

	float deep = ImGui::GetIO().MouseWheel;
	deep *= distanceSensitivity;
	cameraDeltaDistance += deep;

	if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
		ImVec2 m = ImGui::GetIO().MouseDelta;
		cameraDeltaRollPitchYaw.z() += m.x * -rotationSensitivity;
		cameraDeltaRollPitchYaw.y() += m.y * rotationSensitivity;
		isUsing = true;
	}

	if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
		ImVec2 m = ImGui::GetIO().MouseDelta;
		cameraDeltaDistance += m.y * distanceSensitivity;
		isUsing = true;
	}
}

bool Gizmo::drawHandle(void* id, HandleType type, TransformSpace space, Matrix4f& matrix, const Vector3f& snapVector)
{
	ImGuizmo::OPERATION operation = ImGuizmo::OPERATION::TRANSLATE;
	switch (type)
	{
	case HandleType::Transition:
		operation = ImGuizmo::OPERATION::TRANSLATE;
		break;
	case HandleType::Rotation:
		operation = ImGuizmo::OPERATION::ROTATE;
		break;
	case HandleType::Scaling:
		operation = ImGuizmo::OPERATION::SCALE;
		break;
	default:
		break;
	}

	ImGuizmo::MODE mode = ImGuizmo::MODE::WORLD;
	switch (space)
	{
	case TransformSpace::WORLD:
		mode = ImGuizmo::MODE::WORLD;
		break;
	case TransformSpace::LOCAL:
		mode = ImGuizmo::MODE::LOCAL;
		break;
	default:
		break;
	}
	ImGuizmo::SetID(id);
	bool operated = ImGuizmo::Manipulate(viewMatrix.data(), projMatrix.data(),
		operation, mode, matrix.data(), NULL, snapVector == Vector3f::Zero() ? NULL : snapVector.data());
	isUsing |= ImGuizmo::IsUsing();
	return operated;
}

bool Gizmo::drawMesh(MeshPart& meshPart, Material& material, int instanceBase, int instanceCount)
{
	if (instanceBase < 0)
		return false;
	meshes.push_back({ &meshPart, &material, Matrix4f::Identity(), instanceBase, instanceCount });
	return true;
}

bool Gizmo::drawMesh(MeshPart& meshPart, Material& material, const Matrix4f& transformMat)
{
	meshes.push_back({ &meshPart, &material, transformMat, -1, 1 });
	return true;
}

void Gizmo::doScreenHit(MeshPart& meshPart, const InstancedTransformRenderDataHandle& handle, list<IRenderData*> bindings)
{
	screenHits.push_back({ &meshPart, handle.batchDrawData, (int)handle.instanceID, (int)handle.instanceCount, bindings });
}

void Gizmo::setCameraControl(CameraControlMode mode, float transitionSensitivity, float rotationSensitivity, float distanceSensitivity)
{
	cameraControlMode = mode;
	this->transitionSensitivity = transitionSensitivity;
	this->rotationSensitivity = rotationSensitivity;
	this->distanceSensitivity = distanceSensitivity;
}

void Gizmo::reset()
{
	isUsing = false;
	picked = false;
	camera = NULL;
}

void Gizmo::beginFrame(ImDrawList* drawList)
{
	if (camera == NULL)
		return;
	this->drawList = drawList;

	windowPos = ImGui::GetWindowPos();

	ImGuizmo::SetOrthographic(camera->mode == camera->Orthotropic);
	ImGuizmo::SetRect(windowPos.x, windowPos.y, camera->size.x, camera->size.y);

	ImGuizmo::SetDrawlist(drawList);

	projMatrix = camera->getProjectionMatrix();
	viewMatrix = camera->getViewMatrix();

	ScreenHitInfo hitInfo;
	if (camera->cameraRender.fetchScreenHit(hitInfo)) {
		picked = true;
		Object* object = (Object*)Brane::getPtrByInsID(hitInfo.hitObjectIDHigh << 16 | hitInfo.hitObjectIDLow);
		EditorManager::selectObject(object);
	}

	if (ImGui::IsWindowHovered()) {
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) || ImGui::IsMouseClicked(ImGuiMouseButton_Middle)) {
			ImGui::SetWindowFocus();
		}
	}

	isFocused = ImGui::IsWindowFocused();
}

void Gizmo::endFrame()
{
	if (camera == NULL)
		return;
	if (ImGui::IsWindowFocused() && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
		if ((!ImGuizmo::IsOver() && !picked && !isUsing && !isLastUsing)) {
			EditorManager::selectObject(NULL);
			ImVec2 mousePos = ImGui::GetMousePos();
			ImVec2 winPos = ImGui::GetWindowPos();
			ImVec2 pos = { mousePos.x - winPos.x, mousePos.y - winPos.y };
			if (pos.x >= 0 && pos.y >= 0)
				camera->cameraRender.triggerScreenHit({ (unsigned int)pos.x, (unsigned int)pos.y });
		}
	}
	isLastUsing = isUsing;
}

void Gizmo::begineWindow()
{
	if (camera == NULL)
		return;
	const ImU32 flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoMove;

#ifdef IMGUI_HAS_VIEWPORT
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowViewport(viewport->ID);
#else
	ImGuiIO& io = ImGui::GetIO();
	ImGui::SetWindowSize(io.DisplaySize);
	ImGui::SetWindowPos(ImVec2(0, 0));
#endif

	ImGui::PushStyleColor(ImGuiCol_WindowBg, 0);
	ImGui::PushStyleColor(ImGuiCol_Border, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);

	ImGui::Begin("gizmo", NULL, flags);

	beginFrame(ImGui::GetBackgroundDrawList());
}

void Gizmo::endWindow()
{
	if (camera == NULL)
		return;
	endFrame();
	ImGui::End();
	ImGui::PopStyleVar();
	ImGui::PopStyleColor(2);
}

void Gizmo::onGUI(Object* root)
{
	if (!enableGUI)
		return;
	GizmoInfo info = { this, camera };
	ObjectIterator iter(root);
	do{
		Editor* editor = EditorManager::getEditor(iter.current());
		if (editor != NULL)
			editor->onGizmo(info);
	} while (iter.next());
}

void Gizmo::onUpdate(Camera& camera)
{
	this->camera = &camera;

	GizmoCameraInfo cameraInfo = CameraEditor::getGizmoCameraInfo(&camera);
	if (cameraInfo.canControlledByEditor) {
		switch (cameraControlMode)
		{
		case CameraControlMode::None:
			break;
		case CameraControlMode::Free:
		{
			controlFreeCamera(transitionSensitivity * cameraInfo.speedInEditor, rotationSensitivity);
		}
		break;
		case CameraControlMode::Turn:
		{
			controlTurnCamera(transitionSensitivity * cameraInfo.speedInEditor, distanceSensitivity, rotationSensitivity);
		}
		break;
		default:
			break;
		}
	}

	if (cameraDeltaTransition != Vector3f::Zero()) {
		camera.translate(cameraDeltaTransition.x(), cameraDeltaTransition.y(), 0);
		camera.translate(0, 0, cameraDeltaTransition.z(), WORLD);
		cameraDeltaTransition = Vector3f::Zero();
	}

	if (cameraDeltaRollPitchYaw != Vector3f::Zero()) {
		camera.rotate(cameraDeltaRollPitchYaw.x(), cameraDeltaRollPitchYaw.y(), 0);
		camera.rotate(0, 0, cameraDeltaRollPitchYaw.z(), WORLD);
		cameraDeltaRollPitchYaw = Vector3f::Zero();
	}

	if (cameraDeltaDistance != 0) {
		camera.distance += cameraDeltaDistance;
		camera.distance = std::max(0.0f, camera.distance);
		cameraDeltaDistance = 0;
	}
}

bool in01(float x)
{
	return x <= 1 && x >= 0;
}

bool in01(const Vector2f& v)
{
	return in01(v.x()) && in01(v.y());
}

void clip(Vector3f& p0, const Vector3f p1, const Vector3f& camDir, const Vector3f& camPos, float zNear)
{
	Vector3f p = camPos + camDir * zNear;
	float w = camDir.dot(p);
	Vector3f a = p1 - p0;
	float t = (w - p0.dot(camDir)) / a.dot(camDir);
	p0 += a * t;
}

void Gizmo::onRender2D(ImDrawList* drawList)
{
	if (camera == NULL)
		return;
	Camera& camera = *this->camera;
	//ImGui::SetWindowSize(ImVec2(camera.size.x, camera.size.y));
	//ImGui::SetWindowPos(windowPos);

	ImDrawList* list = drawList ? drawList : this->drawList;
	list->PushClipRect(windowPos, { windowPos.x + camera.size.x, windowPos.y + camera.size.y });
	Matrix4f pvm = camera.getProjectionMatrix() * camera.getViewMatrix();
	Vector3f camDir = camera.getForward(WORLD);
	Vector3f camPos = camera.getPosition(WORLD);

	for (PxU32 i = 0; i < points.size(); i++)
	{
		const PointDraw& point = points[i];
		Vector3f p = point.p;

		float dis = (p - camPos).norm();

		if (dis > cullDistance)
			continue;

		Vector4f _p = pvm * Vector4f(p.x(), p.y(), p.z(), 1);

		if ((_p.z() < camera.zNear || _p.z() > camera.zFar))
			continue;

		_p /= _p.w();

		Vector2f __p((_p.x() + 1.0f) * 0.5f, (1.0f - _p.y()) * 0.5f);

		list->AddCircleFilled({ windowPos.x + __p.x() * camera.size.x, windowPos.y + __p.y() * camera.size.y },
			point.size, ImColor(point.color.r, point.color.g, point.color.b, point.color.a));
	}

	// for (PxU32 i = 0; i < lines.size(); i++)
	// {
	// 	const LineDraw& line = lines[i];
	// 	Vector3f p0 = line.p0;
	// 	Vector3f p1 = line.p1;
	//
	// 	float dis0 = (p0 - camPos).norm();
	// 	float dis1 = (p1 - camPos).norm();
	//
	// 	if (dis0 > cullDistance && dis1 > cullDistance)
	// 		continue;
	//
	// 	Vector4f _p0 = pvm * Vector4f(p0.x(), p0.y(), p0.z(), 1);
	// 	Vector4f _p1 = pvm * Vector4f(p1.x(), p1.y(), p1.z(), 1);
	//
	// 	if ((_p0.z() < camera.zNear || _p0.z() > camera.zFar) &&
	// 		(_p1.z() < camera.zNear || _p1.z() > camera.zFar))
	// 		continue;
	//
	// 	if (_p0.z() < camera.zNear) {
	// 		clip(p0, p1, camDir, camPos, camera.zNear);
	// 		_p0 = pvm * Vector4f(p0.x(), p0.y(), p0.z(), 1);
	// 	}
	//
	// 	if (_p1.z() < camera.zNear) {
	// 		clip(p1, p0, camDir, camPos, camera.zNear);
	// 		_p1 = pvm * Vector4f(p1.x(), p1.y(), p1.z(), 1);
	// 	}
	//
	// 	//if (_p0.w() > 0) {
	// 	//	_p0 /= _p0.w();
	// 	//}
	// 	//else {
	// 	//	_p0 *= -_p0.w();// *tan(info.camera->fov * 0.5f);
	// 	//}
	//
	// 	//if (_p1.w() > 0) {
	// 	//	_p1 /= _p1.w();
	// 	//}
	// 	//else {
	// 	//	_p1 *= -_p1.w();// .tan(info.camera->fov * 0.5f);
	// 	//}
	//
	// 	_p0 /= _p0.w();
	// 	_p1 /= _p1.w();
	//
	// 	Vector2f __p0((_p0.x() + 1.0f) * 0.5f, (1.0f - _p0.y()) * 0.5f);
	// 	Vector2f __p1((_p1.x() + 1.0f) * 0.5f, (1.0f - _p1.y()) * 0.5f);
	//
	// 	//CohenSutherlandLineClipAndDraw(__p0, __p1);
	//
	// 	/*if (in01(__p0) && in01(__p1))*/
	//
	// 	list->AddLine({ windowPos.x + __p0.x() * camera.size.x, windowPos.y + __p0.y() * camera.size.y },
	// 		{ windowPos.x + __p1.x() * camera.size.x, windowPos.y + __p1.y() * camera.size.y },
	// 		ImColor(line.c0.r, line.c0.g, line.c0.b, line.c0.a));
	// }

	for (int i = 0; i < texts.size(); i++) {
		TextDraw& text = texts[i];

		float dis = (text.position - camPos).norm();

		if (dis > cullDistance)
			continue;

		Vector4f p = pvm * Vector4f(text.position.x(), text.position.y(), text.position.z(), 1);

		if ((p.z() < camera.zNear || p.z() > camera.zFar))
			continue;

		p /= p.w();
		p.x() = (p.x() + 1.0f) * 0.5f * camera.size.x;
		p.y() = (1.0f - p.y()) * 0.5f * camera.size.y;

		ImVec2 size = ImGui::CalcTextSize(text.text.c_str());
		size.x *= 0.5;
		size.y *= 0.5;
		
		list->AddText({ p.x() + windowPos.x - size.x, p.y() + windowPos.y - size.y },
			ImColor(text.color.r, text.color.g, text.color.b, text.color.a), text.text.c_str());
	}

	for (int i = 0; i < icons.size(); i++) {
		IconDraw& icon = icons[i];

		float dis = (icon.position - camPos).norm();

		if (dis > cullDistance)
			continue;

		Vector4f p = pvm * Vector4f(icon.position.x(), icon.position.y(), icon.position.z(), 1);

		if ((p.z() < camera.zNear || p.z() > camera.zFar))
			continue;

		p /= p.w();
		p.x() = (p.x() + 1.0f) * 0.5f * camera.size.x;
		p.y() = (1.0f - p.y()) * 0.5f * camera.size.y;

		float w = icon.icon->getWidth();
		float h = icon.icon->getHeight();

		float sizeX;
		float sizeY;

		if (w > h) {
			sizeX = icon.size * 0.5f;
			sizeY = sizeX * h / w;
		}
		else {
			sizeY = icon.size * 0.5f;
			sizeX = sizeY * w / h;
		}
		if (icon.icon->bind() <= 0)
			continue;
		list->AddImage(icon.icon->getTextureID(),
			{ p.x() + windowPos.x - sizeX, p.y() + windowPos.y - sizeY },
			{ p.x() + windowPos.x + sizeX, p.y() + windowPos.y + sizeY },
			{ 0, 0 }, { 1, 1 },
			ImColor(icon.color.r, icon.color.g, icon.color.b, icon.color.a));
	}
	list->PopClipRect();
	points.clear();
	texts.clear();
	icons.clear();
}

void Gizmo::onRender3D(RenderInfo& info)
{
	RENDER_THREAD_ENQUEUE_TASK(ReflectionCaptureProbeUpdate, ([
		lines = std::move(lines), meshes = std::move(meshes), screenHits = std::move(screenHits)] (RenderThreadContext& context)
	{
		context.sceneRenderData->debugRenderData.updateLineData = std::move(lines);
		MeshBatchDrawData batchDrawData = context.sceneRenderData->getBatchDrawData(false);
	
		for (auto draw : meshes) {
			if (draw.instanceID < 0) {
				MeshTransformData data;
				data.localToWorld = draw.transformMat;
				Vector3f pos, sca;
				Quaternionf rot;
				draw.transformMat.decompose(pos, rot, sca);
				data.worldScale = sca;
				const BoundBox& bound = draw.meshPart->bound;
				data.localCenter = bound.getCenter();
				data.localExtent = bound.getExtent();
				data.localRadius = data.localExtent.norm();
				data.flag = 0;
				draw.instanceID = context.sceneRenderData->setMeshTransform(data);
				draw.instanceCount = 1;
			}
			MeshRenderCommand cmd;
			cmd.mesh = draw.meshPart;
			cmd.materialRenderData = draw.material->getMaterialRenderData();
			cmd.sceneData = context.sceneRenderData;
			cmd.instanceID = draw.instanceID;
			cmd.instanceIDCount = draw.instanceCount;
			cmd.hasShadow = false;
			cmd.batchDrawData = batchDrawData;
			MeshBatchDrawKey renderKey(draw.meshPart, cmd.materialRenderData, batchDrawData.transformData->getMeshTransform(draw.instanceID).isNegativeScale());
			cmd.meshBatchDrawCall = batchDrawData.batchDrawCommandArray->setMeshBatchDrawCall(renderKey, draw.instanceID);
			cmd.reverseCullMode = renderKey.negativeScale;

			collectRenderDataInCommand(context.renderGraph, cmd);
			context.renderGraph->setRenderCommand(cmd);
		}

		for (auto& hit : screenHits) {
			if (hit.instanceID < 0 || hit.instanceCount <= 0) {
				continue;
			}
			ScreenHitRenderCommand cmd;
			cmd.mesh = hit.meshPart;
			cmd.sceneData = context.sceneRenderData;
			cmd.instanceID = hit.instanceID;
			cmd.instanceIDCount = hit.instanceCount;
			cmd.batchDrawData = hit.batchDrawData.isValid() ? hit.batchDrawData : batchDrawData;
			cmd.reverseCullMode = batchDrawData.transformData->getMeshTransform(hit.instanceID).isNegativeScale();
			cmd.bindings = hit.bindings;

			collectRenderDataInCommand(context.renderGraph, cmd);
			context.renderGraph->setRenderCommand(cmd);
		}
	}));
}
