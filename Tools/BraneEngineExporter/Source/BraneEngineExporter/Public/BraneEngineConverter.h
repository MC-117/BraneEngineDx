#pragma once
#include "CoreMinimal.h"
#include "BraneActor.h"
#include "MaterialOptions.h"
#include "Components/PointLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PointLight.h"
#include "Engine/SphereReflectionCapture.h"

namespace Brane
{
	class BTexture : public Texture
	{
	public:
		UTexture* texture = nullptr;
		float outputGamma = 1;

		BTexture(UTexture& texture);

		bool exportTexture(const FString& rootPath);
	};

	typedef shared_ptr<BTexture> BTextureRef;

	class BMaterial : public Material
	{
	public:
		BMaterial() = default;

		bool exportMaterial(const FString& rootPath);
	};

	typedef shared_ptr<BMaterial> BMaterialRef;
	
	class BStaticMesh : public Mesh
	{
	public:
		UStaticMesh* staticMesh = nullptr;

		TArray<BTextureRef> textures;
		TArray<BMaterialRef> materials;
		//<SectionIndex, MaterialIndex>
		TMap<uint32, uint32> materialMap;
		
		BStaticMesh(UStaticMesh& staticMesh);
		virtual ~BStaticMesh();

		void bakeMaterial(UMaterialOptions& MaterialOptions);
		bool exportMesh(const FString& rootPath);
	};

	typedef shared_ptr<BStaticMesh> BStaticMeshRef;

	inline Vector3f convertPosition(const FVector& right)
	{
		Vector3f left;
		left.x = right.X;
		left.y = -right.Y;
		left.z = right.Z;
		return left;
	}

	inline Vector3f convertScale(const FVector& right)
	{
		Vector3f left;
		left.x = right.X;
		left.y = right.Y;
		left.z = right.Z;
		return left;
	}

	inline Vector3f convertRotation(const FVector& right)
	{
		Vector3f left;
		left.x = right.X;
		left.y = right.Y;
		left.z = right.Z;
		return left;
	}

	inline Quaternionf convertRotation(const FQuat& right)
	{
		Quaternionf left;
		left.x = -right.X;
		left.y = right.Y;
		left.z = -right.Z;
		left.w = right.W;
		return left;
	}

	inline Color toColor(const FLinearColor& right)
	{
		Color left;
		left.r = right.R;
		left.g = right.G;
		left.b = right.B;
		left.a = right.A;
		return left;
	}

	class BStaticMeshActor : public MeshActor
	{
	public:
		AActor* ownedActor = nullptr;
		UStaticMeshComponent* staticMeshComponent = nullptr;

		BStaticMeshActor(AStaticMeshActor& staticMeshActor);
		BStaticMeshActor(UStaticMeshComponent& staticMeshComponent);

		void collectMaterial();
		virtual void write(ostream& os);
	};

	typedef shared_ptr<BStaticMeshActor> BStaticMeshActorRef;

	class BDirectLight : public DirectLight
	{
	public:
		ADirectionalLight* directLight = nullptr;

		BDirectLight(ADirectionalLight& directLight);
	};

	typedef shared_ptr<BDirectLight> BDirectLightRef;

	class BPointLight : public PointLight
	{
	public:
		APointLight* pointLight = nullptr;

		BPointLight(APointLight& pointLight);
	};

	typedef shared_ptr<BPointLight> BPointLightRef;

	class BReflectionProbe : public CaptureProbe
	{
	public:
		ASphereReflectionCapture* sphereReflectionCapture = nullptr;

		BReflectionProbe(ASphereReflectionCapture& sphereReflectionCapture);
	};

	typedef shared_ptr<BReflectionProbe> BReflectionProbeRef;

	class BConverter : public TSharedFromThis<BConverter>
	{
	public:
		BConverter();

		TArray<TWeakObjectPtr<UObject>> getBakeOptions();

		void ExportStaticMesh(MeshRef mesh);
		void ExportStaticMesh(UStaticMesh& staticMesh);
		void ExportSelectedActor();

		TSharedRef<class SDockTab> OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs);
	protected:
		TObjectPtr<UMaterialOptions> MaterialOptions;
		TSharedPtr<FAssetThumbnailPool> AssetThumbnailPool;

		UStaticMesh* ExportedStaticMesh = nullptr;
		FString OutFolderPath;
		bool bOnlyExportWorldFile = false;

		void BrowseOutFolderPath();
		bool ConfigMaterialBakeOption();
		FString GetExportedStaticMeshPath() const;
		FText GetOutFolderPath() const;
	};
}
