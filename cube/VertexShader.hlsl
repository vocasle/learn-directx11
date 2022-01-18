struct Vertex
{
    float4 position     : SV_Position;
    float4 color        : COLOR0;
};

struct Interpolants
{
    float4 position     : SV_Position;
    float4 color        : COLOR0;
};

cbuffer Constants : register(b0)
{
    float4x4 mWorld;
    float4x4 mView;
    float4x4 mProjection;
};

Interpolants main(Vertex In)
{
    Interpolants Out = In;
    Out.position = mul(Out.position, mWorld);
    Out.position = mul(Out.position, mView);
    Out.position = mul(Out.position, mProjection);
    return Out;
}