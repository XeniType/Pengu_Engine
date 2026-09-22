/**
 * @file GameObject.hpp
 * @brief Representation of a renderable object in the scene.
 */

#ifndef GAMEOBJECT_HPP
#define GAMEOBJECT_HPP 1

#include "Pengu_Engine/Graphics/Mesh.hpp"
#include "Pengu_Engine/Graphics/Material.hpp"
#include <memory>
#include <vector>

namespace Pengu::Graphics {
	/**
	 * @struct SubMeshes
	 * @brief A combination of a mesh and a material that forms part of a GameObject.
	 */
	struct SubMeshes {
		std::shared_ptr<Pengu::Graphics::Mesh> mesh_;      ///< The geometry mesh.
		std::shared_ptr<Pengu::Graphics::Material> material_; ///< The material to apply to this mesh.
		bool visible = true; ///< Per-submesh visibility flag.
	};

	/**
	 * @class GameObject
	 * @brief A collection of meshes and materials that can be rendered.
	 *
	 * A GameObject can consist of multiple sub-meshes, each with its own material.
	 * It also tracks its current Level of Detail (LOD) state.
	 */
	class GameObject {
	public:
		/**
		 * @brief Default constructor.
		 */
		GameObject() = default;

		/**
		 * @brief Destructor that clears sub-meshes.
		 */
		~GameObject() { subMeshes.clear(); };

		std::vector<SubMeshes> subMeshes; ///< List of sub-meshes forming this object.

		/**
		 * @brief Adds a sub-mesh to the game object.
		 * @param mesh Shared pointer to the mesh.
		 * @param mat Shared pointer to the material.
		 */
		void addSubMesh(std::shared_ptr<Pengu::Graphics::Mesh> mesh,
			std::shared_ptr<Pengu::Graphics::Material> mat);

		bool bVisible = true; ///< Global visibility flag for the object.

		int currentLOD = 0; ///< Currently active Level of Detail level.

		/**
		 * @brief Sets the currently active LOD level.
		 * @param level The LOD level index.
		 */
		void SetActiveLOD(int level) { currentLOD = level; };
	};
}

#endif // !GAMEOBJECT_HPP
