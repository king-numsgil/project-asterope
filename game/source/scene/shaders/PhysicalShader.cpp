#include <Corrade/Containers/Reference.h>
#include <Corrade/Utility/FormatStl.h>
#include <Corrade/Utility/Resource.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/GL/Version.h>
#include <Magnum/GL/Shader.h>

#include "PhysicalShader.hpp"

using namespace Magnum;

PhysicalShader::PhysicalShader(u32 lightCount)
		: _lightCount{lightCount}, _lightColorsLocation{_lightPositionsLocation + (i32) lightCount}
{
	Utility::Resource rs("AsteropeShaders");

	GL::Shader vert{GL::Version::GL450, GL::Shader::Type::Vertex}, frag{GL::Version::GL450, GL::Shader::Type::Fragment};

	vert.addSource(rs.get("generic.glsl"))
			.addSource(rs.get("pbr.vert.glsl"));
	frag.addSource(Utility::formatString("#define LIGHT_COUNT {}\n", _lightCount))
			.addSource(Utility::formatString("#define LIGHT_COLORS_LOCATION {}\n", _lightColorsLocation))
			.addSource(rs.get("pbr.frag.glsl"));

	CORRADE_INTERNAL_ASSERT_OUTPUT(GL::Shader::compile({vert, frag}));
	attachShaders({vert, frag});
	CORRADE_INTERNAL_ASSERT_OUTPUT(link());

	setEmissivePower(0.f);
}

PhysicalShader& PhysicalShader::setViewProjectionMatrix(const f32mat4& viewProj)
{
	setUniform(_viewProjMatrixLocation, viewProj);
	return *this;
}

PhysicalShader& PhysicalShader::setModelMatrix(const f32mat4& model)
{
	setUniform(_modelMatrixLocation, model);
	return *this;
}

PhysicalShader& PhysicalShader::setCameraPosition(const f32vec3& position)
{
	setUniform(_camPositionLocation, position);
	return *this;
}

PhysicalShader& PhysicalShader::setEmissivePower(f32 power)
{
	setUniform(_emissivePowerLocation, power);
	return *this;
}

PhysicalShader& PhysicalShader::setLightParameters(u32 index, const f32vec3& position, f32col3 const& color)
{
	CORRADE_ASSERT(index < _lightCount, "PhysicalShader::setLightParameters(): light ID"
			<< index
			<< "is out of bounds for"
			<< _lightCount
			<< "lights", *this);

	setUniform(_lightPositionsLocation + (i32) index, position);
	setUniform(_lightColorsLocation + (i32) index, color);
	return *this;
}

PhysicalShader& PhysicalShader::bindTextures(GL::Texture2D* albedo,
                                             GL::Texture2D* normal,
                                             GL::Texture2D* metallic,
                                             GL::Texture2D* roughness,
                                             GL::Texture2D* ambientOcclusion,
                                             GL::Texture2D* emissive)
{
	GL::AbstractTexture::bind(0, {albedo, normal, metallic, roughness, ambientOcclusion, emissive});
	return *this;
}
