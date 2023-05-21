#pragma once
#include "BraneCore.h"
#include <map>

namespace Brane
{
	using namespace std;

	class Texture;
	typedef shared_ptr<Texture> TextureRef;

	class Texture : public IWritable
	{
	public:
		BESerialize(Texture);

		static TextureRef DefaultNormalMap;

		string path;

		Texture() = default;
		Texture(const string& path);

		virtual void write(ostream& os);
	};

	class Material : public IWritable
	{
	public:
		BESerialize(AssetSearch);
		
		string baseMatPath;

		string name;
		string path;
		
		bool isTwoSide = false;
		bool cullFront = false;
		bool canCastShadow = true;
		map<string, float> scalars;
		map<string, int> counts;
		map<string, Color> colors;
		map<string, TextureRef> textures;

		Material() = default;
		virtual ~Material() = default;

		void setCount(const string& param, float count);
		void setScalar(const string& param, float scalar);
		void setColor(const string& param, const Color& color);
		void setTexture(const string& param, TextureRef tex);

		virtual void save(ostream& os) const;
		virtual void write(ostream& os);
	};

	typedef shared_ptr<Material> MaterialRef;
}