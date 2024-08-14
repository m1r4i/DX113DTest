cbuffer ConstantBuffer : register(b0)
{
    matrix world; // ���[���h�ϊ��s��
    matrix inWorld; // ���]�������[���h�ϊ��s��
    matrix view; // �r���[�ϊ��s��
    matrix projection; // �v���W�F�N�V�����s��
    float4 lightCol; // ���C�g�̐F
    float3 lightLoc; // ���C�g�̈ʒu
    float lightAmb; // ���C�g�̋���
};

Texture2D grayTexture : register(t0);
SamplerState samLinear : register(s0);


struct VS_INPUT
{
    float4 Pos : POSITION;
    float3 Normal : NORMAL;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 Normal : NORMAL;
};

PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output;

    // ���[���h��Ԃւ̕ϊ�
    float4 worldPos = mul(input.Pos, world);

    // �r���[��Ԃւ̕ϊ�
    output.Pos = mul(worldPos, view);
    output.Pos = mul(output.Pos, projection);

    // �@���̕ϊ��Ɛ��K��
    float3 worldNormal = mul(input.Normal, (float3x3) world); // ���[���h�s��Ŗ@����ϊ�
    output.Normal = normalize(mul(worldNormal, (float3x3) inWorld)); // ���]���[���h�s��Ŗ@����ϊ����A���K��

    return output;
}


float4 PS(PS_INPUT input) : SV_Target
{
    // �e�N�X�`������̃T���v�����O
    float4 texColor = grayTexture.Sample(samLinear, input.Pos.xy);

    // �D�F�̐F
    float3 grayColor = float3(0.5f, 0.5f, 0.5f);

    // ���C�g�����x�N�g���̌v�Z
    float3 lightDir = normalize(lightLoc - input.Pos.xyz);
    float distance = length(lightLoc - input.Pos.xyz);
    float attenuation = 1.0 / (distance * distance);

    // �����Ɗg�U���̌v�Z
    float ambientStrength = lightAmb;
    float diffuseStrength = max(dot(normalize(input.Normal), lightDir), 0.0f);
    // * attenuation
    // �F�̌v�Z
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
