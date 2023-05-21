#include "BraneMaterial.h"
#include <string>

namespace Brane
{
	TextureRef Texture::DefaultNormalMap = make_shared<Texture>("Engine/Textures/Default_N.png");

	Texture::Texture(const string& path) : path(path)
	{
	}

	void Texture::write(ostream& os)
	{
		os << path;
	}

	void Material::setCount(const string & param, float count)
	{
		counts[param] = count;
	}

	void Material::setScalar(const string & param, float scalar)
	{
		scalars[param] = scalar;
	}

	void Material::setColor(const string & param, const Color& color)
	{
		colors[param] = color;
	}

	void Material::setTexture(const string & param, TextureRef tex)
	{
		textures[param] = tex;
	}

	void Material::save(ostream& os) const
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
			os << ("Texture " + b->first + ": " + (b->second->path.empty() ? "white" : b->second->path) + '\n');
		}
	}

	void Material::write(ostream & os)
	{
		os << "path: \"" << path << "\", pathType: \"path\"";
	}
}
