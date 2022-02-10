struct Vertex
{
    float3 position     : SV_Position;
    float3 normal       : NORMAL;
    float2 textCoord    : TEXTCOORD;
};

struct PSInput
{
    float4 position     : SV_Position;
    float3 normal       : NORMAL;
    float2 textCoord    : TEXTCOORD;
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
    Out.normal = In.normal;
    Out.textCoord = In.textCoord;
    Out.position = mul(float4(In.position, 1.0f), mWorld);
    Out.position = mul(Out.position, mView);
    Out.position = mul(Out.position, mProjection);
    Out.normal = mul(float4(Out.normal, 0.0f), mWorld).xyz;
    Out.positionW = mul(float4(In.position, 1.0f), mWorld).xyz;
    return Out;
}