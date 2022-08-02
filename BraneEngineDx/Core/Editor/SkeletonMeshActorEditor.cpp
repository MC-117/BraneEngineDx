#include "SkeletonMeshActorEditor.h"
#include "../GUI/GUIUtility.h"
#include "../../ThirdParty/ImGui/imgui_stdlib.h"
#include "../Skeleton/BoneConstraintEditor.h"
#include "../GUI/BlendSpaceWindow.h"

RegistEditor(SkeletonMeshActor);

void SkeletonMeshActorEditor::setInspectedObject(void* object)
{
	skeletonMeshActor = dynamic_cast<SkeletonMeshActor*>((Object*)object);
	ActorEditor::setInspectedObject(skeletonMeshActor);
}

void SkeletonMeshActorEditor::onSkeletonGUI(EditorInfo& info)
{
	if (skeletonMeshActor->getBoneHidden()) {
		if (ImGui::Button("Show All Bones", { -1, 36 }))
			skeletonMeshActor->setBoneHidden(false);
	}
	else if (ImGui::Button("Hide All Bones", { -1, 36 }))
		skeletonMeshActor->setBoneHidden(true);
	if (ImGui::Button("Set Reference Pose", { -1, 36 }))
		skeletonMeshActor->setReferencePose();
	editingBoneConstraint = ImGui::CollapsingHeader("BoneConstraint");
	if (editingBoneConstraint) {
		ImGui::Indent(10);
		vector<Serialization*> boneConstraintTypes;
		BoneConstraint::BoneConstraintSerialization::serialization.getChildren(boneConstraintTypes);
		string boneConstraintTypeName = selectedBoneConstraintType == NULL ? "(null)" : selectedBoneConstraintType->type;
		if (ImGui::BeginCombo("Type", boneConstraintTypeName.c_str())) {
			for (int i = 0; i < boneConstraintTypes.size(); i++) {
				Serialization* serialization = boneConstraintTypes[i];
				ImGui::PushID(i);
				if (ImGui::Selectable(serialization->type.c_str())) {
					selectedBoneConstraintType = serialization;
				}
				ImGui::PopID();
			}
			ImGui::EndCombo();
		}
		if (ImGui::Button(("Add " + boneConstraintTypeName).c_str(), { -1, 36 })) {
			if (selectedBoneConstraintType != NULL) {
				BoneConstraint* constraint = (BoneConstraint*)selectedBoneConstraintType->instantiate(SerializationInfo());
				if (constraint != NULL)
					skeletonMeshActor->skeleton.addBoneConstraint(constraint);
			}
		}
		for (int i = 0; i < skeletonMeshActor->skeleton.constraints.size(); i++) {
			BoneConstraint* constraint = skeletonMeshActor->skeleton.constraints[i];
			bool selected = constraint == selectedBoneConstraint;
			ImGui::PushID(i);
			if (ImGui::Checkbox("##SelectCB", &selected)) {
				selectedBoneConstraint = selected ? constraint : NULL;
			}
			ImGui::SameLine();
			if (ImGui::CollapsingHeader((to_string(i) + ". " + constraint->getSerialization().type).c_str())) {
				ImGui::Indent(10);
				BoneConstraintEditor* editor = dynamic_cast<BoneConstraintEditor*>(EditorManager::getEditor(*constraint));
				if (editor != NULL) {
					editor->setSkeletonMeshActor(skeletonMeshActor);
					editor->onGUI(info);
				}
				ImGui::Unindent(10);
			}
			ImGui::PopID();
		}
		ImGui::Unindent(10);
	}
	if (ImGui::CollapsingHeader("MorphTarget")) {
		for (int i = 0; i < skeletonMeshActor->morphTargetRemapper.getMorphCount(); i++) {
			float w = 0;
			skeletonMeshActor->morphTargetRemapper.getMorphWeight(i, w);
			if (ImGui::SliderFloat(skeletonMeshActor->morphTargetRemapper.morphNames[i].c_str(),
				&w, 0, 1))
				skeletonMeshActor->morphTargetRemapper.setMorphWeight(i, w);
		}
	}
}

void SkeletonMeshActorEditor::onAnimationGUI(EditorInfo& info)
{
	Asset* assignAsset = EditorManager::getSelectedAsset();
	if (assignAsset != NULL && assignAsset->assetInfo.type == "AnimationClipData") {
		if (ImGui::Button("Add Selected Animation", { -1, 36 })) {
			AnimationClipData* animData = (AnimationClipData*)assignAsset->load();
			if (animData != NULL)
				skeletonMeshActor->addAnimationClip(*animData);
		}
	}
	ImGui::InputText("BlendSpace Name", &blendSpaceName);
	if (!blendSpaceName.empty()) {
		if (ImGui::Button("Add BlendSpace", { -1, 36 })) {
			skeletonMeshActor->addBlendSpaceAnimation(blendSpaceName);
		}
	}
	ImGui::Separator();
	int i = 0;
	for (auto b = skeletonMeshActor->animationClipList.begin(),
		e = skeletonMeshActor->animationClipList.end(); b != e; b++, i++) {
		ImGui::PushID(i);
		AnimationBase* base = skeletonMeshActor->animationClips[b->second];
		AnimationClip* clip = dynamic_cast<AnimationClip*>(base);
		BlendSpaceAnimation* blend = dynamic_cast<BlendSpaceAnimation*>(base);
		if (ImGui::Button(("Play Animation[" + b->first + "]").c_str(), { -50, 36 })) {
			skeletonMeshActor->activeAnimationClip(b->second);
			base->play();
#ifdef AUDIO_USE_OPENAL
			if (b->second < skeletonMeshActor->audioSources.size())
				skeletonMeshActor->audioSources[b->second]->play();
#endif // AUDIO_USE_OPENAL
		}
		ImGui::SameLine();
		bool loop = base->isLoop();
		if (ImGui::Checkbox("Loop", &loop))
			base->setLoop(loop);
		/*if (ImGui::Button("Show Bind", { -1, 36 })) {
			if (clip != NULL) {
				for (int i = 0; i < clip->animationPlayer.size(); i++) {
					auto& player = clip->animationPlayer[i];
					Console::log("[%d]%s : %s", i, player.transformAnimationData->objectName.c_str(),
						player.targetTransform == NULL ? "null" : player.targetTransform->name.c_str());
				}
			}
		}*/
		if (blend != NULL) {
			if (ImGui::Button("Edit BlendSpace", { -1, 36 })) {
				BlendSpaceWindow* win = dynamic_cast<BlendSpaceWindow*>(info.gui->getUIControl("BlendSpaceWindow"));
				if (win == NULL) {
					win = new BlendSpaceWindow();
					info.gui->addUIControl(*win);
				}
				win->show = true;
				win->setBlendSpaceAnimation(*blend);
			}
		}
		ImGui::PopID();
	}
	if (skeletonMeshActor->animationClip != NULL) {
		if (skeletonMeshActor->animationClip->playing()) {
			ImGui::Separator();
			if (ImGui::Button("Pause", { -1, 36 })) {
				skeletonMeshActor->animationClip->pause();
#ifdef AUDIO_USE_OPENAL
				for (int i = 0; i < skeletonMeshActor->audioSources.size(); i++) {
					skeletonMeshActor->audioSources[i]->pause();
				}
#endif // AUDIO_USE_OPENAL
			}
			if (ImGui::Button("Stop", { -1, 36 })) {
				skeletonMeshActor->animationClip->stop();
#ifdef AUDIO_USE_OPENAL
				for (int i = 0; i < skeletonMeshActor->audioSources.size(); i++) {
					skeletonMeshActor->audioSources[i]->stop();
				}
#endif // AUDIO_USE_OPENAL
			}
		}
		AnimationClip* clip = dynamic_cast<AnimationClip*>(skeletonMeshActor->animationClip);
		if (clip != NULL) {
			float time = clip->getTime();
			if (ImGui::SliderFloat("Time", &time, 0, clip->animationClipData->duration)) {
				clip->setTime(time);
			}
		}
	}
}

void SkeletonMeshActorEditor::onSkeletonPhysicsGUI(EditorInfo& info)
{
	editingSkeletonPhysics = ImGui::CollapsingHeader("SkeletonPhysics");
	if (editingSkeletonPhysics) {
		if (ImGui::Button("Apply Skeleton Physics", { -1, 36 })) {
			skeletonMeshActor->skeletonPhysics.applySkeletonPhysicsInfo();
		}

		SkeletonPhysics::RigidBodyInfo* selectedRigidBodyInfo = getSelectedRigidBodyInfo();
		if (selectedRigidBodyInfo != NULL) {
			ImGui::DragFloat3("Position", selectedRigidBodyInfo->positionOffset.data(), 0.01);
			ImGui::DragFloat3("Rotation", selectedRigidBodyInfo->rotationOffset.data(), 0.01);
			const char* PhysicsTypeName[3] = {
				"Static", "Dynamic", "NoCollision"
			};
			if (ImGui::BeginCombo("PhysicsType", PhysicsTypeName[selectedRigidBodyInfo->material.physicalType]))
			{
				for (int i = 0; i < 3; i++) {
					if (ImGui::Selectable(PhysicsTypeName[i],
						selectedRigidBodyInfo->material.physicalType == i )) {
						selectedRigidBodyInfo->material.physicalType = (PhysicalType)i;
					}
				}
				ImGui::EndCombo();
			}
			if (ImGui::DragFloat("Mass", &selectedRigidBodyInfo->material.mass))
				selectedRigidBodyInfo->material.mass = max(0, selectedRigidBodyInfo->material.mass);
			if (ImGui::BeginCombo("PhysicsLayer", to_string(selectedRigidBodyInfo->layer.layer).c_str()))
			{
				for (int i = 0; i < 32; i++) {
					if (ImGui::Selectable(to_string(i).c_str(),
						selectedRigidBodyInfo->layer.layer == i)) {
						selectedRigidBodyInfo->layer.layer = i;
					}
				}
				ImGui::EndCombo();
			}
			ImGui::BeginChild("##MaskView", { 0, 40 });
			ImGui::BeginHorizontal("Mask");
			for (int i = 0; i < 32; i++) {
				bool ignore = selectedRigidBodyInfo->layer.isIgnoreLayer(i);
				if (ignore) {
					if (ImGui::Button(to_string(i).c_str(), { 36, 36 })) {
						selectedRigidBodyInfo->layer.setIgnoreLayer(i, false);
					}
				}
			}
			if (ImGui::BeginPopupContextVoid("##MaskPopup", 0))
			{
				for (int i = 0; i < 32; i++) {
					bool ignore = selectedRigidBodyInfo->layer.isIgnoreLayer(i);
					if (!ignore) {
						if (ImGui::MenuItem(to_string(i).c_str())) {
							selectedRigidBodyInfo->layer.setIgnoreLayer(i, true);
						}
					}
				}
				ImGui::EndPopup();
			}
			ImGui::Button("+", { 36, 36 });
			ImGui::OpenPopupOnItemClick("##MaskPopup");
			ImGui::EndHorizontal();
			ImGui::EndChild();
			SkeletonPhysics::ShapeInfo& shapeInfo = selectedRigidBodyInfo->shape;
			switch (shapeInfo.type)
			{
			case SkeletonPhysics::ShapeInfo::BoxShape:
			{
				Vector3f extend = shapeInfo.bound.maxVal - shapeInfo.bound.minVal;
				extend.x() = abs(extend.x());
				extend.y() = abs(extend.y());
				extend.z() = abs(extend.z());
				if (ImGui::DragFloat3("Extend", extend.data(), 0.01)) {
					extend.x() = max(0, extend.x());
					extend.y() = max(0, extend.y());
					extend.z() = max(0, extend.z());

					shapeInfo.bound.maxVal = extend * 0.5f;
					shapeInfo.bound.minVal = -shapeInfo.bound.maxVal;
				}
				break;
			}
			case SkeletonPhysics::ShapeInfo::SphereShape:
			{
				float radius = abs(shapeInfo.bound.maxVal.x() - shapeInfo.bound.minVal.x()) * 0.5f;
				if (ImGui::DragFloat("Radius", &radius, 0.01)) {
					radius = max(0, radius);

					shapeInfo.bound.maxVal = Vector3f(radius, radius, radius);
					shapeInfo.bound.minVal = -shapeInfo.bound.maxVal;
				}
				break;
			}
			case SkeletonPhysics::ShapeInfo::CapsuleShape:
			{
				float radius = abs(shapeInfo.bound.maxVal.x() - shapeInfo.bound.minVal.x()) * 0.5f;
				float halfLength = (abs(shapeInfo.bound.maxVal.z() - shapeInfo.bound.minVal.z()) -
					abs(shapeInfo.bound.maxVal.x() - shapeInfo.bound.minVal.x())) * 0.5f;
				bool modify = ImGui::DragFloat("Radius", &radius, 0.01);
				modify |= ImGui::DragFloat("HalfLength", &halfLength, 0.01);
				if (modify) {
					radius = max(0, radius);
					halfLength = max(0, halfLength);

					shapeInfo.bound.maxVal = Vector3f(radius, radius, radius + halfLength);
					shapeInfo.bound.minVal = -shapeInfo.bound.maxVal;
				}
				break;
			}
			default:
				break;
			}
		}
	}
}

void SkeletonMeshActorEditor::onHandleGizmo(GizmoInfo& info)
{
	ActorEditor::onHandleGizmo(info);

	if (editingBoneConstraint) {
		if (selectedBoneConstraint != NULL) {
			BoneConstraintEditor* editor = dynamic_cast<BoneConstraintEditor*>(EditorManager::getEditor(*selectedBoneConstraint));
			if (editor != NULL) {
				editor->setSkeletonMeshActor(skeletonMeshActor);
				editor->onGizmo(info);
			}
		}
	}

	if (editingSkeletonPhysics) {

		SkeletonPhysics::RigidBodyInfo* selectedRigidBodyInfo = getSelectedRigidBodyInfo();
		SkeletonPhysics::SkeletonPhysicsInfo& spInfo = skeletonMeshActor->skeletonPhysics.info;
		for (int i = 0; i < spInfo.rigidbodies.size(); i++) {
			SkeletonPhysics::RigidBodyInfo& rbInfo = spInfo.rigidbodies[i];
			Bone* bone = skeletonMeshActor->getBone(rbInfo.boneName);
			if (bone == NULL)
				continue;
			Matrix4f boneMat = bone->getTransformMat();
			Matrix4f T = Matrix4f::Identity();
			T.block(0, 3, 3, 1) = rbInfo.positionOffset;
			Matrix4f R = Matrix4f::Identity();
			R.block(0, 0, 3, 3) = rbInfo.rotationOffset.toRotationMatrix();
			Matrix4f mat = boneMat * T * R;
			Range<Vector3f> bound = rbInfo.shape.bound;

			if (selectedRigidBodyInfo == &rbInfo) {
				if (info.gizmo->drawHandle(selectedRigidBodyInfo,
					handleOperation, handleSpace, mat,
					snapVector == NULL ? Vector3f::Zero() : *snapVector)) {
					Matrix4f dmat = boneMat.inverse() * mat;
					dmat.decompose(rbInfo.positionOffset, rbInfo.rotationOffset, Vector3f::Zero());
				}
			}

			Color color = Color::HSV({ rbInfo.layer.getLayer() * 24.0f, 0.64f, 0.94f });
			switch (rbInfo.shape.type)
			{
			case SkeletonPhysics::ShapeInfo::BoxShape:
				info.gizmo->drawAABB(bound, mat, color);
				break;
			case SkeletonPhysics::ShapeInfo::SphereShape:
			{
				float radius = abs(bound.maxVal.x() - bound.minVal.x()) * 0.5f;
				info.gizmo->drawSphere(Vector3f::Zero(), radius, mat, color);
				if (info.gizmo->pickSphere(Vector3f::Zero(), radius, Vector2f::Zero(), mat))
					selectRigidBodyInfo(i);
				break;
			}
			case SkeletonPhysics::ShapeInfo::CapsuleShape:
			{
				float radius = abs(bound.maxVal.x() - bound.minVal.x()) * 0.5f;
				float halfLength = (abs(bound.maxVal.z() - bound.minVal.z()) - abs(bound.maxVal.x() - bound.minVal.x())) * 0.5f;
				info.gizmo->drawCapsuleX(Vector3f::Zero(), radius, halfLength, mat, color);
				if (info.gizmo->pickCapsuleX(Vector3f::Zero(), radius, halfLength, Vector2f::Zero(), mat))
					selectRigidBodyInfo(i);
				break;
			}
			default:
				break;
			}
		}

		for (int i = 0; i < spInfo.constraints.size(); i++) {
			SkeletonPhysics::D6ConstraintInfo& cInfo = spInfo.constraints[i];
			Bone* bone = skeletonMeshActor->getBone(cInfo.boneName);
			if (bone == NULL)
				continue;
			Bone* tbone = skeletonMeshActor->getBone(cInfo.targetBoneName);
			if (tbone == NULL)
				continue;

		}
	}
}

void SkeletonMeshActorEditor::onSkeletonMeshActorGUI(EditorInfo& info)
{
	onSkeletonGUI(info);
	onAnimationGUI(info);
	onSkeletonPhysicsGUI(info);
}

void SkeletonMeshActorEditor::onActorGUI(EditorInfo& info)
{
	ActorEditor::onActorGUI(info);
	if (ImGui::CollapsingHeader("SkeletonMeshActor"))
		onSkeletonMeshActorGUI(info);
}

void SkeletonMeshActorEditor::onRenderersGUI(EditorInfo& info)
{
	Asset* assignAsset = EditorManager::getSelectedAsset();
	if (assignAsset != NULL && assignAsset->assetInfo.type == "SkeletonMesh") {
		if (ImGui::Button("Add Selected Skeleton Mesh", { -1, 36 })) {
			skeletonMeshActor->addSkeletonMesh(*(SkeletonMesh*)assignAsset->load());
		}
	}

	for (int i = 0; i < skeletonMeshActor->skeletonMeshRenders.size();) {
		ImGui::PushID(i);
		ImGui::Indent(10);
		if (ImGui::CollapsingHeader(("Render " + to_string(i)).c_str())) {
			ImGui::Indent(10);
			if (ImGui::Button("Remove", { -1, 36 })) {
				skeletonMeshActor->removeSkeletonMesh(i);
				continue;
			}
			Editor* editor = EditorManager::getEditor("SkeletonMeshRender", skeletonMeshActor->skeletonMeshRenders[i]);
			editor->onGUI(info);
			ImGui::Unindent(10);
		}
		ImGui::Unindent(10);
		ImGui::PopID();
		i++;
	}
}

SkeletonPhysics::RigidBodyInfo* SkeletonMeshActorEditor::getSelectedRigidBodyInfo()
{
	if (skeletonMeshActor == NULL ||
		selectedRigidBodyInfo == NULL || selectedRigidBodyIndex < 0 ||
		selectedRigidBodyIndex >= skeletonMeshActor->skeletonPhysics.info.rigidbodies.size() ||
		selectedRigidBodyInfo != &skeletonMeshActor->skeletonPhysics.info.rigidbodies[selectedRigidBodyIndex]) {
		selectedRigidBodyInfo = NULL;
		selectedRigidBodyIndex = -1;
		return NULL;
	}
	return selectedRigidBodyInfo;
}

void SkeletonMeshActorEditor::selectRigidBodyInfo(int index)
{
	if (index >= 0) {
		if (skeletonMeshActor != NULL &&
			index < skeletonMeshActor->skeletonPhysics.info.rigidbodies.size()) {
			selectedRigidBodyInfo = &skeletonMeshActor->skeletonPhysics.info.rigidbodies[index];
			selectedRigidBodyIndex = index;
		}
	}
	else if (index == -1) {
		selectedRigidBodyInfo = NULL;
		selectedRigidBodyIndex = -1;
	}
}
