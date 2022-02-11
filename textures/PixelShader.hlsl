struct Material
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
};

struct DirectionalLight
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float3 Direction;
    float Pad;
};

struct PointLight
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;

    float3 Position;
    float Range;

    float3 Att;
    float Pad;
};

struct SpotLight
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;

    float3 Position;
    float Range;

    float3 Direction;
    float Spot;

    float3 Att;
    float Pad;
};

cbuffer LightingData
{
    DirectionalLight dirLight;
    PointLight pointLight;
    SpotLight spotLight;
    float3 eyePos;
    Material material;
};

struct PSInput
{
    float4 position     : SV_Position;
    float3 normal       : NORMAL;
    float2 textCoord    : TEXTCOORD;
    float3 positionW    : POSITION;
};

struct Pixel
{
    float4 color    : SV_Target;
};

Texture2D diffuseMap;
SamplerState samLinear;

void ComputeDirectionalLight(Material mat,
    DirectionalLight dl,
    float3 normal,
    float3 toEye,
    out float4 ambient,
    out float4 diffuse,
    out float4 specular)
{
    ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

    // the light vector aims the opposite direction the light rays travel
    float3 lightVec = -dl.Direction;

    ambient = mat.Ambient * dl.Ambient;
    float diffuseFactor = dot(lightVec, normal);

    if (diffuseFactor > 0.0f)
    {
        float3 v = reflect(lightVec, normal);
        float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);

        diffuse = diffuseFactor * mat.Diffuse * dl.Diffuse;
        specular = specFactor * mat.Specular * dl.Specular;
    }
}

void ComputePointlLight(Material mat,
    PointLight pl,
    float3 normal,
    float3 pos,
    float3 toEye,
    out float4 ambient,
    out float4 diffuse,
    out float4 specular)
{
    ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

    // The vector from the surface to the light
    float3 lightVec = pl.Position.xyz - pos;
    float d = length(lightVec);

    if (d > pl.Range)
        return;

    lightVec = normalize(lightVec);

    ambient = mat.Ambient * pl.Ambient;

    // Add diffuse and specular term, provided the surface is in
    // the line of site of the light
    float diffuseFactor = dot(lightVec, normal);

    if (diffuseFactor > 0.0f)
    {
        float3 v = reflect(-lightVec, normal);
        float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);

        diffuse = diffuseFactor * mat.Diffuse * pl.Diffuse;
        specular = specFactor * mat.Specular * pl.Specular;
    }

    // attenuate
    float att = 1.0f / dot(pl.Att, float3(1.0f, d, d * d));

    diffuse *= att;
    specular *= att;
}

void ComputeSpotLight(Material mat, SpotLight sl,
    float3 pos, float3 normal, float3 toEye,
    out float4 ambient, out float4 diffuse, out float4 spec)
{
    // Initialize outputs.
    ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
    // The vector from the surface to the light.
    float3 lightVec = sl.Position - pos;
    // The distance from surface to light.
    float d = length(lightVec);
    // Range test.
    if (d > sl.Range)
        return;
    // Normalize the light vector.
    lightVec = normalize(lightVec);
    // Ambient term.
    ambient = mat.Ambient * sl.Ambient;
    // Add diffuse and specular term, provided the surface is in
    // the line of site of the light.
    float diffuseFactor = dot(lightVec, normal);
    
    if (diffuseFactor > 0.0f)
    {
        float3 v = reflect(-lightVec, normal);
        float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);
        diffuse = diffuseFactor * mat.Diffuse * sl.Diffuse;
        spec = specFactor * mat.Specular * sl.Specular;
    }
    // Scale by spotlight factor and attenuate.
    float spot = pow(max(dot(-lightVec, sl.Direction), 0.0f), sl.Spot);
    // Scale by spotlight factor and attenuate.
    float att = spot / dot(sl.Att, float3(1.0f, d,
        d * d));
    ambient *= spot;
    diffuse *= att;
    spec *= att;
}

Pixel main(PSInput In)
{
    Pixel Out;
    float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

    float4 A = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 D = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 S = float4(0.0f, 0.0f, 0.0f, 0.0f);

    // Interpolated normal could be unnormalized
    In.normal = normalize(In.normal);
    float3 toEye = normalize(eyePos - In.positionW);

    ComputeDirectionalLight(material, dirLight, In.normal, toEye,
        A, D, S);

    ambient += A;
    diffuse += D;
    specular += S;

    ComputePointlLight(material,
        pointLight,
        In.normal,
        In.positionW,
        toEye,
        A,
        D,
        S);

    ambient += A;
    diffuse += D;
    specular += S;

    ComputeSpotLight(material,
        spotLight,
        In.positionW,
        In.normal,
        toEye,
        A, D, S);

    ambient += A;
    diffuse += D;
    specular += S;

    float4 texColor = diffuseMap.Sample(samLinear, In.textCoord);

    Out.color = texColor * (ambient + diffuse) + specular;
    Out.color.w = material.Diffuse.w;

    return Out;
}