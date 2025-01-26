#include "Object3d.hlsli"


struct Material
{

    float32_t4 color;
    int32_t enableLighting;
    float32_t4x4 uvTransform;
    float32_t shiniess;

};

struct DirectionalLight
{

    float32_t4 color; //ライトの色
    float32_t3 direction; //ライトの向き
    float intensity;
};

struct Camera
{
    float32_t3 worldPositon;
    
};

ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);
ConstantBuffer<Camera> gCamera : register(b2);

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);



struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
    
    
};

PixelShaderOutput main(VertexShaderOutput input)
{
    float4 transformdUV = mul(float32_t4(input.texcoord,0.0, 1.0f), gMaterial.uvTransform);
    float32_t4 textureColor = gTexture.Sample(gSampler, transformdUV.xy);
    float32_t3 toEye = normalize(gCamera.worldPositon - input.worldPosition);
    float32_t3 reflectLight = reflect(gDirectionalLight, gDirectionalLight, nomalize(input.normal));
    PixelShaderOutput output;
    if (gMaterial.enableLighting != 0)
    {
        float NdotL = dot(normalize(input.normal), -gDirectionalLight.direction);
        float cos = pow(NdotL * 0.5f + 0.5f, 2.0);
        float RdotE = dot(reflectLight, toEye);
        float specularPow = pow(saturate(RdotE), gMaterial.shiniess);
        
        //拡散反社
        float32_t3 diffuse = gMaterial.color * textureColor * gDirectionalLight.color * cos * gDirectionalLight.intensity;
        //鏡面反射
        float32_t3 specular = gDirectionalLight.color * gDirectionalLight.intensity * specularPow * float3x2(1.0, 1.0, 1.0);
        //拡散反射と鏡面反射の合成
        output.color = diffuse + specular;
        //アルファは今まで通り
        output.color.a = gMaterial.color.a * textureColor.a;
       
    }
    else if (gMaterial.shiniess)
    {
        
        
    }
    else
    {
        output.color = gMaterial.color * textureColor;
        
    }
    

    return output;

}