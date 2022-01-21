#pragma once

#include <Magnum/GL/AbstractShaderProgram.h>

#include "../../Types.hpp"

class PhysicalShader : public Magnum::GL::AbstractShaderProgram
{
public:
	using Position = Magnum::GL::Attribute<0, f32vec3>;
	using Normal = Magnum::GL::Attribute<1, f32vec3>;
	using TextureCoordinates = Magnum::GL::Attribute<2, f32vec2>;

	explicit PhysicalShader(u32 lightCount = 1);

	explicit PhysicalShader(NoCreateT) noexcept: Magnum::GL::AbstractShaderProgram(NoCreate) {}

	PhysicalShader(PhysicalShader const&) = delete;

	PhysicalShader& operator=(PhysicalShader const&) = delete;

	PhysicalShader(PhysicalShader&&) noexcept = default;

	PhysicalShader& operator=(PhysicalShader&&) noexcept = default;

private:
	u32 _lightCount;
	i32 _viewProjMatrixLocation{0},
			_modelMatrixLocation{1},
			_camPositionLocation{2},
			_lightPositionsLocation{10},
			_lightColorsLocation;
};
