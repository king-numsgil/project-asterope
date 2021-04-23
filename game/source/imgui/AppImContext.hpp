#pragma once

#include "AbstractImContext.hpp"

class AppImContext : public AbstractImContext
{
public:
	explicit AppImContext(f32vec2 const& size, i32vec2 const& windowSize, i32vec2 const& framebufferSize)
			: AbstractImContext{size, windowSize, framebufferSize}
	{}

	explicit AppImContext(i32vec2 const& size)
			: AbstractImContext{size}
	{}

	explicit AppImContext(ImGuiContext& context, ImPlotContext& plotCtx, f32vec2 const& size, i32vec2 const& windowSize,
	                      i32vec2 const& framebufferSize)
			: AbstractImContext{context, plotCtx, size, windowSize, framebufferSize}
	{}

	explicit AppImContext(ImGuiContext& context, ImPlotContext& plotCtx, i32vec2 const& size)
			: AbstractImContext{context, plotCtx, size}
	{}

	explicit AppImContext(Magnum::NoCreateT) noexcept
			: AbstractImContext{NoCreate}
	{}

	template<class MouseEvent>
	inline bool handleMousePressEvent(MouseEvent& event)
	{ return handleMouseEvent(event.button(), event.position(), true); }

	template<class MouseEvent>
	inline bool handleMouseReleaseEvent(MouseEvent& event)
	{ return handleMouseEvent(event.button(), event.position(), false); }

	template<class MouseScrollEvent>
	inline bool handleMouseScrollEvent(MouseScrollEvent& event)
	{
		makeCurrent();

		ImGuiIO& io = ImGui::GetIO();
		io.MousePos = ImVec2{f32vec2{event.position()} * _eventScaling};
		io.MouseWheel += event.offset().y();
		io.MouseWheelH += event.offset().x();
		return io.WantCaptureMouse;
	}

	template<class MouseMoveEvent>
	inline bool handleMouseMoveEvent(MouseMoveEvent& event)
	{
		makeCurrent();

		ImGuiIO& io = ImGui::GetIO();
		io.MousePos = ImVec2{f32vec2{event.position()} * _eventScaling};
		return io.WantCaptureMouse;
	}

	template<class KeyEvent>
	inline bool handleKeyPressEvent(KeyEvent& event)
	{ return handleKeyEvent(event.key(), true); }

	template<class KeyEvent>
	inline bool handleKeyReleaseEvent(KeyEvent& event)
	{ return handleKeyEvent(event.key(), false); }

	template<class TextInputEvent>
	inline bool handleTextInputEvent(TextInputEvent& event)
	{
		makeCurrent();

		ImGui::GetIO().AddInputCharactersUTF8(event.text().data());
		return false;
	}

	template<class Application>
	inline void updateApplicationCursor(Application& application)
	{
		makeCurrent();

		switch (ImGui::GetMouseCursor())
		{
			case ImGuiMouseCursor_TextInput:
				application.setCursor(Application::Cursor::TextInput);
				return;
			case ImGuiMouseCursor_ResizeNS:
				application.setCursor(Application::Cursor::ResizeNS);
				return;
			case ImGuiMouseCursor_ResizeEW:
				application.setCursor(Application::Cursor::ResizeWE);
				return;
			case ImGuiMouseCursor_Hand:
				application.setCursor(Application::Cursor::Hand);
				return;
			case ImGuiMouseCursor_None:
				application.setCursor(Application::Cursor::Hidden);
				return;

				/* For unknown cursors we set Arrow as well */
			case ImGuiMouseCursor_Arrow:
			default:
				application.setCursor(Application::Cursor::Arrow);
				return;
		}

		CORRADE_INTERNAL_ASSERT_UNREACHABLE();
	}
};
