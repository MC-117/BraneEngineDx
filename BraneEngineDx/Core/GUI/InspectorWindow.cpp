#include "InspectorWindow.h"
#include "../Engine.h"
#include <thread>
#include <fstream>
#include "../../ThirdParty/ImGui/imgui_stdlib.h"
#include "BlendSpaceWindow.h"
#include "TextureViewer.h"
#include "../Console.h"
#include "../PostProcess/PostProcessingCamera.h"
#include "../../Actors/VehicleActor.h"
#include "../../Core/ParticleSystem.h"

InspectorWindow::InspectorWindow(Object & object, string name, bool defaultShow) : UIWindow(object, name, defaultShow)
{
	events.registerFunc("assignAsset", [](void* win, void* asset) {
		((InspectorWindow*)win)->assignAsset = (Asset*)asset;
	});
}

void InspectorWindow::onRenderWindow(GUIRenderInfo& info)
{
	if (target == NULL) {

	}
	else {
		Object& obj = *target;
		ImGui::BeginGroup();
		ImGui::Text("Object Name: %s", obj.name.c_str());
		ImGui::SameLine();
		if (ImGui::Button("Copy")) {
			ImGui::SetClipboardText(obj.name.c_str());
		}
		ImGui::SameLine();
		if (ImGui::Button("Destroy")) {
			obj.destroy();
			target = NULL;
		}
		if (ImGui::Button("Load Profile", { -1, 40 })) {
			thread td = thread([](Object* tar) {
				FileDlgDesc desc;
				desc.title = "asset";
				desc.filter = "asset(*.asset)\0*.asset\0";
				desc.initDir = "Content";
				desc.defFileExt = "asset";
				if (openFileDlg(desc)) {
					ifstream f = ifstream(desc.filePath);
					SerializationInfoParser parser = SerializationInfoParser(f);
					if (!parser.parse()) {
						MessageBox(NULL, "Load failed", "Error", MB_OK);
						Console::error("SerializationInfoParser error: %s", parser.parseError.c_str());
					}
					else if (!parser.infos.empty()) {
						if (!tar->deserialize(parser.infos[0])) {
							MessageBox(NULL, "Deserialize failed", "Error", MB_OK);
							Console::error("SerializationInfoParser error: %s", parser.parseError.c_str());
						}
					}
					f.close();
				}
			}, &obj);
			td.detach();
		}
		if (ImGui::Button("Save Profile", { -1, 40 })) {
			thread td = thread([](Object* tar) {
				FileDlgDesc desc;
				desc.title = "asset";
				desc.filter = "asset(*.asset)\0*.asset\0";
				desc.initDir = "Content";
				desc.save = true;
				desc.defFileExt = "asset";
				if (openFileDlg(desc)) {
					SerializationInfo info;
					if (!tar->serialize(info)) {
						MessageBox(NULL, "Serialize failed", "Error", MB_OK);
						return;
					}
					ofstream f = ofstream(desc.filePath);
					SerializationInfoWriter writer = SerializationInfoWriter(f);
					writer.write(info);
					f.close();
					MessageBox(NULL, "Complete", "Info", MB_OK);
				}
			}, &obj);
			td.detach();
		}
		long long address = (long long)&obj;
		string addressStr = to_string(address);
		ImGui::InputText("Address", &addressStr);
		if (obj.parent != NULL) {
			if (ImGui::Button("Duplicate", { -1, 36 })) {
				ImGui::OpenPopup("DuplicatePopup");
			}
			if (ImGui::BeginPopup("DuplicatePopup")) {
				static string name;
				ImGui::InputText("Name", &name);
				if (!name.empty() && Brane::find(typeid(Object).hash_code(), name) == NULL) {
					if (ImGui::Button("Create", { -1, 36 })) {
						SerializationInfo sinfo;
						if (obj.serialize(sinfo)) {
							sinfo.name = name;
							Serialization* serialization = SerializationManager::getSerialization(sinfo.type);
							if (serialization != NULL) {
								Object* nobj = dynamic_cast<Object*>(serialization->instantiate(sinfo));
								if (nobj != NULL) {
									if (nobj->deserialize(sinfo)) {
										obj.parent->addChild(*nobj);
									}
									else
										delete nobj;
								}
							}
						}
					}
				}
				ImGui::EndPopup();
			}
		}
		if (ImGui::CollapsingHeader("Events")) {
			for (auto b = obj.events.voidFuncField.begin(), e = obj.events.voidFuncField.end();
				b != e; b++) {
				if (ImGui::Button(b->first.c_str(), { -1, 36 }))
					b->second(&obj);
			}
			static vector<pair<string, Object*>> objectNames;
			if (obj.events.objectFuncField.size() != objectNames.size()) {
				objectNames.clear();
				objectNames.resize(obj.events.objectFuncField.size());
			}
			int i = 0;
			for (auto b = obj.events.objectFuncField.begin(), e = obj.events.objectFuncField.end();
				b != e; b++, i++) {
				if (ImGui::InputText(("Object Name##" + b->first + "Name").c_str(), &objectNames[i].first)) {
					objectNames[i].second = (Object*)Brane::find(typeid(Object).hash_code(), objectNames[i].first);
				}
				if (ImGui::Button(b->first.c_str(), { -1, 36 }))
					b->second(&obj, objectNames[i].second);
			}
			static vector<int> ints;
			if (obj.events.intFuncField.size() != ints.size()) {
				ints.clear();
				ints.resize(obj.events.intFuncField.size());
			}
			i = 0;
			for (auto b = obj.events.intFuncField.begin(), e = obj.events.intFuncField.end();
				b != e; b++, i++) {
				ImGui::InputInt(("Int##" + b->first + "Int").c_str(), &ints[i]);
				if (ImGui::Button(b->first.c_str(), { -1, 36 }))
					b->second(&obj, ints[i]);
			}
			static vector<float> floats;
			if (obj.events.floatFuncField.size() != floats.size()) {
				floats.clear();
				floats.resize(obj.events.floatFuncField.size());
			}
			i = 0;
			for (auto b = obj.events.floatFuncField.begin(), e = obj.events.floatFuncField.end();
				b != e; b++, i++) {
				ImGui::InputFloat(("Float##" + b->first + "Float").c_str(), &floats[i]);
				if (ImGui::Button(b->first.c_str(), { -1, 36 }))
					b->second(&obj, floats[i]);
			}
			static vector<string> strings;
			if (obj.events.stringFuncField.size() != strings.size()) {
				strings.clear();
				strings.resize(obj.events.stringFuncField.size());
			}
			i = 0;
			for (auto b = obj.events.stringFuncField.begin(), e = obj.events.stringFuncField.end();
				b != e; b++, i++) {
				ImGui::InputText(("String##" + b->first + "String").c_str(), &strings[i]);
				if (ImGui::Button(b->first.c_str(), { -1, 36 }))
					b->second(&obj, strings[i]);
			}
		}
		ImGui::Separator();
		::Transform* tf = dynamic_cast<::Transform*>(&obj);
		if (tf != NULL) {
			if (ImGui::CollapsingHeader("Transform")) {
				float v[3] = { tf->position.x(), tf->position.y(), tf->position.z() };
				if (ImGui::DragFloat3("Position", v, 0.01)) {
					tf->setPosition(v[0], v[1], v[2]);
				}
				Vector3f rv = tf->getEulerAngle();
				float r[3] = { rv.x(), rv.y(), rv.z() };
				if (ImGui::DragFloat3("Rotation", r, 0.1)) {
					tf->setRotation(r[0], r[1], r[2]);
				}
				float s[3] = { tf->scale.x(), tf->scale.y(), tf->scale.z() };
				if (ImGui::DragFloat3("Scale", s, 0.01))
					tf->setScale(s[0], s[1], s[2]);

				static ImGuizmo::OPERATION operation = ImGuizmo::TRANSLATE;
				static float tSnap[3] = { 0, 0, 0 };
				static float rSnap = 0;
				static float sSnap = 0;
				Matrix4f transMat;

				if (ImGui::IsKeyPressed('Z'))
					operation = ImGuizmo::TRANSLATE;
				if (ImGui::IsKeyPressed('X'))
					operation = ImGuizmo::ROTATE;
				if (ImGui::IsKeyPressed('C'))
					operation = ImGuizmo::SCALE;

				float* snapv = NULL;

				if (ImGui::IsKeyDown('V')) {
					switch (operation)
					{
					case ImGuizmo::TRANSLATE:
						ImGui::DragFloat("Snap", tSnap, 0.1);
						if (tSnap[0] <= 0)
							tSnap[0] = 0.1;
						tSnap[1] = tSnap[2] = tSnap[0];
						snapv = tSnap;
						break;
					case ImGuizmo::ROTATE:
						ImGui::DragFloat("Snap", &rSnap, 0.1);
						if (rSnap <= 0)
							rSnap = 0.1;
						snapv = &rSnap;
						break;
					case ImGuizmo::SCALE:
						ImGui::DragFloat("Snap", &sSnap, 0.1);
						if (sSnap <= 0)
							sSnap = 0.1;
						snapv = &sSnap;
						break;
					case ImGuizmo::BOUNDS:
						break;
					default:
						break;
					}
				}

				transMat = tf->getMatrix(WORLD);

				Camera &curcam = world.getCurrentCamera();
				Matrix4f projm = curcam.getProjectionMatrix();
				Matrix4f viewm = curcam.getViewMatrix();

				ImGuizmo::Manipulate(viewm.data(), projm.data(), operation, ImGuizmo::WORLD, transMat.data(), NULL, snapv);

				if (ImGuizmo::IsUsing()) {
					tf->setMatrix(transMat, WORLD);
				}
				if (ImGui::Button("Add ParticleSystem", { -1, 36 })) {
					string psName = tf->name + "_PS";
					int index = 0;
					while (tf->findChild(psName + to_string(index)) != NULL) index++;
					ParticleSystem& ps = *new ParticleSystem(psName);
					tf->addChild(ps);
				}
			}
		}
		Light* lit = dynamic_cast<Light*>(&obj);
		if (lit != NULL) {
			if (ImGui::CollapsingHeader("Light")) {
				Color color = lit->color;
				if (ImGui::ColorEdit4("Color", (float*)&color))
					lit->color = color;
				float val = lit->intensity;
				if (ImGui::SliderFloat("Intensity", &val, 0, 10))
					lit->intensity = val;
			}
		}
		/*SpringArm* springArm = dynamic_cast<SpringArm*>(&obj);
		if (springArm != NULL) {
			if (ImGui::CollapsingHeader("SpringArm")) {
				float r = springArm->getRadius(), l = springArm->getSpringLength();
				if (ImGui::DragFloat("Radius", &r))
					springArm->setRadius(r);
				if (ImGui::DragFloat("SpringLength", &l))
					springArm->setSpringLength(l);
			}
		}*/
		Actor* a = dynamic_cast<Actor*>(&obj);
		if (a != NULL) {
			if (ImGui::CollapsingHeader("Actor")) {
				ImGui::BeginGroup();
				if (a->isHidden()) {
					if (ImGui::Button("Show Actor"))
						a->setHidden(false);
				}
				else
					if (ImGui::Button("Hide Actor"))
						a->setHidden(true);
				if (ImGui::Button("Collision")) {
					ImGui::OpenPopup("ActorCollision");
				}
				ImGui::Separator();
#ifdef AUDIO_USE_OPENAL
				ImGui::Text("AudioSource: %d", a->audioSources.size());
				if (assignAsset != NULL && assignAsset->assetInfo.type == "AudioData") {
					if (ImGui::Button("Add Selected Audio", { -1, 36 })) {
						AudioData* auData = (AudioData*)assignAsset->load();
						if (auData != NULL)
							a->addAudioSource(*auData);
					}
				}
				ImGui::Separator();
				for (int i = 0; i < a->audioSources.size(); i++) {
					ImGui::Text(a->audioSources[i]->audioData->name.c_str());
					ImGui::SameLine();
					if (a->audioSources[i]->getState() == AudioSource::Playing) {
						if (ImGui::Button(("Pause##Pause" + to_string(i)).c_str()))
							a->audioSources[i]->pause();
						ImGui::SameLine();
						if (ImGui::Button(("Stop##Stop" + to_string(i)).c_str()))
							a->audioSources[i]->stop();
					}
					else if (ImGui::Button(("Play##Play" + to_string(i)).c_str()))
						a->audioSources[i]->play();
				}
#endif // AUDIO_USE_OPENAL
				ParticleSystem* ps = dynamic_cast<ParticleSystem*>(&obj);
				if (ps != NULL) {
					if (ImGui::CollapsingHeader("ParticleSystem")) {
						if (ImGui::Button("Add Emitter", { -1, 30 })) {
							ParticleEmitter& emitter = ps->particleRender.addEmitter();
						}
						int i = 0;
						for (auto b = ps->particleRender.particleEmtters.begin(),
							e = ps->particleRender.particleEmtters.end(); b != e; b++, i++) {
							if (ImGui::TreeNode(("Emitter " + to_string(i)).c_str())) {
								ImGui::PushID(i);
								if (ImGui::Button("Activate", { -1, 30 }))
									b->activate();
								if (ImGui::Button("Reset", { -1, 30 }))
									b->reset();
								if (ImGui::Button("Delete", { -1, 30 })) {
									ps->particleRender.removeEmitter(i);
									break;
								}
								int type = b->screenAlignmentType;
								if (ImGui::Combo("AlignmentType", &type, "Sprite\0Velocity\0Beam\0Trail\0"))
									b->screenAlignmentType = (ParticleEmitter::ScreenAlignmentType)type;
								Asset* meshAsset = b->getSpawnMesh() == NULL ? NULL : MeshAssetInfo::getInstance().getAsset(b->getSpawnMesh());
								ImGui::Text(meshAsset == NULL ? "No Spawn Mesh" : meshAsset->name.c_str());
								ImGui::SameLine();
								if (ImGui::ArrowButton("AssignSpawnMeshBT", ImGuiDir_Left)) {
									if (assignAsset != NULL && assignAsset->assetInfo.type == "Mesh") {
										b->setSpawnMesh(*(Mesh*)assignAsset->load());
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
								multimap<string, unsigned int> partNames;
								partNames.insert(pair<string, int>("Particle", 0));
								vector<Material*> mats;
								mats.push_back(b->material);
								showMaterial(partNames, mats, info.gui);
								b->material = mats[0];
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
				/*Character* chr = dynamic_cast<Character*>(&obj);
				if (chr != NULL) {
					if (ImGui::CollapsingHeader("Character")) {
						Vector3f gv = chr->physicalController.gravityVelocity;
						Vector3f mv = chr->physicalController.moveVelocity;
						ImGui::Text("GravityVelocity: %f, %f, %f", gv.x(), gv.y(), gv.z());
						ImGui::Text("MoveVelocity: %f, %f, %f", mv.x(), mv.y(), mv.z());
						ImGui::Text("IsFly: %s", chr->isFly() ? "true" : "false");
					}
				}*/
				SkeletonMeshActor* sma = dynamic_cast<SkeletonMeshActor*>(&obj);
				if (sma != NULL) {
					if (ImGui::CollapsingHeader("SkeletonMeshActor")) {
						if (sma->getBoneHidden()) {
							if (ImGui::Button("Show All Bones", { -1, 36 }))
								sma->setBoneHidden(false);
						}
						else if (ImGui::Button("Hide All Bones", { -1, 36 }))
							sma->setBoneHidden(true);
						if (ImGui::Button("Set Reference Pose", { -1, 36 }))
							sma->setReferencePose();
						if (assignAsset != NULL && assignAsset->assetInfo.type == "AnimationClipData") {
							if (ImGui::Button("Add Selected Animation", { -1, 36 })) {
								AnimationClipData* animData = (AnimationClipData*)assignAsset->load();
								if (animData != NULL)
									sma->addAnimationClip(*animData);
							}
						}
						static string blendSpaceName;
						ImGui::InputText("BlendSpace Name", &blendSpaceName);
						if (!blendSpaceName.empty()) {
							if (ImGui::Button("Add BlendSpace", { -1, 36 })) {
								sma->addBlendSpaceAnimation(blendSpaceName);
							}
						}
						ImGui::Separator();
						for (auto b = sma->animationClipList.begin(), e = sma->animationClipList.end(); b != e; b++) {
							ImGui::PushID(b->second);
							char str[100];
							AnimationBase* base = sma->animationClips[b->second];
							AnimationClip* clip = dynamic_cast<AnimationClip*>(base);
							BlendSpaceAnimation* blend = dynamic_cast<BlendSpaceAnimation*>(base);
							sprintf_s(str, "Play Animation[%s]", b->first.c_str());
							if (ImGui::Button(str, { -20, 36 })) {
								sma->activeAnimationClip(b->second);
								base->play();
#ifdef AUDIO_USE_OPENAL
								if (b->second < sma->audioSources.size())
									sma->audioSources[b->second]->play();
#endif // AUDIO_USE_OPENAL
							}
							ImGui::SameLine();
							bool loop = base->isLoop();
							if (ImGui::Checkbox("Loop", &loop))
								base->setLoop(loop);
							if (blend != NULL) {
								if (ImGui::Button("Edit BlendSpace", { -1, 36 })) {
									BlendSpaceWindow *win = dynamic_cast<BlendSpaceWindow*>(info.gui.getUIControl("BlendSpaceWindow"));
									if (win == NULL) {
										win = new BlendSpaceWindow();
										info.gui.addUIControl(*win);
									}
									win->show = true;
									win->setBlendSpaceAnimation(*blend);
								}
							}
							ImGui::PopID();
						}
						if (sma->animationClip != NULL) {
							if (sma->animationClip->playing()) {
								ImGui::Separator();
								if (ImGui::Button("Pause", { -1, 36 })) {
									sma->animationClip->pause();
#ifdef AUDIO_USE_OPENAL
									for (int i = 0; i < sma->audioSources.size(); i++) {
										sma->audioSources[i]->pause();
									}
#endif // AUDIO_USE_OPENAL
								}
								if (ImGui::Button("Stop", { -1, 36 })) {
									sma->animationClip->stop();
#ifdef AUDIO_USE_OPENAL
									for (int i = 0; i < sma->audioSources.size(); i++) {
										sma->audioSources[i]->stop();
									}
#endif // AUDIO_USE_OPENAL
								}
							}
						}
						if (ImGui::TreeNode("MorphTarget")) {
							for (int i = 0; i < sma->skeletonMeshRender.morphWeights.size(); i++) {
								float w = sma->skeletonMeshRender.morphWeights[i];
								if (ImGui::SliderFloat(sma->skeletonMeshRender.skeletonMesh.morphName[i].c_str(),
									&w, 0, 1))
									sma->skeletonMeshRender.setMorphWeight(i, w);
							}
							ImGui::TreePop();
						}
					}
				}
				Render* rd = a->getRender();
				if (rd != NULL) {
					if (ImGui::CollapsingHeader("Render")) {
						bool can = rd->canCastShadow;
						if (ImGui::Checkbox("CastShadow", &can)) {
							rd->canCastShadow = can;
						}
					}
					Shape* shp = rd->getShape();
					if (shp != NULL) {
						if (ImGui::CollapsingHeader("Shape")) {
							ImGui::Text("Bound: (%.4f, %.4f, %.4f)-(%.4f, %.4f, %.4f)",
								shp->bound.minVal[0],
								shp->bound.minVal[1],
								shp->bound.minVal[2],
								shp->bound.maxVal[0],
								shp->bound.maxVal[1],
								shp->bound.maxVal[2]);
							Vector3f center = shp->getCenter();
							ImGui::Text("Center: (%.4f, %.4f, %.4f)",
								center.x(),
								center.y(),
								center.z());
							ImGui::Text("Width: %.4f, Height: %.4f, Depth: %.4f",
								shp->getWidth(),
								shp->getHeight(),
								shp->getDepth());
							Mesh* mesh = dynamic_cast<Mesh*>(shp);
							SkeletonMesh* smesh = dynamic_cast<SkeletonMesh*>(shp);
							if (mesh != NULL) {
								ImGui::Separator();
								ImGui::Text("Vertex Count: %d", mesh->vertCount);
								ImGui::Text("Face Count: %d", mesh->faceCount);
								if (ImGui::TreeNode("MeshParts")) {
									for (auto b = mesh->meshPartNames.begin(), e = mesh->meshPartNames.end(); b != e; b++) {
										if (ImGui::TreeNode(b._Ptr, b->first.c_str())) {
											ImGui::Text("Vertex Count: %d", mesh->meshParts[b->second].vertexCount);
											ImGui::Text("Face Count: %d", mesh->meshParts[b->second].elementCount);
											if (smesh != NULL && smesh->partHasMorph[b->second])
												ImGui::TextColored({ 1.f, 1.f, 0.f, 1.f }, "Has MorphTarget");
											ImGui::TreePop();
										}
									}
									ImGui::TreePop();
								}
							}
						}
					}
					MeshRender* mR = dynamic_cast<MeshRender*>(rd);
					if (mR == NULL) {
						/*if (ps == NULL) {*/
							Material* mat = rd->getMaterial();
							showMaterial(mat, rd);
						/*}*/
					}
					else {
						if (ImGui::CollapsingHeader("Outline")) {
							ImGui::Checkbox("Enable", &mR->enableOutline);
							if (mR->enableOutline && mR->outlineMaterial != NULL) {
								for (auto b = mR->outlineMaterial->getColorField().begin(), e = mR->outlineMaterial->getColorField().end(); b != e; b++) {
									Color color = b->second.val;
									if (ImGui::ColorEdit4(b->first.c_str(), (float*)&color))
										b->second.val = color;
								}
								for (auto b = mR->outlineMaterial->getScalarField().begin(), e = mR->outlineMaterial->getScalarField().end(); b != e; b++) {
									float val = b->second.val;
									if (ImGui::DragFloat(b->first.c_str(), &val, 0.01))
										b->second.val = val;
								}
								for (auto b = mR->outlineMaterial->getTextureField().begin(), e = mR->outlineMaterial->getTextureField().end(); b != e; b++) {
									unsigned long long id = b->second.val->getTextureID();
									if (id == 0)
										id = Texture2D::blackRGBDefaultTex.getTextureID();
									ImGui::Image((void*)id, { 64, 64 }, { 0, 1 }, { 1, 0 });
									ImGui::SameLine();
									ImGui::Text(b->first.c_str());
								}
							}
						}
						showMaterial(mR->mesh.meshPartNames, mR->materials, info.gui);
					}
					ImGui::EndGroup();
				}
			}
		}
		VehicleActor* vehicle = dynamic_cast<VehicleActor*>(&obj);
		if (vehicle != NULL) {
			if (ImGui::CollapsingHeader("Vehicle")) {
				ImGui::BeginChild("VehicleSubWindow");
				ImGui::Text("Force: %f", vehicle->force);
				ImGui::Text("Brake: %f", vehicle->brake);
				ImGui::Text("F: %f, %f, %f", vehicle->state.F.x(), vehicle->state.F.y(), vehicle->state.F.z());
				ImGui::Text("M: %f, %f, %f", vehicle->state.M.x(), vehicle->state.M.y(), vehicle->state.M.z());
				ImGui::Text("v: %f, %f, %f", vehicle->state.v.x(), vehicle->state.v.y(), vehicle->state.v.z());
				ImGui::Separator();
				ImGui::Columns(2, "Wheels");
				ImGui::Text("Steer: %f", vehicle->wheels[2]->steer* vehicle->wheels[2]->desc.steerAngle / PI * 180.0f);
				ImGui::Text("Mass: %f", vehicle->wheels[2]->desc.mass);
				ImGui::Text("Radius: %f", vehicle->wheels[2]->desc.radius);
				ImGui::Text("F: %f, %f, %f", vehicle->wheels[2]->state.F.x(), vehicle->wheels[2]->state.F.y(), vehicle->wheels[2]->state.F.z());
				ImGui::Text("Rot: %f", vehicle->wheels[2]->rotAngle);
				ImGui::Text("M: %f", vehicle->wheels[2]->state.M.y());
				ImGui::NextColumn();
				ImGui::Text("Steer: %f", vehicle->wheels[3]->steer* vehicle->wheels[3]->desc.steerAngle / PI * 180.0f);
				ImGui::Text("Mass: %f", vehicle->wheels[3]->desc.mass);
				ImGui::Text("Radius: %f", vehicle->wheels[3]->desc.radius);
				ImGui::Text("F: %f, %f, %f", vehicle->wheels[3]->state.F.x(), vehicle->wheels[3]->state.F.y(), vehicle->wheels[3]->state.F.z());
				ImGui::Text("Rot: %f", vehicle->wheels[3]->rotAngle);
				ImGui::Text("M: %f", vehicle->wheels[3]->state.M.y());
				ImGui::NextColumn();
				ImGui::Separator();
				ImGui::Text("Steer: %f", vehicle->wheels[0]->steer* vehicle->wheels[0]->desc.steerAngle / PI * 180.0f);
				ImGui::Text("Mass: %f", vehicle->wheels[0]->desc.mass);
				ImGui::Text("Radius: %f", vehicle->wheels[0]->desc.radius);
				ImGui::Text("F: %f, %f, %f", vehicle->wheels[0]->state.F.x(), vehicle->wheels[0]->state.F.y(), vehicle->wheels[0]->state.F.z());
				ImGui::Text("Rot: %f", vehicle->wheels[0]->rotAngle);
				ImGui::Text("M: %f", vehicle->wheels[0]->state.M.y());
				ImGui::NextColumn();
				ImGui::Text("Steer: %f", vehicle->wheels[1]->steer* vehicle->wheels[1]->desc.steerAngle / PI * 180.0f);
				ImGui::Text("Mass: %f", vehicle->wheels[1]->desc.mass);
				ImGui::Text("Radius: %f", vehicle->wheels[1]->desc.radius);
				ImGui::Text("F: %f, %f, %f", vehicle->wheels[1]->state.F.x(), vehicle->wheels[1]->state.F.y(), vehicle->wheels[1]->state.F.z());
				ImGui::Text("Rot: %f", vehicle->wheels[1]->rotAngle);
				ImGui::Text("M: %f", vehicle->wheels[1]->state.M.y());
				ImGui::NextColumn();
				ImGui::Columns();
				ImGui::EndChild();
			}
		}
		Camera* cam = dynamic_cast<Camera*>(&obj);
		if (cam != NULL) {
			if (ImGui::CollapsingHeader("Camera")) {
				if (&world.getCurrentCamera() != cam)
					if (ImGui::Button("SetCamera", { -1, 36 }))
						world.switchCamera(*cam);
				float fov = cam->fov;
				if (ImGui::DragFloat("FOV", &fov, 0.1, 0, 180)) {
					cam->fov = fov;
				}
				float distance = cam->distance;
				if (ImGui::DragFloat("Distance", &distance, 0.1)) {
					cam->distance = distance;
				}
				Color clearColor = cam->clearColor;
				if (ImGui::ColorEdit4("ClearColor", (float*)&clearColor))
					cam->clearColor = clearColor;
				int massLevel = cam->cameraRender.renderTarget.getMultisampleLevel() / 4;
				if (ImGui::Combo("MSAA", &massLevel, "None\0""4x\0""8x\0""16x\0")) {
					cam->cameraRender.renderTarget.setMultisampleLevel(massLevel * 4);
				}
				ostringstream otext;
				Matrix4f promat = cam->getProjectionMatrix();
				for (int i = 0; i < 4; i++) {
					otext << promat(i, 0) << ", " <<
						promat(i, 1) << ", " <<
						promat(i, 2) << ", " <<
						promat(i, 3) << ",\n";
				}
				string protext = otext.rdbuf()->str();
				ImGui::InputTextMultiline("Promat", &protext, { -1, 90 }, ImGuiInputTextFlags_ReadOnly);

				otext.clear();
				promat = cam->getViewMatrix();
				for (int i = 0; i < 4; i++) {
					otext << promat(i, 0) << ", " <<
						promat(i, 1) << ", " <<
						promat(i, 2) << ", " <<
						promat(i, 3) << ",\n";
				}
				string vtext = otext.rdbuf()->str();
				ImGui::InputTextMultiline("Matrix", &vtext, { -1, 90 }, ImGuiInputTextFlags_ReadOnly);
			}
			if (ImGui::CollapsingHeader("Animation")) {
				if (ImGui::Button("Add Select Camera Animation", { -1, 40 })) {
					if (assignAsset != NULL && assignAsset->assetInfo.type == "AnimationClipData") {
						cam->setAnimationClip(*(AnimationClipData*)assignAsset->load());
					}
				}
				static char inputName[100];
				static SkeletonMeshActor* chr = NULL;
				if (ImGui::InputText("Name", inputName, 100)) {
					chr = dynamic_cast<SkeletonMeshActor*>((Object*)Brane::find(typeid(Object).hash_code(), inputName));
				}
				static int activeAnimIndex = 0;
				static int activeAudioIndex = 0;
				if (chr != NULL) {
					string animStr;
					vector<unsigned int> animIndexes;
					if (chr->animationClipList.empty())
						activeAnimIndex = -1;
					else for (auto b = chr->animationClipList.begin(), e = chr->animationClipList.end(); b != e; b++) {
						animStr += b->first + '\0';
						animIndexes.push_back(b->second);
					}
					ImGui::Combo("Anim", &activeAnimIndex, animStr.c_str());

#ifdef AUDIO_USE_OPENAL
					string audioStr;
					if (chr->audioSources.empty())
						activeAudioIndex = -1;
					else for (auto b = chr->audioSources.begin(), e = chr->audioSources.end(); b != e; b++) {
						audioStr += (*b)->audioData->name + '\0';
					}
					ImGui::Combo("Audio", &activeAudioIndex, audioStr.c_str());
#endif // AUDIO_USE_OPENAL

					if (cam->animationClip.animationClipData != NULL && activeAnimIndex >= 0) {
						if (cam->animationClip.playing()) {
							if (ImGui::Button("Pause")) {
								cam->animationClip.pause();
								chr->activeAnimationClip(animIndexes[activeAnimIndex]);
								chr->animationClip->pause();
#ifdef AUDIO_USE_OPENAL
								if (activeAudioIndex >= 0)
									chr->audioSources[activeAudioIndex]->pause();
#endif // AUDIO_USE_OPENAL
							}
							if (ImGui::Button("Stop")) {
								cam->animationClip.stop();
								chr->activeAnimationClip(animIndexes[activeAnimIndex]);
								chr->animationClip->stop();
#ifdef AUDIO_USE_OPENAL
								if (activeAudioIndex >= 0)
									chr->audioSources[activeAudioIndex]->stop();
#endif // AUDIO_USE_OPENAL
							}
						}
						else if (ImGui::Button("Play")) {
							cam->animationClip.play();
							chr->activeAnimationClip(animIndexes[activeAnimIndex]);
							chr->animationClip->play();
#ifdef AUDIO_USE_OPENAL
							if (activeAudioIndex >= 0)
								chr->audioSources[activeAudioIndex]->play();
#endif // AUDIO_USE_OPENAL
						}
					}
				}
			}
			if (ImGui::CollapsingHeader("Postprocess")) {
				/*if (cam->cameraRender.material.isNull())
					ImGui::TextColored(ImVec4(0.4, 0.5, 1, 1), "No Material");
				else {
					multimap<string, unsigned int> names = { { "PostMaterial", 0 } };
					vector<Material*> mat = { &cam->cameraRender.material };
					showMaterial(names, mat, info.gui);
				}*/
				PostProcessingCamera* ppCam = dynamic_cast<PostProcessingCamera*>(cam);
				if (ppCam == NULL)
					ImGui::TextColored(ImVec4(0.4, 0.5, 1, 1), "No PostProcess");
				else {
					PostProcessGraph& graph = ppCam->postProcessCameraRender.graph;
					int i = 0;
					for (auto b = graph.passes.begin(), e = graph.passes.end(); b != e; b++, i++) {
						ImGui::PushID(i);
						if (ImGui::CollapsingHeader((*b)->getName().c_str())) {
							bool enable = (*b)->getEnable();
							if (ImGui::Checkbox("Enable", &enable))
								(*b)->setEnable(enable);
							multimap<string, unsigned int> names = { { "Material", 0 } };
							vector<Material*> mat = { (*b)->getMaterial() };
							showMaterial(names, mat, info.gui);
							(*b)->setMaterial(mat[0]);
						}
						ImGui::PopID();
					}
				}
			}
		}
		ImGui::EndGroup();
	}
}

void InspectorWindow::setTargetObject(Object & obj)
{
	target = &obj;
}

void InspectorWindow::showMaterial(Material * mat, Render* render)
{
	if (mat != NULL) {
		if (ImGui::CollapsingHeader("Material")) {
			if (ImGui::Button("Assign Material"))
				ImGui::OpenPopup("MatSelectPopup");
			ImGui::Text("Shader: %s", mat->getShaderName().c_str());
			ImGui::Checkbox("TwoSide", &mat->isTwoSide);
			ImGui::Checkbox("CastShadow", &mat->canCastShadow);
			for (auto b = mat->getColorField().begin(), e = mat->getColorField().end(); b != e; b++) {
				Color color = b->second.val;
				if (ImGui::ColorEdit4(b->first.c_str(), (float*)&color))
					b->second.val = color;
			}
			for (auto b = mat->getScalarField().begin(), e = mat->getScalarField().end(); b != e; b++) {
				float val = b->second.val;
				if (ImGui::DragFloat(b->first.c_str(), &val, 0.01))
					b->second.val = val;
			}
			static string choice;
			for (auto b = mat->getTextureField().begin(), e = mat->getTextureField().end(); b != e; b++) {
				if (b->first == "depthMap")
					continue;
				unsigned long long id = b->second.val->getTextureID();
				//if (id == 0)
				//	id = Texture2D::blackRGBDefaultTex.bind();
				if (ImGui::ImageButton((void*)id, { 64, 64 }, { 0, 1 }, { 1, 0 })) {
					//std::thread td = std::thread([](Material* mat, string path) {
					/*CFileDialog dialog(true, NULL, NULL, 6UL, _T("tga(*.tga)|*.tga|png(*.png)|*.png|jpg(*.jpg)|*.jpg|bmp(*.bmp)|*.bmp|All Files(*.*)|*.*"));
					if (dialog.DoModal() == IDOK) {
					Texture2D* tex = new Texture2D();
					if (tex->load(string(CT2A(dialog.GetPathName().GetString())))) {
					mat->setTexture(b->first, *tex);
					}
					else {
					delete tex;
					MessageBox(NULL, _T("Load Texture Failed"), _T("Error"), MB_OK);
					}
					}*/
					//}, mat, b->first);
					//td.join();
					ImGui::OpenPopup("TexSelectPopup");
					choice = b->first;
				}
				ImGui::SameLine();
				ImGui::Text(b->first.c_str());
			}
			if (ImGui::BeginPopup("MatSelectPopup")) {
				for (auto _b = MaterialAssetInfo::assetInfo.assets.begin(), _e = MaterialAssetInfo::assetInfo.assets.end();
					_b != _e; _b++) {
					if (ImGui::Selectable(_b->first.c_str(), false, 0, { 0, 20 })) {
						MeshRender* mR = dynamic_cast<MeshRender*>(render);
						if (mR != NULL)
							mR->setMaterial(0, *((Material*)_b->second->asset[0]));
					}
				}
				ImGui::EndPopup();
			}
			if (ImGui::BeginPopup("TexSelectPopup")) {
				for (auto _b = Texture2DAssetInfo::assetInfo.assets.begin(), _e = Texture2DAssetInfo::assetInfo.assets.end();
					_b != _e; _b++) {
					if (_b->second->asset[0] == NULL)
						continue;
					unsigned int id = ((Texture2D*)_b->second->asset[0])->getTextureID();
					if (id == 0)
						id = Texture2D::blackRGBDefaultTex.getTextureID();
					if (ImGui::Selectable(_b->first.c_str(), false, 0, { 0, 60 })) {
						mat->setTexture(choice, *((Texture2D*)_b->second->asset[0]));
					}
					ImGui::SameLine(100);
					ImGui::Image((void*)id, { 56, 56 });
				}
				ImGui::EndPopup();
			}
		}
	}
}

void InspectorWindow::showMaterial(multimap<string, unsigned int>& meshPartNames, vector<Material*>& materials, GUI& gui)
{
	if (ImGui::CollapsingHeader("Material")) {
		int ii = 0;
		for (auto mb = meshPartNames.begin(), me = meshPartNames.end(); mb != me; mb++, ii++) {
			Material* mat = materials[mb->second];
			ImGui::PushID(ii);
			if (ImGui::TreeNode(mb->first.c_str())) {
				static bool create = false;
				if (ImGui::Button("Assign Selected Material")) {
					if (assignAsset != NULL && assignAsset->assetInfo.type == "Material") {
						materials[mb->second] = (Material*)assignAsset->load();
					}
				}
				if (ImGui::Button("Assign Material"))
					ImGui::OpenPopup("MatSelectPopup");
				if (ImGui::Button("Create Material")) {
					ImGui::OpenPopup("MatSelectPopup");
					create = true;
				}
				if (mat == NULL) {
					ImGui::Text("No Material");
				}
				else {
					if (ImGui::Button("Save Material")) {
						thread td = thread([](Material* mat) {
							FileDlgDesc desc;
							desc.title = "imat";
							desc.filter = "imat(*.imat)\0*.imat\0";
							filesystem::path ep = Engine::windowContext.executionPath;
							desc.initDir = ep.replace_filename("Content").generic_u8string().c_str();
							desc.save = true;
							desc.defFileExt = "imat";
							desc.addToRecent = false;
							if (openFileDlg(desc)) {
								if (!Material::MaterialLoader::saveMaterialInstance(desc.filePath, *mat))
									MessageBox(NULL, "Save failed", "Error", MB_OK);
								filesystem::path p = desc.filePath;
								filesystem::path ep = Engine::windowContext.executionPath;
								string filename = p.filename().string();
								vector<string> ps = split(p.relative_path().make_preferred().remove_filename().string(), '\\');
								vector<string> eps = split(ep.relative_path().make_preferred().remove_filename().string(), '\\');
								bool found = false;
								string rpath;
								if (ps.size() > eps.size()) {
									for (int i = 0; i < ps.size(); i++) {
										if (i < eps.size()) {
											if (ps[i] != eps[i])
												break;
										}
										else if (found)
											rpath += ps[i] + '/';
										else if (ps[i] == "Content" || ps[i] == "Engine") {
											found = true;
											rpath += ps[i] + '/';
										}
										else
											break;
									}
								}
								if (!found) {
									MessageBox(NULL, "File not in engine workfolder", "Warning", MB_OK);
									return;
								}
								rpath += filename;
								Asset *asset = new Asset(&MaterialAssetInfo::assetInfo, filename, rpath);
								asset->asset[0] = mat;
								MaterialAssetInfo::assetInfo.regist(*asset);
							}
						}, mat);
						td.detach();
					}
					ImGui::Text("Shader: %s", mat->getShaderName().c_str());
					ImGui::Checkbox("TwoSide", &mat->isTwoSide);
					ImGui::Checkbox("CastShadow", &mat->canCastShadow);
					for (auto b = mat->getColorField().begin(), e = mat->getColorField().end(); b != e; b++) {
						Color color = b->second.val;
						if (ImGui::ColorEdit4(b->first.c_str(), (float*)&color, ImGuiColorEditFlags_HDR))
							b->second.val = color;
					}
					for (auto b = mat->getScalarField().begin(), e = mat->getScalarField().end(); b != e; b++) {
						float val = b->second.val;
						if (ImGui::DragFloat(b->first.c_str(), &val, 0.01))
							b->second.val = val;
					}
					for (auto b = mat->getCountField().begin(), e = mat->getCountField().end(); b != e; b++) {
						int val = b->second.val;
						if (ImGui::DragInt(b->first.c_str(), &val))
							b->second.val = val;
					}
					static string choice;
					ImGui::Columns(2, "TextureColumn", false);
					for (auto b = mat->getTextureField().begin(), e = mat->getTextureField().end(); b != e; b++) {
						if (b->first == "depthMap")
							continue;
						ImGui::PushID(b._Ptr);
						unsigned long long id = b->second.val->getTextureID();
						if (ImGui::ImageButton((void*)id, { 64, 64 }, { 0, 1 }, { 1, 0 })) {
							//ImGui::OpenPopup("TexSelectPopup");
							TextureViewer::showTexture(gui, *b->second.val);
							choice = b->first;
						}
						ImGui::NextColumn();
						ImGui::Text(b->first.c_str());
						if (ImGui::ArrowButton("AssignTexBT", ImGuiDir_Left)) {
							if (assignAsset != NULL && assignAsset->assetInfo.type == "Texture2D") {
								mat->setTexture(b->first, *(Texture2D*)assignAsset->load());
							}
						}
						ImGui::NextColumn();
						ImGui::PopID();
					}
					ImGui::Columns();
				}
				if (ImGui::BeginPopup("MatSelectPopup")) {
					for (auto _b = MaterialAssetInfo::assetInfo.assets.begin(), _e = MaterialAssetInfo::assetInfo.assets.end();
						_b != _e; _b++) {
						if (ImGui::Selectable(_b->first.c_str(), false, 0, { 0, 20 })) {
							if (create)
								materials[mb->second] = &((Material*)_b->second->asset[0])->instantiate();
							else
								materials[mb->second] = (Material*)_b->second->asset[0];
						}
					}
					ImGui::EndPopup();
				}
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
	}
}
