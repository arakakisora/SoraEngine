//#include "Fullscreen.hlsli"


//Texture2D gprev : register(t0);//前シーン
//Texture2D gNext : register(t1); //次シーン

//SamplerState gSamp : register(s0);

//cbuffer TrailParams: register(b0)
//{
//    float time; // 0.0 - 1.0
//    float direction; // 0.0 - left, 1.0 - right
//    float width;//帯の大きさ
//    float softness; //帯のぼかし具合
//    float headRoundness; //帯の先端の丸み
//    float glow; //帯の光り具合、強さ
//    float trailLength; //帯の長さ
//    float2 vignette; //ビネット効果の強さ
//    float2 pad; //パディング
    
//};

//float smoothMask(float x, float edge, float s)
//{
//    return smoothstep(edge - s, edge + s, x);

//}

//float mainMask(float2 uv)
//{
//    //進行エッジ位置（右ー＞左の時は１ーｔ）
//    float edege = (direction > 0) ? time : (1.0f - time);
    
//    // 先端を丸く：距離場を“横=進行、縦=丸み”で合成
//    float head = 1.0 - saturate(abs(uv.y - 0.5) / max(1e-3, width)) * headRoundness;
//    float x = uv.x + (headRoundness > 0 ? (headRoundness * (uv.y - 0.5)) : 0);
    
//    //本体マスク：edgeまでがnext
//    float m = smoothMask(x, edege, softness);
    
//     // 残光（edge の手前側に減衰つきで足す）
//    float d = (direction > 0) ? (edege - x) : (x - edege);
//    float tail = saturate(1.0 - d / max(1e-3, trailLength));
//    //上下で少し減衰
//    float vAttn = 1.0 - abs(uv.y - 0.5) * vignette.x - (uv.y - 0.5) * (uv.y - 0.5) * vignette.y;
//    m = saturate(m + glow * tail * vAttn * 0.6);
//    return m;

//}



//struct PixelShaderOutput
//{
//    float4 color : SV_TARGET;
//};

//PixelShaderOutput main(VertexShaderOutput input)
//{
//    float prevColor = gprev.Sample(gSamp, input.texcoord).rgb;
//    float nextColor = gNext.Sample(gSamp, input.texcoord).rgb;
    
//    float m = mainMask(input.texcoord); //0=prev, 1=next
//    float shift = (direction > 0 ? 1 : -1) * 0.0015 * (1.0 - m);
//    float3 nextCA;
//    nextCA.r = gNext.Sample(gSamp, input.texcoord + float2(shift, 0)).r;
//    nextCA.g = gNext.Sample(gSamp, input.texcoord).g;
//    nextCA.b = gNext.Sample(gSamp, input.texcoord - float2(shift, 0)).b;
    
//    float3 color = lerp(prevColor, nextCA, m);
    
//    return float4(color, 1.0f);
    
//}