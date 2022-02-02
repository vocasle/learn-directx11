#include "pch.h"
#include "Model.h"

#include <fstream>
#include <string>
#include <iostream>
#include <sstream>

Model::Model():
	m_positions(),
	m_faces(),
	m_normals()
{
}

std::unique_ptr<Model> ParseObjectFile(std::fstream& f)
{
	std::vector<Face> faces;
	std::vector<Position> positions;
	std::vector<Normal> normals;

	char ch;
	Face face;
	Position p;
	Normal n;

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
				unsigned int v[3] = {0, 0, 0};
				unsigned int vt[3] = { 0, 0, 0 };
				unsigned int vn[3] = { 0, 0, 0 };
				in >> ch >> v[0] >> ch >> vt[0] >> ch >> vn[0];
				in >> v[1] >> ch >> vt[1] >> ch >> vn[1];
				in >> v[2] >> ch >> vt[2] >> ch >> vn[2];
				faces.emplace_back(--v[0], --v[1], --v[2]);
				faces.emplace_back(--vt[0], --vt[1], --vt[2]);
				faces.emplace_back(--vn[0], --vn[1], --vn[2]);
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
		else if (line.find("vn ") != std::string::npos)
		{
			in >> ch >> ch >> n.X >> n.Y >> n.Z;
			assert(ch == 'n');
			normals.push_back(n);
		}
		in.clear();
	}

	return std::make_unique<Model>(std::move(positions), std::move(faces), std::move(normals));
}

std::unique_ptr<Model> Model::LoadModel(const std::string& filepath)
{
	if (auto f = std::fstream(filepath))
	{
		return ParseObjectFile(f);
	}
	assert(false && "Failed to load model, because *.obj file not found!");
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

const std::vector<Normal>& Model::GetNormals() const
{
	return m_normals;
}

const bool Model::HasNormalFaces() const
{
	return !m_normals.empty();
}

const Normal& Model::GetNormalForIdx(unsigned int i) const
{
	return m_normals[i];
}

Model::Model(std::vector<Position>&& positions,
	std::vector<Face>&& faces, 
	std::vector<Normal>&& normals):
	m_positions(positions),
	m_faces(faces),
	m_normals(normals)
{
}

Face::Face(unsigned int x, unsigned int y, unsigned int z):
	X(x),
	Y(y),
	Z(z)
{
}
