#pragma once

#include "DeviceResources.h"
#include "Model.h"

#include <vector>

namespace Render
{
    using namespace DirectX;
    struct Vertex
    {
        XMFLOAT3 Pos;
        XMFLOAT3 Norm;
        XMFLOAT2 Tex;
    };

    struct SceneParams
    {
        XMFLOAT4X4 WorldMat;
        XMFLOAT4X4 ViewMat;
        XMFLOAT4X4 ProjMat;
        XMFLOAT4 EyePos;
    };

    struct DirectionalLight
    {
        DirectionalLight() { ZeroMemory(this, sizeof(DirectionalLight)); }

        XMFLOAT4 Ambient;
        XMFLOAT4 Diffuse;
        XMFLOAT4 Specular;
        XMFLOAT3 Direction;
        float Pad;
    };

    struct PointLight
    {
        PointLight() { ZeroMemory(this, sizeof(PointLight)); }

        XMFLOAT4 Ambient;
        XMFLOAT4 Diffuse;
        XMFLOAT4 Specular;

        XMFLOAT3 Position;
        float Range;

        XMFLOAT3 Att;
        float Pad;
    };

    struct SpotLight
    {
        SpotLight() { ZeroMemory(this, sizeof(SpotLight)); }

        XMFLOAT4 Ambient;
        XMFLOAT4 Diffuse;
        XMFLOAT4 Specular;

        XMFLOAT3 Position;
        float Range;

        XMFLOAT3 Direction;
        float Spot;

        XMFLOAT3 Att;
        float Pad;
    };

    struct LightingParams
    {
        DirectionalLight DirLight;
        PointLight PointLight;
        SpotLight SpotLight;
    };

    static_assert((sizeof(SceneParams) % 16) == 0, "Constant buffer must always be 16-byte aligned");
    static_assert((sizeof(LightingParams) % 16) == 0, "Constant buffer must always be 16-byte aligned");


class Renderer
{
public:

	void Render(const std::vector<Model>& models) const;
    void Init(DX::DeviceResources* deviceResources, const std::vector<Model>& models);
    void Deinit();

private:
	DX::DeviceResources* m_deviceResources;
    // Sample objects
    Microsoft::WRL::ComPtr<ID3D11InputLayout>       m_inputLayout;
    Microsoft::WRL::ComPtr<ID3D11Buffer>            m_vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>            m_indexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>            m_cbSceneParams;
    Microsoft::WRL::ComPtr<ID3D11Buffer>            m_cbLightingParams;
    Microsoft::WRL::ComPtr<ID3D11VertexShader>      m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>       m_pixelShader;

    SceneParams                                     m_sceneParams;
    LightingParams                                  m_lightingParams;

    // textures
    std::vector<ID3D11ShaderResourceView*>   m_textureViews;
    std::vector<ID3D11Texture2D*>            m_textures;
    std::vector<ID3D11SamplerState*>         m_samplers;
};

} // namespace Renderer