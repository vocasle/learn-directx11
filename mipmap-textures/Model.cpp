#include "pch.h"
#include "Model.h"

#include <fstream>
#include <string>
#include <iostream>
#include <sstream>

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

	std::string line;
	std::istringstream in;
	
	while (std::getline(f, line))
	{
		in.str(line);
		if (line.find("v ") != std::string::npos)
		{
			in >> ch >> p.X >> p.Y >> p.Z;
			assert(ch == 'v');
			positions.push_back(p);
		}
		else if (line.find("f ") != std::string::npos)
		{
			// NOTE: Found by debugging. Indices in *.obj start from 1, not from 0!
			if (line.find("/") != std::string::npos)
			{
				unsigned int v = 0;
				unsigned int vt = 0;
				unsigned int vn = 0;
				in >> ch >> v >> ch >> vt >> ch >> vn;
				face.X = v;
				in >> v >> ch >> vt >> ch >> vn;
				face.Y = v;
				in >> v >> ch >> vt >> ch >> vn;
				face.Z = v;
				--face.X;
				--face.Y;
				--face.Z;
				faces.push_back(face);
			}
			else
			{
				in >> ch >> face.X >> face.Y >> face.Z;
				assert(ch == 'f');
				--face.X;
				--face.Y;
				--face.Z;
				faces.push_back(face);
			}
		}
		in.clear();
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

