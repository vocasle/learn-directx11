#pragma once

#include <vector>
#include <string>

#include <d3d11.h>

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

struct TextCoord
{
	float X;
	float Y;
};

struct Material
{
	Material() { ZeroMemory(this, sizeof(Material)); }

	float Ambient[4];
	float Diffuse[4];
	float Specular[4]; // w = SpecPower
	float Reflect[4];
};

struct SurfaceParams
{
	Material Mat;
	bool HasTexture = false;
};

class Model
{
public:
	Model();
	Model(
		std::vector<Position>&& positions, 
		std::vector<Face>&& faces, 
		std::vector<Normal>&& normals,
		std::vector<TextCoord>&& textCoords,
		const char* textPath);
	static std::unique_ptr<Model> LoadModel(const std::string& filepath);

	const std::vector<Position>& GetPositions() const;
	const std::vector<Face>& GetFaces() const;
	const std::vector<Normal>& GetNormals() const;
	const std::vector<TextCoord>& GetTextCoords() const;
	void LoadTexture(ID3D11Device* device);
	ID3D11SamplerState* GetTextureSampler();
	ID3D11ShaderResourceView* GetTextureView();

private:

	std::vector<Position>		m_positions;
	std::vector<Face>			m_faces;
	std::vector<Normal>			m_normals;
	std::vector<TextCoord>		m_textCoords;
	std::string					m_textPath;

	// texture related
	Microsoft::WRL::ComPtr<ID3D11Texture2D>				m_text;
	Microsoft::WRL::ComPtr<ID3D11SamplerState>			m_sampler;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_textView;

};

