#include "Pengu_Engine/Graphics/Material.hpp"
#include "Pengu_Engine/Graphics/Texture.hpp"
#include "Pengu_Engine/Graphics/Shader.hpp"

namespace Pengu::Graphics {
	void Material::Bind(const Shader& s) const
	{
		// Albedo
		if (albedoMap) {
			albedoMap->Bind(0);
			s.setInt("u_hasAlbedoMap", 1);
			s.setInt("u_albedoMap", 0);
		}
		else {
			s.setInt("u_hasAlbedoMap", 0);
			s.setVec4("u_albedoColor", albedoColor);
		}

		// Normal map
		if (normalMap) {
			normalMap->Bind(1);
			s.setInt("u_hasNormalMap", 1);
			s.setInt("u_normalMap", 1);
		}
		else {
			s.setInt("u_hasNormalMap", 0);
		}

		// Roughness map
		if (roughnessMap) {
			roughnessMap->Bind(2);
			s.setInt("u_hasRoughnessMap", 1);
			s.setInt("u_roughnessMap", 2);
		}
		else {
			s.setInt("u_hasRoughnessMap", 0);
			s.setFloat("u_roughness", roughness);
		}

		// Metallic map
		if (metallicMap) {
			metallicMap->Bind(3);
			s.setInt("u_hasMetallicMap", 1);
			s.setInt("u_metallicMap", 3);
		}
		else {
			s.setInt("u_hasMetallicMap", 0);
			s.setFloat("u_metallic", metallic);
		}

		// AO map
		if (aoMap) {
			aoMap->Bind(4);
			s.setInt("u_hasAOMap", 1);
			s.setInt("u_aoMap", 4);
		}
		else {
			s.setInt("u_hasAOMap", 0);
		}
	}
}