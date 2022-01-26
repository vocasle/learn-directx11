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
};

class Model
{
public:
	Model();
	Model(const std::vector<Position>& positions, const std::vector<Face>& faces);
	static std::unique_ptr<Model> LoadModel(const std::string& filepath);

	const std::vector<Position>& GetPositions() const;
	const std::vector<Face>& GetFaces() const;

private:
	std::vector<Position> m_positions;
	std::vector<Face> m_faces;
};

