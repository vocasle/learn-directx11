#include "pch.h"
#include "Model.h"

#include <string>
#include <iostream>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

Model::Model():
	m_positions(),
	m_faces(),
	m_normals()
{
}

std::unique_ptr<Model> ParseObjectFile(const std::string& filepath)
{
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(filepath,
		aiProcess_JoinIdenticalVertices |
		aiProcess_ImproveCacheLocality |
		aiProcess_FlipUVs |
		aiProcess_GenNormals
	);
	if (!scene)
	{
		std::cerr << "ERROR: Failed to import " << filepath << std::endl;
		return nullptr;
	}

	assert(scene->mNumMeshes == 1); // for now we support only one mesh
	
	std::vector<Position> positions;
	const aiMesh* mesh = scene->mMeshes[0];
	positions.reserve(mesh->mNumVertices);
	for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
	{
		positions.push_back({ mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z });
	}

	std::vector<Normal> normals;
	if (mesh->HasNormals())
	{
		normals.reserve(mesh->mNumVertices);
		for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
		{
			normals.push_back({ mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z });
		}
	}

	std::vector<Face> faces;
	faces.reserve(mesh->mNumFaces);

	for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
	{
		assert(mesh->mFaces[i].mNumIndices == 3); // we support triangulated meshes only
		faces.push_back({ mesh->mFaces[i].mIndices[0],
			mesh->mFaces[i].mIndices[1] ,
			mesh->mFaces[i].mIndices[2] });
	}
	std::vector<TextCoord> textCoords;
	if (mesh->HasTextureCoords(0))
	{
		textCoords.reserve(mesh->mNumVertices);
		for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
		{
			textCoords.push_back({ mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y });
		}
	}

	return std::make_unique<Model>(std::move(positions), std::move(faces), std::move(normals), std::move(textCoords));
}

std::unique_ptr<Model> Model::LoadModel(const std::string& filepath)
{
	return ParseObjectFile(filepath);
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

const std::vector<TextCoord>& Model::GetTextCoords() const
{
	return m_textCoords;
}

Model::Model(std::vector<Position>&& positions,
	std::vector<Face>&& faces, 
	std::vector<Normal>&& normals,
	std::vector<TextCoord>&& textCoords):
	m_positions(positions),
	m_faces(faces),
	m_normals(normals),
	m_textCoords(textCoords)
{
}

Face::Face(unsigned int x, unsigned int y, unsigned int z):
	X(x),
	Y(y),
	Z(z)
{
}
