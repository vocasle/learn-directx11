#pragma once

#include <vector>
#include <string>

struct Position
{
	float X;
	float Y;
	float Z;
};

struct Face
{
	unsigned int X;
	unsigned int Y;
	unsigned int Z;

	Face(unsigned int x, unsigned int y, unsigned int z);
	Face() = default;
};

struct Normal
{
	float X;
	float Y;
	float Z;
};

class Model
{
public:
	Model();
	Model(
		std::vector<Position>&& positions, 
		std::vector<Face>&& faces, 
		std::vector<Normal>&& normals);
	static std::unique_ptr<Model> LoadModel(const std::string& filepath);

	const std::vector<Position>& GetPositions() const;
	const std::vector<Face>& GetFaces() const;
	const std::vector<Normal>& GetNormals() const;

private:

	std::vector<Position> m_positions;
	std::vector<Face> m_faces;
	std::vector<Normal> m_normals;
};

