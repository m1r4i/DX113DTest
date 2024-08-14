cbuffer ConstantBuffer : register(b0)
{
    matrix world; // ���[���h�ϊ��s��
    matrix inWorld; // ���]�������[���h�ϊ��s��
    matrix view; // �r���[�ϊ��s��
    matrix projection; // �v���W�F�N�V�����s��
    float4 lightCol; // ���C�g�̐F
    float3 lightLoc; // ���C�g�̈ʒu
    float lightAmb; // ���C�g�̋���
    float3 eyePos; // ���_�ʒu
    float padding; // �p�f�B���O
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
    
    // ���[���h��Ԃւ̕ϊ�
    float4 worldPos = mul(input.Pos, world);

    // �r���[��Ԃւ̕ϊ�
    output.Pos = mul(worldPos, view);
    output.Pos = mul(output.Pos, projection);

    float4 N = input.Normal;
    N.w = 0.0f; // �@���̓x�N�g���Ȃ̂�W�̒l��0�ɂ���

    N = mul(N, world);
    N = normalize(N);

    output.Normal = N;
    
    return output;
}

float4 PS(PS_INPUT input) : SV_Target
{
    float4 col;
    float4 N = normalize(input.Normal);
    
    // �e�N�X�`������̃T���v�����O
    float4 texColor = grayTexture.Sample(samLinear, input.Pos.xy);

    // ���C�g�����x�N�g���̌v�Z
    float4 lightDir = float4(lightLoc - input.Pos.xyz, 1.0f);
    float distance = length(lightDir);
    lightDir.w = 0.0f;
    lightDir = normalize(lightDir);
    
    // �����x�N�g�����v�Z����
    float4 V = float4(eyePos - input.Pos.xyz, 0.0f);
    V = normalize(V);
    
    // �n�[�t�x�N�g�����v�Z 
    float4 H = normalize(lightDir + V);
    
    // �����o�[�g�̗]����
    float d = max(0.0f, dot(lightDir, N));
    float4 diffuse = lightCol * d;
    
    // �u�����t�H���v�Z
    float s = pow(max(0.0f, dot(N, H)), 50);
    float4 specular = lightCol * s;

    col = diffuse;

    // �����Ō���������
    float att = 1.0f / (0.001f * distance * distance);

    // col = col * att;

    // �����I�ȐF�ɃX�y�L�������������Z
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
