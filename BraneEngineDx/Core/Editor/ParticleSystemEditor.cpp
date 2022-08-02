#include "ParticleSystemEditor.h"
#include "MaterialEditor.h"

RegistEditor(ParticleSystem);

void ParticleSystemEditor::setInspectedObject(void* object)
{
	particleSystem = dynamic_cast<ParticleSystem*>((Base*)object);
	ActorEditor::setInspectedObject(particleSystem);
}

void ParticleSystemEditor::onPersistentGizmo(GizmoInfo& info)
{
}

void ParticleSystemEditor::onActorGUI(EditorInfo& info)
{
	if (ImGui::CollapsingHeader("ParticleSystem")) {
		if (ImGui::Button("Add Emitter", { -1, 30 })) {
			ParticleEmitter& emitter = particleSystem->particleRender.addEmitter();
		}
		int i = 0;
		for (auto b = particleSystem->particleRender.particleEmtters.begin(),
			e = particleSystem->particleRender.particleEmtters.end(); b != e; b++, i++) {
			if (ImGui::TreeNode(("Emitter " + to_string(i)).c_str())) {
				ImGui::PushID(i);
				if (ImGui::Button("Activate", { -1, 30 }))
					b->activate();
				if (ImGui::Button("Reset", { -1, 30 }))
					b->reset();
				if (ImGui::Button("Delete", { -1, 30 })) {
					particleSystem->particleRender.removeEmitter(i);
					break;
				}
				int type = b->screenAlignmentType;
				if (ImGui::Combo("AlignmentType", &type, "Sprite\0Velocity\0Beam\0Trail\0"))
					b->screenAlignmentType = (ParticleEmitter::ScreenAlignmentType)type;
				Asset* meshAsset = b->getSpawnMesh() == NULL ? NULL : MeshAssetInfo::getInstance().getAsset(b->getSpawnMesh());
				ImGui::Text(meshAsset == NULL ? "No Spawn Mesh" : meshAsset->name.c_str());
				ImGui::SameLine();
				if (ImGui::ArrowButton("AssignSpawnMeshBT", ImGuiDir_Left)) {
					Asset* selectedAsset = EditorManager::getSelectedAsset();
					if (selectedAsset && selectedAsset->assetInfo.type == "Mesh") {
						b->setSpawnMesh(*(Mesh*)selectedAsset->load());
					}
				}
				if (meshAsset != NULL) {
					ImGui::SameLine();
					if (ImGui::Button("¡Á"))
						b->removeSpawnMesh();
				}
				bool hasCollision = b->hasCollision;
				if (ImGui::Checkbox("HasCollision", &hasCollision))
					b->hasCollision = hasCollision;
				if (ImGui::TreeNode("BoundDamp")) {
					ImGui::DragFloat("Min", &b->boundDamp.minVal, 0.01);
					b->boundDamp.minVal = b->boundDamp.minVal < 0 ? 0 : b->boundDamp.minVal;
					ImGui::DragFloat("Max", &b->boundDamp.maxVal, 0.01);
					b->boundDamp.maxVal = b->boundDamp.maxVal < 0 ? 0 : b->boundDamp.maxVal;
					ImGui::TreePop();
				}
				int speed = b->emitCountSec;
				if (ImGui::DragInt("Speed", &speed))
					b->emitCountSec = speed > 0 ? speed : 0;
				int initCount = b->initCount;
				if (ImGui::DragInt("InitCount", &initCount))
					b->initCount = initCount > 0 ? initCount : 0;
				int maxCount = b->maxCount;
				if (ImGui::DragInt("MaxCount", &maxCount))
					b->maxCount = maxCount > 0 ? maxCount : 0;
				ImGui::ColorEdit4("InitColor", (float*)&b->initColor, ImGuiColorEditFlags_HDR);
				if (ImGui::TreeNode("Lifetime")) {
					ImGui::DragFloat("Min", &b->lifetime.minVal, 0.01);
					b->lifetime.minVal = b->lifetime.minVal < 0 ? 0 : b->lifetime.minVal;
					ImGui::DragFloat("Max", &b->lifetime.maxVal, 0.01);
					b->lifetime.maxVal = b->lifetime.maxVal < 0 ? 0 : b->lifetime.maxVal;
					ImGui::TreePop();
				}
				if (ImGui::TreeNode("InitPosition")) {
					ImGui::DragFloat3("Min", b->initPosition.minVal.data(), 0.01);
					ImGui::DragFloat3("Max", b->initPosition.maxVal.data(), 0.01);
					ImGui::TreePop();
				}
				if (type == 2) {
					if (ImGui::TreeNode("TargetPosition")) {
						ImGui::DragFloat3("Min", b->targetPosition.minVal.data(), 0.01);
						ImGui::DragFloat3("Max", b->targetPosition.maxVal.data(), 0.01);
						ImGui::TreePop();
					}
					if (ImGui::TreeNode("ShapeTimeRate")) {
						ImGui::DragFloat("Min", &b->shapeTimeRate.minVal, 0.01);
						ImGui::DragFloat("Max", &b->shapeTimeRate.maxVal, 0.01);
						ImGui::TreePop();
					}
				}
				if (ImGui::TreeNode("InitScale")) {
					ImGui::DragFloat3("Min", b->initScale.minVal.data(), 0.01);
					ImGui::DragFloat3("Max", b->initScale.maxVal.data(), 0.01);
					ImGui::TreePop();
				}
				if (ImGui::TreeNode("InitVelocity")) {
					ImGui::DragFloat3("Min", b->initVelocity.minVal.data(), 0.01);
					ImGui::DragFloat3("Max", b->initVelocity.maxVal.data(), 0.01);
					ImGui::TreePop();
				}
				ImGui::DragFloat3("Acceleration", b->acceleration.data(), 0.01);
				if (ImGui::TreeNode("ColorCurve")) {
					if (ImGui::Button("Add Key", { -1, 30 })) {
						if (b->colorCurve.empty())
							b->colorCurve.insert(0, CurveValue<Color>(
								CurveValue<Color>::Linear, Color(1.f, 1.f, 1.f)));
						else
							b->colorCurve.insert(b->colorCurve.keys.cend()->first + 1,
								b->colorCurve.keys.cend()->second);
					}
					int cci = 0;
					for (auto ccb = b->colorCurve.keys.begin(), cce = b->colorCurve.keys.end(); ccb != cce; cci++) {
						if (ImGui::TreeNode(("Key " + to_string(cci)).c_str())) {
							ImGui::PushID(cci);
							bool willDelete = false;
							bool willAdd = false;
							float key = ccb->first;
							int keyMode = ccb->second.mode;
							Color color = ccb->second.value;
							if (ImGui::Button("Delete")) {
								willDelete = true;
							}
							if (ImGui::InputFloat("Time", &key)) {
								if (ccb->first != key) {
									willDelete = true;
									willAdd = true;
								}
							}
							if (ImGui::Combo("KeyMode", &keyMode, "Linear\0Step\0Auto")) {
								ccb->second.mode = (CurveValue<Color>::KeySmoothMode)keyMode;
							}
							if (ImGui::ColorEdit4("Color", (float*)&color, ImGuiColorEditFlags_HDR)) {
								ccb->second.value = color;
							}
							if (willDelete) {
								auto t = ccb;
								ccb++;
								b->colorCurve.keys.erase(t);
							}
							else
								ccb++;
							if (willAdd) {
								b->colorCurve.insert(key, CurveValue<Color>((CurveValue<Color>::KeySmoothMode)keyMode, color));
								break;
							}
							ImGui::PopID();
							ImGui::TreePop();
						}
						else
							ccb++;
					}
					ImGui::TreePop();
				}
				if (ImGui::TreeNode("ScaleCurve")) {
					if (ImGui::Button("Add Key", { -1, 30 })) {
						if (b->scaleCurve.empty())
							b->scaleCurve.insert(0, CurveValue<Vector3f>(
								CurveValue<Vector3f>::Linear, Vector3f(1.f, 1.f, 1.f)));
						else
							b->scaleCurve.insert(b->scaleCurve.keys.cend()->first + 0.1,
								b->scaleCurve.keys.cend()->second);
					}
					int cci = 0;
					for (auto ccb = b->scaleCurve.keys.begin(), cce = b->scaleCurve.keys.end(); ccb != cce; cci++) {
						if (ImGui::TreeNode(("Key " + to_string(cci)).c_str())) {
							ImGui::PushID(cci);
							bool willDelete = false;
							bool willAdd = false;
							float key = ccb->first;
							int keyMode = ccb->second.mode;
							Vector3f scale = ccb->second.value;
							if (ImGui::Button("Delete")) {
								willDelete = true;
							}
							if (ImGui::InputFloat("Time", &key)) {
								if (ccb->first != key) {
									willDelete = true;
									willAdd = true;
								}
							}
							if (ImGui::Combo("KeyMode", &keyMode, "Linear\0Step\0Auto")) {
								ccb->second.mode = (CurveValue<Vector3f>::KeySmoothMode)keyMode;
							}
							if (ImGui::DragFloat3("Scale", (float*)&scale, 0.01)) {
								ccb->second.value = scale;
							}
							if (willDelete) {
								auto t = ccb;
								ccb++;
								b->scaleCurve.keys.erase(t);
							}
							else
								ccb++;
							if (willAdd) {
								b->scaleCurve.insert(key, CurveValue<Vector3f>((CurveValue<Vector3f>::KeySmoothMode)keyMode, scale));
								break;
							}
							ImGui::PopID();
							ImGui::TreePop();
						}
						else
							ccb++;
					}
					ImGui::TreePop();
				}
				MaterialEditor* editor = dynamic_cast<MaterialEditor*>(EditorManager::getEditor("Material", b->material));
				if (editor) {
					if (ImGui::CollapsingHeader("Material"))
						editor->onGUI(info);
				}
				/*multimap<string, unsigned int> partNames;
				partNames.insert(pair<string, int>("Particle", 0));
				vector<Material*> mats;
				mats.push_back(b->material);
				showMaterial(partNames, mats, info.gui);
				b->material = mats[0];*/
				if (ImGui::TreeNode("Detail")) {
					ImGui::Text("Count: %d", b->particles.size());
					int j = 0;
					for (auto pb = b->particles.begin(), pe = b->particles.end(); pb != pe; pb++, j++) {
						ImGui::PushID(j);
						if (ImGui::TreeNode(("P " + to_string(j)).c_str())) {
							ImGui::Text("pos: %.3f, %.3f, %.3f", pb->position.x(), pb->position.y(), pb->position.z());
							ImGui::Text("sca: %.3f, %.3f, %.3f", pb->scale.x(), pb->scale.y(), pb->scale.z());
							ImGui::Text("vel: %.3f, %.3f, %.3f", pb->velocity.x(), pb->velocity.y(), pb->velocity.z());
							ImGui::Text("acc: %.3f, %.3f, %.3f", pb->acceleration.x(), pb->acceleration.y(), pb->acceleration.z());
							ImGui::TreePop();
						}
						ImGui::PopID();
					}
				}
				ImGui::PopID();
				ImGui::TreePop();
			}
		}
	}
}

void ParticleSystemEditor::onRenderersGUI(EditorInfo& info)
{
}
