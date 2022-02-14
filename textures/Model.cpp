#include "pch.h"
#include "Model.h"

#include <string>
#include <iostream>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <wincodec.h>

Model::Model():
	m_positions(),
	m_faces(),
	m_normals()
{
}

std::vector<uint8_t> LoadBGRAImage(const wchar_t* filename, uint32_t& width, uint32_t& height)
{
	using namespace Microsoft::WRL;
	ComPtr<IWICImagingFactory> wicFactory;
	DX::ThrowIfFailed(CoCreateInstance(CLSID_WICImagingFactory2, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&wicFactory)));

	ComPtr<IWICBitmapDecoder> decoder;
	DX::ThrowIfFailed(wicFactory->CreateDecoderFromFilename(filename, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, decoder.GetAddressOf()));

	ComPtr<IWICBitmapFrameDecode> frame;
	DX::ThrowIfFailed(decoder->GetFrame(0, frame.GetAddressOf()));

	DX::ThrowIfFailed(frame->GetSize(&width, &height));

	WICPixelFormatGUID pixelFormat;
	DX::ThrowIfFailed(frame->GetPixelFormat(&pixelFormat));

	uint32_t rowPitch = width * sizeof(uint32_t);
	uint32_t imageSize = rowPitch * height;

	std::vector<uint8_t> image;
	image.resize(size_t(imageSize));

	if (memcmp(&pixelFormat, &GUID_WICPixelFormat32bppBGRA, sizeof(GUID)) == 0)
	{
		DX::ThrowIfFailed(frame->CopyPixels(nullptr, rowPitch, imageSize, reinterpret_cast<BYTE*>(image.data())));
	}
	else
	{
		ComPtr<IWICFormatConverter> formatConverter;
		DX::ThrowIfFailed(wicFactory->CreateFormatConverter(formatConverter.GetAddressOf()));

		BOOL canConvert = FALSE;
		DX::ThrowIfFailed(formatConverter->CanConvert(pixelFormat, GUID_WICPixelFormat32bppBGRA, &canConvert));
		if (!canConvert)
		{
			throw std::exception("CanConvert");
		}

		DX::ThrowIfFailed(formatConverter->Initialize(frame.Get(), GUID_WICPixelFormat32bppBGRA,
			WICBitmapDitherTypeErrorDiffusion, nullptr, 0, WICBitmapPaletteTypeMedianCut));

		DX::ThrowIfFailed(formatConverter->CopyPixels(nullptr, rowPitch, imageSize, reinterpret_cast<BYTE*>(image.data())));
	}

	return image;
}

std::wstring StrToWstr(const std::string& s)
{
	size_t numConverted = 0;
	std::unique_ptr<wchar_t*> bytes = std::make_unique<wchar_t*>(new wchar_t[s.size() + 1]);
	mbstowcs_s(&numConverted, *bytes, s.size() + 1, s.c_str(), s.size());
	return std::wstring(*bytes);
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
	std::vector<TextCoord> textCoords;
	textCoords.reserve(mesh->mNumVertices);

	for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
	{
		positions.push_back({ mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z });
		if (mesh->mTextureCoords[0])
			textCoords.push_back({ mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y });
		else
			textCoords.push_back({ 0.0f, 0.0f });
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

	const aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
	aiString textPath;
	// TODO: add support for specular and normal maps
	mat->GetTexture(aiTextureType_DIFFUSE, 0, &textPath);


	return std::make_unique<Model>(std::move(positions), std::move(faces), std::move(normals), std::move(textCoords), textPath.C_Str());
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
	std::vector<TextCoord>&& textCoords,
	const char* textPath):
	m_positions(positions),
	m_faces(faces),
	m_normals(normals),
	m_textCoords(textCoords),
	m_textPath(textPath)
{
}

Face::Face(unsigned int x, unsigned int y, unsigned int z):
	X(x),
	Y(y),
	Z(z)
{
}

std::string FindTexture(const std::string& filename)
{
	// TODO: replace with real filesystem search
	return "../assets/" + filename;
}

void Model::LoadTexture(ID3D11Device* device)
{
	uint32_t width = 0;
	uint32_t height = 0;
	const std::string fullTextPath = FindTexture(m_textPath);
	const std::vector<uint8_t> bytes = LoadBGRAImage(StrToWstr(fullTextPath).c_str(), width, height);

	D3D11_TEXTURE2D_DESC txtDesc = {};
	txtDesc.MipLevels = txtDesc.ArraySize = 1;
	txtDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB; // sunset.jpg is in sRGB colorspace
	txtDesc.SampleDesc.Count = 1;
	txtDesc.Usage = D3D11_USAGE_IMMUTABLE;
	txtDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	txtDesc.Width = width;
	txtDesc.Height = height;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = &bytes[0];
	initData.SysMemPitch = txtDesc.Width * sizeof(uint32_t);

	DX::ThrowIfFailed(device->CreateTexture2D(&txtDesc, &initData, m_text.ReleaseAndGetAddressOf()));
	DX::ThrowIfFailed(device->CreateShaderResourceView(m_text.Get(), nullptr, m_textView.ReleaseAndGetAddressOf()));

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	DX::ThrowIfFailed(
		device->CreateSamplerState(&samplerDesc,
			m_sampler.ReleaseAndGetAddressOf()));
}

ID3D11SamplerState* Model::GetTextureSampler()
{
	return m_sampler.Get();
}

ID3D11ShaderResourceView* Model::GetTextureView()
{
	return m_textView.Get();
}
