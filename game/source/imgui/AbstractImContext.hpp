#pragma once

#include <Magnum/Platform/GlfwApplication.h>
#include <Magnum/GL/AbstractShaderProgram.h>
#include <Magnum/Shaders/Flat.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/GL/Buffer.h>
#include <Magnum/Timeline.h>
#include <Magnum/GL/Mesh.h>
#include <implot.h>
#include <imgui.h>

#include "../Types.hpp"

using MouseButton = Magnum::Platform::GlfwApplication::MouseEvent::Button;
using KeyCode = Magnum::Platform::GlfwApplication::KeyEvent::Key;

namespace Magnum::Math::Implementation
{
	template<>
	struct VectorConverter<2, f32, ImVec2>
	{
		static f32vec2 from(ImVec2 const& other)
		{
			return {other.x, other.y};
		}

		static ImVec2 to(f32vec2 const& other)
		{
			return {other[0], other[1]};
		}
	};

	template<>
	struct VectorConverter<4, f32, ImVec4>
	{
		static f32vec4 from(ImVec4 const& other)
		{
			return {other.x, other.y, other.z, other.w};
		}

		static ImVec4 to(f32vec4 const& other)
		{
			return {other[0], other[1], other[2], other[3]};
		}
	};

	template<>
	struct VectorConverter<4, f32, ImColor>
	{
		static f32vec4 from(ImColor const& other)
		{
			return f32vec4{other.Value};
		}

		static ImColor to(f32vec4 const& other)
		{
			return ImVec4{other};
		}
	};

	template<>
	struct VectorConverter<3, f32, ImColor>
	{
		static f32vec3 from(ImColor const& other)
		{
			return f32vec3{other.Value.x, other.Value.y, other.Value.z};
		}

		static ImColor to(f32vec3 const& other)
		{
			return ImVec4{f32vec4{other[0], other[1], other[2], 1.0f}};
		}
	};
}

class AbstractImContext
{
public:
	AbstractImContext(AbstractImContext const&) = delete;

	AbstractImContext(AbstractImContext&& other) noexcept;

	virtual ~AbstractImContext();

	AbstractImContext& operator=(AbstractImContext const&) = delete;

	AbstractImContext& operator=(AbstractImContext&& other) noexcept;

	void makeCurrent() const
	{
		ImGui::SetCurrentContext(_context);
		ImPlot::SetCurrentContext(_plotCtx);
	}

	ImGuiContext* context()
	{ return _context; }

	ImGuiContext* release();

	Magnum::GL::Texture2D& atlasTexture()
	{ return _texture; }

	[[nodiscard]] f32vec2 size() const
	{ return _size; }

	void relayout(f32vec2 const& size, i32vec2 const& windowSize, i32vec2 const& framebufferSize);

	void relayout(i32vec2 const& size);

	void newFrame();

	virtual void drawFrame();

protected:
	explicit AbstractImContext(f32vec2 const& size, i32vec2 const& windowSize, i32vec2 const& framebufferSize);

	explicit AbstractImContext(i32vec2 const& size);

	explicit AbstractImContext(ImGuiContext& context, ImPlotContext& plotCtx, f32vec2 const& size, i32vec2 const& windowSize,
			i32vec2 const& framebufferSize);

	explicit AbstractImContext(ImGuiContext& context, ImPlotContext& plotCtx, i32vec2 const& size);

	explicit AbstractImContext(Magnum::NoCreateT) noexcept;

	bool handleKeyEvent(KeyCode key, bool pressed);

	bool handleMouseEvent(MouseButton button, i32vec2 position, bool pressed);

	ImGuiContext* _context;
	ImPlotContext* _plotCtx;
	Magnum::Shaders::Flat2D _shader;
	Magnum::GL::Texture2D _texture{Magnum::NoCreate};
	Magnum::GL::Buffer _vertexBuffer{Magnum::GL::Buffer::TargetHint::Array};
	Magnum::GL::Buffer _indexBuffer{Magnum::GL::Buffer::TargetHint::ElementArray};
	Magnum::Timeline _timeline;
	Magnum::GL::Mesh _mesh;
	f32vec2 _size, _supersamplingRatio, _eventScaling;
	Magnum::BoolVector3 _mousePressed, _mousePressedInThisFrame;
};
