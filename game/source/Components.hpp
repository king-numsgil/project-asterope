#pragma once

#include <entt/entity/handle.hpp>
#include <Magnum/GL/Texture.h>
#include <Magnum/GL/Mesh.h>

#include "Types.hpp"

struct TransformComponent
{
	f32dquat transform{IdentityInit};
	entt::const_handle parent;

	explicit TransformComponent(entt::registry const& registry) : parent{registry, entt::null}
	{}

	[[nodiscard]] f32dquat world_transform() const
	{
		if (parent)
			return parent.get<TransformComponent>().transform * transform;
		else
			return transform;
	}

	TransformComponent& set_parent(entt::const_handle const& handle)
	{
		parent = handle;
		return *this;
	}

	TransformComponent& apply_transform(f32dquat const& dq)
	{
		transform = transform * dq;
		return *this;
	}
};

struct CameraComponent
{
	f32mat4 proj;

	explicit CameraComponent(Magnum::Math::IdentityInitT) : proj{IdentityInit} {}

	explicit CameraComponent(f32mat4 const& m) : proj{m} {}
};

struct MeshComponent
{
	Magnum::GL::Mesh mesh;

	explicit MeshComponent(Magnum::NoCreateT) : mesh{NoCreate} {}

	explicit MeshComponent(function<void(Magnum::GL::Mesh*)> const& buildFn)
	{
		mesh = Magnum::GL::Mesh{};
		buildFn(&mesh);
	}
};

struct PhongMaterialComponent
{
	f32col3 diffuse;

	explicit PhongMaterialComponent(f32col3 Diffuse = {1.f, 1.f, 1.f}) : diffuse{Diffuse} {}
};
