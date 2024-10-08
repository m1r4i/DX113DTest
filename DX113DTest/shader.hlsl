cbuffer ConstantBuffer : register(b0)
{
    matrix world; // ワールド変換行列
    matrix inWorld; // 反転したワールド変換行列
    matrix view; // ビュー変換行列
    matrix projection; // プロジェクション行列
    float4 lightCol; // ライトの色
    float3 lightLoc; // ライトの位置
    float lightAmb; // ライトの強さ
    float3 eyePos;
    float padding;
};

Texture2D grayTexture : register(t0);
SamplerState samLinear : register(s0);


struct VS_INPUT
{
    float4 Pos : POSITION;
    float4 Normal : NORMAL;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float4 Normal : NORMAL;
};

PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output;

    
    // ワールド空間への変換
    float4 worldPos = mul(input.Pos, world);

    // ビュー空間への変換
    output.Pos = mul(worldPos, view);
    output.Pos = mul(output.Pos, projection);

    
    /*
    // 法線の変換と正規化
    float3 worldNormal = mul(input.Normal, (float3x3) world); // ワールド行列で法線を変換
    output.Normal = normalize(mul(worldNormal, (float3x3) inWorld)); // 反転ワールド行列で法線を変換し、正規化
    */
    
    float4 N = input.Normal;
    N.w = 0.0f; // 法線はベクトルなのでＷの値を０にする。

    N = mul(N, world);
    N = normalize(N);

    output.Normal = N;
    
    return output;
}


float4 PS(PS_INPUT input) : SV_Target
{
    float4 col;
    float4 N = normalize(input.Normal);
    
    // テクスチャからのサンプリング
    float4 texColor = grayTexture.Sample(samLinear, input.Pos.xy);

    // 灰色の色
    float3 grayColor = float3(0.5f, 0.5f, 0.5f);

    // ライト方向ベクトルの計算
    float4 lightDir = float4(lightLoc - input.Pos.xyz, 1);
    float distance = length(lightDir);
    lightDir.w = 0.0f;
    lightDir = normalize(lightDir);
    
    float attenuation = 1.0 / (distance * distance);

    // 環境光と拡散光の計算
    float ambientStrength = lightAmb;
    float diffuseStrength = max(dot(N, lightDir), 0.0f) * attenuation;
    // * attenuation
    // 色の計算
    float3 color = (ambientStrength + diffuseStrength) * lightCol.xyz * grayColor;
    // float3 color = diffuseStrength;

    return float4(color, 1.0f);
    
}




technique11 Render
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }
}
