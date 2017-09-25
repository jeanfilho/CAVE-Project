
#pragma once

#include <string>

static std::string _vertex_shader =
"#extension GL_ARB_gpu_shader5 : enable\n"
"varying vec3 ViewDirection;\n"
"varying vec3 ObjectPosition;\n"
"varying vec3 Normal;\n"
"varying vec2 TexCoord;\n"
"uniform mat4 ViewMatrix;\n"
"void main(void)\n"
"{\n"
"	TexCoord       = vec2(gl_MultiTexCoord0);\n"
"   ObjectPosition = vec3(gl_ModelViewMatrix * gl_Vertex);\n"
"   ViewDirection  = - ObjectPosition.xyz;\n"
"   Normal         = gl_NormalMatrix * gl_Normal;\n"
"	vec4 displacedPosition = inverse(ViewMatrix) * gl_ModelViewMatrix * gl_Vertex;\n"
"   displacedPosition += vec4(0, -0.00003f * (displacedPosition.z*displacedPosition.z), 0, 0);\n"
"   gl_Position    = gl_ProjectionMatrix * ViewMatrix *displacedPosition;\n"
"   gl_FrontColor  = gl_FrontMaterial.diffuse;\n"
"}\n"
"\n";

static std::string _water_vertex_shader =
"#extension GL_ARB_gpu_shader5 : enable\n"
"varying vec3 ViewDirection;\n"
"varying vec3 ObjectPosition;\n"
"varying vec3 Normal;\n"
"varying vec2 TexCoord;\n"
"uniform float elapsedTime;\n"
"uniform mat4 ViewMatrix;\n"
"void main(void)\n"
"{\n"
"	TexCoord = vec2(gl_MultiTexCoord0);\n"
"   ObjectPosition = vec3(gl_ModelViewMatrix * gl_Vertex);\n"
"   ViewDirection  = - ObjectPosition.xyz;\n"
"   Normal         = gl_NormalMatrix * gl_Normal;\n"
"	vec4 displacedPosition = inverse(ViewMatrix) * gl_ModelViewMatrix * gl_Vertex;\n"
"   displacedPosition += vec4(0, -0.00003f * (displacedPosition.z*displacedPosition.z) + 10 * sin(displacedPosition.z/10), 0, 0);\n"
"   gl_Position    = gl_ProjectionMatrix * ViewMatrix *displacedPosition;\n"
"   gl_FrontColor  = gl_FrontMaterial.diffuse;\n"
"}\n"
"\n";

static std::string _fragment_shader =
"float SpecularPower = 1.0f;\n"
"\n"
"uniform sampler2D baseMap;\n"
"uniform int useTexture;\n"
"\n"
"varying vec2 TexCoord;\n"
"varying vec3 ViewDirection;\n"
"varying vec3 ObjectPosition;\n"
"varying vec3 Normal;\n"
"\n"
"void main( void )\n"
"{\n"
"   vec3  fvLightDirection = normalize( gl_LightSource[0].position.xyz);\n"
"   vec3  fvNormal         = normalize( Normal );\n"
"   float fNDotL           = dot( fvNormal, fvLightDirection ); \n"
"   \n"
"   vec3  fvReflection     = normalize( ( ( 2.0 * fvNormal ) * fNDotL ) - fvLightDirection ); \n"
"   vec3  fvViewDirection  = normalize( ViewDirection );\n"
"   float fRDotV           = max( 0.0, dot( fvReflection, fvViewDirection ) );\n"
"   \n"
"   vec4  fvBaseColor      = texture2D(baseMap, TexCoord);\n"
"   \n"
"   vec4  fvTotalAmbient   = gl_LightSource[0].ambient * fvBaseColor; \n"
"   vec4  fvTotalDiffuse   = gl_LightSource[0].diffuse * fNDotL * fvBaseColor; \n"
"   vec4  fvTotalSpecular  = gl_LightSource[0].specular * ( pow( fRDotV, SpecularPower ) );\n"
"   \n"
"   gl_FragColor = ( fvTotalAmbient + fvTotalDiffuse + fvTotalSpecular );\n"
"}\n";