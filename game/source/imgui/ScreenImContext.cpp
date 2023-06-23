#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/RenderbufferFormat.h>
#include <Magnum/Math/Intersection.h>
#include <Magnum/GL/TextureFormat.h>
#include <Magnum/GL/Renderer.h>

#include "ScreenImContext.hpp"

#include <imgui_internal.h>

using namespace Magnum;

ScreenImContext::ScreenImContext(f32vec2 const& size, i32vec2 const& windowSize, i32vec2 const& framebufferSize)
		: AbstractImContext(size, windowSize, framebufferSize)
{ create_resources(i32vec2{size}); }

ScreenImContext::ScreenImContext(i32vec2 const& size) : AbstractImContext(size)
{ create_resources(size); }

ScreenImContext::ScreenImContext(ImGuiContext& context, ImPlotContext& plotCtx, f32vec2 const& size,
                                 i32vec2 const& windowSize, i32vec2 const& framebufferSize)
		: AbstractImContext(context, plotCtx, size, windowSize, framebufferSize)
{ create_resources(i32vec2{size}); }

ScreenImContext::ScreenImContext(ImGuiContext& context, ImPlotContext& plotCtx, i32vec2 const& size)
		: AbstractImContext(context, plotCtx, size)
{ create_resources(size); }

ScreenImContext::ScreenImContext(Magnum::NoCreateT) noexcept
		: AbstractImContext{NoCreate}
{}

ScreenImContext::ScreenImContext(ScreenImContext&& other) noexcept
		: AbstractImContext{std::move(other)}, _fb{std::move(other._fb)}, _stencil{std::move(other._stencil)},
		  _color{std::move(other._color)}
{}

ScreenImContext& ScreenImContext::operator=(ScreenImContext&& other) noexcept
{
	std::swap(_fb, other._fb);
	std::swap(_stencil, other._stencil);
	std::swap(_color, other._color);
	AbstractImContext::operator=(std::move(other));
	return *this;
}

void ScreenImContext::create_resources(i32vec2 const& size)
{
	_stencil = GL::Renderbuffer{};
	_stencil.setStorage(GL::RenderbufferFormat::StencilIndex8, size);

	_color = GL::Texture2D{};
	_color.setStorage(1, GL::TextureFormat::RGBA8, size)
	      .setMagnificationFilter(GL::SamplerFilter::Linear)
	      .setMinificationFilter(GL::SamplerFilter::Linear);

	_fb = GL::Framebuffer{{{}, size}};
	_fb.attachTexture(GL::Framebuffer::ColorAttachment{0}, _color, 0)
	   .attachRenderbuffer(GL::Framebuffer::BufferAttachment::Stencil, _stencil)
	   .mapForDraw({{Shaders::FlatGL2D::ColorOutput, GL::Framebuffer::ColorAttachment{0}}});
	CORRADE_INTERNAL_ASSERT(_fb.checkStatus(GL::FramebufferTarget::Draw) == GL::Framebuffer::Status::Complete);
}

void ScreenImContext::drawFrame()
{
	_fb.clearColor(0, f32col4{0.f, 0.f, 0.f, 0.f})
	   .clearStencil(0)
	   .bind();

	GL::Renderer::disable(GL::Renderer::Feature::DepthTest);
	GL::Renderer::enable(GL::Renderer::Feature::Blending);
	GL::Renderer::enable(GL::Renderer::Feature::ScissorTest);
	GL::Renderer::disable(GL::Renderer::Feature::FaceCulling);

	AbstractImContext::drawFrame();

	GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);
	GL::Renderer::disable(GL::Renderer::Feature::ScissorTest);
	GL::Renderer::disable(GL::Renderer::Feature::Blending);
	GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
}

void ScreenImContext::processCamera(f32dquat transform, f32dquat cam, bool is_control)
{
	makeCurrent();
	ImGuiIO& io = ImGui::GetIO();

	if (!is_control)
	{
		io.MouseDrawCursor = false;
		return;
	}

	f32vec4 plane = Math::planeEquation(-transform.toMatrix().backward(), transform.translation());
	f32 distance = Math::Intersection::planeLine(plane, cam.translation(), -cam.toMatrix().backward());
	if (distance > 0.f)
	{
		f32vec3 pos = transform.inverted().transformPoint(cam.translation());
		f32vec3 dir = transform.inverted().transformVector(-cam.toMatrix().backward());
		f32vec3 interaction = pos + dir * distance;
		f32vec2 uv{
				interaction.x() / 2.f * _size.x() + (.5f * _size.x()),
				-interaction.y() / 2.f * _size.y() + (.5f * _size.y())
		};

		io.MousePos = {uv.x(), uv.y()};
		if (io.MousePos.x >= 0.f && io.MousePos.x <= _size.x() &&
		    io.MousePos.y >= 0.f && io.MousePos.y <= _size.y())
		{
			io.MouseDrawCursor = true;
		}
		else
		{
			io.MouseDrawCursor = false;
		}
	}
	else
	{
		io.MouseDrawCursor = false;
	}
}

void ScreenImContext::onMouseButton(MouseButton button, bool pressed)
{
	makeCurrent();
	ImGuiIO& io = ImGui::GetIO();

	if (io.MouseDrawCursor)
	{
		handleMouseEvent(button, i32vec2{f32vec2{io.MousePos}}, pressed);
	}
}
