#include "pch.h"
#include "Renderer.h"

#include "ReadData.h"

using namespace Render;

void Renderer::Render(const std::vector<Model>& models) const
{
    ID3D11DeviceContext* context = m_deviceResources->GetD3DDeviceContext();
    // Set the vertex buffer
    constexpr UINT strides = sizeof(Vertex);
    constexpr UINT offsets = 0;
    context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &strides, &offsets);
    // Set the index buffer
    context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    // Set input assembler state
    context->IASetInputLayout(m_inputLayout.Get());
    // Set the primitive topology
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Set scene data cbuffer
    {
        D3D11_MAPPED_SUBRESOURCE mapped;
        DX::ThrowIfFailed(context->Map(m_cbSceneParams.Get(), 0,
            D3D11_MAP_WRITE_DISCARD, 0, &mapped));
        memcpy(mapped.pData, &m_sceneParams, sizeof(SceneParams));
        context->Unmap(m_cbSceneParams.Get(), 0);
    }

    // Set lighting data cbuffer
    {
        D3D11_MAPPED_SUBRESOURCE mapped;
        DX::ThrowIfFailed(context->Map(m_cbLightingParams.Get(), 0,
            D3D11_MAP_WRITE_DISCARD, 0, &mapped));
        memcpy(mapped.pData, &m_lightingParams, sizeof(LightingParams));
        context->Unmap(m_cbLightingParams.Get(), 0);
    }

    // Vertex shader needs view and projection matrices to perform vertex transform
    context->VSSetConstantBuffers(0, 1, m_cbSceneParams.GetAddressOf());
    // Pixel shader needs lighting data
    context->PSSetConstantBuffers(0, 1, m_cbLightingParams.GetAddressOf());
    // Set vertex shader
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    // Set pixel shader
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

    assert(m_samplers.size() == m_textureViews.size());

    // Set texture and sampler.
    context->PSSetSamplers(0, m_samplers.size(), &m_samplers[0]);

    context->PSSetShaderResources(0, m_textureViews.size(), &m_textureViews[0]);

    // Experiment with rasterizer state
    //D3D11_RASTERIZER_DESC rsDesc;
    //ZeroMemory(&rsDesc, sizeof(D3D11_RASTERIZER_DESC));
    //rsDesc.FillMode = D3D11_FILL_SOLID;
    //rsDesc.CullMode = D3D11_CULL_NONE;
    //rsDesc.FrontCounterClockwise = false;
    //rsDesc.DepthClipEnable = false;

    //ComPtr<ID3D11RasterizerState> pRastState;
    //DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateRasterizerState(&rsDesc, pRastState.ReleaseAndGetAddressOf()));
    //context->RSSetState(pRastState.Get());

    unsigned int indexOffset = 0;
    unsigned int vertexOffset = 0;
    for (const Model& m : models)
    {
        // Draw indexed
        context->DrawIndexed(m.GetFaces().size() * 3, indexOffset, vertexOffset);
        indexOffset += m.GetFaces().size() * 3;
        vertexOffset += m.GetPositions().size();
    }
}

void Renderer::Init(DX::DeviceResources* deviceResources, const std::vector<Model>& models)
{
    m_deviceResources = deviceResources;

    // preoccupy memory
    std::vector<Vertex> vertexData;
    std::vector<unsigned int> indexData;
    size_t numVertices = 0;
    size_t numIndices = 0;
    for (const Model& m : models)
    {
        numVertices += m.GetPositions().size();
        numIndices += m.GetFaces().size() * 3;
    }
    vertexData.reserve(numVertices);
    indexData.reserve(numIndices);

    ID3D11Device* device = deviceResources->GetD3DDevice();

    // Load and create shaders
    {
        const auto vertexShaderBlob = DX::ReadData(L"VertexShader.cso");

        DX::ThrowIfFailed(
            device->CreateVertexShader(vertexShaderBlob.data(), vertexShaderBlob.size(),
                nullptr, m_vertexShader.ReleaseAndGetAddressOf()));

        const auto pixelShaderBlob = DX::ReadData(L"PixelShader.cso");

        DX::ThrowIfFailed(
            device->CreatePixelShader(pixelShaderBlob.data(), pixelShaderBlob.size(),
                nullptr, m_pixelShader.ReleaseAndGetAddressOf()));

        // Create input layout
        static constexpr D3D11_INPUT_ELEMENT_DESC s_inputElementDesc[3] = {
            {"SV_Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXTCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
        };

        DX::ThrowIfFailed(
            device->CreateInputLayout(s_inputElementDesc, _countof(s_inputElementDesc),
                vertexShaderBlob.data(), vertexShaderBlob.size(),
                m_inputLayout.ReleaseAndGetAddressOf()));
    }

    // create vertex buffer
    {
        for (const Model& m : models)
        {
            for (unsigned int i = 0; i < m.GetPositions().size(); ++i)
            {
                const Position& p = m.GetPositions()[i];
                const Normal& n = m.GetNormals()[i];
                if (m.GetTextCoords().empty())
                {
                    vertexData.push_back({
                        XMFLOAT3(p.X, p.Y, p.Z),
                        XMFLOAT3(n.X, n.Y, n.Z),
                        XMFLOAT2(0.0f, 0.0f)
                        });
                }
                else
                {
                    const TextCoord& tc = m.GetTextCoords()[i];
                    vertexData.push_back({
                        XMFLOAT3(p.X, p.Y, p.Z),
                        XMFLOAT3(n.X, n.Y, n.Z),
                        XMFLOAT2(tc.X, tc.Y)
                        });
                }
            }
        }

        D3D11_SUBRESOURCE_DATA initialData = {};
        initialData.pSysMem = &vertexData[0];

        D3D11_BUFFER_DESC bufferDesc = {};
        bufferDesc.ByteWidth = sizeof(Vertex) * vertexData.size();
        bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bufferDesc.StructureByteStride = sizeof(Vertex);

        DX::ThrowIfFailed(device->CreateBuffer(&bufferDesc, 
            &initialData, 
            m_vertexBuffer.ReleaseAndGetAddressOf()));
    }

    // create index buffer
    {
	    for (const Model& m : models)
	    {
            for (unsigned int i = 0; i < m.GetFaces().size(); ++i)
            {
                indexData.push_back(m.GetFaces()[i].X);
                indexData.push_back(m.GetFaces()[i].Y);
                indexData.push_back(m.GetFaces()[i].Z);
            }
	    }

        D3D11_SUBRESOURCE_DATA initialData = {};
        initialData.pSysMem = &indexData[0];

        D3D11_BUFFER_DESC bufferDesc = {};
        bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        bufferDesc.ByteWidth = sizeof(unsigned int) * indexData.size();
        bufferDesc.StructureByteStride = 0;
        bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

        DX::ThrowIfFailed(device->CreateBuffer(&bufferDesc,
            &initialData,
            m_indexBuffer.ReleaseAndGetAddressOf()));
    }
}

void Renderer::Deinit()
{
    m_inputLayout.Reset();
    m_vertexBuffer.Reset();
    m_indexBuffer.Reset();
    m_vertexShader.Reset();
    m_pixelShader.Reset();
}
