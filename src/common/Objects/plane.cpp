#include "Pengu_Engine/Objects/plane.hpp"

Plane::Plane(const int rows, const int cols, const int size)
{
	int half_rows = rows / 2;
	int half_cols = cols / 2;

	for (int row = 0; row < rows; row++)
	{
		for (int col = 0; col < cols; col++)
		{
			Pengu::Graphics::Vertex v;
			v.position = { (col - half_cols) * size, 0.0f, (row - half_rows) * size };
			v.normal = { 0.0f, 1.0f, 0.0f };
			v.uv = { static_cast<float>(col / (cols - 1)), 1.0f - static_cast<float>(row / (rows - 1)) };
			data_.vertices.push_back(v);
		}
	}

	for (int row = 0; row < rows - 1; row++)
	{
		for (int col = 0; col < cols - 1; col++)
		{
			unsigned int tl = row * cols + col;
			unsigned int tr = row * cols + col + 1;
			unsigned int bl = (row + 1) * cols + col;
			unsigned int br = (row + 1) * cols + col + 1;
			data_.indices.insert(data_.indices.end(), { tl, bl, tr, bl, br, tr });
		}
	}
}
