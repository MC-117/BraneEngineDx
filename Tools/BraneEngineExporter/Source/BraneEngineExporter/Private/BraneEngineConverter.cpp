#include "BraneEngineConverter.h"

#include <filesystem>
#include <fstream>

#include "AssetExportTask.h"
#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"
#include "IMaterialBakingModule.h"
#include "IMeshMergeUtilities.h"
#include "MaterialBakingStructures.h"
#include "MaterialOptions.h"
#include "MaterialUtilities.h"
#include "MeshMergeModule.h"
#include "PNGExportOption.h"
#include "StaticMeshAdapter.h"
#include "StaticMeshAttributes.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/DrawSphereComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Exporters/Exporter.h"
#include "Exporters/FbxExportOption.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "PropertyEditor/Public/PropertyCustomizationHelpers.h"
#include "Subsystems/EditorActorSubsystem.h"
#include "UObject/GCObjectScopeGuard.h"
#include "Widgets/Text/STextBlock.h"

namespace Brane
{
	void MakeContentPath(FString& InPath)
	{
		const TCHAR* Game = TEXT("/Game");
		const TCHAR* Content = TEXT("Content");
		if (InPath.Find(Game) == 0)
			InPath.ReplaceInline(Game, Content);
		else
			InPath = FPaths::Combine(Content, InPath);
	}
	
	BTexture::BTexture(UTexture& texture) : texture(&texture)
	{
		FString upath = FPaths::Combine(FPaths::GetPath(texture.GetPathName()), texture.GetName() + TEXT(".png"));
		MakeContentPath(upath);
		path = TCHAR_TO_UTF8(*upath);
	}

	bool BTexture::exportTexture(const FString& rootPath)
	{
		if (!texture)
			return false;
		FString outPath = FPaths::Combine(rootPath, FString(this->path.c_str()));
		UPNGExportOption* Option = NewObject<UPNGExportOption>();
		Option->bWithAlpha = true;
		Option->gamma = outputGamma;
		
		UAssetExportTask* ExportTask = NewObject<UAssetExportTask>();
		FGCObjectScopeGuard ExportTaskGuard(ExportTask);
		ExportTask->Object = texture;
		ExportTask->Exporter = nullptr;
		ExportTask->Filename = outPath;
		ExportTask->bSelected = false;
		ExportTask->bReplaceIdentical = true;
		ExportTask->bPrompt = false;
		ExportTask->bUseFileArchive = false;
		ExportTask->bWriteEmptyFiles = false;
		ExportTask->bAutomated = true;
		ExportTask->Options = Option;
		return UExporter::RunAssetExportTask(ExportTask);
	}

	bool BMaterial::exportMaterial(const FString& rootPath)
	{
		string outPath = TCHAR_TO_UTF8(*FPaths::Combine(rootPath, path.c_str()));
		ofstream file = ofstream(outPath);
		if (file.fail())
			return false;
		save(file);
		file.close();
		return true;
	}

	BStaticMesh::BStaticMesh(UStaticMesh& staticMesh) : staticMesh(&staticMesh)
	{
		FString upath = FPaths::ChangeExtension(staticMesh.GetPathName(), TEXT("fbx"));
		MakeContentPath(upath);
		path = TCHAR_TO_UTF8(*upath);
	}

	BStaticMesh::~BStaticMesh()
	{
		for (auto& texture : textures)
		{
			texture->texture->MarkAsGarbage();
			texture->texture = nullptr;
		}
	}

	void grabTexture(UPackage* OuterPackage, const FString& AssetName, const FBakeOutput& BakeOutput, MaterialRef Material, TArray<BTextureRef>& OutTextures)
	{
		TArray<EMaterialProperty> SRGBEnabledProperties{ MP_BaseColor, MP_EmissiveColor, MP_SubsurfaceColor };

		// Certain material properties use differen compression settings
		TMap<EMaterialProperty, TextureCompressionSettings> SpecialCompressionSettingProperties;
		SpecialCompressionSettingProperties.Add(MP_Normal, TC_Normalmap);
		SpecialCompressionSettingProperties.Add(MP_Opacity, TC_Grayscale);
		SpecialCompressionSettingProperties.Add(MP_OpacityMask, TC_Grayscale);
		SpecialCompressionSettingProperties.Add(MP_AmbientOcclusion, TC_Grayscale);
		
		for (auto Iterator : BakeOutput.PropertySizes)
		{
			EMaterialProperty Property = Iterator.Key;
			FIntPoint DataSize = Iterator.Value;
			// Retrieve pixel data for the current property
			const TArray<FColor>& ColorData = BakeOutput.PropertyData.FindChecked(Property);

			// Look up the property name string 
			const UEnum* PropertyEnum = StaticEnum<EMaterialProperty>();
			FName PropertyName = PropertyEnum->GetNameByValue(Property);
			FString TrimmedPropertyName = PropertyName.ToString();
			TrimmedPropertyName.RemoveFromStart(TEXT("MP_"));

			// If the pixel data isn't constant create a texture for it 
			if (ColorData.Num() > 1)
			{
				FMaterialParameterInfo ParameterInfo(*(TrimmedPropertyName + TEXT("Texture")));

				FCreateTexture2DParameters CreateParams;
				CreateParams.TextureGroup = TEXTUREGROUP_HierarchicalLOD;
				CreateParams.CompressionSettings = SpecialCompressionSettingProperties.Contains(Property) ? SpecialCompressionSettingProperties.FindChecked(Property) : TC_Default;
				CreateParams.bSRGB = SRGBEnabledProperties.Contains(Property);

				UTexture* Texture = FMaterialUtilities::CreateTexture(OuterPackage, TEXT("T_") + AssetName + TEXT("_") + TrimmedPropertyName, DataSize, ColorData, CreateParams, RF_Public | RF_Standalone);

				BTextureRef BETexture = make_shared<BTexture>(*Texture);

				OutTextures.Add(BETexture);

				switch (Property)
				{
				case MP_EmissiveColor:
					Material->setTexture("emissionMap", BETexture); break;
				case MP_BaseColor:
					Material->setTexture("colorMap", BETexture); break;
				case MP_Metallic:
					BETexture->outputGamma = 2.2f;
					Material->setTexture("metallicMap", BETexture);
					Material->setScalar("metallic", 1);break;
				case MP_Roughness:
					BETexture->outputGamma = 2.2f;
					Material->setTexture("roughnessMap", BETexture);
					Material->setScalar("roughness", 1); break;
				case MP_Normal: Material->setTexture("normalMap", BETexture); break;
				case MP_AmbientOcclusion:
					BETexture->outputGamma = 2.2f;
					Material->setTexture("aoMap", BETexture);
					Material->setScalar("ao", 1); break;
				default: break;
				}
			}
			else
			{
				// Otherwise set either float4 or float constant values on instance material
				FMaterialParameterInfo ParameterInfo(*(TrimmedPropertyName + TEXT("Const")));

				Color color = toColor(ColorData[0]);

				switch (Property)
				{
				case MP_BaseColor: Material->setColor("baseColor", color); break;
				case MP_Metallic: Material->setScalar("metallic", color.r); break;
				case MP_Roughness: Material->setScalar("roughness", color.r); break;
				case MP_AmbientOcclusion: Material->setScalar("ao", color.r); break;
				default: break;
				}
			}
		}

		if (Material->textures.find("normalMap") == Material->textures.end())
		{
			Material->setTexture("normalMap", Texture::DefaultNormalMap);
		}
	}

	void BStaticMesh::bakeMaterial(UMaterialOptions& MaterialOptions)
	{
		const auto& mesh = staticMesh->GetLODForExport(0);
		TArray<int32> SectionsNeedBake;
		for (auto& section : mesh.Sections)
		{
			const int32 index = section.MaterialIndex;
			UMaterial* material = staticMesh->GetMaterial(index)->GetBaseMaterial();
			if (material->GetPathName() != TEXT("/Engine/EngineMaterials/FlattenMaterial.FlattenMaterial"))
				SectionsNeedBake.Add(index);
			UE_LOG(LogTemp, Log, TEXT("%s"), *material->GetFullName());
		}
		
		FStaticMeshAdapter Adapter(staticMesh);

		// Mesh / LOD index	
		TMap<uint32, FMeshDescription> RawMeshLODs;

		// Unique set of sections in mesh
		TArray<FSectionInfo> UniqueSections;

		TArray<FSectionInfo> Sections;

		// LOD index, <original section index, unique section index>
		TMap<int32, int32> UniqueSectionIndex;

		// Retrieve raw mesh data and unique sections
		{
			int32 LODIndex = 0;
			
			// Reset section for reuse
			Sections.SetNum(0, false);

			// Extract raw mesh data 
			const bool bProcessedLOD = MaterialOptions.LODIndices.Contains(LODIndex);
			if (bProcessedLOD)
			{
				FMeshDescription& RawMesh = RawMeshLODs.Add(LODIndex);
				FStaticMeshAttributes(RawMesh).Register();
				Adapter.RetrieveRawMeshData(LODIndex, RawMesh, MaterialOptions.bUseMeshData);
			}

			// Extract sections for given LOD index from the mesh 
			Adapter.RetrieveMeshSections(LODIndex, Sections);

			for (int32 SectionIndex = 0; SectionIndex < Sections.Num(); ++SectionIndex)
			{
				FSectionInfo& Section = Sections[SectionIndex];
				Section.bProcessed = bProcessedLOD;
				const int32 UniqueIndex = UniqueSections.AddUnique(Section);
				UniqueSectionIndex.Emplace(SectionIndex, UniqueIndex);
			}
		}

		TArray<UMaterialInterface*> UniqueMaterials;
		TMap<UMaterialInterface*, int32> MaterialIndices;
		TMultiMap<uint32, uint32> UniqueMaterialToUniqueSectionMap;
		// Populate list of unique materials and store section mappings
		for (int32 SectionIndex = 0; SectionIndex < UniqueSections.Num(); ++SectionIndex)
		{
			FSectionInfo& Section = UniqueSections[SectionIndex];
			const int32 UniqueIndex = UniqueMaterials.AddUnique(Section.Material);
			UniqueMaterialToUniqueSectionMap.Add(UniqueIndex, SectionIndex);
		}

		TArray<FMeshData> GlobalMeshSettings;
		TArray<FMaterialData> GlobalMaterialSettings;

		for (int32 MaterialIndex = 0; MaterialIndex < UniqueMaterials.Num(); ++MaterialIndex)
		{
			UMaterialInterface* Material = UniqueMaterials[MaterialIndex];
			// Retrieve all sections using this material 
			TArray<uint32> SectionIndices;
			UniqueMaterialToUniqueSectionMap.MultiFind(MaterialIndex, SectionIndices);

			// If we are not using the mesh data we aren't doing anything special, just bake out uv range
			FMeshData MeshSettings;
			for (const auto& Pair : UniqueSectionIndex)
			{
				if (SectionIndices.Contains(Pair.Value))
				{
					MeshSettings.MaterialIndices.Add(Pair.Key);
				}
			}

			if (MeshSettings.MaterialIndices.Num())
			{
				MeshSettings.MeshDescription = nullptr;
				MeshSettings.TextureCoordinateBox = FBox2D(FVector2D(0.0f, 0.0f), FVector2D(1.0f, 1.0f));
				MeshSettings.TextureCoordinateIndex = 0;

				FMaterialData MaterialSettings;
				MaterialSettings.Material = Material;

				// Add all user defined properties for baking out
				for (const FPropertyEntry& Entry : MaterialOptions.Properties)
				{
					if (!Entry.bUseConstantValue && Material->IsPropertyActive(Entry.Property) && Entry.Property != MP_MAX)
					{
						MaterialSettings.PropertySizes.Add(Entry.Property, Entry.bUseCustomSize ? Entry.CustomSize : MaterialOptions.TextureSize);
					}
				}

				for (const auto& Pair : UniqueSectionIndex)
				{
					if (SectionIndices.Contains(Pair.Value))
					{
						/// For each original material index add an entry to the corresponding LOD and bake output index 
						materialMap.Emplace(Pair.Key, GlobalMeshSettings.Num());
					}
				}

				GlobalMeshSettings.Add(MeshSettings);
				GlobalMaterialSettings.Add(MaterialSettings);
			}
		}

		TArray<FMeshData*> MeshSettingPtrs;
		for (int32 SettingsIndex = 0; SettingsIndex < GlobalMeshSettings.Num(); ++SettingsIndex)
		{
			MeshSettingPtrs.Add(&GlobalMeshSettings[SettingsIndex]);
		}

		TArray<FMaterialData*> MaterialSettingPtrs;
		for (int32 SettingsIndex = 0; SettingsIndex < GlobalMaterialSettings.Num(); ++SettingsIndex)
		{
			MaterialSettingPtrs.Add(&GlobalMaterialSettings[SettingsIndex]);
		}

		TArray<FBakeOutput> BakeOutputs;
		IMaterialBakingModule& Module = FModuleManager::Get().LoadModuleChecked<IMaterialBakingModule>("MaterialBaking");
		Module.BakeMaterials(MaterialSettingPtrs, MeshSettingPtrs, BakeOutputs);

		// Append constant properties which did not require baking out
		TArray<FColor> ConstantData;
		FIntPoint ConstantSize(1, 1);
		for (const FPropertyEntry& Entry : MaterialOptions.Properties)
		{
			if (Entry.bUseConstantValue && Entry.Property != MP_MAX)
			{
				ConstantData.SetNum(1, false);
				ConstantData[0] = FColor(Entry.ConstantValue * 255.0f, Entry.ConstantValue * 255.0f, Entry.ConstantValue * 255.0f);
				for (FBakeOutput& Ouput : BakeOutputs)
				{
					Ouput.PropertyData.Add(Entry.Property, ConstantData);
					Ouput.PropertySizes.Add(Entry.Property, ConstantSize);
				}
			}
		}

		FString PackageName = Adapter.GetBaseName();

		for (int32 OutputIndex = 0; OutputIndex < BakeOutputs.Num(); ++OutputIndex)
		{
			// Create merged material asset
			FString MaterialAssetName = TEXT("M_") + FPackageName::GetShortName(PackageName) + TEXT("_") + MaterialSettingPtrs[OutputIndex]->Material->GetName();
			FString MaterialPackageName = FPackageName::GetLongPackagePath(PackageName) + TEXT("/") + MaterialAssetName;

			FBakeOutput& Output = BakeOutputs[OutputIndex];
			FMeshData& MeshData = *MeshSettingPtrs[OutputIndex];
			FMaterialData& MaterialData = *MaterialSettingPtrs[OutputIndex];
			// Optimize output 
			for (auto DataPair : Output.PropertyData)
			{
				FMaterialUtilities::OptimizeSampleArray(DataPair.Value, Output.PropertySizes[DataPair.Key]);
			}

			BMaterialRef BEMaterial = make_shared<BMaterial>();
			BEMaterial->baseMatPath = "Engine/Shaders/PBR.mat";
			BEMaterial->name = TCHAR_TO_UTF8(*MaterialAssetName);
			BEMaterial->path = TCHAR_TO_UTF8(*FPaths::Combine(FPaths::GetPath(path.c_str()), MaterialAssetName + TEXT(".imat")));

			UPackage* MaterialPackage = Adapter.GetOuter();

			if (!MaterialPackage)
			{
				MaterialPackage = CreatePackage( *PackageName);
				check(MaterialPackage);
				MaterialPackage->FullyLoad();
				MaterialPackage->Modify(false);
			}

			grabTexture(MaterialPackage, MaterialAssetName, Output, BEMaterial, textures);
			
			materials.Add(BEMaterial);
		}
	}

	bool BStaticMesh::exportMesh(const FString& rootPath)
	{
		if (!staticMesh)
			return false;
		
		FString folderPath = FPaths::GetPath(this->path.c_str());
		FString outPath = FPaths::Combine(rootPath, this->path.c_str());

		if (!filesystem::exists(*folderPath))
			filesystem::create_directories(*folderPath);

		for (auto& texture : textures)
		{
			texture->exportTexture(rootPath);
			UE_LOG(LogTemp, Log, TEXT("Export texture %s"), *FString(texture->path.c_str()));
		}

		for (const auto& material : materials)
		{
			material->exportMaterial(rootPath);
			UE_LOG(LogTemp, Log, TEXT("Export material %s"), *FString(material->path.c_str()));
		}

		UFbxExportOption* Option = NewObject<UFbxExportOption>();
		Option->bASCII = false;
		Option->Collision = false;
		Option->bExportPreviewMesh = false;
		Option->LevelOfDetail = false;
		
		UAssetExportTask* ExportTask = NewObject<UAssetExportTask>();
		FGCObjectScopeGuard ExportTaskGuard(ExportTask);
		ExportTask->Object = staticMesh;
		ExportTask->Exporter = nullptr;
		ExportTask->Filename = outPath;
		ExportTask->bSelected = false;
		ExportTask->bReplaceIdentical = true;
		ExportTask->bPrompt = false;
		ExportTask->bUseFileArchive = false;
		ExportTask->bWriteEmptyFiles = false;
		ExportTask->bAutomated = true;
		ExportTask->Options = Option;
		const bool success = UExporter::RunAssetExportTask(ExportTask);
		
		UE_LOG(LogTemp, Log, TEXT("Export FBX to %s"), *outPath);
		
		return success;
	}

	BStaticMeshActor::BStaticMeshActor(AStaticMeshActor& staticMeshActor)
		: ownedActor(&staticMeshActor), staticMeshComponent(staticMeshActor.GetStaticMeshComponent())
	{
		name = TCHAR_TO_UTF8(*staticMeshActor.GetActorNameOrLabel());
		
		position = convertPosition(staticMeshActor.GetTransform().GetLocation());
		rotation = convertRotation(staticMeshActor.GetTransform().GetRotation());
		scale = convertScale(staticMeshActor.GetTransform().GetScale3D());
	}

	BStaticMeshActor::BStaticMeshActor(UStaticMeshComponent& staticMeshComponent)
		: ownedActor(staticMeshComponent.GetOwner()), staticMeshComponent(&staticMeshComponent)
	{
		name = TCHAR_TO_UTF8(*staticMeshComponent.GetName().Replace(TEXT("."), TEXT("_")));

		position = convertPosition(staticMeshComponent.GetRelativeLocation());
		rotation = convertRotation(staticMeshComponent.GetRelativeRotation().Quaternion());
		scale = convertScale(staticMeshComponent.GetRelativeScale3D());
	}

	void BStaticMeshActor::collectMaterial()
	{
		BStaticMesh* staticMesh = static_cast<BStaticMesh*>(mesh.get());
		if (!staticMesh)
			return;
		materials.clear();
		for (const auto& material : staticMesh->materials)
		{
			setMaterial(material->name, material);
		}
	}

	void BStaticMeshActor::write(ostream& os)
	{
		collectMaterial();
		MeshActor::write(os);
	}

	BDirectLight::BDirectLight(ADirectionalLight& directLight) : directLight(&directLight)
	{
		name = TCHAR_TO_UTF8(*directLight.GetActorNameOrLabel());
		
		position = convertPosition(directLight.GetTransform().GetLocation());
		rotation = convertRotation(directLight.GetTransform().GetRotation());
		scale = convertScale(directLight.GetTransform().GetScale3D());
		
		color = toColor(directLight.GetLightColor());
		intensity = directLight.GetComponent()->Intensity;
	}

	BPointLight::BPointLight(APointLight& pointLight) : pointLight(&pointLight)
	{
		name = TCHAR_TO_UTF8(*pointLight.GetActorNameOrLabel());
		
		position = convertPosition(pointLight.GetTransform().GetLocation());
		rotation = convertRotation(pointLight.GetTransform().GetRotation());
		scale = convertScale(pointLight.GetTransform().GetScale3D());
		
		color = toColor(pointLight.GetLightColor());
		intensity = pointLight.PointLightComponent->Intensity;
		radius = pointLight.PointLightComponent->AttenuationRadius;
	}

	BReflectionProbe::BReflectionProbe(ASphereReflectionCapture& sphereReflectionCapture) : sphereReflectionCapture(&sphereReflectionCapture)
	{
		name = TCHAR_TO_UTF8(*sphereReflectionCapture.GetActorNameOrLabel());
		
		position = convertPosition(sphereReflectionCapture.GetTransform().GetLocation());
		rotation = convertRotation(sphereReflectionCapture.GetTransform().GetRotation());
		scale = convertScale(sphereReflectionCapture.GetTransform().GetScale3D());

		radius = sphereReflectionCapture.GetDrawCaptureRadius()->GetScaledSphereRadius();
	}

	BConverter::BConverter()
	{
		AssetThumbnailPool = MakeShareable(new FAssetThumbnailPool(24));
	}

	TArray<TWeakObjectPtr<UObject>> BConverter::getBakeOptions()
	{
		if (MaterialOptions.IsNull() || !MaterialOptions->IsValidLowLevel())
		{
			MaterialOptions = NewObject<UMaterialOptions>();
			MaterialOptions->Properties.Add(FPropertyEntry(EMaterialProperty::MP_Metallic));
			MaterialOptions->Properties.Add(FPropertyEntry(EMaterialProperty::MP_Roughness));
			MaterialOptions->Properties.Add(FPropertyEntry(EMaterialProperty::MP_Normal));
			MaterialOptions->Properties.Add(FPropertyEntry(EMaterialProperty::MP_AmbientOcclusion));
			MaterialOptions->TextureSize = { 1024, 1024 };
		}
		return { MaterialOptions };
	}

	void BConverter::ExportStaticMesh(MeshRef mesh)
	{
		BStaticMesh* staticMesh = static_cast<BStaticMesh*>(mesh.get());

		if (!staticMesh)
			return;
		
		staticMesh->bakeMaterial(*MaterialOptions);
		staticMesh->exportMesh(OutFolderPath);

		MeshActor meshActor;
		meshActor.name = TCHAR_TO_UTF8(*staticMesh->staticMesh->GetName());
		meshActor.mesh = mesh;

		for (const auto& material : staticMesh->materials)
		{
			meshActor.setMaterial(material->name, material);
		}

		meshActor.writeToFile(TCHAR_TO_UTF8(*FPaths::Combine(OutFolderPath, FPaths::ChangeExtension(mesh->path.c_str(), TEXT("asset")))));
	}

	void BConverter::ExportStaticMesh(UStaticMesh& staticMesh)
	{
		if (OutFolderPath.Len() == 0 || !FPaths::ValidatePath(OutFolderPath))
			return;
		
		if (!ConfigMaterialBakeOption())
			return;
		
		if (!filesystem::exists(*OutFolderPath))
			filesystem::create_directories(*OutFolderPath);

		BStaticMeshRef mesh = make_shared<BStaticMesh>(staticMesh);
		ExportStaticMesh(mesh);
	}

	void BConverter::ExportSelectedActor()
	{
		if (OutFolderPath.Len() == 0 || !FPaths::ValidatePath(OutFolderPath))
			return;
		
		if (!ConfigMaterialBakeOption())
			return;
		
		if (!filesystem::exists(*OutFolderPath))
			filesystem::create_directories(*OutFolderPath);
		
		UEditorActorSubsystem* editor = GEditor->GetEditorSubsystem<UEditorActorSubsystem>();

		TArray<AActor*> actors = editor->GetSelectedLevelActors();

		TMap<UStaticMesh*, MeshRef> meshes;

		auto getOrAddMesh = [&meshes](UStaticMesh* staticMesh)
		{
			MeshRef& mesh = meshes.FindOrAdd(staticMesh);
			if (!mesh)
				mesh = make_shared<BStaticMesh>(*staticMesh);
			return mesh;
		};

		World world;
		world.name = "RootWorld";

		TransformRef camera = make_shared<DebugCamera>();
		world.addChild(camera);
		TransformRef skySphere = make_shared<SkySphere>();
		world.addChild(skySphere);

		TransformRef sceneRoot = make_shared<Transform>();
		sceneRoot->name = "SceneRoot";
		world.addChild(sceneRoot);

		TransformRef meshRoot = make_shared<Transform>();
		meshRoot->name = "MeshRoot";
		sceneRoot->addChild(meshRoot);

		TransformRef lightRoot = make_shared<Transform>();
		lightRoot->name = "LightRoot";
		sceneRoot->addChild(lightRoot);

		TransformRef captureRoot = make_shared<Transform>();
		captureRoot->name = "CaptureRoot";
		sceneRoot->addChild(captureRoot);

		auto addSceneComponent = [&getOrAddMesh](auto&& addSceneComponent, TransformRef rootTansform, USceneComponent* rootComponent)
		{
			if (rootTansform == nullptr || rootComponent == nullptr)
				return;
			TArray<USceneComponent*> childern;
			rootComponent->GetChildrenComponents(false, childern);
			for (auto child : childern)
			{
				UStaticMeshComponent* staticMeshComponent = Cast<UStaticMeshComponent>(child);
				if (staticMeshComponent)
				{
					BStaticMeshActorRef bStaticMeshActor = make_shared<BStaticMeshActor>(*staticMeshComponent);
					UStaticMesh* staticMesh = staticMeshComponent->GetStaticMesh();
					if (staticMesh)
					{
						bStaticMeshActor->mesh = getOrAddMesh(staticMesh);
					}
					rootTansform->addChild(bStaticMeshActor);
					addSceneComponent(addSceneComponent, bStaticMeshActor, child);
				}
				else
				{
					TransformRef childAcrtor = make_shared<Transform>();
					childAcrtor->name = TCHAR_TO_UTF8(*child->GetName().Replace(TEXT("."), TEXT("_")));
					childAcrtor->position = convertPosition(child->GetRelativeLocation());
					childAcrtor->rotation = convertRotation(child->GetRelativeRotation().Quaternion());
					childAcrtor->scale = convertScale(child->GetRelativeScale3D());
					rootTansform->addChild(childAcrtor);
					addSceneComponent(addSceneComponent, childAcrtor, child);
				}
			}
		};

		for (AActor* actor : actors)
		{
			TransformRef actorTransform = nullptr;
			if (AStaticMeshActor* staticMeshActor = Cast<AStaticMeshActor>(actor))
			{
				BStaticMeshActorRef bStaticMeshActor = make_shared<BStaticMeshActor>(*staticMeshActor);
				UStaticMesh* staticMesh = staticMeshActor->GetStaticMeshComponent()->GetStaticMesh();
				if (staticMesh)
				{
					bStaticMeshActor->mesh = getOrAddMesh(staticMesh);
				}
				meshRoot->addChild(bStaticMeshActor);
				actorTransform = bStaticMeshActor;
			}
			else if (ADirectionalLight* directLight = Cast<ADirectionalLight>(actor))
			{
				actorTransform = make_shared<BDirectLight>(*directLight);
				lightRoot->addChild(actorTransform);
			}
			else if (APointLight* pointLight = Cast<APointLight>(actor))
			{
				actorTransform = make_shared<BPointLight>(*pointLight);
				lightRoot->addChild(actorTransform);
			}
			else if (ASphereReflectionCapture* sphereReflectionCapture = Cast<ASphereReflectionCapture>(actor))
			{
				BReflectionProbeRef bReflectionProbe = make_shared<BReflectionProbe>(*sphereReflectionCapture);
				captureRoot->addChild(bReflectionProbe);
				actorTransform = bReflectionProbe;
			}
			else if (actor->FindComponentByClass<UStaticMeshComponent>())
			{
				actorTransform = make_shared<Transform>();
				actorTransform->name = TCHAR_TO_UTF8(*actor->GetActorNameOrLabel());
				actorTransform->position = convertPosition(actor->GetTransform().GetLocation());
				actorTransform->rotation = convertRotation(actor->GetTransform().GetRotation());
				actorTransform->scale = convertScale(actor->GetTransform().GetScale3D());
				sceneRoot->addChild(actorTransform);
				addSceneComponent(addSceneComponent, actorTransform, actor->GetRootComponent());
			}

			if (actorTransform)
			{
				actorTransform->position *= 0.1;
				actorTransform->scale *= 0.1;
			}
		}

		if (!bOnlyExportWorldFile)
		{
			for (auto& pair : meshes)
			{
				ExportStaticMesh(pair.Value);
			}
		}

		world.writeToFile(TCHAR_TO_UTF8(*FPaths::Combine(OutFolderPath, TEXT("Content/world.asset"))));
	}

	TSharedRef<SDockTab> BConverter::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
	{
		return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			// Put your tab content here!
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				[
					SNew(SObjectPropertyEntryBox)
					.AllowedClass(UStaticMesh::StaticClass())
					.ThumbnailPool(AssetThumbnailPool)
					.ObjectPath(this, &BConverter::GetExportedStaticMeshPath)
					.OnObjectChanged(FOnSetObject::CreateLambda([this](const FAssetData& Data)
					{
						ExportedStaticMesh = Cast<UStaticMesh>(Data.GetAsset());
					}))
				]
				+ SVerticalBox::Slot()
				[
					SNew(SEditableTextBox)
					.Text(this, &BConverter::GetOutFolderPath)
					.OnTextCommitted(FOnTextCommitted::CreateLambda([this](const FText& InText, ETextCommit::Type InCommitType)
					{
						OutFolderPath = InText.ToString();
					}))
				]
				+ SVerticalBox::Slot()
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.Text(FText::FromString(TEXT("Browse")))
					.OnClicked(FOnClicked::CreateLambda([this]()
					{
						BrowseOutFolderPath();
						return FReply::Handled();
					}))
				]
				+ SVerticalBox::Slot()
				[
					SNew(SCheckBox)
					.HAlign(HAlign_Center)
					.IsChecked_Lambda([this]()
					{
						return bOnlyExportWorldFile ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
					})
					.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState)
					{
						bOnlyExportWorldFile = NewState == ECheckBoxState::Checked;
					})
					.Content()
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("OnlyExportWorldFile")))
					]
				]
				+ SVerticalBox::Slot()
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.Text(FText::FromString(TEXT("ConfigBakeOption")))
					.OnClicked(FOnClicked::CreateLambda([this]()
					{
						ConfigMaterialBakeOption();
						return FReply::Handled();
					}))
				]
				+ SVerticalBox::Slot()
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.Text(FText::FromString(TEXT("Export StaticMesh")))
					.OnClicked(FOnClicked::CreateLambda([this]()
					{
						if (ExportedStaticMesh)
							ExportStaticMesh(*ExportedStaticMesh);
						return FReply::Handled();
					}))
				]
				+ SVerticalBox::Slot()
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.Text(FText::FromString(TEXT("Export Selected Actor")))
					.OnClicked(FOnClicked::CreateLambda([this]()
					{
						ExportSelectedActor();
						return FReply::Handled();
					}))
				]
			]
		];
	}

	void BConverter::BrowseOutFolderPath()
	{
		IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
		if (DesktopPlatform)
		{
			DesktopPlatform->OpenDirectoryDialog(nullptr, TEXT("OutPath"), FString(""), OutFolderPath);
		}
	}

	bool BConverter::ConfigMaterialBakeOption()
	{
		IMaterialBakingModule& Module = FModuleManager::Get().LoadModuleChecked<IMaterialBakingModule>("MaterialBaking");
		auto BakeOptions = getBakeOptions();
		return Module.SetupMaterialBakeSettings(BakeOptions, 1);
	}

	FString BConverter::GetExportedStaticMeshPath() const
	{
		return ExportedStaticMesh ? ExportedStaticMesh->GetPathName() : FString("");
	}

	FText BConverter::GetOutFolderPath() const
	{
		return FText::FromString(OutFolderPath);
	}
}
