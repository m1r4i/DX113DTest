cbuffer ConstantBuffer : register(b0)
{
    matrix world; // ワールド変換行列
    matrix inWorld; // 反転したワールド変換行列
    matrix view; // ビュー変換行列
    matrix projection; // プロジェクション行列
    float4 lightCol; // ライトの色
    float3 lightLoc; // ライトの位置
    float lightAmb; // ライトの強さ
    float3 eyePos; // 視点位置
    float padding; // パディング
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

    float4 N = input.Normal;
    N.w = 0.0f; // 法線はベクトルなのでWの値を0にする

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

    // ライト方向ベクトルの計算
    float4 lightDir = float4(lightLoc - input.Pos.xyz, 1.0f);
    float distance = length(lightDir);
    lightDir.w = 0.0f;
    lightDir = normalize(lightDir);
    
    // 視線ベクトルを計算する
    float4 V = float4(eyePos - input.Pos.xyz, 0.0f);
    V = normalize(V);
    
    // ハーフベクトルを計算 
    float4 H = normalize(lightDir + V);
    
    // ランバートの余弦則
    float d = max(0.0f, dot(lightDir, N));
    float4 diffuse = lightCol * d;
    
    // ブリンフォン計算
    float s = pow(max(0.0f, dot(N, H)), 50);
    float4 specular = lightCol * s;

    col = diffuse;

    // 距離で減衰させる
    float att = 1.0f / (0.001f * distance * distance);

    // col = col * att;

    // 総合的な色にスペキュラ成分を加算
    float4 totalcolor = col + specular;
    totalcolor.a = 1.0f;

    return totalcolor;
}

technique11 Render
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }
}
