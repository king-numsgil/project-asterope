#include <Magnum/Platform/GlfwApplication.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/Primitives/UVSphere.h>
#include <Magnum/MeshTools/Transform.h>
#include <Magnum/MeshTools/Reference.h>
#include <Magnum/MeshTools/Compile.h>
#include <Magnum/Primitives/Plane.h>
#include <Magnum/Primitives/Cube.h>
#include <Magnum/Trade/MeshData.h>
#include <Magnum/GL/DebugOutput.h>
#include <Magnum/GL/PixelFormat.h>
#include <Magnum/Shaders/Phong.h>
#include <Magnum/Shaders/Flat.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/Timeline.h>
#include <Magnum/Image.h>

#include <entt/entity/registry.hpp>

#include "imgui/ScreenImContext.hpp"
#include "imgui/AppImContext.hpp"
#include "scene/Scene.hpp"

using namespace Magnum;

class AsteropeGame : public Platform::Application
{
public:
	explicit AsteropeGame(Arguments const& arguments)
			: Platform::Application{arguments, NoCreate}
	{
		create(Configuration{}
				       .setTitle("Asterope Game Rendering Window")
				       .setSize({1280, 768}),
		       GLConfiguration{}
				       .setSrgbCapable(true)
				       .setSampleCount(4)
#ifndef NDEBUG
				       .addFlags(GLConfiguration::Flag::Debug)
#endif
		);
		setSwapInterval(1);

#ifndef NDEBUG
		GL::Renderer::enable(GL::Renderer::Feature::DebugOutput);
		GL::Renderer::enable(GL::Renderer::Feature::DebugOutputSynchronous);
		GL::DebugOutput::setDefaultCallback();
		GL::DebugOutput::setEnabled(GL::DebugOutput::Source::Api, GL::DebugOutput::Type::Other, {131185}, false);
#endif

		GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
		GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);
		GL::Renderer::disable(GL::Renderer::Feature::ScissorTest);
		GL::Renderer::disable(GL::Renderer::Feature::Blending);

		GL::Renderer::setDepthFunction(GL::Renderer::DepthFunction::Less);
		GL::Renderer::setBlendEquation(GL::Renderer::BlendEquation::Add, GL::Renderer::BlendEquation::Add);
		GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::SourceAlpha,
		                               GL::Renderer::BlendFunction::OneMinusSourceAlpha);

		_ctx = AppImContext{windowSize()};
		_time.start();

		_scene.create(framebufferSize());
		_camParent = _scene.createEntity();
		_plane = _scene.createEntity();
		_cube = _scene.createEntity();
		_cam = _scene.createEntity();

		_cube.emplace<TransformComponent>(_scene.registry());
		_cube.emplace<MeshComponent>(
				[](GL::Mesh* mesh)
				{ *mesh = MeshTools::compile(Primitives::cubeSolid()); });
		_cube.emplace<PhongMaterialComponent>(f32col3::fromHsv({35.0_degf, 1.0f, 1.0f}));

		_cam.emplace<TransformComponent>(_scene.registry());
		_cam.emplace<CameraComponent>(Scene::createReverseProjectionMatrix(
				60.0_degf,
				f32vec2{framebufferSize()}.aspectRatio(),
				0.1f
		));

		_camParent.emplace<TransformComponent>(_scene.registry()).transform = f32dquat::translation({0.f, 5.f, 5.f});
		_cam.get<TransformComponent>().parent = _camParent;

		_plane.emplace<TransformComponent>(_scene.registry())
				.set_parent(_camParent)
				.apply_transform(f32dquat::translation(f32vec3::zAxis(-2.f)));
		_plane.emplace<MeshComponent>(
				[](GL::Mesh* mesh)
				{ *mesh = MeshTools::compile(Primitives::planeSolid(Primitives::PlaneFlag::TextureCoordinates)); });
		_plane.emplace<ScreenComponent>("Test Screen", i32vec2{512, 512})
				.set_function(
						[this](entt::const_handle entity)
						{
							ImGui::Text("Hello World!");
							ImGui::Text("Toggle Me!");
							ImGui::SameLine();
							ImGui::ToggleButton("ToggleTest", &_testToggle);
							if (_testToggle)
								ImGui::Text("ACTIVE!");
							else
								ImGui::Text("INACTIVE :(");
						}
				);

		_scene.phongShader().setLightColor(0, 0xffffff_rgbf)
				.setLightPosition(0, {0.f, 1.5f, 1.7f})
				.setAmbientColor(0x101010_rgbf);

		const f32 earthRadius = 6'378'000.f, moonRadius = 1'737'500.f;

		auto earth = _scene.createEntity();
		earth.emplace<PhongMaterialComponent>(0x275f91_rgbf);
		earth.emplace<TransformComponent>(_scene.registry())
				.apply_transform(f32dquat::translation(f32vec3::yAxis(-earthRadius - 1.f)));
		earth.emplace<MeshComponent>(
				[earthRadius](GL::Mesh* mesh)
				{
					auto data = MeshTools::owned(Primitives::uvSphereSolid(30, 30));
					MeshTools::transformPointsInPlace(f32mat4::scaling({earthRadius, earthRadius, earthRadius}),
					                                  data.mutableAttribute<f32vec3>(Trade::MeshAttribute::Position));
					*mesh = MeshTools::compile(data);
				}
		);

		auto moon = _scene.createEntity();
		moon.emplace<PhongMaterialComponent>(0xe6ea98_rgbf);
		moon.emplace<TransformComponent>(_scene.registry())
				.set_parent(earth)
				.apply_transform(f32dquat::translation(f32vec3::yAxis(384'400'000.f)));
		moon.emplace<MeshComponent>(
				[moonRadius](GL::Mesh* mesh)
				{
					auto data = MeshTools::owned(Primitives::uvSphereSolid(30, 30));
					MeshTools::transformPointsInPlace(f32mat4::scaling({moonRadius, moonRadius, moonRadius}),
					                                  data.mutableAttribute<f32vec3>(Trade::MeshAttribute::Position));
					*mesh = MeshTools::compile(data);
				}
		);
	}

	virtual ~AsteropeGame() = default;

private:
	AppImContext _ctx{NoCreate};
	Timeline _time{};

	Scene _scene{NoCreate};
	entt::handle _cam, _camParent, _cube, _plane;

	f32deg _camPitch{-45.f}, _camYaw{0.f};
	bool _camControl{false}, _testToggle{false};

	void drawEvent() override
	{
		updateCamera();
		_scene.render(_cam, _camControl);

		GL::defaultFramebuffer
				.clearColor(0xa5c9ea_rgbf)
				.clearDepthStencil(1.f, 0)
				.bind();
		_scene.blitToDefaultFramebuffer();
		if (!_camControl) _ctx.updateApplicationCursor(*this);

		_ctx.newFrame();

		ImGui::Begin("Depth Buffer");
		ImGui::Image(reinterpret_cast<ImTextureID>(&_scene.depthTarget()),
		             {(f32) framebufferSize().x() / 4.f, (f32) framebufferSize().y() / 4.f},
		             {0.f, 1.f}, {1.f, 0.f});
		ImGui::End();

		ImGui::ShowMetricsWindow();

		renderMainImgui();

		swapBuffers();
		redraw();
		_time.nextFrame();
	}

	void keyReleaseEvent(KeyEvent& event) override
	{ _ctx.handleKeyReleaseEvent(event); }

	void keyPressEvent(KeyEvent& event) override
	{
		if (!_ctx.handleKeyPressEvent(event))
		{
			if (event.key() == KeyEvent::Key::Esc)
			{
				exit();
				return;
			}

			if (event.key() == KeyEvent::Key::Space)
			{
				if (_camControl)
				{
					_camControl = false;
					setCursor(Cursor::Arrow);
				}
				else
				{
					_camControl = true;
					setCursor(Cursor::HiddenLocked);
				}
			}
		}
	}

	void mouseReleaseEvent(MouseEvent& event) override
	{
		if (_camControl)
		{
			_scene.registry().view<ScreenComponent>().each(
					[&event](auto entity, auto& screen)
					{
						screen.context.onMouseButton(event.button(), false);
					}
			);
		}
		else if (!_ctx.handleMouseReleaseEvent(event))
		{}
	}

	void mousePressEvent(MouseEvent& event) override
	{
		if (_camControl)
		{
			_scene.registry().view<ScreenComponent>().each(
					[&event](auto entity, auto& screen)
					{
						screen.context.onMouseButton(event.button(), true);
					}
			);
		}
		else if (!_ctx.handleMousePressEvent(event))
		{}
	}

	void mouseMoveEvent(MouseMoveEvent& event) override
	{
		if (_camControl)
		{
			_camPitch -= f32deg{static_cast<f32>(event.relativePosition().y()) * _time.previousFrameDuration() * 3.f};
			_camYaw -= f32deg{static_cast<f32>(event.relativePosition().x()) * _time.previousFrameDuration() * 3.f};

			if (_camPitch >= 90.0_degf)
				_camPitch = 90.0_degf;
			if (_camPitch <= -90.0_degf)
				_camPitch = -90.0_degf;
		}
		else if (!_ctx.handleMouseMoveEvent(event))
		{
		}
	}

	void mouseScrollEvent(MouseScrollEvent& event) override
	{ _ctx.handleMouseScrollEvent(event); }

	void textInputEvent(TextInputEvent& event) override
	{ _ctx.handleTextInputEvent(event); }

	void updateCamera()
	{
		_cam.get<TransformComponent>().transform = f32dquat::rotation(_camYaw, f32vec3::yAxis()) *
		                                           f32dquat::rotation(_camPitch, f32vec3::xAxis());

		if (_camControl)
		{
			f32mat4 m = _cam.get<TransformComponent>().world_transform().toMatrix();
			auto& cam = _camParent.get<TransformComponent>();
			float rate = _time.previousFrameDuration() *
			             (glfwGetKey(window(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ? 4.f : 2.f);

			if (glfwGetKey(window(), GLFW_KEY_W) == GLFW_PRESS)
				cam.apply_transform(f32dquat::translation(-m.backward() * rate));
			if (glfwGetKey(window(), GLFW_KEY_S) == GLFW_PRESS)
				cam.apply_transform(f32dquat::translation(m.backward() * rate));

			if (glfwGetKey(window(), GLFW_KEY_D) == GLFW_PRESS)
				cam.apply_transform(f32dquat::translation(m.right() * rate));
			if (glfwGetKey(window(), GLFW_KEY_A) == GLFW_PRESS)
				cam.apply_transform(f32dquat::translation(-m.right() * rate));
		}
	}

	void renderMainImgui()
	{
		GL::Renderer::disable(GL::Renderer::Feature::DepthTest);
		GL::Renderer::enable(GL::Renderer::Feature::Blending);
		GL::Renderer::enable(GL::Renderer::Feature::ScissorTest);
		GL::Renderer::disable(GL::Renderer::Feature::FaceCulling);

		_ctx.makeCurrent();
		_ctx.drawFrame();

		GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);
		GL::Renderer::disable(GL::Renderer::Feature::ScissorTest);
		GL::Renderer::disable(GL::Renderer::Feature::Blending);
		GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
	}
};

int main(int argc, char** argv)
{
	glfwInitHint(GLFW_JOYSTICK_HAT_BUTTONS, GLFW_FALSE);

	AsteropeGame app{{argc, argv}};
	return app.exec();
}
