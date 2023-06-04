#include <ShlObj.h>
#include <filesystem>
#include "AnimationConverter.h"
#include "../../Core/Skeleton/TwoBoneIKConstraint.h"
#include "../../Core/Skeleton/BoneParentConstraint.h"
#include "../../Core/Engine.h"
#include "../../Core/SkeletonMeshActor.h"
#include "../../Core/Importer/AssimpImporter.h"
#include "../../Core/WUI/ProgressUI.h"
#include "../../ThirdParty/ImGui/imgui_stdlib.h"
#include "../../Core/GUI/SerializationEditor.h"
#include "utf8/SJ2UTF8Table.h"
#include "../../ThirdParty/ImGui/imgui_internal.h"

namespace ImGui {

	bool BufferingBar(const char* label, float value, const ImVec2& size_arg, const ImU32& bg_col, const ImU32& fg_col) {
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(label);

		ImVec2 pos = window->DC.CursorPos;
		ImVec2 size = size_arg;
		size.x -= style.FramePadding.x * 2;

		const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
		ItemSize(bb, style.FramePadding.y);
		if (!ItemAdd(bb, id))
			return false;

		// Render
		const float circleStart = size.x * 0.7f;
		const float circleEnd = size.x;
		const float circleWidth = circleEnd - circleStart;

		window->DrawList->AddRectFilled(bb.Min, ImVec2(pos.x + circleStart, bb.Max.y), bg_col);
		window->DrawList->AddRectFilled(bb.Min, ImVec2(pos.x + circleStart*value, bb.Max.y), fg_col);

		const float t = g.Time;
		const float r = size.y / 2;
		const float speed = 1.5f;

		const float a = speed * 0;
		const float b = speed*0.333f;
		const float c = speed*0.666f;

		const float o1 = (circleWidth + r) * (t + a - speed * (int)((t + a) / speed)) / speed;
		const float o2 = (circleWidth + r) * (t + b - speed * (int)((t + b) / speed)) / speed;
		const float o3 = (circleWidth + r) * (t + c - speed * (int)((t + c) / speed)) / speed;

		window->DrawList->AddCircleFilled(ImVec2(pos.x + circleEnd - o1, bb.Min.y + r), r, bg_col);
		window->DrawList->AddCircleFilled(ImVec2(pos.x + circleEnd - o2, bb.Min.y + r), r, bg_col);
		window->DrawList->AddCircleFilled(ImVec2(pos.x + circleEnd - o3, bb.Min.y + r), r, bg_col);
	}

	bool Spinner(const char* label, float radius, int thickness, const ImU32& color) {
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(label);

		ImVec2 pos = window->DC.CursorPos;
		ImVec2 size((radius) * 2, (radius + style.FramePadding.y) * 2);

		const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
		ItemSize(bb, style.FramePadding.y);
		if (!ItemAdd(bb, id))
			return false;

		// Render
		window->DrawList->PathClear();

		int num_segments = 30;
		int start = abs(ImSin(g.Time*1.8f)*(num_segments - 5));

		const float a_min = IM_PI*2.0f * ((float)start) / (float)num_segments;
		const float a_max = IM_PI*2.0f * ((float)num_segments - 3) / (float)num_segments;

		const ImVec2 centre = ImVec2(pos.x + radius, pos.y + radius + style.FramePadding.y);

		for (int i = 0; i < num_segments; i++) {
			const float a = a_min + ((float)i / (float)num_segments) * (a_max - a_min);
			window->DrawList->PathLineTo(ImVec2(centre.x + ImCos(a + g.Time * 8) * radius,
				centre.y + ImSin(a + g.Time * 8) * radius));
		}

		window->DrawList->PathStroke(color, false, thickness);
	}

}

AnimationConverter::AnimationConverter(string name, bool defaultShow)
	: UIWindow(*Engine::getCurrentWorld(), name, defaultShow)
{
	baseToonMaterial = getAssetByPath<Material>("Engine/Shaders/Toon.mat");
	basePBRMaterial = getAssetByPath<Material>("Engine/Shaders/PBR.mat");
	outlineMaterial = getAssetByPath<Material>("Engine/Shaders/Outline.mat");
}

AnimationConverter::~AnimationConverter()
{
	if (scene != NULL)
		aiReleaseImport(scene);
	if (fbxInfo != NULL)
		delete fbxInfo;
	if (vmdMotion != NULL)
		delete vmdMotion;
	if (vmdInfo != NULL)
		delete vmdInfo;
}

void AnimationConverter::onRenderWindow(GUIRenderInfo & info)
{
	if (stage == 0) {
		showVmdLoad();
		showPmxLoad();
		if (ImGui::Button("Use Temp Data")) {
			stage = 2;
		}
	}
	else if (stage == 1) {
		ImGui::Spinner("Wait...", 50, 10, ImColor(200, 200, 200));

		DWORD dw;
		dw = WaitForSingleObject(processInfo.hProcess, 0);
		if (dw == WAIT_OBJECT_0) {
			stage = 2;
			filePath = tempPath + "pmx.fbx";
			CloseHandle(processInfo.hThread);
			CloseHandle(processInfo.hProcess);
		}
	}
	else if (stage == 2) {
		if (subStage == -1) {
			subStage = 0;
			thread td = thread([this]() {
				/*if (scene != NULL)
					aiReleaseImport(scene);
				if (fbxInfo != NULL) {
					delete fbxInfo;
					fbxInfo = NULL;
				}
				aiPropertyStore* props = aiCreatePropertyStore();
				aiSetImportPropertyInteger(props, AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);
				aiSetImportPropertyInteger(props, AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, 0);
				aiSetImportPropertyInteger(props, AI_CONFIG_IMPORT_REMOVE_EMPTY_BONES, 1);
				scene = aiImportFileExWithProperties((tempPath + "pmx.fbx").c_str(), aiProcessPreset_TargetRealtime_MaxQuality, NULL, props);
				aiReleasePropertyStore(props);
				subStage = 1;
				if (scene != NULL) {
					fbxInfo = new SerializationInfo[scene->mNumAnimations];
					for (int i = 0; i < scene->mNumAnimations; i++)
						serializeBoneAnimation(fbxInfo[i], *scene->mAnimations[i]);
				}
				subStage = 2;
				if (vmdMotion != NULL)
					delete vmdMotion;
				if (vmdInfo != NULL) {
					delete vmdInfo;
					vmdInfo = NULL;
				}
				vmdMotion = vmd::VmdMotion::LoadFromFile((tempPath + "vmd.vmd").c_str());
				subStage = 3;
				if (vmdMotion != NULL) {
					vmdInfo = new SerializationInfo;
					serializeMorphAnimation(*vmdInfo, *vmdMotion);
				}
				subStage = 4;*/
				if (animationData != NULL)
					delete animationData;
				AssimpImporter imp(tempPath + "pmx.fbx");
				if (imp.isLoad()) {
					vector<AnimationClipData*> anims;
					subStage = 1;
					if (imp.getAnimation(anims)) {
						subStage = 2;
						animationData = anims[0];
						vmdMotion = vmd::VmdMotion::LoadFromFile((tempPath + "vmd.vmd").c_str());
						subStage = 3;
						if (vmdMotion != NULL) {
							getMorphAnimation(*animationData, *vmdMotion);
						}
					}
				}
				else
					Console::error("Impoter: %s", imp.getError().c_str());
				subStage = 4;
			});
			td.detach();
		}
		else if (subStage == 4) {
			subStage = -1;
			stage = 3;
		}
		else {
			const char* str;
			switch (subStage)
			{
			case 0: str = "Load Fbx"; break;
			case 1: str = "Load Bone Animation"; break;
			case 2: str = "Load Vmd"; break;
			case 3: str = "Load Morph Animation"; break;
			default: str = "Error"; break;
			}
			ImGui::Text(str);
			ImGui::SameLine();
			ImGui::Spinner("Wait...", 10, 5, ImColor(200, 200, 200));
			ImGui::BufferingBar("Wait...2", subStage / 4.0f, { 200, 5 }, ImColor(20, 20, 20), ImColor(200, 200, 200));
		}
	}
	else if (stage == 3) {
		if (ImGui::Button("Save Animation")) {
			thread td = thread([this]() {
				FileDlgDesc desc;
				desc.title = "Save Animation";
				desc.filter = "charanim(*.charanim)|*.charanim";
				desc.initDir = "Content";
				desc.defFileExt = "charanim";
				desc.save = true;
				if (openFileDlg(desc)) {
					if (fbxInfo != NULL && vmdInfo != NULL) {
						ofstream ofs = ofstream(desc.filePath);
						SerializationInfoWriter w = SerializationInfoWriter(ofs);
						ofs << "Motion { ";
						if (fbxInfo != NULL) {
							ofs << fbxInfo->name << ": ";
							w.write(*fbxInfo);
							ofs << ", ";
						}
						if (vmdInfo != NULL) {
							ofs << vmdInfo->name << ": ";
							w.write(*vmdInfo);
						}
						ofs << " }";
						ofs.close();
					}
					if (animationData != NULL) {
						struct AnimationWriterInfo
						{
							AnimationClipData* data;
							string path;
							ProgressUI ui = ProgressUI("Write Animation");
							bool result;
						};
						AnimationWriterInfo info = { animationData, desc.filePath };
						info.ui.doModelAsync([](WUIControl& ctrl, void* ptr) {
							AnimationWriterInfo* info = (AnimationWriterInfo*)ptr;
							info->result = AnimationLoader::writeAnimation(*info->data, info->path, &info->ui.work);
							}, & info);
						if (!info.result) {
							MessageBox(NULL, "Serialize failed", "Error", MB_OK);
						}
						info.ui.close();
					}
				}
			});
			td.detach();
		}
		ImGui::BeginChild("Scene", { 0, 0 });
		if (scene != NULL && ImGui::CollapsingHeader("FBX")) {
			if (fbxInfo == NULL) {
				if (ImGui::Button("SerializeFbx", { -1, 36 })) {
					fbxInfo = new SerializationInfo[scene->mNumAnimations];
					for (int i = 0; i < scene->mNumAnimations; i++)
						serializeBoneAnimation(fbxInfo[i], *scene->mAnimations[i]);
				}
			}
			else {
				for (int i = 0; i < scene->mNumAnimations; i++) {
					string bstr = "Show ";
					bstr += scene->mAnimations[i]->mName.C_Str();
					if (ImGui::Button(bstr.c_str(), { -1, 36 })) {
						SerializationEditor::showSerializationInfo(info.gui, fbxInfo[i]);
					}
				}
			}
			showAIScene(scene);
		}
		if (vmdMotion != NULL && ImGui::CollapsingHeader("VMD")) {
			if (vmdInfo == NULL) {
				if (ImGui::Button("SerializeVmd", { -1, 36 })) {
					vmdInfo = new SerializationInfo;
					serializeMorphAnimation(*vmdInfo, *vmdMotion);
				}
			}
			else {
				if (ImGui::Button("ShowVmdInfo", { -1, 36 })) {
					SerializationEditor::showSerializationInfo(info.gui, *vmdInfo);
				}
			}
			showVMDMorph(vmdMotion);
		}
		ImGui::EndChild();
	}
}

void AnimationConverter::showFBXLoad()
{
	if (ImGui::Button("ChooseFbx")) {
		thread td = thread([](string* str) {
			FileDlgDesc desc;
			desc.title = "Browse";
			desc.filter = "fbx(*.fbx)|*.fbx";
			desc.defFileExt = "fbx";
			if (openFileDlg(desc)) {
				*str = desc.filePath;
			}
		}, &filePath);
		td.detach();
	}
	ImGui::SameLine();
	ImGui::InputText("Fbx", &filePath);
	if (ImGui::Button("LoadFbx", { -1, 40 })) {
		if (scene != NULL)
			aiReleaseImport(scene);
		if (fbxInfo != NULL) {
			delete fbxInfo;
			fbxInfo = NULL;
		}
		aiPropertyStore* props = aiCreatePropertyStore();
		aiSetImportPropertyInteger(props, AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);
		aiSetImportPropertyInteger(props, AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, 0);
		aiSetImportPropertyInteger(props, AI_CONFIG_IMPORT_REMOVE_EMPTY_BONES, 1);
		scene = aiImportFileExWithProperties(filePath.c_str(), aiProcessPreset_TargetRealtime_MaxQuality, NULL, props);
		aiReleasePropertyStore(props);
	}
}

void AnimationConverter::showVmdLoad()
{
	if (ImGui::Button("ChooseVmd")) {
		thread td = thread([](string* str) {
			FileDlgDesc desc;
			desc.title = "Browse";
			desc.filter = "vmd(.vmd)|*.vmd";
			desc.defFileExt = "vmd";
			if (openFileDlg(desc)) {
				*str = desc.filePath;
			}
		}, &vmdPath);
		td.detach();
	}
	ImGui::SameLine();
	ImGui::InputText("Vmd", &vmdPath);
	ImGui::InputFloat("CameraMotionScale", &camMotionScale);
	ImGui::SameLine();
	ImGui::InputFloat("CameraFovScale", &camFovScale);
	if (ImGui::Button("ConvertVmd", { -1, 40 })) {
		if (vmdMotion != NULL)
			delete vmdMotion;
		if (vmdInfo != NULL) {
			delete vmdInfo;
			vmdInfo = NULL;
		}
		vmdMotion = vmd::VmdMotion::LoadFromFile(vmdPath.c_str());
		if (vmdMotion == NULL) {
			MessageBoxA(NULL, "Load vmd failed", "Error", MB_OK);
		}
		else {
			FileDlgDesc desc;
			desc.title = "Browse";
			desc.filter = "anim(.anim)|*.anim";
			desc.defFileExt = "anim";
			desc.save = true;
			if (openFileDlg(desc)) {
				ofstream file(filesystem::u8path(desc.filePath), ios::binary);
				if (file.fail()) {
					MessageBoxA(NULL, "Save failed", "Error", MB_OK);
				}
				else {
					AnimationClipData data("anim");
					getAnimation(data, *vmdMotion, camMotionScale, camFovScale);
					struct AnimationInfo
					{
						AnimationClipData& data;
						ofstream& file;
						ProgressUI ui = ProgressUI("Write Animation");
					};
					AnimationInfo info = { data, file };
					info.ui.doModelAsync([](WUIControl& ctrl, void* ptr) {
						AnimationInfo* info = (AnimationInfo*)ptr;
						info->data.write(info->file, &info->ui.work);
					}, & info);
					MessageBoxA(NULL, "Save Complete", "Info", MB_OK);
					info.ui.close();
				}
			}
		}
	}
}

void AnimationConverter::showPmxLoad()
{
	if (ImGui::Button("ChoosePmx")) {
		thread td = thread([](string* str) {
			FileDlgDesc desc;
			desc.title = "Browse";
			desc.filter = "pmx(.pmx)|*.pmx";
			desc.defFileExt = "pmx";
			if (openFileDlg(desc)) {
				*str = desc.filePath;
			}
		}, &pmxPath);
		td.detach();
	}
	ImGui::SameLine();
	ImGui::InputText("Pmx", &pmxPath);
	ImGui::InputFloat("BoneScale", &boneScale, 0.01f);
	if (ImGui::Button("Pmx2Fbx", { -1, 40 })) {
		if (!vmdPath.empty() && !pmxPath.empty()) {
			prepare();
		}
	}

	if (ImGui::Button("TargetMatPath")) {
		thread td = thread([](string* str) {
			FolderDlgDesc desc;
			filesystem::path contentPath = filesystem::current_path();
			contentPath += "\\Content\\";
			desc.initDir = contentPath.generic_u8string();
			desc.title = "Choose Target Folder";
			if (openFolderDlg(desc))
				*str = desc.folderPath;
		}, &targetMatPath);
		td.detach();
	}
	ImGui::SameLine();
	ImGui::InputText("Target", &targetMatPath);
	ImGui::Checkbox("Encoding", &encoding);
	ImGui::SameLine();
	ImGui::Checkbox("Physics", &enablePhysics);
	ImGui::SameLine();
	ImGui::Checkbox("IK", &enableIK);
	ImGui::Checkbox("PBR", &enablePBRMat);
	
	if (ImGui::Button("GenerateMaterial", { -1, 40 })) {
		if (!pmxPath.empty()) {
			if (pmxModel != NULL)
				delete pmxModel;
			pmxModel = new pmx::PmxModel;
			pmxModel->setting.encoding = 1;
			filesystem::path ppmxPath = filesystem::u8path(pmxPath);
			ifstream f = ifstream(ppmxPath, ios::binary | ios::in);
			if (f.fail()) {
				MessageBoxA(NULL, "Load pmx failed", "Error", MB_OK);
			}
			else {
				pmxModel->Read(&f);
				f.close();
				generateAsset();
			}
		}
	}

	static string toonFile;
	if (ImGui::Button("ChooseToon")) {
		thread td = thread([](string* str) {
			FileDlgDesc desc;
			desc.title = "Browse";
			desc.filter = "bmp(*.bmp)|*.bmp";
			desc.defFileExt = "bmp";
			if (openFileDlg(desc)) {
				*str = desc.filePath;
			}
		}, &toonFile);
		td.detach();
	}
	ImGui::SameLine();
	ImGui::InputText("Toon", &toonFile);
	if (ImGui::Button("Test", { -1, 40 })) {
		if (!toonFile.empty()) {
			ToonParameter tp;
			getToonParameter(toonFile, tp);
		}
	}
}

void AnimationConverter::prepare()
{
	if (!filesystem::exists(tempPath))
		CreateDirectoryA(tempPath.c_str(), NULL);
	filesystem::path ppmxPath = filesystem::u8path(pmxPath);
	filesystem::path pvmdPath = filesystem::u8path(vmdPath);
	filesystem::path ptempPath = filesystem::u8path(tempPath);
	filesystem::path ptempPmxPath = ptempPath;
	ptempPmxPath += "pmx.pmx";
	filesystem::path ptempVmdPath = ptempPath;
	ptempVmdPath += "vmd.vmd";
	if (!CopyFileW(ppmxPath.generic_wstring().c_str(), ptempPmxPath.generic_wstring().c_str(), 0))
		Console::error("Fail to copy %s", pmxPath.c_str());
	if (!CopyFileW(pvmdPath.generic_wstring().c_str(), ptempVmdPath.generic_wstring().c_str(), 0))
		Console::error("Fail to copy %s", vmdPath.c_str());
	STARTUPINFOA si = { sizeof(si) };	
	sprintf_s(execCmdChars, "%s %spmx.pmx %svmd.vmd", pmx2fbxPath.c_str(), tempPath.c_str(), tempPath.c_str());
	if (!CreateProcessA(NULL, execCmdChars, NULL, NULL, false, 0, NULL, NULL, &si, &processInfo)) {
		Console::error("Fail to launch %s", pmx2fbxPath.c_str());
		return;
	}
	stage = 1;
}

Vector3f toVector3f(float* v)
{
	return Vector3f(v[0], v[1], v[2]);
}

Vector3f toVector3fInvZ(float* v)
{
	return Vector3f(v[0], v[1], -v[2]);
}

Vector3f convertPosition(float* v)
{
	return Vector3f(v[0], v[1], v[2]);
}

Vector3f convertEular(float* v)
{
	return Vector3f(v[0], v[1], v[2]);
}

Vector3f convertEular(const Vector3f& v)
{
	return Vector3f(v.x(), v.y(), v.z());
}

bool testParent(Object* child, Object* parent)
{
	if (child == NULL || parent == NULL)
		return false;
	child = child->parent;
	while (child != NULL) {
		if (child == parent)
			return true;
		child = child->parent;
	}
	return false;
}

string to_hex(const string& str)
{
	string out = string(str.length() * 2, '0');
	for (int i = 0; i < str.length(); i++) {
		char hex[2];
		sprintf_s(hex, "%2x", str[i]);
		out[i * 2] = hex[0];
		out[i * 2 + 1] = hex[1];
	}
	return out;
}

void AnimationConverter::generateAsset()
{
	typedef filesystem::path FPath;
	namespace FS = filesystem;
	if (pmxModel == NULL || targetMatPath.empty())
		return;

	SerializationInfo info;
	info.type = "SkeletonMeshActor";
	info.add("materials");
	info.add("outlines");
	info.set("position", SVector3f(0, 0, 0));
	info.set("rotation", SVector3f(90, 0, 0));
	info.set("scale", SVector3f(boneScale, boneScale, boneScale));
	SerializationInfo& minfo = *info.get("materials");
	minfo.type = "MaterialMap";
	SerializationInfo& outlineinfo = *info.get("outlines");
	outlineinfo.type = "OutlineMap";

	FPath executionPath = filesystem::current_path();
	FPath folderPath = targetMatPath.substr(executionPath.generic_u8string().length() + 1);
	string folderName = folderPath.filename().generic_u8string();
	FPath ppath = FS::u8path(pmxPath);
	set<string> texturePaths;
	for (int i = 0; i < pmxModel->material_count; i++) {
		pmx::PmxMaterial& mat = pmxModel->materials[i];
		//string name = encoding ? sj2utf8(mat.material_name) : mat.material_name;// sjDecode ? sj2utf8(mat.material_name) : mat.material_name;
		string name = mat.material_name;
		SerializationInfo* mi = minfo.add(name);
		SerializationInfo* oi = outlineinfo.add(name);
		if (mi == NULL)
			continue;
		if (oi == NULL)
			continue;
		ToonParameter tp;
		if (mat.common_toon_flag == 0) {
			if (mat.toon_texture_index != -1) {
				FPath p = ppath;
				p.replace_filename(FS::u8path(pmxModel->textures[mat.toon_texture_index]));
				getToonParameter(p.generic_u8string(), tp);
			}
		}

		if (mat.diffuse_texture_index != -1) {
			string texPath = pmxModel->textures[mat.diffuse_texture_index];
			//if (encoding)
			//	texPath = sj2utf8(texPath);
			if (texturePaths.find(texPath) == texturePaths.end())
				texturePaths.insert(texPath);

			FPath texFileName = FS::u8path(folderName + '_' + FS::u8path(texPath).filename().generic_u8string());
			FPath targetTexPath = folderPath;
			targetTexPath /=  texFileName;
			tp.texPath = targetTexPath.generic_u8string();
		}

		bool enableOutline = mat.edge_size > 0;
		string imat;
		oi->set("enable", enableOutline ? "true" : "false");
		if (enableOutline) {
			OutlineParameter op;

			op.baseColor.r = mat.edge_color[0] * 0.35;
			op.baseColor.g = mat.edge_color[1] * 0.35;
			op.baseColor.b = mat.edge_color[2] * 0.35;
			op.baseColor.a = 1.0f;
			op.refenceDistance = 0;
			op.borderWidth = mat.edge_size * 0.02f;

			MaterialInfo info = MaterialInfo(*outlineMaterial);
			info.baseMatPath = "Engine/Shaders/Outline.mat";
			info.setOutlineParameter(op);

			ostringstream stream;
			info.write(stream);
			imat = stream.str();
		}
		oi->set("material", imat);

		FPath matPath;

		if (enablePBRMat) {
			MaterialInfo pbrInfo = MaterialInfo(*basePBRMaterial);
			pbrInfo.baseMatPath = "Engine/Shaders/PBR.mat";
			pbrInfo.setPBRParameter(tp);

			matPath = folderPath / FS::u8path(folderName + '_' + name + ".imat");

			ofstream pf = ofstream(matPath);
			if (!pf.fail()) {
				pbrInfo.write(pf);
				pf.close();
			}
		}
		else {
			MaterialInfo toonInfo = MaterialInfo(*baseToonMaterial);
			toonInfo.baseMatPath = "Engine/Shaders/Toon.mat";
			toonInfo.setToonParameter(tp);

			matPath = folderPath / FS::u8path(folderName + '_' + name + ".imat");

			ofstream tf = ofstream(matPath);
			if (!tf.fail()) {
				toonInfo.write(tf);
				tf.close();
			}
		}

		mi->type = "AssetSearch";
		mi->add("path", matPath.generic_u8string());
		mi->add("pathType", "path");

		/*Console::log("Material: %s", path.c_str());
		Console::log("BaseColor: %f, %f, %f, %f", tp.baseColor.r, tp.baseColor.g, tp.baseColor.b, tp.baseColor.a);
		Console::log("ShadowColor: %f, %f, %f, %f", tp.shadowColor.r, tp.shadowColor.g, tp.shadowColor.b, tp.shadowColor.a);
		Console::log("ShadowCut: %f, ShadowSmooth: %f", tp.shadowCut, tp.shadowSmooth);*/
	}

	enum BoneFlag
	{
		Tail = 1 << 0, Rotatable = 1 << 1, Translatable = 1 << 2, Visible = 1 << 3, Enabled = 1 << 4, IK = 1 << 5,
		InheritRotation = 1 << 8, InheritTranslation = 1 << 9, FixedAxis = 1 << 10, LocalCoordinate = 1 << 11,
		PhysicsAfterDeform = 1 << 12, ExternalParentDeform = 1 << 13
	};

	const char* BoneFlags[] = {
		"Tail", "Rotatable", "Translatable", "Visible", "Enabled", "IK", "", "",
		"InheritRotation", "InheritTranslation", "FixedAxis", "LocalCoordinate",
		"PhysicsAfterDeform", "ExternalParentDeform"
	};

	AssimpImporter imp = AssimpImporter(pmxPath);
	if (imp.isLoad()) {
		SkeletonMesh skm = SkeletonMesh();
		if (imp.getSkeletonMesh(skm)) {
			SkeletonMeshActor skma = SkeletonMeshActor(skm, *baseToonMaterial);
			vector<string> TranslatableBones;
			if (enableIK) {
				SerializationInfo& boneConstraintsInfo = *info.add("boneConstraints");
				for (int bi = 0; bi < pmxModel->bone_count; bi++) {
					pmx::PmxBone& bone = pmxModel->bones[bi];
					Bone* skBone = skma.getBone(bone.bone_name);
					if (skBone == NULL)
						continue;
					string boneInfo = "----------------\nbone: " + bone.bone_name;
					boneInfo += "\nflags: ";
					for (int i = 0; i < 14; i++) {
						if (bone.bone_flag & (1 << i)) {
							boneInfo += '[' + string(BoneFlags[i]) + ']';
						}
					}
					if (bone.bone_flag & BoneFlag::Translatable) {
						TranslatableBones.push_back(bone.bone_name);
					}
					if (bone.bone_flag & BoneFlag::IK) {
						pmx::PmxBone& ikTargetBone = pmxModel->bones[bone.ik_target_bone_index];
						boneInfo += "\nikTarget: " + ikTargetBone.bone_name;
						boneInfo += "\nLink: ";

						vector<Bone*> linekBones;
						linekBones.resize(bone.ik_link_count);
						for (int i = 0; i < bone.ik_link_count; i++) {
							pmx::PmxIkLink& link = bone.ik_links[i];
							pmx::PmxBone& linkBone = pmxModel->bones[link.link_target];
							boneInfo += '[' + linkBone.bone_name + ']';
							linekBones[i] = skma.getBone(linkBone.bone_name);
						}

						if (bone.ik_link_count == 1) {
							Bone* aBone = linekBones[0];
							Bone* bBone = skma.getBone(ikTargetBone.bone_name);

							if (aBone != NULL && bBone != NULL) {

							}
						}
						else if (bone.ik_link_count == 2) {
							Bone* aBone = linekBones[1];
							Bone* bBone = linekBones[0];
							Bone* cBone = skma.getBone(ikTargetBone.bone_name);

							if (aBone != NULL && bBone != NULL && cBone != NULL) {
								TwoBoneIKConstraint ik;
								ik.setSerializeUseBoneName(true);
								ik.rootBone = aBone;
								ik.midBone = bBone;
								ik.endBone = cBone;
								ik.effectTransform = skBone;
								ik.poleSpace = LOCAL;

								ik.setup();

								SerializationInfo& boneConstraintInfo = *boneConstraintsInfo.add(
									to_string(boneConstraintsInfo.sublists.size()));
								ik.serialize(boneConstraintInfo);
							}
						}

					}

					bool inheritRotation = bone.bone_flag & BoneFlag::InheritRotation;
					bool inheritTranslation = bone.bone_flag & BoneFlag::InheritTranslation;
					if (inheritRotation || inheritTranslation) {
						pmx::PmxBone& parentBone = pmxModel->bones[bone.grant_parent_index];
						Bone* pBone = skma.getBone(parentBone.bone_name);
						if (pBone != NULL) {
							BoneParentConstraint pc;
							pc.setSerializeUseBoneName(true);
							pc.parentBone = pBone;
							pc.childBone = skBone;
							pc.weight = bone.grant_weight;

							if (inheritRotation)
								pc.flags |= BoneParentConstraint::Rotation;
							if (inheritTranslation)
								pc.flags |= BoneParentConstraint::Position;

							pc.setup();

							SerializationInfo& boneConstraintInfo = *boneConstraintsInfo.add(
								to_string(boneConstraintsInfo.sublists.size()));
							pc.serialize(boneConstraintInfo);
						}
					}
					Console::log(boneInfo.c_str());
				}
				string boneTransinfo = "\n";
				for (int i = 0; i < TranslatableBones.size(); i++) {
					boneTransinfo += '"' + TranslatableBones[i] + "\",\n";
				}

				Console::log(boneTransinfo.c_str());
			}
			if (enablePhysics) {
				SkeletonPhysics::SkeletonPhysicsInfo physicsInfo;

				for (int ri = 0; ri < pmxModel->rigid_body_count; ri++) {
					pmx::PmxRigidBody& rigid = pmxModel->rigid_bodies[ri];
					if (rigid.target_bone < 0 || rigid.target_bone >= pmxModel->bone_count)
						continue;
					pmx::PmxBone& bone = pmxModel->bones[rigid.target_bone];
					Bone* bbone = skma.getBone(bone.bone_name);
					if (bbone == NULL)
						continue;

					physicsInfo.rigidbodies.emplace_back();
					SkeletonPhysics::RigidBodyInfo& rigidBodyInfo = physicsInfo.rigidbodies.back();

					Vector3f bonePos = bbone->getPosition(WORLD);
					Vector3f pos = Vector3f(rigid.position[0], rigid.position[1], -rigid.position[2]);
					Quaternionf boneQuat = bbone->getRotation(WORLD);
					Quaternionf quat = Quaternionf::FromEularAngles(Vector3f(rigid.orientation[0],
						rigid.orientation[1], rigid.orientation[2]));

					quat.z() *= -1;
					quat.w() *= -1;

					Vector3f zUnit = quat * Vector3f::UnitZ();

					rigidBodyInfo.boneName = bbone->name;
					rigidBodyInfo.material.mass = rigid.mass * 10000;
					rigidBodyInfo.material.physicalType = rigid.physics_calc_type > 0 ? PhysicalType::DYNAMIC : PhysicalType::STATIC;
					rigidBodyInfo.material.angularDamping = rigid.rotation_attenuation * 0.5f;
					rigidBodyInfo.material.linearDamping = rigid.move_attenuation * 0.5f;
					rigidBodyInfo.material.friction = rigid.friction;
					rigidBodyInfo.material.restitution = rigid.repulsion;
					rigidBodyInfo.layer.layer = rigid.group;
					rigidBodyInfo.layer.ignoreMask = 0x0000ffff & (~(uint32_t)rigid.mask);

					Shape* shape = NULL;
					switch (rigid.shape)
					{
					case 0: // Sphere
						shape = new Sphere(rigid.size[0]);
						break;
					case 1: // Box
						shape = new Box(Vector3f(-rigid.size[0], -rigid.size[1], -rigid.size[2]), Vector3f(rigid.size[0], rigid.size[1], rigid.size[2]));
						break;
					case 2: // Capsule
					{
						quat = quat * Quaternionf::FromTwoVectors(Vector3f::UnitY(), Vector3f::UnitZ());
						shape = new Capsule(rigid.size[0], rigid.size[1] * 0.5f);
						break;
					}
					default:
						break;
					}

					rigidBodyInfo.positionOffset = pos - bonePos;
					rigidBodyInfo.rotationOffset = boneQuat.inverse() * quat;
					if (isnan(rigidBodyInfo.rotationOffset.x()))
						rigidBodyInfo.rotationOffset.x() = 0;
					if (isnan(rigidBodyInfo.rotationOffset.y()))
						rigidBodyInfo.rotationOffset.y() = 0;
					if (isnan(rigidBodyInfo.rotationOffset.z()))
						rigidBodyInfo.rotationOffset.z() = 0;
					if (isnan(rigidBodyInfo.rotationOffset.w()))
						rigidBodyInfo.rotationOffset.w() = 1;

					rigidBodyInfo.shape.fromShape(shape);
					delete shape;

					if (rigid.physics_calc_type == 2) {
						Transform* parent = dynamic_cast<Transform*>(bbone->parent);
						if (parent != NULL) {
							SkeletonPhysics::D6ConstraintInfo& constraintInfo = physicsInfo.constraints.emplace_back();
							/*Vector3f ppos = parent->getPosition(WORLD);
							Quaternionf pquat = parent->getRotation(WORLD);
							Vector3f pos = bbone->getPosition(WORLD);
							Quaternionf quat = bbone->getRotation(WORLD);
							Vector3f cpos = pos + rigidBodyInfo.positionOffset;
							Quaternionf cquat = quat * rigidBodyInfo.rotationOffset;
							constraintInfo.boneName = parent->name;
							constraintInfo.targetBoneName = bbone->name;
							constraintInfo.positionOffset = cpos - ppos;
							constraintInfo.rotationOffset = pquat.inverse() * cquat;
							constraintInfo.targetPositionOffset = rigidBodyInfo.positionOffset;
							constraintInfo.targetRotationOffset = rigidBodyInfo.rotationOffset;*/
							Vector3f lpos = bbone->getPosition(LOCAL);
							Quaternionf lquat = bbone->getRotation(LOCAL);
							constraintInfo.boneName = parent->name;
							constraintInfo.targetBoneName = bbone->name;
							constraintInfo.positionOffset = lpos;
							constraintInfo.rotationOffset = lquat;
							constraintInfo.targetPositionOffset = Vector3f::Zero();
							constraintInfo.targetRotationOffset = Quaternionf::Identity();
							constraintInfo.config.enableCollision = false;
							constraintInfo.config.rotationDamping = rigid.rotation_attenuation * 0.5f;
							constraintInfo.config.xMotion = D6Constraint::Locked;
							constraintInfo.config.yMotion = D6Constraint::Locked;
							constraintInfo.config.zMotion = D6Constraint::Locked;
							constraintInfo.config.twistMotion = D6Constraint::Free;
							constraintInfo.config.swing1Motion = D6Constraint::Free;
							constraintInfo.config.swing2Motion = D6Constraint::Free;
						}
					}
				}

				for (int ji = 0; ji < pmxModel->joint_count; ji++) {
					pmx::PmxJoint& joint = pmxModel->joints[ji];
					if (joint.joint_type != pmx::PmxJointType::Generic6DofSpring) {
						continue;
					}
					if (joint.param.rigid_body1 < 0 || joint.param.rigid_body1 >= pmxModel->rigid_body_count ||
						joint.param.rigid_body2 < 0 || joint.param.rigid_body2 >= pmxModel->rigid_body_count)
						continue;
					pmx::PmxRigidBody& rigid1 = pmxModel->rigid_bodies[joint.param.rigid_body1];
					pmx::PmxRigidBody& rigid2 = pmxModel->rigid_bodies[joint.param.rigid_body2];
					if (rigid1.target_bone < 0 || rigid1.target_bone >= pmxModel->bone_count ||
						rigid2.target_bone < 0 || rigid2.target_bone >= pmxModel->bone_count)
						continue;
					pmx::PmxBone& bone1 = pmxModel->bones[rigid1.target_bone];
					pmx::PmxBone& bone2 = pmxModel->bones[rigid2.target_bone];
					Bone* bbone1 = skma.getBone(bone1.bone_name);
					Bone* bbone2 = skma.getBone(bone2.bone_name);
					if (bbone1 == NULL || bbone2 == NULL)
						continue;

					if (!testParent(bbone1, bbone2)) {
						if (testParent(bbone2, bbone1)) {
							Bone* t = bbone1;
							bbone1 = bbone2;
							bbone2 = t;

							Console::log("%s is %s's parent", bbone2->name, bbone1->name);
						}
					}

					SkeletonPhysics::D6ConstraintInfo& constraintInfo = physicsInfo.constraints.emplace_back();
					constraintInfo.config.enableCollision = false;

					float rotationDamping = rigid1.rotation_attenuation;
					float positionDamping = rigid1.move_attenuation;

					Vector3f bonePos1 = bbone1->getPosition(WORLD);
					Vector3f bonePos2 = bbone2->getPosition(WORLD);

					Quaternionf boneQuat1 = bbone1->getRotation(WORLD);
					Quaternionf boneQuat2 = bbone2->getRotation(WORLD);

					Vector3f position1 = toVector3fInvZ(joint.param.position) - bonePos1;
					Vector3f position2 = toVector3fInvZ(joint.param.position) - bonePos2;

					Quaternionf rotation1 = Quaternionf::FromEularAngles(toVector3f(joint.param.orientaiton));
					rotation1.z() *= -1;
					rotation1.w() *= -1;
					//rotation1 = rotation1 * Quaternionf::FromTwoVectors(Vector3f::UnitY(), Vector3f::UnitZ());
					Quaternionf rotation2 = Quaternionf::FromEularAngles(toVector3f(joint.param.orientaiton));
					rotation2.z() *= -1;
					rotation2.w() *= -1;
					//rotation2 = rotation2 * Quaternionf::FromTwoVectors(Vector3f::UnitY(), Vector3f::UnitZ());

					constraintInfo.boneName = bbone1->name;
					constraintInfo.targetBoneName = bbone2->name;
					constraintInfo.positionOffset = position1;
					constraintInfo.targetPositionOffset = position2;
					constraintInfo.rotationOffset = boneQuat1.inverse() * rotation1;
					constraintInfo.targetRotationOffset = boneQuat2.inverse() * rotation2;
					constraintInfo.config.positionLimit.min = convertPosition(joint.param.move_limitation_min);
					constraintInfo.config.positionLimit.max = convertPosition(joint.param.move_limitation_max);
					constraintInfo.config.positionSpring = convertPosition(joint.param.spring_move_coefficient);
					constraintInfo.config.positionDamping = Vector3f(positionDamping, positionDamping, positionDamping);

					//Quaternionf rotMin = Quaternionf::FromEularAngles(toVector3f(joint.param.rotation_limitation_min));
					///*rotMin.z() *= -1;
					//rotMin.w() *= -1;
					//rotMin = rotMin * Quaternionf::FromAngleAxis(PI * 0.5, Vector3f::UnitZ());*/
					//Quaternionf rotMax = Quaternionf::FromEularAngles(toVector3f(joint.param.rotation_limitation_max));
					///*rotMax.z() *= -1;
					//rotMax.w() *= -1;
					//rotMax = rotMax * Quaternionf::FromAngleAxis(PI * 0.5, Vector3f::UnitZ());*/
					//Quaternionf rotSpring = Quaternionf::FromEularAngles(toVector3f(joint.param.rotation_limitation_max));
					///*rotSpring.z() *= -1;
					//rotSpring.w() *= -1;
					//rotSpring = rotSpring * Quaternionf::FromAngleAxis(PI * 0.5, Vector3f::UnitZ());*/
					constraintInfo.config.rotationLimit.min = convertEular(joint.param.rotation_limitation_min);
					constraintInfo.config.rotationLimit.max = convertEular(joint.param.rotation_limitation_max);
					constraintInfo.config.rotationSpring = convertEular(joint.param.spring_rotation_coefficient);
					constraintInfo.config.rotationDamping = Vector3f(rotationDamping, rotationDamping, rotationDamping);

					constraintInfo.config.xMotion = D6Constraint::Locked;
					constraintInfo.config.yMotion = D6Constraint::Locked;
					constraintInfo.config.zMotion = D6Constraint::Locked;

					/*constraintInfo.config.xMotion = D6Constraint::Limited;
					constraintInfo.config.yMotion = D6Constraint::Limited;
					constraintInfo.config.zMotion = D6Constraint::Limited;*/

					/*constraintInfo.config.xMotion = (constraintInfo.config.positionLimit.min.x() == 0 &&
						constraintInfo.config.positionLimit.max.x() == 0) ?
						D6Constraint::Locked : D6Constraint::Limited;
					constraintInfo.config.yMotion = (constraintInfo.config.positionLimit.min.y() == 0 &&
						constraintInfo.config.positionLimit.max.y() == 0) ?
						D6Constraint::Locked : D6Constraint::Limited;
					constraintInfo.config.zMotion = (constraintInfo.config.positionLimit.min.z() == 0 &&
						constraintInfo.config.positionLimit.max.z() == 0) ?
						D6Constraint::Locked : D6Constraint::Limited;*/

					constraintInfo.config.twistMotion = D6Constraint::Limited;
					constraintInfo.config.swing1Motion = D6Constraint::Limited;
					constraintInfo.config.swing2Motion = D6Constraint::Limited;
				}

				SerializationInfo& spinfo = *info.add("skeletonPhysics");
				physicsInfo.serialize(spinfo);
			}
		}
	}


	for (auto b = texturePaths.begin(), e = texturePaths.end(); b != e; b++) {
		FPath texturePath = FS::u8path(*b);
		FPath texp = ppath;
		texp.replace_filename(texturePath);
		FPath targetTexPath = FPath(targetMatPath) / FS::u8path(folderName + "_" + texturePath.filename().generic_u8string());
		wstring texpW = texp.generic_wstring();
		wstring targetTexPathW = targetTexPath.generic_wstring();
		if (CopyFileW(texpW.c_str(), targetTexPathW.c_str(), 0))
			Console::log("Copy %s to %s\n", texp.generic_u8string().c_str(), targetTexPath.generic_u8string().c_str());
		else
			Console::error("Failed to copy %s to %s\n", texp.generic_u8string().c_str(), targetTexPath.generic_u8string().c_str());
	}

	FPath targetPmxPath = folderPath;
	targetPmxPath += "/" + folderName + ".pmx";
	info.add("skeletonMesh", targetPmxPath.generic_u8string().c_str());
	if (CopyFileW(ppath.generic_wstring().c_str(), targetPmxPath.generic_wstring().c_str(), 0))
		Console::log("Copy %s to %s\n", pmxPath.c_str(), targetPmxPath.generic_u8string().c_str());
	else
		Console::error("Failed to copy %s to %s\n", pmxPath.c_str(), targetPmxPath.generic_u8string().c_str());

	ofstream f = ofstream(targetMatPath + '/' + folderName + ".asset");
	if (!f.fail()) {
		SerializationInfoWriter siw(f);
		siw.write(info);
		f.close();
	}
}

void AnimationConverter::showAIScene(const aiScene * scene)
{
	if (scene == NULL)
		return;
	for (int i = 0; i < scene->mNumAnimations; i++) {
		aiAnimation* anim = scene->mAnimations[i];
		ImGui::PushID(i);
		if (ImGui::TreeNode(anim->mName.C_Str())) {
			string animName = anim->mName.C_Str();
			if (ImGui::InputText("Name", &animName))
				anim->mName = animName;
			for (int c = 0; c < anim->mNumChannels; c++) {
				aiNodeAnim* nanim = anim->mChannels[c];
				ImGui::PushID(c);
				if (ImGui::TreeNode(nanim->mNodeName.C_Str())) {
					string nodeName = nanim->mNodeName.C_Str();
					if (ImGui::InputText("Name", &nodeName))
						nanim->mNodeName = nodeName;
					ImGui::Text("PositionKeys: %d", nanim->mNumPositionKeys);
					ImGui::Text("RotationKeys: %d", nanim->mNumRotationKeys);
					ImGui::Text("ScalingKeys: %d", nanim->mNumScalingKeys);
					ImGui::TreePop();
				}
				ImGui::PopID();
			}
			ImGui::TreePop();
		}
		ImGui::PopID();
	}
}

void AnimationConverter::showVMDMorph(const vmd::VmdMotion * vmdMotion)
{
}

void AnimationConverter::serializeNodeAnimation(SerializationInfo & info, const aiNodeAnim & data, float tickPerSec)
{
	info.name = data.mNodeName.C_Str();
	info.type = "Array";
	info.arrayType = "BoneFrame";
	struct Frame
	{
		aiVectorKey* pos;
		aiQuatKey* rot;

		SVector3f getPos()
		{
			return SVector3f(pos->mValue.x, pos->mValue.y, pos->mValue.z);
		}

		SQuaternionf getRot()
		{
			return SQuaternionf(rot->mValue.x, rot->mValue.y, rot->mValue.z, rot->mValue.w);
		}
	};
	map<float, Frame> frames;
	for (int i = 0; i < data.mNumPositionKeys; i++) {
		aiVectorKey* key = &data.mPositionKeys[i];
		frames[key->mTime] = Frame{ key, NULL };
	}
	for (int i = 0; i < data.mNumRotationKeys; i++) {
		aiQuatKey* key = &data.mRotationKeys[i];
		auto iter = frames.find(key->mTime);
		if (iter == frames.end())
			frames[key->mTime] = Frame{ NULL, key };
		else
			iter->second.rot = key;
	}
	for (auto b = frames.begin(), e = frames.end(); b != e; b++) {
		SerializationInfo& fi = *info.push();
		fi.set("frame", b->first / tickPerSec);
		if (b->second.pos != NULL)
			fi.set("position", b->second.getPos());
		if (b->second.rot != NULL)
			fi.set("rotation", b->second.getRot());
	}
}

void AnimationConverter::serializeBoneAnimation(SerializationInfo & info, const aiAnimation & data)
{
	info.name = "boneFrames";
	info.type = "Frames";
	for (int i = 0; i < data.mNumChannels; i++) {
		aiNodeAnim& nodeAnim = *data.mChannels[i];
		serializeNodeAnimation(*info.add(nodeAnim.mNodeName.C_Str()), nodeAnim, data.mTicksPerSecond);
	}
}

void AnimationConverter::serializeMorphAnimation(SerializationInfo & info, const vmd::VmdMotion & data)
{
	info.name = "morphFrames";
	info.type = "Frames";
	for (int i = 0; i < data.face_frames.size(); i++) {
		const vmd::VmdFaceFrame& frame = data.face_frames[i];
		string face_name = sj2utf8(frame.face_name);
		SerializationInfo* mfs = info.get(face_name);
		SerializationInfo* mf;
		if (mfs == NULL) {
			mfs = info.add(face_name);
			mfs->type = "Array";
			mfs->arrayType = "MorphFrame";
			mf = mfs->push();
		}
		else {
			mf = mfs->push();
		}
		mf->set("frame", frame.frame / 30.f);
		mf->set("weight", frame.weight);
	}
}

void AnimationConverter::getMorphAnimation(AnimationClipData & anim, const vmd::VmdMotion & data)
{
	for (int i = 0; i < data.face_frames.size(); i++) {
		const vmd::VmdFaceFrame& frame = data.face_frames[i];
		string face_name = sj2utf8(frame.face_name);
		Curve<float, float>& curve = anim.addMorphAnimationData(face_name);
		curve.insert(frame.frame / 30.f, CurveValue<float>(CurveValue<float>::Linear, frame.weight));
	}
}

struct CameraInterpolation
{
	Vector3f posOutRate;
	Vector3f posOutTan;
	Vector3f posInRate;
	Vector3f posInTan;
	float rotOutRate = 0;
	float rotOutTan = 0;
	float rotInRate = 0;
	float rotInTan = 0;
	float distanceOutRate = 0;
	float distanceOutTan = 0;
	float distanceInRate = 0;
	float distanceInTan = 0;
	float fovOutRate = 0;
	float fovOutTan = 0;
	float fovInRate = 0;
	float fovInTan = 0;

	void parse(const char data[6][4])
	{
		posOutRate.x() = data[0][0] / 127.0f;
		posOutTan.x() = data[0][1] / 127.0f;
		posInRate.x() = data[0][2] / 127.0f;
		posInTan.x() = data[0][3] / 127.0f;

		posOutRate.z() = data[1][0] / 127.0f;
		posOutTan.z() = data[1][1] / 127.0f;
		posInRate.z() = data[1][2] / 127.0f;
		posInTan.z() = data[1][3] / 127.0f;

		posOutRate.y() = data[2][0] / 127.0f;
		posOutTan.y() = data[2][1] / 127.0f;
		posInRate.y() = data[2][2] / 127.0f;
		posInTan.y() = data[2][3] / 127.0f;

		rotOutRate = data[3][0] / 127.0f;
		rotOutTan = data[3][1] / 127.0f;
		rotInRate = data[3][2] / 127.0f;
		rotInTan = data[3][3] / 127.0f;

		distanceOutRate = data[4][0] / 127.0f;
		distanceOutTan = data[4][1] / 127.0f;
		distanceInRate = data[4][2] / 127.0f;
		distanceInTan = data[4][3] / 127.0f;

		fovOutRate = data[5][0] / 127.0f;
		fovOutTan = data[5][1] / 127.0f;
		fovInRate = data[5][2] / 127.0f;
		fovInTan = data[5][3] / 127.0f;
	}

	void copyInFrom(const CameraInterpolation& other)
	{
		posInRate = other.posInRate;
		posInTan = other.posInTan;
		rotInRate = other.rotInRate;
		rotInTan = other.rotInTan;
		distanceInRate = other.distanceInRate;
		distanceInTan = other.distanceInTan;
		fovInRate = other.fovInRate;
		fovInTan = other.fovInTan;
	}

	void copyOutFrom(const CameraInterpolation& other)
	{
		posOutRate = other.posOutRate;
		posOutTan = other.posOutTan;
		rotOutRate = other.rotOutRate;
		rotOutTan = other.rotOutTan;
		distanceOutRate = other.distanceOutRate;
		distanceOutTan = other.distanceOutTan;
		fovOutRate = other.fovOutRate;
		fovOutTan = other.fovOutTan;
	}
};

void AnimationConverter::getCameraAnimation(AnimationClipData& anim, const vmd::VmdMotion& data, float motionScale, float fovScale)
{
	float maxTime = 0;
	TransformAnimationData& tranData = anim.addAnimatinData("Camera");
	Curve<float, float>& fovCurve = anim.addMorphAnimationData(0);
	Curve<float, float>& disCurve = anim.addMorphAnimationData(1);
	CameraInterpolation lastInterp;
	for (int i = 0; i < data.camera_frames.size(); i++) {
		const vmd::VmdCameraFrame& frame = data.camera_frames[i];
		float time = frame.frame / 30.0f;
		CameraInterpolation curInterp;
		curInterp.parse(frame.interpolation);
		CameraInterpolation interp;
		interp.copyInFrom(lastInterp);
		interp.copyOutFrom(curInterp);
		lastInterp = curInterp;

		Vector3f position;
		position.x() = frame.position[0];
		position.y() = frame.position[2];
		position.z() = frame.position[1];

		position *= motionScale;

		Vector3f rotation;
		rotation.x() = frame.orientation[0];
		rotation.y() = -frame.orientation[2];
		rotation.z() = fmodf(frame.orientation[1] + 90.0f, 360.0f);

		float distance = -frame.distance * motionScale;

		float fov = frame.angle * fovScale;

		tranData.positionCurve.insert(time, CurveValue<Vector3f>(position, interp.posInTan,
			interp.posOutTan, interp.posInRate, interp.posOutRate));
		Quaternionf R = Quaternionf::FromAngleAxis(rotation.x() / 180.0 * PI, Vector3f::UnitX()) *
			Quaternionf::FromAngleAxis(rotation.y() / 180.0 * PI, Vector3f::UnitY()) *
			Quaternionf::FromAngleAxis(rotation.z() / 180.0 * PI, Vector3f::UnitZ());
		tranData.rotationCurve.insert(time, CurveValue<Quaternionf>(R, interp.rotInTan,
			interp.rotOutTan, interp.rotInRate, interp.rotOutRate));
		disCurve.insert(time, CurveValue<float>(distance, interp.distanceInTan,
			interp.distanceOutTan, interp.distanceInRate, interp.distanceOutRate));
		fovCurve.insert(time, CurveValue<float>(fov, interp.fovInTan,
			interp.fovOutTan, interp.fovInRate, interp.fovOutRate));

		if (maxTime < time)
			maxTime = time;
	}

	tranData.positionCurve.duration = maxTime;
	tranData.rotationCurve.duration = maxTime;
	fovCurve.duration = maxTime;
	disCurve.duration = maxTime;
	anim.duration = maxTime;
	anim.pack();
}

struct BoneInterpolation
{
	Vector3f posOutRate;
	Vector3f posOutTan;
	Vector3f posInRate;
	Vector3f posInTan;
	float rotOutRate = 0;
	float rotOutTan = 0;
	float rotInRate = 0;
	float rotInTan = 0;

	void parse(const char data[4][4][4])
	{
		posOutRate.x() = data[0][0][0] / 127.0f;
		posOutTan.x() = data[0][1][0] / 127.0f;
		posInRate.x() = data[0][2][0] / 127.0f;
		posInTan.x() = data[0][3][0] / 127.0f;

		posOutRate.z() = data[1][0][0] / 127.0f;
		posOutTan.z() = data[1][1][0] / 127.0f;
		posInRate.z() = data[1][2][0] / 127.0f;
		posInTan.z() = data[1][3][0] / 127.0f;

		posOutRate.y() = data[2][0][0] / 127.0f;
		posOutTan.y() = data[2][1][0] / 127.0f;
		posInRate.y() = data[2][2][0] / 127.0f;
		posInTan.y() = data[2][3][0] / 127.0f;

		rotOutRate = data[3][0][0] / 127.0f;
		rotOutTan = data[3][1][0] / 127.0f;
		rotInRate = data[3][2][0] / 127.0f;
		rotInTan = data[3][3][0] / 127.0f;
	}

	void copyInFrom(const BoneInterpolation& other)
	{
		posInRate = other.posInRate;
		posInTan = other.posInTan;
		rotInRate = other.rotInRate;
		rotInTan = other.rotInTan;
	}

	void copyOutFrom(const BoneInterpolation& other)
	{
		posOutRate = other.posOutRate;
		posOutTan = other.posOutTan;
		rotOutRate = other.rotOutRate;
		rotOutTan = other.rotOutTan;
	}
};

void AnimationConverter::getBoneAnimation(AnimationClipData& anim, const vmd::VmdMotion& data)
{
	BoneInterpolation lastInterp;
	float maxTime = 0;
	for (int i = 0; i < data.bone_frames.size(); i++) {
		const vmd::VmdBoneFrame& frame = data.bone_frames[i];
		string name = sj2utf8(frame.name);
		TransformAnimationData& tranData = anim.addAnimatinData(name);

		float time = frame.frame / 30.0f;
		BoneInterpolation curInterp;
		curInterp.parse(frame.interpolation);
		BoneInterpolation interp;
		interp.copyInFrom(lastInterp);
		interp.copyOutFrom(curInterp);
		lastInterp = curInterp;

		Vector3f position;
		position.x() = frame.position[0];
		position.y() = frame.position[2];
		position.z() = frame.position[1];

		Quaternionf rotation;
		rotation.x() = frame.orientation[0];
		rotation.y() = frame.orientation[1];
		rotation.z() = -frame.orientation[2];
		rotation.w() = -frame.orientation[3];

		tranData.positionCurve.insert(time, CurveValue<Vector3f>(position, interp.posInTan,
			interp.posOutTan, interp.posInRate, interp.posOutRate));
		tranData.rotationCurve.insert(time, CurveValue<Quaternionf>(rotation, interp.rotInTan,
			interp.rotOutTan, interp.rotInRate, interp.rotOutRate));

		if (maxTime < time)
			maxTime = time;
	}

	for (int i = 0; i < anim.transformAnimationDatas.size(); i++) {
		TransformAnimationData& animData = anim.transformAnimationDatas[i];
		animData.positionCurve.duration = maxTime;
		animData.rotationCurve.duration = maxTime;
	}
}

void AnimationConverter::getAnimation(AnimationClipData& anim, const vmd::VmdMotion& data, float motionScale, float fovScale)
{
	getBoneAnimation(anim, data);
	getMorphAnimation(anim, data);
	getCameraAnimation(anim, data, motionScale, fovScale);
}

bool AnimationConverter::getToonParameter(const string & toonFile, ToonParameter& toonParameter)
{
	Texture2D tex;
	if (!tex.load(toonFile))
		return false;
	unsigned int w = tex.getWidth(), h = tex.getHeight();
	vector<int> zeroCount;
	string text;
	string ztext;
	unsigned int count = 0;
	bool isPure = true;
	for (int i = 0; i < h; i++) {
		Color color, colorL, colorR;
		tex.getPixel(color, i, w / 2);
		if (!tex.getPixel(colorL, i - 1, w / 2)) {
			colorL = color;
		}
		if (!tex.getPixel(colorR, i + 1, w / 2)) {
			colorR = color;
		}
		float diff = fabsf(color.r - colorL.r) + fabsf(color.r - colorR.r) +
			fabsf(color.g - colorL.g) + fabsf(color.g - colorR.g) +
			fabsf(color.b - colorL.b) + fabsf(color.b - colorR.b);
		text += to_string(diff) + " ";
		if (isPure) {
			if (diff == 0)
				count++;
			else if (count != 0) {
				zeroCount.push_back(count);
				count = 1;
				isPure = false;
			}
		}
		else {
			if (diff != 0)
				count++;
			else if (count != 0) {
				zeroCount.push_back(count);
				count = 1;
				isPure = true;
			}
		}
	}
	if (count != 0) {
		zeroCount.push_back(count);
	}
	if (zeroCount.size() < 3) {
		zeroCount.push_back(0);
	}
	while (zeroCount.size() > 3) {
		unsigned int minCount = h, index = 0;
		for (int i = 0; i < zeroCount.size(); i++) {
			if (zeroCount[i] < minCount) {
				minCount = zeroCount[index];
				index = i;
			}
		}
		if (index == 0 || index == zeroCount.size() - 1) {
			zeroCount[zeroCount.size() - 2] += minCount;
			zeroCount.erase(zeroCount.begin() + index);
		}
		else {
			zeroCount[index - 1] += minCount + zeroCount[index + 1];
			zeroCount.erase(zeroCount.begin() + index, zeroCount.begin() + index + 1);
		}
	}
	if (zeroCount.size() < 3) {
		zeroCount.push_back(0);
	}

	Color baseScale, shadowScale;
	tex.getPixel(baseScale, 0, w / 2);
	tex.getPixel(shadowScale, h - 1, w / 2);

	toonParameter.baseColor = baseScale * toonParameter.baseColor.r;
	toonParameter.baseColor.a = 1;
	toonParameter.shadowColor = shadowScale * toonParameter.shadowColor.r;
	toonParameter.shadowColor.a = 1;
	
	toonParameter.shadowCut = (zeroCount[2] == 0 ? 0.0f : (zeroCount[1] / 2.0f + zeroCount[2])) / w;
	toonParameter.shadowSmooth = zeroCount[1] / (float)w;
	return true;
}

AnimationConverter::MaterialInfo::MaterialInfo()
{
}

AnimationConverter::MaterialInfo::MaterialInfo(const Material & mat) : baseMat(&mat)
{
	vector<string> name = split(mat.getShaderName(), '.');
	if (name.empty()) {
		Asset* shd = AssetManager::getAsset("Material", name[0]);
		if (shd != NULL)
			baseMatPath = shd->path;
	}
	isTwoSide = mat.isTwoSide;
	cullFront = mat.cullFront;
	canCastShadow = mat.canCastShadow;
	auto scalarField = mat.getScalarField();
	for (auto b = scalarField.begin(), e = scalarField.end(); b != e; b++) {
		scalars[b->first] = b->second.val;
	}
	auto colorField = mat.getColorField();
	for (auto b = colorField.begin(), e = colorField.end(); b != e; b++) {
		colors[b->first] = b->second.val;
	}
	auto texField = mat.getTextureField();
	for (auto b = texField.begin(), e = texField.end(); b != e; b++) {
		string path = AssetInfo::getPath(b->second.val);
		if (path.empty())
			path = "white";
		textures[b->first] = path;
	}
}

void AnimationConverter::MaterialInfo::setCount(const string & name, float count)
{
	auto iter = counts.find(name);
	if (iter != counts.end())
		iter->second = count;
}

void AnimationConverter::MaterialInfo::setScalar(const string & name, float scalar)
{
	auto iter = scalars.find(name);
	if (iter != scalars.end())
		iter->second = scalar;
}

void AnimationConverter::MaterialInfo::setColor(const string & name, const Color& color)
{
	auto iter = colors.find(name);
	if (iter != colors.end())
		iter->second = color;
}

void AnimationConverter::MaterialInfo::setTexture(const string & name, const string & path)
{
	auto iter = textures.find(name);
	if (iter != textures.end())
		iter->second = path;
}

void AnimationConverter::MaterialInfo::setTexture(const string & name, const Texture & tex)
{
	auto iter = textures.find(name);
	if (iter != textures.end()) {
		string path = AssetInfo::getPath((void*)&tex);
		if (path.empty())
			path = "white";
		iter->second = path;
	}
}

void AnimationConverter::MaterialInfo::setToonParameter(const ToonParameter & toon)
{
	setScalar("highlightCut", 0.999);
	setScalar("rimCut", toon.rimCut);
	setScalar("rimSmooth", toon.rimSmooth);
	setScalar("shadowCut", toon.shadowCut);
	setScalar("shadowSmooth", toon.shadowSmooth);
	setColor("baseColor", toon.baseColor);
	setColor("highlightColor", toon.highlistColor);
	setColor("overColor", toon.overColor);
	setColor("shadowColor", toon.shadowColor);
	setTexture("colorMap", toon.texPath);
	setTexture("normalMap", "Engine/Textures/Default_N.png");
}

void AnimationConverter::MaterialInfo::setPBRParameter(const ToonParameter& toon)
{
	setColor("baseColor", toon.baseColor);
	setTexture("colorMap", toon.texPath);
	setTexture("normalMap", "Engine/Textures/Default_N.png");
}

void AnimationConverter::MaterialInfo::setOutlineParameter(const OutlineParameter& outline)
{
	setScalar("borderWidth", outline.borderWidth);
	setScalar("refenceDistance", outline.refenceDistance);
	setColor("baseColor", outline.baseColor);
}

ostream & AnimationConverter::MaterialInfo::write(ostream & os)
{
	if (!baseMatPath.empty())
		os << "#material " << baseMatPath << endl;
	os << "#twoside " << (isTwoSide ? "true\n" : "false\n");
	if (cullFront)
		os << "#cullfront true\n";
	os << "#castshadow " << (canCastShadow ? "true\n" : "false\n");
	for (auto b = counts.begin(), e = counts.end(); b != e; b++) {
		os << ("Count " + b->first + ": " + to_string(b->second) + '\n');
	}
	for (auto b = scalars.begin(), e = scalars.end(); b != e; b++) {
		os << ("Scalar " + b->first + ": " + to_string(b->second) + '\n');
	}
	for (auto b = colors.begin(), e = colors.end(); b != e; b++) {
		os << ("Color " + b->first + ": " + to_string(b->second.r) + ", " +
			to_string(b->second.g) + ", " + to_string(b->second.b) + ", " +
			to_string(b->second.a) + '\n');
	}
	for (auto b = textures.begin(), e = textures.end(); b != e; b++) {
		os << ("Texture " + b->first + ": " + (b->second.empty() ? "white" : b->second) + '\n');
	}
	return os;
}
