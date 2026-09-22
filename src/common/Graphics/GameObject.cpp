#include "Pengu_Engine/Graphics/GameObject.hpp"
#include "Pengu_Engine/Graphics/Mesh.hpp"
#include "Pengu_Engine/Graphics/Material.hpp"

namespace Pengu::Graphics {
	void GameObject::addSubMesh(std::shared_ptr<Pengu::Graphics::Mesh> mesh, std::shared_ptr<Pengu::Graphics::Material> mat)
	{
		subMeshes.push_back({ mesh, mat });
	}
}