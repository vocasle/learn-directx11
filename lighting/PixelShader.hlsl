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

struct Interpolants
{
    float4 position : SV_Position;
    float4 color    : COLOR0;
    float3 normal   : NORMAL;
};

struct Pixel
{
    float4 color    : SV_Target;
};

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

Pixel main(Interpolants In)
{
    Pixel Out;
    //Out.color = In.color;
    float4 ambient;
    float4 diffuse;
    float4 specular;

    // Interpolated normal could be unnormalized
    In.normal = normalize(In.normal);
    float3 toEye = normalize(eyePos - float3(In.position.x, In.position.y, In.position.z));

    ComputeDirectionalLight(material, dirLight, In.normal, toEye,
        ambient, diffuse, specular);

    Out.color = ambient + diffuse + specular;
    //Out.color = float4(1.0f, 0.0f, 0.0f, 1.0f);

    return Out;
}