#include "BraneActor.h"
#include <fstream>

namespace Brane
{
	void Mesh::write(ostream& os)
	{
		os << "path: \"" << path << "\", pathType: \"path\"";
	}

	void Transform::addChild(TransformRef child)
	{
		child->unparent();
		child->parent = this;
		children.insert(child);
	}

	void Transform::unparent()
	{
		if (parent)
		{
			for (auto b = parent->children.begin(), e = parent->children.end(); b != e; b++)
			{
				if (b->get() == this)
				{
					parent->children.erase(b);
					break;
				}
			}
			parent = nullptr;
		}
	}

	void Transform::write(ostream& os)
	{
		os << "position: " << position << ", rotation: " << rotation
		<< ", scale: " << scale << ", children: None { ";
		for (auto& child : children)
		{
			os << child->name << ": " << *child << ", ";
		}
		os << " }";
	}

	bool Transform::writeToFile(const string& path)
	{
		ofstream file = ofstream(path);
		if (file.fail())
			return false;
		if (name.empty())
			file << *this;
		else
			file << name << ": " << *this;
		file.close();
		return true;
	}

	void MeshActor::setMaterial(const string& slotName, MaterialRef material)
	{
		materials.push_back(make_pair(slotName, material));
	}

	void MeshActor::write(ostream& os)
	{
		Transform::write(os);
		os << ", mesh: " << *mesh << ", materials: AssetSearch [ ";
		for (auto& material : materials)
		{
			os << "{ ";
			material.second->write(os);
			os << " }, ";
		}
		os << " ]";
	}

	Camera::Camera()
	{
		ssaoMat.baseMatPath = "Engine/Shaders/PostProcess/SSAOPassFS.mat";
		ssaoMat.setScalar("ssaoIntensity", 1.5);
		ssaoMat.setScalar("worldRadius", 10);
		ssaoMat.setCount("GTAO_NUMTAPS", 10);
		ssaoMat.setCount("numAngles", 4);
		ssaoMat.setColor("baseColor", Color(1.0f, 1.0f, 1.0f, 1.0f));

		dofMat.baseMatPath = "Engine/Shaders/PostProcess/DOFPassFS.mat";
		dofMat.setScalar("blurIter", 3);
		dofMat.setScalar("blurRadius", 4);
		dofMat.setScalar("focusDistance", 100);
		dofMat.setColor("baseColor", Color(1.0f, 1.0f, 1.0f, 1.0f));

		bloomMat.baseMatPath = "Engine/Shaders/PostProcess/BloomPassFS.mat";
		bloomMat.setScalar("bloomThreshold", 0.8);

		volumetricLightMat.baseMatPath = "Engine/Shaders/PostProcess/VolumetricLightFS.mat";
		volumetricLightMat.setScalar("G_SCATTERING", 0.5);
		volumetricLightMat.setScalar("fogDistance", 1000);
		volumetricLightMat.setCount("volumnicFogBlurIter", 3);
		volumetricLightMat.setCount("volumnicFogStep", 50);
		volumetricLightMat.setCount("volumnicFogStepDebug", 0);
		volumetricLightMat.setColor("baseColor", Color(1.0f, 1.0f, 1.0f, 1.0f));
		volumetricLightMat.setColor("volumnicLightColor", Color(0.819608, 0.654902, 0.4, 1));

		toneMapMat.baseMatPath = "Engine/Shaders/PostProcess/ToneMapPassFS.mat";
		toneMapMat.setScalar("exposure", 3);
		toneMapMat.setScalar("gamma", 1);
		toneMapMat.setScalar("lutBlend", 0);
		toneMapMat.setScalar("vignettePower", 2);
		toneMapMat.setScalar("vignetteRate", 0);
		toneMapMat.setColor("baseColor", Color(1.0f, 1.0f, 1.0f, 1.0f));
		toneMapMat.setColor("vignetteColor", Color(0.0f, 0.0f, 0.0f, 1.0f));
		TextureRef lutTexture = make_shared<Texture>();
		lutTexture->path = "Engine/LUTS/sedona_lut.png";
		toneMapMat.setTexture("lutMap", lutTexture);
	}

	void Camera::write(ostream& os)
	{
		Transform::write(os);
		os << ", fov: " << fov << ", distance: " << distance << ", clearColor: "
		<< clearColor << ", postProcessingGraph: PostProcessGraph { ";
		
		os << "SSAO: PostProcessPass { name: \"SSAO\", enable: \"true\", material: \"";
		ssaoMat.save(os);
		os << "\" }, ";

		os << "DOF: PostProcessPass { name: \"DOF\", enable: \"true\", material: \"";
		dofMat.save(os);
		os << "\" }, ";

		os << "Bloom: PostProcessPass { name: \"Bloom\", enable: \"true\", material: \"";
		bloomMat.save(os);
		os << "\" }, ";

		os << "VolumetricLight: PostProcessPass { name: \"VolumetricLight\", enable: \"true\", material: \"";
		volumetricLightMat.save(os);
		os << "\" }, ";

		os << "ToneMap: PostProcessPass { name: \"ToneMap\", enable: \"true\", material: \"";
		toneMapMat.save(os);
		os << "\" }, }";
	}

	void Light::write(ostream& os)
	{
		Transform::write(os);
		os << ", intensity: " << intensity << ", attenuation: " << attenuation << ", color: " << color;
	}

	void PointLight::write(ostream& os)
	{
		Light::write(os);
		os << ", radius: " << radius;
	}

	DebugCamera::DebugCamera()
	{
		name = "DebugCamera";
	}

	void DebugCamera::write(ostream& os)
	{
		Camera::write(os);
	}

	SkySphere::SkySphere()
	{
		name = "SkySphere";
	}

	void SkySphere::write(ostream& os)
	{
		os << "guid: \"3E8DD40C-CCB8-4E4D-B5F7-E930ACD6904C\", behaviors: None {  }, children: None {  }, "
		"position: SVector3f { x: 0.000000, y: 0.000000, z: 0.000000,  }, "
		"rotation: SQuaternionf { x: 0.000000, y: 0.000000, z: 0.000000, w: 1.000000,  }, "
		"scale: SVector3f { x: 80.000000, y: 80.000000, z: 80.000000,  }, Audio: AudioSources {  }";
	}

	void CaptureProbe::write(ostream& os)
	{
		Transform::write(os);
		os << ", captureProbeRender: ReflectionCaptureProbeRender { radius: " << radius << ",  }";
	}
}
