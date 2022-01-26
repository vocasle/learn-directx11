#include "pch.h"
#include "Model.h"

#include <fstream>

Model::Model()
{
}

Model::Model(const std::vector<Position>& positions, const std::vector<Face>& faces):
	m_positions(positions),
	m_faces(faces)
{
}

std::unique_ptr<Model> ParseObjectFile(std::fstream& f)
{
	std::vector<Face> faces;
	std::vector<Position> positions;

	char ch;
	Face face;
	Position p;
	
	while (!f.eof())
	{
		f >> ch;
		if (ch == 'v')
		{
			f >> p.X >> p.Y >> p.Z;
			positions.push_back(p);
		}
		else if (ch == 'f')
		{
			f >> face.X >> face.Y >> face.Z;
			faces.push_back(face);
		}
	}

	return std::make_unique<Model>(positions, faces);
}

std::unique_ptr<Model> Model::LoadModel(const std::string& filepath)
{
	if (auto f = std::fstream(filepath))
	{
		return ParseObjectFile(f);
	}
	return std::make_unique<Model>();
}

const std::vector<Position>& Model::GetPositions() const
{
	return m_positions;
}

const std::vector<Face>& Model::GetFaces() const
{
	return m_faces;
}

