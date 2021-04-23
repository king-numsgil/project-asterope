#pragma once

#include <Magnum/GL/Renderbuffer.h>
#include <Magnum/GL/Framebuffer.h>
#include <entt/entity/fwd.hpp>
#include <utility>

#include "AbstractImContext.hpp"

class ScreenImContext : public AbstractImContext
{
public:
	explicit ScreenImContext(f32vec2 const& size, i32vec2 const& windowSize, i32vec2 const& framebufferSize);

	explicit ScreenImContext(i32vec2 const& size);

	explicit ScreenImContext(ImGuiContext& context, ImPlotContext& plotCtx, f32vec2 const& size,
	                         i32vec2 const& windowSize,
	                         i32vec2 const& framebufferSize);

	explicit ScreenImContext(ImGuiContext& context, ImPlotContext& plotCtx, i32vec2 const& size);

	explicit ScreenImContext(Magnum::NoCreateT) noexcept;

	ScreenImContext(ScreenImContext&& other) noexcept;

	ScreenImContext& operator=(ScreenImContext&& other) noexcept;

	Magnum::GL::Texture2D& color()
	{ return _color; }

	void drawFrame() override;

	void processCamera(f32dquat transform, f32dquat cam, bool is_control);

	void onMouseButton(MouseButton button, bool pressed);

protected:
	void create_resources(i32vec2 const& size);

	Magnum::GL::Framebuffer _fb{NoCreate};
	Magnum::GL::Renderbuffer _depth{NoCreate};
	Magnum::GL::Texture2D _color{NoCreate};
};

struct ScreenComponent
{
	ScreenImContext context;
	string title;
	function<void(entt::const_handle)> fn;

	explicit ScreenComponent(Magnum::NoCreateT)
			: context{NoCreate}, title{}, fn{}
	{}

	explicit ScreenComponent(string Title, i32vec2 const& size = {1024, 1024})
			: context{size}, title{std::move(Title)}, fn{}
	{}

	ScreenComponent& set_function(function<void(entt::const_handle)> const& f)
	{
		fn = f;
		return *this;
	}
};
