#include "Object3d.hlsli"

struct Material
{
    float4 color;
    int enableLighting;
    float4x4 uvTransform;
    float shininess;
};

struct DirectionalLight
{
    float4 color; // ライトの色
    float3 direction; // ライトの向き
    float intensity;
};

struct pointLight
{
    float4 color; //ライトの色
    float3 position; //ライトの位置
    float intensity; //ライトの強さ
       
    
};

struct Camera
{
    float3 worldPosition;
};

ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);
ConstantBuffer<Camera> gCamera : register(b2);
ConstantBuffer<pointLight> gPointLight : register(b3);

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    
    float4 transformedUV = mul(float4(input.texcoord, 0.0, 1.0f), gMaterial.uvTransform); //テクスチャ座標の変換
    float4 textureColor = gTexture.Sample(gSampler, transformedUV.xy); //テクスチャの色を取得
    
    if (textureColor.a <= 0.5)
    {
        discard;
    }
    if (textureColor.a <= 0.0)
    {
        discard;
    }
    
    PixelShaderOutput output;
    
    if (gMaterial.enableLighting != 0)
    {
        
        //poinntLight
        float3 pointLightDirection = normalize(input.worldPosition - gPointLight.position);
        
        
        float NdotL = dot(normalize(input.normal), normalize(-gDirectionalLight.direction)); //ライトの方向と法線ベクトルの内積
        float cos = pow(NdotL * 0.5f + 0.5f, 2.0f); //ライトの方向と法線ベクトルの内積を0~1に変換
        
        float3 toEye = normalize(gCamera.worldPosition - input.worldPosition); //カメラの位置から頂点の位置へのベクトル
        float3 reflectLight = reflect(normalize(gDirectionalLight.direction), normalize(input.normal)); //反射ベクトル
        
        float3 halfVector = normalize(-gDirectionalLight.direction + toEye); //ハーフベクトル
        float NDotH = dot(normalize(input.normal), halfVector); //ハーフベクトルと法線ベクトルの内積
        float specularPOW = pow(saturate(NDotH), gMaterial.shininess); //ハーフベクトルと法線ベクトルの内積を0~1に変換
        
        
        float3 directionLightdiffuse = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity; //ライトの色とテクスチャの色を乗算
        float3 directionLightspecular = gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPOW * float3(1.0f, 1.0f, 1.0f); //ライトの色とハーフベクトルと法線ベクトルの内積を乗算

        //ポイントライト
        float3 pointLightdiffuse = gMaterial.color.rgb * textureColor.rgb * gPointLight.color.rgb * cos * gPointLight.intensity;
        float3 pointLightspecular = gPointLight.color.rgb * gPointLight.intensity * specularPOW * float3(1.0f, 1.0f, 1.0f);
       
        
        output.color.rgb = directionLightdiffuse + directionLightspecular + pointLightdiffuse + pointLightspecular;
        output.color.a = gMaterial.color.a * textureColor.a;
    }
    else
    {
        output.color = gMaterial.color * textureColor;
    }

    return output;
}
