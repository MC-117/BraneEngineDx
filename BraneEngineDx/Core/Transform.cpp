#include "Transform.h"
#include "Utility.h"
#if ENABLE_PHYSICS
#include "RigidBody.h"
#include "PhysicalWorld.h"
#endif
#include "Asset.h"

Transform::TransformSerialization(Transform::TransformSerialization::serialization);

Serialization& Transform::getSerialization() const
{
	return TransformSerialization::serialization;
}

Transform::Transform(string name) : Object::Object(name)
{
}

Transform::~Transform()
{
#if ENABLE_PHYSICS
	if (rigidBody != NULL) {
		rigidBody->removeFromWorld();
		delete rigidBody;
	}
#endif
}

void Transform::begin()
{
	Object::begin();
	//updataRigidBody();
}

void Transform::tick(float deltaTime)
{
	Object::tick(deltaTime);
}

void Transform::afterTick()
{
	Object::afterTick();
#if ENABLE_PHYSICS
	if (rigidBody != NULL) {
		rigidBody->updateObjectTransform();
	}
#endif
	updateTransform();
}

#if ENABLE_PHYSICS
void Transform::updataRigidBody(Shape* shape, ShapeComplexType complexType, const PhysicalMaterial& physicalMaterial)
{
	Shape* _shape = shape;
	if (rigidBody != NULL) {
		rigidBody->removeFromWorld();
		if (_shape == NULL) {
			_shape = rigidBody->shape;
			complexType = rigidBody->shapeComplexType;
		}
		delete rigidBody;
	}
	if (_shape == NULL)
		return;
	rigidBody = new RigidBody(*this, physicalMaterial, _shape, complexType);
}

void * Transform::getPhysicalBody()
{
	return rigidBody;
}

void Transform::setupPhysics(PhysicalWorld & physicalWorld)
{
	if (rigidBody != NULL && rigidBody->physicalWorld == NULL) {
		physicalWorld.addPhysicalBody(*rigidBody);
		//physicalWorld.physicsScene->updateSingleAabb(rigidBody);
		//rigidBody->activate(true);
	}
}

void Transform::releasePhysics(PhysicalWorld & physicalWorld)
{
	if (rigidBody != NULL) {
		rigidBody->removeFromWorld();
	}
}

void Transform::collisionHappened(const ContactInfo & info)
{
	//cout << path.c_str() << " Collsion with " << info.otherObject->path.c_str() << endl;
}
#endif

Matrix4f Transform::getMatrix(TransformSpace space)
{
	if (space == WORLD) {
		updateTransform();
		return transformMat;
	}
	else {
		Matrix4f T = Matrix4f::Identity();
		T(0, 3) = position.x();
		T(1, 3) = position.y();
		T(2, 3) = position.z();
		Matrix4f S = Matrix4f::Identity();
		S(0, 0) = scale.x();
		S(1, 1) = scale.y();
		S(2, 2) = scale.z();
		Matrix4f R = Matrix4f::Identity();
		R.block(0, 0, 3, 3) = rotation.toRotationMatrix();
		return T * R * S;
	}
}

Vector3f Transform::getPosition(TransformSpace space)
{
	if (space == WORLD) {
		updateTransform();
		return transformMat.block(0, 3, 3, 1);
	}
	else
		return position;
}

Quaternionf Transform::getRotation(TransformSpace space)
{
	if (space == WORLD) {
		Quaternionf rot = rotation;
		Object* obj = this->parent;
		while (obj != NULL) {
			Transform* t = dynamic_cast<Transform*>(obj);
			if (t != NULL) {
				rot = rot * t->rotation;
			}
			obj = obj->parent;
		}
		return rot;
	}
	else
		return rotation;
}

Vector3f Transform::getEulerAngle(TransformSpace space)
{
	return getRotation(space).toRotationMatrix().eulerAngles() / PI * 180;
}

Vector3f(Transform::getScale)(TransformSpace space)
{
	if (space == WORLD) {
		Vector3f sca = scale;
		Object* obj = this->parent;
		while (obj != NULL) {
			Transform* t = dynamic_cast<Transform*>(obj);
			if (t != NULL) {
				sca = sca.cwiseProduct(t->scale);
			}
			obj = obj->parent;
		}
		return sca;
	}
	else
		return scale;
}

Vector3f Transform::getForward(TransformSpace space)
{
	if (space == WORLD) {
		updateTransform();
		return (Matrix3f(transformMat.block(0, 0, 3, 3)) * Vector3f(1, 0, 0)).normalized();
	}
	else
		return forward;
}

Vector3f Transform::getRightward(TransformSpace space)
{
	if (space == WORLD) {
		updateTransform();
		return (Matrix3f(transformMat.block(0, 0, 3, 3)) * Vector3f(0, 1, 0)).normalized();
	}
	else
		return rightward;
}

Vector3f Transform::getUpward(TransformSpace space)
{
	if (space == WORLD) {
		updateTransform();
		return (Matrix3f(transformMat.block(0, 0, 3, 3)) * Vector3f(0, 0, 1)).normalized();
	}
	else
		return upward;
}

void Transform::setMatrix(const Matrix4f & mat, TransformSpace space)
{
	if (space == WORLD) {
		Object *obj = this;parent;
		Transform* t = NULL;
		while (obj->parent != NULL) {
			obj = obj->parent;
			t = dynamic_cast<Transform*>(obj);
			if (t != NULL)
				break;
		}
		if (t == NULL)
			return;
		t->updateTransform();
		Matrix4f dm = t->transformMat.inverse() * mat;
		dm.decompose(position, rotation, scale);
		forward = (rotation * Vector3f::UnitX()).normalized();
		rightward = (rotation * Vector3f::UnitY()).normalized();
		upward = (rotation * Vector3f::UnitZ()).normalized();
		transformMat = mat;
		invalidate(Transform::UpdateState(Pos | Rot | Sca));
	}
	else {
		mat.decompose(position, rotation, scale);
		forward = (rotation * Vector3f::UnitX()).normalized();
		rightward = (rotation * Vector3f::UnitY()).normalized();
		upward = (rotation * Vector3f::UnitZ()).normalized();
		invalidate(Transform::UpdateState(Pos | Rot | Sca));
	}
}

void Transform::setPosition(float x, float y, float z, TransformSpace space)
{
	setPosition(Vector3f(x, y, z));
}

void Transform::setRotation(float x, float y, float z, TransformSpace space)
{
	Quaternionf R = Quaternionf::FromEularAngles(Vector3f(x / 180.0 * PI, y / 180.0 * PI, z / 180.0 * PI));
	if (space == WORLD) {
		rotation.setIdentity();
		rotation = R * getRotation(WORLD).inverse();
	}
	else {
		rotation = R;
	}
	rotation.normalize();
	forward = rotation * Vector3f::UnitX();
	rightward = rotation * Vector3f::UnitY();
	upward = rotation * Vector3f::UnitZ();
	invalidate(Rot);
}

void Transform::translate(float x, float y, float z, ::TransformSpace space)
{
	if (space == WORLD) {
		position.x() += x;
		position.y() += y;
		position.z() += z;
	}
	else if (space == LOCAL) {
		position += forward * x + rightward * y + upward * z;
	}
	else {
		Object* obj = this;
		Transform* t = NULL;
		while (obj != NULL) {
			t = dynamic_cast<Transform*>(obj);
			if (t != NULL) {
				break;
			}
			obj = obj->parent;
		}
		if (t == NULL) {
			position += Vector3f(x, y, z);
		}
		else {
			position += t->forward * x + t->rightward * y + t->upward * z;
		}
	}
	invalidate(Pos);
}

void Transform::rotate(float x, float y, float z, ::TransformSpace space)
{
	Quaternionf R;
	if (space == WORLD) {
		R = Quaternionf::FromAngleAxis(x / 180.0 * PI, Vector3f::UnitX()) *
			Quaternionf::FromAngleAxis(y / 180.0 * PI, Vector3f::UnitY()) *
			Quaternionf::FromAngleAxis(z / 180.0 * PI, Vector3f::UnitZ());
	}
	else if (space == LOCAL) {
		R = Quaternionf::FromAngleAxis(x / 180.0 * PI, forward) *
			Quaternionf::FromAngleAxis(y / 180.0 * PI, rightward) *
			Quaternionf::FromAngleAxis(z / 180.0 * PI, upward);
	}
	else {
		Object* obj = this;
		Transform* t = NULL;
		while (obj != NULL) {
			t = dynamic_cast<Transform*>(obj);
			if (t != NULL) {
				break;
			}
			obj = obj->parent;
		}
		if (t == NULL) {
			R = Quaternionf::FromAngleAxis(x / 180.0 * PI, Vector3f::UnitX()) *
				Quaternionf::FromAngleAxis(y / 180.0 * PI, Vector3f::UnitY()) *
				Quaternionf::FromAngleAxis(z / 180.0 * PI, Vector3f::UnitZ());
		}
		else {
			R = Quaternionf::FromAngleAxis(x / 180.0 * PI, t->forward) *
				Quaternionf::FromAngleAxis(y / 180.0 * PI, t->rightward) *
				Quaternionf::FromAngleAxis(z / 180.0 * PI, t->upward);
		}
	}
	rotation = R * rotation;
	rotation.normalize();
	forward = rotation * Vector3f::UnitX();
	rightward = rotation * Vector3f::UnitY();
	upward = rotation * Vector3f::UnitZ();
	invalidate(Rot);
}

void Transform::setPosition(const Vector3f& v, TransformSpace space)
{
	if (space == WORLD) {
		position += v - getPosition(WORLD);
		//transformMat.block(0, 3, 3, 1) = v;
	}
	else {
		position = v;
	}
	invalidate(Pos);
}

void Transform::setRotation(const Vector3f& v, TransformSpace space)
{
	Quaternionf R = Quaternionf::FromEularAngles(v / (180.0 * PI));
	if (space == WORLD) {
		rotation.setIdentity();
		rotation = R * getRotation(WORLD).inverse();
	}
	else {
		rotation = R;
	}
	rotation.normalize();
	forward = rotation * Vector3f::UnitX();
	rightward = rotation * Vector3f::UnitY();
	upward = rotation * Vector3f::UnitZ();
	invalidate(Rot);
}

void Transform::setRotation(const Quaternionf & q, TransformSpace space)
{
	if (space == WORLD) {
		rotation.setIdentity();
		rotation = q * getRotation(WORLD).inverse();
	}
	else {
		rotation = q;
	}
	rotation.normalize();
	forward = rotation * Vector3f::UnitX();
	rightward = rotation * Vector3f::UnitY();
	upward = rotation * Vector3f::UnitZ();
	invalidate(Rot);
}

void Transform::translate(const Vector3f& v, ::TransformSpace space)
{
	if (space == WORLD) {
		position += v;
	}
	else if (space == LOCAL) {
		position += forward * v.x() + rightward * v.y() + upward * v.z();
	}
	else {
		Object* obj = this;
		Transform* t = NULL;
		while (obj != NULL) {
			t = dynamic_cast<Transform*>(obj);
			if (t != NULL) {
				break;
			}
			obj = obj->parent;
		}
		if (t == NULL) {
			position += v;
		}
		else {
			position += t->forward * v.x() + t->rightward * v.y() + t->upward * v.z();
		}
	}
	invalidate(Pos);
}

void Transform::rotate(const Vector3f& v, ::TransformSpace space)
{
	Quaternionf R;
	if (space == WORLD) {
		R = Quaternionf::FromAngleAxis(v.x() / 180.0 * PI, Vector3f::UnitX()) *
			Quaternionf::FromAngleAxis(v.y() / 180.0 * PI, Vector3f::UnitY()) *
			Quaternionf::FromAngleAxis(v.z() / 180.0 * PI, Vector3f::UnitZ());
	}
	else if (space == LOCAL) {
		R = Quaternionf::FromAngleAxis(v.x() / 180.0 * PI, forward) *
			Quaternionf::FromAngleAxis(v.y() / 180.0 * PI, rightward) *
			Quaternionf::FromAngleAxis(v.z() / 180.0 * PI, upward);
	}
	else {
		Object* obj = this;
		Transform* t = NULL;
		while (obj != NULL) {
			t = dynamic_cast<Transform*>(obj);
			if (t != NULL) {
				break;
			}
			obj = obj->parent;
		}
		if (t == NULL) {
			R = Quaternionf::FromAngleAxis(v.x() / 180.0 * PI, Vector3f::UnitX()) *
				Quaternionf::FromAngleAxis(v.y() / 180.0 * PI, Vector3f::UnitY()) *
				Quaternionf::FromAngleAxis(v.z() / 180.0 * PI, Vector3f::UnitZ());
		}
		else {
			R = Quaternionf::FromAngleAxis(v.x() / 180.0 * PI, t->forward) *
				Quaternionf::FromAngleAxis(v.y() / 180.0 * PI, t->rightward) *
				Quaternionf::FromAngleAxis(v.z() / 180.0 * PI, t->upward);
		}
	}
	rotation = R * rotation;
	rotation.normalize();
	forward = rotation * Vector3f::UnitX();
	rightward = rotation * Vector3f::UnitY();
	upward = rotation * Vector3f::UnitZ();
	invalidate(Rot);
}

void Transform::rotate(const Quaternionf & v)
{
	rotation = v * rotation;
	rotation.normalize();
	forward = rotation * Vector3f::UnitX();
	rightward = rotation * Vector3f::UnitY();
	upward = rotation * Vector3f::UnitZ();
	invalidate(Rot);
}

void Transform::setScale(float x, float y, float z)
{
	if (scale.x() == x && scale.y() == y && scale.z() == z)
		return;
	scale.x() = x;
	scale.y() = y;
	scale.z() = z;
	/*if (rigidBody != NULL) {
		updataRigidBody();
	}*/
	invalidate(Sca);
}

void Transform::setScale(const Vector3f& v)
{
	if (scale == v)
		return;
	scale = v;
	/*if (rigidBody != NULL) {
		updataRigidBody();
	}*/
	invalidate(Sca);
}

void Transform::scaling(float x, float y, float z)
{
	if (x == 1 && y == 1 && z ==1)
		return;
	scale.x() *= x;
	scale.y() *= y;
	scale.z() *= z;
	/*if (rigidBody != NULL) {
		updataRigidBody();
	}*/
	invalidate(Sca);
}

void Transform::scaling(const Vector3f& v)
{
	if (v == Vector3f(1, 1, 1))
		return;
	scale.x() *= v.x();
	scale.y() *= v.y();
	scale.z() *= v.z();
	/*if (rigidBody != NULL) {
		updataRigidBody();
	}*/
	invalidate(Sca);
}

void Transform::attachTo(Transform & target, const Transform::AttachRuleEnum & rule)
{
	TransformSpace space = rule == KeepWorld ? WORLD : RELATE;
	Matrix4f mat = getMatrix(space);
	Object::setParent(target);
	setMatrix(mat, space);
}

void Transform::setParent(Object & parent)
{
	if (isClassOf<Transform>(&parent))
		attachTo(*(Transform*)&parent);
	else
		Object::setParent(parent);
}

Matrix4f & Transform::getTransformMat()
{
	return transformMat;
}

Serializable * Transform::instantiate(const SerializationInfo & from)
{
	Transform* t = new Transform(from.name);
	ChildrenInstantiate(Object, from, t);
	return t;
}

bool Transform::deserialize(const SerializationInfo & from)
{
	if (!Object::deserialize(from))
		return false;
	SVector3f pos, sca;
	if (from.get("position", pos))
		setPosition(pos.x, pos.y, pos.z);
	const SerializationInfo* rotInfo = from.get("rotation");
	if (rotInfo->type == "SQuaternionf") {
		SQuaternionf rot;
		if (from.get("rotation", rot))
			setRotation(rot);
	}
	else if (rotInfo->type == "SVector3f") {
		SVector3f rot;
		if (from.get("rotation", rot))
			setRotation(rot.x, rot.y, rot.z);
	}
	if (from.get("scale", sca))
		setScale(sca.x, sca.y, sca.z);
#if ENABLE_PHYSICS
	const SerializationInfo* cinfo = from.get("collision");
	if (cinfo != NULL) {
		const SerializationInfo* sinfo = cinfo->get("shape");
		if (sinfo != NULL) {
			Shape* shape = NULL;
			if (sinfo->type == "AssetSearch") {
				string pathType, path;
				if (sinfo->get("pathType", pathType))
					if (sinfo->get("path", path)) {
						if (pathType == "path") {
							shape = getAssetByPath<Mesh>(path);
						}
						else if (pathType == "name") {
							shape = getAsset<Mesh>("Mesh", path);
						}
					}
			}
			else if (sinfo->serialization != NULL) {
				Serializable* ser = sinfo->serialization->instantiate(*sinfo);
				shape = dynamic_cast<Shape*>(ser);
				if (shape == NULL && ser != NULL)
					delete ser;
				shape->deserialize(*sinfo);
			}
			float complexType = 0;
			cinfo->get("complexType", complexType);
			PhysicalMaterial pmat;
			const SerializationInfo* pminfo = cinfo->get("physicalMaterial");
			if (pminfo != NULL) {
				float mass = 0, type = 0;
				pminfo->get("mass", mass);
				pminfo->get("type", type);
				pmat.mass = mass;
				pmat.physicalType = (PhysicalType)(int)type;
			}
			if (shape != NULL)
				updataRigidBody(shape, (ShapeComplexType)(int)complexType, pmat);
		}
	}
#endif
	return true;
}

bool Transform::serialize(SerializationInfo & to)
{
	if (!Object::serialize(to))
		return false;
	to.type = "Transform";
	to.set("position", SVector3f(position));
	to.set("rotation", SQuaternionf(rotation));
	to.set("scale", SVector3f(scale));
#if ENABLE_PHYSICS
	if (rigidBody == NULL || rigidBody->shape == NULL)
		return true;
	SerializationInfo* cinfo = to.add("collision");
	if (cinfo != NULL) {
		cinfo->type = "Collision";
		Mesh* mesh = dynamic_cast<Mesh*>(rigidBody->shape);
		if (mesh == NULL) {
			cinfo->set("shape", *rigidBody->shape);
		}
		else {
			string path = MeshAssetInfo::getPath(mesh);
			if (!path.empty()) {
				SerializationInfo* sinfo = cinfo->add("shape");
				if (sinfo != NULL) {
					sinfo->type = "AssetSearch";
					sinfo->set("path", path);
					sinfo->set("pathType", "path");
				}
			}
		}
		cinfo->set("complexType", (float)(int)rigidBody->shapeComplexType);
		SerializationInfo* pminfo = cinfo->add("physicalMaterial");
		if (pminfo != NULL) {
			pminfo->type = "PhysicalMaterial";
			pminfo->set("mass", rigidBody->material.mass);
			pminfo->set("type", (float)(int)rigidBody->material.physicalType);
		}
	}
#endif
	return true;
}

void Transform::updateTransform()
{
	if (updateState == None)
		return;
	Matrix4f T = Matrix4f::Identity();
	T(0, 3) = position.x();
	T(1, 3) = position.y();
	T(2, 3) = position.z();
	Matrix4f S = Matrix4f::Identity();
	S(0, 0) = scale.x();
	S(1, 1) = scale.y();
	S(2, 2) = scale.z();
	Matrix4f R = Matrix4f::Identity();
	R.block(0, 0, 3, 3) = rotation.toRotationMatrix();
	transformMat = T * R * S;
	if (parent != NULL) {
		Transform* p = dynamic_cast<Transform*>(parent);
		if (p != NULL) {
			p->updateTransform();
			transformMat = p->transformMat * transformMat;
			/*if (updateState & Rot) {
			Matrix3f r = transformMat.block(0, 0, 3, 3);
			forward = r * Vector3f::UnitX();
			rightward = r * Vector3f::UnitY();
			upward = r * Vector3f::UnitZ();
			forward.normalize();
			rightward.normalize();
			upward.normalize();
			}*/
		}
		else
			cout << parent->name << " is not Transform\n";
	}
	updateState = None;
}

void Transform::invalidate(UpdateState state)
{
	updateState = UpdateState(updateState | state);
	for (auto b = children.begin(), e = children.end(); b != e; b++) {
		Transform* p = dynamic_cast<Transform*>(*b);
		if (p != NULL)
			p->invalidate(state);
	}
}
