#include <Magnum/Platform/GlfwApplication.h>
#include <Magnum/GL/DefaultFramebuffer.h>
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
#include "Components.hpp"

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
				       .setSampleCount(2)
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

		GL::Renderer::setBlendEquation(GL::Renderer::BlendEquation::Add, GL::Renderer::BlendEquation::Add);
		GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::SourceAlpha,
		                               GL::Renderer::BlendFunction::OneMinusSourceAlpha);

		_ctx = AppImContext{windowSize()};
		_time.start();

		_camParent = entt::handle{_reg, _reg.create()};
		_plane = entt::handle{_reg, _reg.create()};
		_cube = entt::handle{_reg, _reg.create()};
		_cam = entt::handle{_reg, _reg.create()};

		_cube.emplace<TransformComponent>(_reg);
		_cube.emplace<MeshComponent>(
				[](GL::Mesh* mesh)
				{ *mesh = MeshTools::compile(Primitives::cubeSolid()); });
		_cube.emplace<PhongMaterialComponent>(f32col3::fromHsv({35.0_degf, 1.0f, 1.0f}));

		_cam.emplace<TransformComponent>(_reg);
		_cam.emplace<CameraComponent>(f32mat4::perspectiveProjection(
				90.0_degf,
				f32vec2{framebufferSize()}.aspectRatio(),
				0.1f, 50.f
		));

		_camParent.emplace<TransformComponent>(_reg).transform = f32dquat::translation({0.f, 5.f, 5.f});
		_cam.get<TransformComponent>().parent = _camParent;

		_plane.emplace<TransformComponent>(_reg)
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

		_phong = Shaders::Phong{Shaders::Phong::Flag::ObjectId, 1};
		_phong.setLightColor(0, 0xffffff_rgbf)
				.setLightPosition(0, {0.f, 1.5f, 1.7f})
				.setAmbientColor(0x101010_rgbf);

		_flat3d = Shaders::Flat3D{Shaders::Flat3D::Flag::Textured};
	}

	virtual ~AsteropeGame() = default;

private:
	AppImContext _ctx{NoCreate};
	Timeline _time{};
	Shaders::Phong _phong{NoCreate};
	Shaders::Flat3D _flat3d{NoCreate};

	entt::registry _reg{};
	entt::handle _cam, _camParent, _cube, _plane;

	f32deg _camPitch{-45.f}, _camYaw{0.f};
	bool _camControl{false}, _testToggle{false};

	void drawEvent() override
	{
		updateCamera();
		renderScreens();

		GL::defaultFramebuffer
				.clearColor(0xa5c9ea_rgbf)
				.clearDepthStencil(1.f, 0)
				.bind();
		_ctx.newFrame();
		if (!_camControl) _ctx.updateApplicationCursor(*this);

		ImGui::ShowMetricsWindow();
		renderEntities();
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
			_reg.view<ScreenComponent>().each(
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
			_reg.view<ScreenComponent>().each(
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
			_camPitch -= f32deg{event.relativePosition().y() * _time.previousFrameDuration() * 3.f};
			_camYaw -= f32deg{event.relativePosition().x() * _time.previousFrameDuration() * 3.f};

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

	void renderScreens()
	{
		GL::Renderer::disable(GL::Renderer::Feature::DepthTest);
		GL::Renderer::enable(GL::Renderer::Feature::Blending);
		GL::Renderer::enable(GL::Renderer::Feature::ScissorTest);
		GL::Renderer::disable(GL::Renderer::Feature::FaceCulling);

		_reg.view<TransformComponent, ScreenComponent>().each(
				[this](entt::entity entity,
				       TransformComponent& transform,
				       ScreenComponent& screen)
				{
					screen.context.processCamera(transform.world_transform(),
					                             _cam.get<TransformComponent>().world_transform(), _camControl);
					screen.context.newFrame();
					ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
					ImGui::SetNextWindowSize(ImVec2{screen.context.size()}, ImGuiCond_Always);
					ImGui::Begin(screen.title.c_str(), nullptr,
					             ImGuiWindowFlags_NoResize |
					             ImGuiWindowFlags_NoCollapse |
					             ImGuiWindowFlags_NoMove |
					             ImGuiWindowFlags_NoSavedSettings);
					screen.fn(entt::const_handle{_reg, entity});
					ImGui::End();
					screen.context.drawFrame();
				});

		GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);
		GL::Renderer::disable(GL::Renderer::Feature::ScissorTest);
		GL::Renderer::disable(GL::Renderer::Feature::Blending);
		GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
	}

	void renderEntities()
	{
		_phong.setProjectionMatrix(_cam.get<CameraComponent>().proj *
		                           _cam.get<TransformComponent>().world_transform().toMatrix().invertedRigid());

		_reg.view<TransformComponent, MeshComponent, PhongMaterialComponent>().each(
				[this](entt::entity entity,
				       TransformComponent& transform,
				       MeshComponent& mesh,
				       PhongMaterialComponent& material)
				{
					_phong.setTransformationMatrix(transform.world_transform().toMatrix())
							.setNormalMatrix(transform.transform.toMatrix().normalMatrix())
							.setDiffuseColor(material.diffuse)
							.setObjectId(entt::to_integral(entity))
							.draw(mesh.mesh);
				});

		_reg.view<TransformComponent, MeshComponent, ScreenComponent>().each(
				[this](entt::entity entity,
				       TransformComponent& transform,
				       MeshComponent& mesh,
				       ScreenComponent& screen)
				{
					_flat3d.setTransformationProjectionMatrix(
									_cam.get<CameraComponent>().proj *
									_cam.get<TransformComponent>().world_transform().toMatrix().invertedRigid() *
									transform.world_transform().toMatrix()
							)
							.bindTexture(screen.context.color())
							.draw(mesh.mesh);
				});
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
