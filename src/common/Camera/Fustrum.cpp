#include "Pengu_Engine/Camera/Fustrum.hpp"

Frustum ExtractFrustum(const glm::mat4& viewProjMatrix) {

	Frustum frustum;

	// Left
	frustum.planes[0].normal.x = viewProjMatrix[0][3] + viewProjMatrix[0][0];
	frustum.planes[0].normal.y = viewProjMatrix[1][3] + viewProjMatrix[1][0];
	frustum.planes[0].normal.z = viewProjMatrix[2][3] + viewProjMatrix[2][0];
	frustum.planes[0].distance = viewProjMatrix[3][3] + viewProjMatrix[3][0];
	// Right
	frustum.planes[1].normal.x = viewProjMatrix[0][3] - viewProjMatrix[0][0];
	frustum.planes[1].normal.y = viewProjMatrix[1][3] - viewProjMatrix[1][0];
	frustum.planes[1].normal.z = viewProjMatrix[2][3] - viewProjMatrix[2][0];
	frustum.planes[1].distance = viewProjMatrix[3][3] - viewProjMatrix[3][0];
	// Bottom
	frustum.planes[2].normal.x = viewProjMatrix[0][3] + viewProjMatrix[0][1];
	frustum.planes[2].normal.y = viewProjMatrix[1][3] + viewProjMatrix[1][1];
	frustum.planes[2].normal.z = viewProjMatrix[2][3] + viewProjMatrix[2][1];
	frustum.planes[2].distance = viewProjMatrix[3][3] + viewProjMatrix[3][1];
	// Top
	frustum.planes[3].normal.x = viewProjMatrix[0][3] - viewProjMatrix[0][1];
	frustum.planes[3].normal.y = viewProjMatrix[1][3] - viewProjMatrix[1][1];
	frustum.planes[3].normal.z = viewProjMatrix[2][3] - viewProjMatrix[2][1];
	frustum.planes[3].distance = viewProjMatrix[3][3] - viewProjMatrix[3][1];
	// Near
	frustum.planes[4].normal.x = viewProjMatrix[0][3] + viewProjMatrix[0][2];
	frustum.planes[4].normal.y = viewProjMatrix[1][3] + viewProjMatrix[1][2];
	frustum.planes[4].normal.z = viewProjMatrix[2][3] + viewProjMatrix[2][2];
	frustum.planes[4].distance = viewProjMatrix[3][3] + viewProjMatrix[3][2];
	// Far
	frustum.planes[5].normal.x = viewProjMatrix[0][3] - viewProjMatrix[0][2];
	frustum.planes[5].normal.y = viewProjMatrix[1][3] - viewProjMatrix[1][2];
	frustum.planes[5].normal.z = viewProjMatrix[2][3] - viewProjMatrix[2][2];
	frustum.planes[5].distance = viewProjMatrix[3][3] - viewProjMatrix[3][2];

	// Normalize planes
	for (int i = 0; i < 6; i++) {
		float length = glm::length(frustum.planes[i].normal);
		frustum.planes[i].normal /= length;
		frustum.planes[i].distance /= length;
	}
	return frustum;
}

bool IsAABBInFrustum(const Frustum& frustum, const glm::vec3& minPoint, const glm::vec3& maxPoint) {
	for (int i = 0; i < 6; i++) {
		glm::vec3 positiveVertex = minPoint;
		if (frustum.planes[i].normal.x >= 0) positiveVertex.x = maxPoint.x;
		if (frustum.planes[i].normal.y >= 0) positiveVertex.y = maxPoint.y;
		if (frustum.planes[i].normal.z >= 0) positiveVertex.z = maxPoint.z;

		if (glm::dot(frustum.planes[i].normal, positiveVertex) + frustum.planes[i].distance < 0) {
			return false;
		}
	}
	return true; // Box is visible
}

bool Frustum::ContainsSphere(const glm::vec3& center, float radius) const
{
	for (int i = 0; i < 6; ++i) {
		float dist = glm::dot(planes[i].normal, center) + planes[i].distance;

		if (dist < -radius) {
			return false;
		}
	}
	return true;
}
