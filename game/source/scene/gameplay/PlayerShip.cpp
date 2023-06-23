#include <Magnum/MeshTools/Transform.h>
#include <Magnum/MeshTools/Compile.h>
#include <Magnum/Primitives/Plane.h>
#include <Magnum/Trade/MeshData.h>
#include <Magnum/MeshTools/Copy.h>

#include "imgui/ScreenImContext.hpp"
#include "PlayerShip.h"

using namespace Magnum;

PlayerShip::PlayerShip(NoCreateT)
{}

PlayerShip::PlayerShip(Scene& scene)
{
	create(scene);
}

void PlayerShip::create(Scene& scene)
{
	_root = scene.createEntity();

	_center_screen = scene.createEntity();
	_center_screen.get<TransformComponent>()
	              .set_parent(_root)
	              .apply_transform(f32dquat::translation(f32vec3{0.f, 1.8f, 0.f}) *
	                               f32dquat::rotation(-30.0_degf, f32vec3::xAxis()));
	_center_screen.emplace<MeshComponent>(
			[](GL::Mesh* mesh)
			{ *mesh = MeshTools::compile(Primitives::planeSolid(Primitives::PlaneFlag::TextureCoordinates)); });
	_center_screen.emplace<ScreenComponent>("Main Screen", i32vec2{512, 512})
	              .set_function([this](entt::const_handle entity)
	                            { process_center_screen(entity); });

	_left_screen = scene.createEntity();
	_left_screen.get<TransformComponent>()
	            .set_parent(_root)
	            .apply_transform(f32dquat::translation(f32vec3{-2.5f, 2.1f, .5f}) *
	                             f32dquat::rotation(-30.0_degf, f32vec3::xAxis()) *
			                             f32dquat::rotation(30.0_degf, f32vec3::yAxis()));
	_left_screen.emplace<MeshComponent>(
			[](GL::Mesh* mesh)
			{ *mesh = MeshTools::compile(Primitives::planeSolid(Primitives::PlaneFlag::TextureCoordinates)); });
	_left_screen.emplace<ScreenComponent>("Left Screen", i32vec2{512, 512})
	            .set_function([this](entt::const_handle entity)
	                          { process_left_screen(entity); });

	_right_screen = scene.createEntity();
	_right_screen.get<TransformComponent>()
	             .set_parent(_root)
	             .apply_transform(f32dquat::translation(f32vec3{2.5f, 2.1f, .5f}) *
	                              f32dquat::rotation(-30.0_degf, f32vec3::xAxis()) *
	                              f32dquat::rotation(-30.0_degf, f32vec3::yAxis()));
	_right_screen.emplace<MeshComponent>(
			[](GL::Mesh* mesh)
			{ *mesh = MeshTools::compile(Primitives::planeSolid(Primitives::PlaneFlag::TextureCoordinates)); });
	_right_screen.emplace<ScreenComponent>("Right Screen", i32vec2{512, 512})
	             .set_function([this](entt::const_handle entity)
	                           { process_right_screen(entity); });
}

void PlayerShip::process_center_screen(entt::const_handle)
{
	ImGui::Text("This is the Main Screen");
}

void PlayerShip::process_left_screen(entt::const_handle)
{
	ImGui::Text("This is the Left Screen");
}

void PlayerShip::process_right_screen(entt::const_handle)
{
	ImGui::Text("This is the Right Screen");
}
