//**************************************************************//
//  Effect File exported by RenderMonkey 1.6
//
//  - Although many improvements were made to RenderMonkey FX  
//    file export, there are still situations that may cause   
//    compilation problems once the file is exported, such as  
//    occasional naming conflicts for methods, since FX format 
//    does not support any notions of name spaces. You need to 
//    try to create workspaces in such a way as to minimize    
//    potential naming conflicts on export.                    
//    
//  - Note that to minimize resulting name collisions in the FX 
//    file, RenderMonkey will mangle names for passes, shaders  
//    and function names as necessary to reduce name conflicts. 
//**************************************************************//

//--------------------------------------------------------------//
// ShadowMapping
//--------------------------------------------------------------//
//--------------------------------------------------------------//
// CreateShadow
//--------------------------------------------------------------//
string ShadowMapping_CreateShadow_Torus : ModelData = "..\\..\\..\\..\\..\\..\\Program Files (x86)\\AMD\\RenderMonkey 1.82\\Examples\\Media\\Models\\Torus.3ds";

texture ShadowMap_Tex : RenderColorTarget
<
   float2 RenderTargetDimensions = {2048,2048};
   string Format="D3DFMT_R32F";
   float  ClearDepth=1.000000;
   int    ClearColor=-1;
>;
float4x4 gWorldMatrix : World;
float4x4 gLightViewMatrix;
float4x4 gLightProjectionMatrix : Projection;
float4 gWorldLightPosition
<
   string UIName = "gWorldLightPosition";
   string UIWidget = "Direction";
   bool UIVisible =  false;
   float4 UIMin = float4( -10.00, -10.00, -10.00, -10.00 );
   float4 UIMax = float4( 10.00, 10.00, 10.00, 10.00 );
   bool Normalize =  false;
> = float4( 500.00, 500.00, -500.00, 1.00 );

struct VS_INPUT
{
   float4 mPosition : POSITION;
};

struct VS_OUTPUT
{
   float4 mPosition : POSITION;
   float4 mClipPosition : TEXCOORD1;
};

VS_OUTPUT ShadowMapping_CreateShadow_Vertex_Shader_vs_main( VS_INPUT Input )
{
   VS_OUTPUT Output;
   
   float4x4 lightViewMatrix = gLightViewMatrix;
   
   float3 dirZ = -normalize(gWorldLightPosition.xyz);
   float3 up = float3(0, 1, 0);
   float3 dirX = cross(up, dirZ);
   float3 dirY = cross(dirZ, dirX);
   
   lightViewMatrix = float4x4(float4(dirX, -dot(gWorldLightPosition.xyz, dirX)), float4(dirY, -dot(gWorldLightPosition.xyz, dirY)), float4(dirZ, -dot(gWorldLightPosition.xyz, dirZ)), float4(0, 0, 0, 1));
   lightViewMatrix = transpose(lightViewMatrix);
   
   Output.mPosition = mul(Input.mPosition, gWorldMatrix);
   Output.mPosition = mul(Output.mPosition, lightViewMatrix);
   Output.mPosition = mul(Output.mPosition, gLightProjectionMatrix);
   Output.mClipPosition = Output.mPosition;
   
   return Output;
}
struct PS_INPUT
{
   float4 mClipPosition : TEXCOORD1;
};

float4 ShadowMapping_CreateShadow_Pixel_Shader_ps_main(PS_INPUT Input) : COLOR
{
   float depth = Input.mClipPosition.z / Input.mClipPosition.w;
   return float4(depth.xxx, 1);
}
//--------------------------------------------------------------//
// ApplyShadowTorus
//--------------------------------------------------------------//
string ShadowMapping_ApplyShadowTorus_Torus : ModelData = "..\\..\\..\\..\\..\\..\\Program Files (x86)\\AMD\\RenderMonkey 1.82\\Examples\\Media\\Models\\Torus.3ds";

struct ShadowMapping_ApplyShadowTorus_Vertex_Shader_VS_INPUT
{
   float4 mPosition : POSITION;
   float3 mNormal : NORMAL;
};

struct ShadowMapping_ApplyShadowTorus_Vertex_Shader_VS_OUTPUT
{
   float4 mPosition : POSITION;
   float4 mClipPosition : TEXCOORD1;
   float mDiffuse : TEXCOORD2;
};

float4x4 ShadowMapping_ApplyShadowTorus_Vertex_Shader_gWorldMatrix : World;
float4x4 ShadowMapping_ApplyShadowTorus_Vertex_Shader_gLightViewMatrix;
float4x4 ShadowMapping_ApplyShadowTorus_Vertex_Shader_gLightProjectionMatrix : Projection;

float4   ShadowMapping_ApplyShadowTorus_Vertex_Shader_gWorldLightPosition
<
   string UIName = "ShadowMapping_ApplyShadowTorus_Vertex_Shader_gWorldLightPosition";
   string UIWidget = "Direction";
   bool UIVisible =  false;
   float4 UIMin = float4( -10.00, -10.00, -10.00, -10.00 );
   float4 UIMax = float4( 10.00, 10.00, 10.00, 10.00 );
   bool Normalize =  false;
> = float4( 500.00, 500.00, -500.00, 1.00 );

float4x4 gViewProjectionMatrix : ViewProjection;

ShadowMapping_ApplyShadowTorus_Vertex_Shader_VS_OUTPUT ShadowMapping_ApplyShadowTorus_Vertex_Shader_vs_main(ShadowMapping_ApplyShadowTorus_Vertex_Shader_VS_INPUT Input)
{
   ShadowMapping_ApplyShadowTorus_Vertex_Shader_VS_OUTPUT Output;
   
   float4x4 lightViewMatrix = ShadowMapping_ApplyShadowTorus_Vertex_Shader_gLightViewMatrix;
   
   float3 dirZ = -normalize(ShadowMapping_ApplyShadowTorus_Vertex_Shader_gWorldLightPosition.xyz);
   float3 up = float3(0, 1, 0);
   float3 dirX = cross(up, dirZ);
   float3 dirY = cross(dirZ, dirX);
   
   lightViewMatrix = float4x4(float4(dirX, -dot(ShadowMapping_ApplyShadowTorus_Vertex_Shader_gWorldLightPosition.xyz, dirX)), float4(dirY, -dot(ShadowMapping_ApplyShadowTorus_Vertex_Shader_gWorldLightPosition.xyz, dirY)), float4(dirZ, -dot(ShadowMapping_ApplyShadowTorus_Vertex_Shader_gWorldLightPosition.xyz, dirZ)), float4(0, 0, 0, 1));
   lightViewMatrix = transpose(lightViewMatrix);
   
   float4 worldPosition = mul(Input.mPosition, ShadowMapping_ApplyShadowTorus_Vertex_Shader_gWorldMatrix); 
   Output.mPosition = mul(worldPosition, gViewProjectionMatrix);
   
   Output.mClipPosition = mul(worldPosition, lightViewMatrix);
   Output.mClipPosition = mul(Output.mClipPosition, ShadowMapping_ApplyShadowTorus_Vertex_Shader_gLightProjectionMatrix);
   
   float3 lightDir = normalize(worldPosition.xyz - ShadowMapping_ApplyShadowTorus_Vertex_Shader_gWorldLightPosition.xyz);
   float3 worldNormal = normalize(mul(Input.mNormal, (float3x3)ShadowMapping_ApplyShadowTorus_Vertex_Shader_gWorldMatrix));
   
   Output.mDiffuse = dot(-lightDir, worldNormal);
   return Output;
 } 
sampler2D ShadowSampler = sampler_state
{
   Texture = (ShadowMap_Tex);
};

float4 gObjectColor
<
   string UIName = "gObjectColor";
   string UIWidget = "Color";
   bool UIVisible =  true;
> = float4( 1.00, 0.94, 0.00, 1.00 );

struct ShadowMapping_ApplyShadowTorus_Pixel_Shader_PS_INPUT
{
   float4 mClipPosition : TEXCOORD1;
   float mDiffuse : TEXCOORD2;
};

float4 ShadowMapping_ApplyShadowTorus_Pixel_Shader_ps_main(ShadowMapping_ApplyShadowTorus_Pixel_Shader_PS_INPUT Input) : COLOR
{
   float3 rgb = saturate(Input.mDiffuse) * gObjectColor;
   
   float currentDepth = Input.mClipPosition.z / Input.mClipPosition.w;
   
   float2 uv = Input.mClipPosition.xy / Input.mClipPosition.w;
   uv.y = -uv.y;
   uv = uv * 0.5 + 0.5;
   
   float shadowDepth = tex2D(ShadowSampler, uv).r;
   
   if (currentDepth > shadowDepth + 0.0000125f)
   {
      rgb *= 0.5f;
   }
   
   return float4(rgb, 1.0f);  
}
   
//--------------------------------------------------------------//
// ApplyShadowDisc
//--------------------------------------------------------------//
string ShadowMapping_ApplyShadowDisc_Disc : ModelData = "..\\..\\..\\..\\..\\..\\Program Files (x86)\\AMD\\RenderMonkey 1.82\\Examples\\Media\\Models\\Disc.3ds";

struct ShadowMapping_ApplyShadowDisc_Vertex_Shader_VS_INPUT
{
   float4 mPosition : POSITION;
   float3 mNormal : NORMAL;
};

struct ShadowMapping_ApplyShadowDisc_Vertex_Shader_VS_OUTPUT
{
   float4 mPosition : POSITION;
   float4 mClipPosition : TEXCOORD1;
   float mDiffuse : TEXCOORD2;
};

float4x4 ShadowMapping_ApplyShadowDisc_Vertex_Shader_gWorldMatrix
<
   string UIName = "ShadowMapping_ApplyShadowDisc_Vertex_Shader_gWorldMatrix";
   string UIWidget = "Numeric";
   bool UIVisible =  false;
> = float4x4( 2.00, 0.00, 0.00, 0.00, 0.00, 2.00, 0.00, -40.00, 0.00, 0.00, 2.00, 0.00, 0.00, 0.00, 0.00, 2.00 );
float4x4 ShadowMapping_ApplyShadowDisc_Vertex_Shader_gLightViewMatrix;
float4x4 ShadowMapping_ApplyShadowDisc_Vertex_Shader_gLightProjectionMatrix : Projection;

float4   ShadowMapping_ApplyShadowDisc_Vertex_Shader_gWorldLightPosition
<
   string UIName = "ShadowMapping_ApplyShadowDisc_Vertex_Shader_gWorldLightPosition";
   string UIWidget = "Direction";
   bool UIVisible =  false;
   float4 UIMin = float4( -10.00, -10.00, -10.00, -10.00 );
   float4 UIMax = float4( 10.00, 10.00, 10.00, 10.00 );
   bool Normalize =  false;
> = float4( 500.00, 500.00, -500.00, 1.00 );

float4x4 ShadowMapping_ApplyShadowDisc_Vertex_Shader_gViewProjectionMatrix : ViewProjection;

ShadowMapping_ApplyShadowDisc_Vertex_Shader_VS_OUTPUT ShadowMapping_ApplyShadowDisc_Vertex_Shader_vs_main(ShadowMapping_ApplyShadowDisc_Vertex_Shader_VS_INPUT Input)
{
   ShadowMapping_ApplyShadowDisc_Vertex_Shader_VS_OUTPUT Output;
   
   float4x4 lightViewMatrix = ShadowMapping_ApplyShadowDisc_Vertex_Shader_gLightViewMatrix;
   
   float3 dirZ = -normalize(ShadowMapping_ApplyShadowDisc_Vertex_Shader_gWorldLightPosition.xyz);
   float3 up = float3(0, 1, 0);
   float3 dirX = cross(up, dirZ);
   float3 dirY = cross(dirZ, dirX);
   
   lightViewMatrix = float4x4(float4(dirX, -dot(ShadowMapping_ApplyShadowDisc_Vertex_Shader_gWorldLightPosition.xyz, dirX)), float4(dirY, -dot(ShadowMapping_ApplyShadowDisc_Vertex_Shader_gWorldLightPosition.xyz, dirY)), float4(dirZ, -dot(ShadowMapping_ApplyShadowDisc_Vertex_Shader_gWorldLightPosition.xyz, dirZ)), float4(0, 0, 0, 1));
   lightViewMatrix = transpose(lightViewMatrix);
   
   float4 worldPosition = mul(Input.mPosition, ShadowMapping_ApplyShadowDisc_Vertex_Shader_gWorldMatrix); 
   Output.mPosition = mul(worldPosition, ShadowMapping_ApplyShadowDisc_Vertex_Shader_gViewProjectionMatrix);
   
   Output.mClipPosition = mul(worldPosition, lightViewMatrix);
   Output.mClipPosition = mul(Output.mClipPosition, ShadowMapping_ApplyShadowDisc_Vertex_Shader_gLightProjectionMatrix);
   
   float3 lightDir = normalize(worldPosition.xyz - ShadowMapping_ApplyShadowDisc_Vertex_Shader_gWorldLightPosition.xyz);
   float3 worldNormal = normalize(mul(Input.mNormal, (float3x3)ShadowMapping_ApplyShadowDisc_Vertex_Shader_gWorldMatrix));
   
   Output.mDiffuse = dot(-lightDir, worldNormal);
   return Output;
 } 
sampler2D ShadowMapping_ApplyShadowDisc_Pixel_Shader_ShadowSampler = sampler_state
{
   Texture = (ShadowMap_Tex);
};

float4 ShadowMapping_ApplyShadowDisc_Pixel_Shader_gObjectColor
<
   string UIName = "ShadowMapping_ApplyShadowDisc_Pixel_Shader_gObjectColor";
   string UIWidget = "Color";
   bool UIVisible =  true;
> = float4( 0.00, 1.00, 1.00, 1.00 );

struct ShadowMapping_ApplyShadowDisc_Pixel_Shader_PS_INPUT
{
   float4 mClipPosition : TEXCOORD1;
   float mDiffuse : TEXCOORD2;
};

float4 ShadowMapping_ApplyShadowDisc_Pixel_Shader_ps_main(ShadowMapping_ApplyShadowDisc_Pixel_Shader_PS_INPUT Input) : COLOR
{
   float3 rgb = saturate(Input.mDiffuse) * ShadowMapping_ApplyShadowDisc_Pixel_Shader_gObjectColor;
   
   float currentDepth = Input.mClipPosition.z / Input.mClipPosition.w;
   
   float2 uv = Input.mClipPosition.xy / Input.mClipPosition.w;
   uv.y = -uv.y;
   uv = uv * 0.5 + 0.5;
   
   float shadowDepth = tex2D(ShadowMapping_ApplyShadowDisc_Pixel_Shader_ShadowSampler, uv).r;
   
   if (currentDepth > shadowDepth + 0.0000125f)
   {
      rgb *= 0.5f;
   }
   
   return float4(rgb, 1.0f);  
}
   
//--------------------------------------------------------------//
// Technique Section for ShadowMapping
//--------------------------------------------------------------//
technique ShadowMapping
{
   pass CreateShadow
   <
      string Script = "RenderColorTarget0 = ShadowMap_Tex;"
                      "ClearColor = (255, 255, 255, 255);"
                      "ClearDepth = 1.000000;";
   >
   {
      VertexShader = compile vs_2_0 ShadowMapping_CreateShadow_Vertex_Shader_vs_main();
      PixelShader = compile ps_2_0 ShadowMapping_CreateShadow_Pixel_Shader_ps_main();
   }

   pass ApplyShadowTorus
   {
      VertexShader = compile vs_2_0 ShadowMapping_ApplyShadowTorus_Vertex_Shader_vs_main();
      PixelShader = compile ps_2_0 ShadowMapping_ApplyShadowTorus_Pixel_Shader_ps_main();
   }

   pass ApplyShadowDisc
   {
      VertexShader = compile vs_2_0 ShadowMapping_ApplyShadowDisc_Vertex_Shader_vs_main();
      PixelShader = compile ps_2_0 ShadowMapping_ApplyShadowDisc_Pixel_Shader_ps_main();
   }

}

