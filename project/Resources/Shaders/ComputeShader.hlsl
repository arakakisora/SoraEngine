
struct Well
{
    float4x4 skeletonSpaceMatrix;
    float4x4 skeletonSpaceInberseTransporseMatrix;
    
};
struct Vertex
{
    float4 position;
    float2 texcoord;
    float3 normal;
    
};
struct VertexInfluence
{
    float4 weight;
    int4 index;
};
struct SkinningInformation
{
    uint numVertices;
};
StructuredBuffer<Well> gMatrixPalette : register(t0); //Palette
StructuredBuffer<Vertex> gInputVertices : register(t1);//頂点情報
StructuredBuffer<VertexInfluence> gInfluences : register(t2);//インフルエンス  
RWStructuredBuffer<Vertex> gOutputVertices : register(u0); //出力頂点情報
ConstantBuffer<SkinningInformation> gSkinningInformation : register(b0); //スキニング情報

[numthreads(1024,1,1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint vertexIndex = DTid.x;
    if (vertexIndex < gSkinningInformation.numVertices)
    {
        //必要なデータをStaructuredbufferから取ってくる
        //SkinningObject3Dでは入力頂点として受け取っていた
        Vertex input = gInputVertices[vertexIndex];
        VertexInfluence influence = gInfluences[vertexIndex];
        
        //skinning後の頂点をを計算
        Vertex skinned;
        skinned.texcoord = input.texcoord;
             
    //ｐ＝Σ,i=1,n=4 (Wi)(U)(Ti)
        skinned.position = mul(input.position, gMatrixPalette[influence.index.x].skeletonSpaceMatrix) *  influence.weight.x;
        skinned.position += mul(input.position, gMatrixPalette[influence.index.y].skeletonSpaceMatrix) * influence.weight.y;
        skinned.position += mul(input.position, gMatrixPalette[influence.index.z].skeletonSpaceMatrix) * influence.weight.z;
        skinned.position += mul(input.position, gMatrixPalette[influence.index.w].skeletonSpaceMatrix) * influence.weight.w;
        skinned.position.w = 1.0f;
    
    //ノーマル法線
        skinned.normal = mul(input.normal, (float3x3) gMatrixPalette[ influence.index.x].skeletonSpaceInberseTransporseMatrix) * influence.weight.x;
        skinned.normal += mul(input.normal, (float3x3) gMatrixPalette[influence.index.y].skeletonSpaceInberseTransporseMatrix) * influence.weight.y;
        skinned.normal += mul(input.normal, (float3x3) gMatrixPalette[influence.index.z].skeletonSpaceInberseTransporseMatrix) * influence.weight.z;
        skinned.normal += mul(input.normal, (float3x3) gMatrixPalette[influence.index.w].skeletonSpaceInberseTransporseMatrix) * influence.weight.w;
        skinned.normal = normalize(skinned.normal);
        
        //Skinningg後の頂点データを格納
        gOutputVertices[vertexIndex] = skinned;
    }
    
}