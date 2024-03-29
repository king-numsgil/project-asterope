#include <Magnum/Trade/AbstractImporter.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/TextureFormat.h>
#include <Magnum/Trade/ImageData.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/ImageView.h>
#include <filesystem>

#include "../imgui/ScreenImContext.hpp"
#include "Scene.hpp"

using namespace Magnum;
using namespace entt;

static GL::Texture2D loadTexture(std::filesystem::path const& filename, Containers::Pointer<Trade::AbstractImporter>& importer)
{
	if (!importer->openFile(filename.string().c_str()))
	{
		Fatal{} << "Could not load texture" << filename.string();
		std::exit(1);
	}

	GL::Texture2D ret{};
	Containers::Optional<Trade::ImageData2D> image = importer->image2D(0);
	CORRADE_INTERNAL_ASSERT(image);

	ret.setWrapping(GL::SamplerWrapping::ClampToEdge)
	   .setMagnificationFilter(GL::SamplerFilter::Linear)
	   .setMinificationFilter(GL::SamplerFilter::Linear)
	   .setStorage(1, GL::textureFormat(image->format()), image->size())
	   .setSubImage(0, {}, *image);
	return ret;
}

void PhysicalMaterialComponent::loadTextures()
{
	Corrade::PluginManager::Manager<Trade::AbstractImporter> manager;
	Containers::Pointer<Trade::AbstractImporter> importer = manager.loadAndInstantiate("StbImageImporter");
	if (!importer)
	{
		Fatal{} << "Could not load plugin StbImageImporter";
		std::exit(1);
	}

	std::filesystem::path textures{path};
	albedo = loadTexture(textures / "albedo.png", importer);
	ambientOcclusion = loadTexture(textures / "ao.png", importer);
	metallic = loadTexture(textures / "metallic.png", importer);
	normal = loadTexture(textures / "normal.png", importer);
	roughness = loadTexture(textures / "roughness.png", importer);
}

f32mat4 Scene::createReverseProjectionMatrix(f32rad fov, f32 aspectRation, f32 near)
{
	f32 f = 1.f / Math::tan(fov / 2.f);

	return f32mat4{
			{f / aspectRation, 0.f, 0.f,  0.f},
			{0.f,              f,   0.f,  0.f},
			{0.f,              0.f, 0.f,  -1.f},
			{0.f,              0.f, near, 0.f}
	};
}

Scene::Scene(i32vec2 const& size, u32 lightCount)
{
	create(size, lightCount);
}

void Scene::create(i32vec2 const& size, u32 lightCount)
{
	GL::Renderer::setClipControl(GL::Renderer::ClipOrigin::LowerLeft, GL::Renderer::ClipDepth::ZeroToOne);

	_size = size;
	_phong = Shaders::PhongGL{Shaders::PhongGL::Configuration{}
			                          .setFlags(Shaders::PhongGL::Flag::ObjectId)};
	_flat = Shaders::FlatGL3D{Shaders::FlatGL3D::Configuration{}
			                          .setFlags(Shaders::FlatGL3D::Flag::Textured | Shaders::FlatGL3D::Flag::AlphaMask)};
	_pbr = PhysicalShader{lightCount};

	_color = GL::Texture2D{};
	_color.setStorage(1, GL::TextureFormat::RGBA8, size);

	_depth = GL::Texture2D{};
	_depth.setStorage(1, GL::TextureFormat::DepthComponent32F, size);

	_fbo = GL::Framebuffer{i32range2{{{}, size}}};
	_fbo.attachTexture(GL::Framebuffer::ColorAttachment{0}, _color, 0)
	    .attachTexture(GL::Framebuffer::BufferAttachment::Depth, _depth, 0)
	    .mapForDraw({{Shaders::PhongGL::ColorOutput, GL::Framebuffer::ColorAttachment{0}}});
	CORRADE_INTERNAL_ASSERT(_fbo.checkStatus(GL::FramebufferTarget::Draw) == GL::Framebuffer::Status::Complete);
}

void Scene::blitToDefaultFramebuffer()
{
	GL::Framebuffer::blit(_fbo, GL::defaultFramebuffer, i32range2{{}, _size}, GL::FramebufferBlit::Color);
}

void Scene::render(const_handle cam, bool isCamControl)
{
	renderScreens(cam, isCamControl);

	_fbo.clearColor(0, f32col4{0.f, 0.f, 0.f, 0.f})
	    .clearDepth(0.f)
	    .bind();

	GL::Renderer::setDepthFunction(GL::Renderer::DepthFunction::Greater);
	renderEntities(cam);
	GL::Renderer::setDepthFunction(GL::Renderer::DepthFunction::Less);
}

void Scene::renderScreens(const_handle cam, bool isCamControl)
{
	GL::Renderer::disable(GL::Renderer::Feature::DepthTest);
	GL::Renderer::enable(GL::Renderer::Feature::Blending);
	GL::Renderer::enable(GL::Renderer::Feature::ScissorTest);
	GL::Renderer::disable(GL::Renderer::Feature::FaceCulling);

	_reg.view<TransformComponent, ScreenComponent>().each(
			[this, &cam, &isCamControl](entt::entity entity,
			                            TransformComponent& transform,
			                            ScreenComponent& screen)
			{
				screen.context.processCamera(transform.world_transform(),
				                             cam.get<TransformComponent>().world_transform(), isCamControl);
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

void Scene::renderEntities(const_handle cam)
{
	const f32mat4 view = cam.get<CameraComponent>().proj * cam.get<TransformComponent>().world_transform().toMatrix().invertedRigid();
	_phong.setProjectionMatrix(view);
	_pbr.setViewProjectionMatrix(view)
	    .setCameraPosition(cam.get<TransformComponent>().world_transform().translation());

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

	_reg.view<TransformComponent, MeshComponent, PhysicalMaterialComponent>().each(
			[this](entt::entity entity,
			       TransformComponent& transform,
			       MeshComponent& mesh,
			       PhysicalMaterialComponent& mat)
			{
				_pbr.setModelMatrix(transform.world_transform().toMatrix())
				    .bindTextures(&mat.albedo, &mat.normal, &mat.metallic, &mat.roughness, &mat.ambientOcclusion)
				    .draw(mesh.mesh);
			});

	GL::Renderer::enable(GL::Renderer::Feature::Blending);
	_reg.view<TransformComponent, MeshComponent, ScreenComponent>().each(
			[this, &view](entt::entity entity,
			             TransformComponent& transform,
			             MeshComponent& mesh,
			             ScreenComponent& screen)
			{
				_flat.setTransformationProjectionMatrix(view * transform.world_transform().toMatrix())
				     .bindTexture(screen.context.color())
				     .draw(mesh.mesh);
			});
	GL::Renderer::disable(GL::Renderer::Feature::Blending);
}

entt::handle Scene::createEntity()
{
	auto ret = entt::handle{_reg, _reg.create()};
	ret.emplace<TransformComponent>();
	return ret;
}
