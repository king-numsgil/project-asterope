#pragma once

#include "scene/Scene.hpp"

class PlayerShip
{
	entt::handle _root{};
	entt::handle _center_screen{}, _left_screen{}, _right_screen{};

public:
	explicit PlayerShip(NoCreateT);

	explicit PlayerShip(Scene& scene);

	void create(Scene& scene);

	auto root()
	{ return _root; }

protected:
	virtual void process_center_screen(entt::const_handle entity);

	virtual void process_left_screen(entt::const_handle entity);

	virtual void process_right_screen(entt::const_handle entity);
};
