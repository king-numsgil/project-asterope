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

	explicit PhysicalShader(NoCreateT) noexcept
			: Magnum::GL::AbstractShaderProgram(NoCreate), _lightCount{1},
			  _lightColorsLocation{_lightPositionsLocation + 1}
	{}

	PhysicalShader(PhysicalShader const&) = delete;

	PhysicalShader& operator=(PhysicalShader const&) = delete;

	PhysicalShader(PhysicalShader&&) noexcept = default;

	PhysicalShader& operator=(PhysicalShader&&) noexcept = default;

	[[nodiscard]] u32 lightCount() const
	{ return _lightCount; }

	PhysicalShader& setViewProjectionMatrix(f32mat4 const& viewProj);

	PhysicalShader& setModelMatrix(f32mat4 const& model);

	PhysicalShader& setCameraPosition(f32vec3 const& position);

	PhysicalShader& setLightParameters(u32 index, f32vec3 const& position, f32col3 const& color);

	PhysicalShader& bindTextures(Magnum::GL::Texture2D* albedo,
	                             Magnum::GL::Texture2D* normal,
	                             Magnum::GL::Texture2D* metallic,
	                             Magnum::GL::Texture2D* roughness,
	                             Magnum::GL::Texture2D* ambientOcclusion = nullptr);

private:
	u32 _lightCount;
	i32 _viewProjMatrixLocation{0},
			_modelMatrixLocation{1},
			_camPositionLocation{2},
			_lightPositionsLocation{10},
			_lightColorsLocation;
};
