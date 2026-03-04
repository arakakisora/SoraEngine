#include "Fullscreen.hlsli"

Texture2D<float4> gTexture0 : register(t0); // current
Texture2D<float4> gTexture1 : register(t1); // next
SamplerState gSampler : register(s0);

cbuffer TransitionCB : register(b0)
{
    float gProgress; // 0 -> 1
    float3 pad;
};

struct PixelShaderOutput
{
    float4 color : SV_TARGET;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;

    float4 a = gTexture0.Sample(gSampler, input.texcoord);
    float4 b = gTexture1.Sample(gSampler, input.texcoord);

    output.color = lerp(a, b, saturate(gProgress));
    return output;
}