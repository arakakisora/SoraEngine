#include "LuminanceBasedOutline.hlsli"

struct Material
{
    float4 color;
    int enableLighting;
    float4x4 uvTransform;
    float shininess;
    float4x4 projextionInverse;
};
    
ConstantBuffer<Material> gMaterial : register(b0); //マテリアルの情報

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

Texture2D<float> gDepthTexture : register(t1);
SamplerState gSamplerPoint : register(s1);

static const float2 kIndex3x3[3][3] =
{
    { { -1.0f, -1.0f }, { 0.0f, -1.0f }, { 1.0f, -1.0f } },
    { { -1.0f, 0.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f } },
    { { -1.0f, 1.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f } },
        
};

//static const float kKernel3x3[3][3] =
//{
//    { 1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f },
//    { 1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f },
//    { 1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f },
//};

static const float kPrewittHorizontalKernel[3][3] =
{
    { -1.0f / 6.0f, 0.0f, 1.0f / 6.0f },
    { -1.0f / 6.0f, 0.0f, 1.0f / 6.0f },
    { -1.0f / 6.0f, 0.0f, 1.0f / 6.0f },
};

static const float kPrewittVerticalKernel[3][3] =
{
    { -1.0f / 6.0f, -1.0f / 6.0f, -1.0f / 6.0f },
    { 0.0f, 0.0f, 0.0f },
    { 1.0f / 6.0f, 1.0f / 6.0f, 1.0f / 6.0f },
};

float Luminance(float3 v)
{
    return dot(v, float3(0.2125f,0.7154f,0.0721f));
}



struct PixelShaderOutput
{
    float4 color : SV_TARGET;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    uint width, height; // 1. uvStepSizeの算出
    gTexture.GetDimensions(width, height);
    float2 uvStepSize = float2(rcp((float) width), rcp((float) height));
    float2 difference = float2(0.0f, 0.0f);//縦横それぞれの畳み込みの結果を格納する
    
    
   

    for (int x = 0; x < 3; ++x)
    { 
        for (int y = 0; y < 3; ++y)
        {
        // 3. 現在のtexcoordを算出
            float2 texcoord = input.texcoord + kIndex3x3[x][y] * uvStepSize;
            
            float ndcDepth = gDepthTexture.Sample(gSamplerPoint, texcoord); // NDC座標系での深度値を取得
                float4 viewSpace = mul(float4(0.0f, 0.0f, ndcDepth, 1.0f), gMaterial.projextionInverse);
                float viewz = viewSpace.z * rcp(viewSpace.w);
                difference.x += viewz * kPrewittHorizontalKernel[x][y];
                difference.y += viewz * kPrewittVerticalKernel[x][y];
                

        }
    }
    float weight = length(difference);
    weight = saturate(weight*6.0);
    PixelShaderOutput output;
    //weitghtが大きいほど暗く表示するようにする
    
    output.color.rgb = (1.0f - weight) * gTexture.Sample(gSampler, input.texcoord).rgb; // 元の色から重みを掛けて暗くする
    output.color.a = 1.0f; // アルファ値は1.0に設定
    
    
    return output;
}