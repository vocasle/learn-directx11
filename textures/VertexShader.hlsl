struct Vertex
{
    float4 position     : SV_Position;
    float4 color        : COLOR0;
    float3 normal       : NORMAL;
};

struct PSInput
{
    float4 position     : SV_Position;
    float4 color        : COLOR0;
    float3 normal       : NORMAL;
    float3 positionW    : POSITION;
};

cbuffer Constants : register(b0)
{
    float4x4 mWorld;
    float4x4 mView;
    float4x4 mProjection;
};

PSInput main(Vertex In)
{
    PSInput Out;
    Out.position = In.position;
    Out.normal = In.normal;
    Out.color = In.color;
    Out.position = mul(Out.position, mWorld);
    Out.position = mul(Out.position, mView);
    Out.position = mul(Out.position, mProjection);
    Out.normal = mul(Out.normal, mWorld);
    Out.positionW = mul(In.position, mWorld).xyz;
    return Out;
}