#include "Line.hlsli"

struct TransformationMatrix
{
    float4x4 WVP;
    float4x4 World;
};

struct LineInstanceData
{
    float4x4 WVP;
    float4x4 World;
    float4 color;
};

// StructuredBuffer でインスタンスごとのデータを渡す
StructuredBuffer<LineInstanceData> gLineInstances : register(t0);

struct VertexShaderInput
{
    float4 position : POSITION0; // 頂点位置（ローカル）
};

VertexShaderOutput main(VertexShaderInput input, uint instanceId : SV_InstanceID)
{
    VertexShaderOutput output;

    // インスタンスごとの行列を使って位置変換
    output.position = mul(input.position, gLineInstances[instanceId].WVP);

    // インスタンスごとの色を出力
    output.color = gLineInstances[instanceId].color;

    return output;
}
