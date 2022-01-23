#pragma once

#include <Magnum/GL/Framebuffer.h>
#include <Magnum/Shaders/Phong.h>
#include <Magnum/Shaders/Flat.h>
#include <Magnum/GL/Texture.h>

#include <entt/entity/registry.hpp>

#include "shaders/PhysicalShader.hpp"
#include "Components.hpp"
#include "../Types.hpp"

class Scene
{
	Magnum::GL::Framebuffer _fbo{NoCreate};
	Magnum::GL::Texture2D _color{NoCreate};
	Magnum::GL::Texture2D _depth{NoCreate};
	Magnum::Shaders::Phong _phong{NoCreate};
	Magnum::Shaders::Flat3D _flat{NoCreate};
	PhysicalShader _pbr{NoCreate};

	i32vec2 _size{0, 0};
	entt::registry _reg{};

public:
	static f32mat4 createReverseProjectionMatrix(f32rad fov, f32 aspectRation, f32 near);

	explicit Scene(i32vec2 const& size);

	explicit Scene(NoCreateT) noexcept
	{}

	~Scene() = default;

	Scene(Scene const&) = delete;

	Scene(Scene&&) noexcept = default;

	Scene& operator=(Scene const&) = delete;

	Scene& operator=(Scene&&) noexcept = default;

	void create(i32vec2 const& size);

	void blitToDefaultFramebuffer();

	void render(entt::const_handle cam, bool isCamControl);

	auto& registry()
	{ return _reg; }

	auto& phongShader()
	{ return _phong; }

	auto& framebuffer()
	{ return _fbo; }

	auto& colorTarget()
	{ return _color; }

	auto& depthTarget()
	{ return _depth; }

	entt::handle createEntity();

private:
	void renderScreens(entt::const_handle cam, bool isCamControl);

	void renderEntities(entt::const_handle cam);
};
