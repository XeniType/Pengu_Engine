/**
 * @file Fustrum.hpp
 * @brief Camera frustum for culling operations.
 */

#ifndef FUSTRUM_HPP
#define FUSTRUM_HPP 1

#include <glm/glm.hpp>

/**
 * @struct Plane
 * @brief Represents a plane in 3D space.
 */
struct Plane {
	/** @brief Normal vector of the plane. */
	glm::vec3 normal;
	/** @brief Distance from the origin. */
	float distance;
};

/**
 * @struct Frustum
 * @brief Represents a camera frustum composed of 6 planes.
 */
struct Frustum {
	/** @brief The 6 planes defining the frustum. */
	Plane planes[6];

	/**
	 * @brief Checks if the frustum contains a sphere.
	 * @param center Center of the sphere.
	 * @param radius Radius of the sphere.
	 * @return true if the sphere is within or intersecting the frustum.
	 */
	bool ContainsSphere(const glm::vec3& center, float radius) const;
};

/**
 * @brief Extracts the frustum from a view-projection matrix.
 * @param viewProjMatrix The view-projection matrix.
 * @return Frustum The extracted frustum.
 */
Frustum ExtractFrustum(const glm::mat4& viewProjMatrix);

/**
 * @brief Checks if an Axis-Aligned Bounding Box (AABB) is in the frustum.
 * @param frustum The frustum to check against.
 * @param minPoint Minimum point of the AABB.
 * @param maxPoint Maximum point of the AABB.
 * @return true if the AABB is within or intersecting the frustum.
 */
bool IsAABBInFrustum(const Frustum& frustum, const glm::vec3& minPoint, const glm::vec3& maxPoint);

#endif // !FUSTRUM_HPP
