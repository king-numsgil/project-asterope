#include <Corrade/Containers/Reference.h>
#include <Corrade/Utility/FormatStl.h>
#include <Corrade/Utility/Resource.h>
#include <Magnum/GL/Version.h>
#include <Magnum/GL/Shader.h>

#include "PhysicalShader.hpp"

using namespace Magnum;

PhysicalShader::PhysicalShader(u32 lightCount)
		: _lightCount{lightCount}, _lightColorsLocation{_lightPositionsLocation + (i32) lightCount}
{
	Utility::Resource rs("AsteropeShaders");

	GL::Shader vert{GL::Version::GL450, GL::Shader::Type::Vertex}, frag{GL::Version::GL450, GL::Shader::Type::Fragment};

	vert.addSource(rs.get("pbr.vert.glsl"));
	frag.addSource(Utility::formatString("#define LIGHT_COUNT {}\n", _lightCount))
			.addSource(Utility::formatString("#define LIGHT_COLORS_LOCATION {}\n", _lightColorsLocation))
			.addSource(rs.get("pbr.frag.glsl"));

	CORRADE_INTERNAL_ASSERT_OUTPUT(GL::Shader::compile({vert, frag}));

	attachShaders({vert, frag});
}
