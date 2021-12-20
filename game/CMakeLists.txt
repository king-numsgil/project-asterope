set_directory_properties(PROPERTIES CORRADE_USE_PEDANTIC_FLAGS ON)

corrade_add_resource(ASTEROPE_SHADERS_RCS resource/shaders/resource.conf)

add_executable(AsteropeGame
	${ASTEROPE_SHADERS_RCS}

	source/main.cpp
	source/imgui/AbstractImContext.cpp
	source/imgui/AbstractImContext.hpp
	source/imgui/AppImContext.cpp
	source/imgui/AppImContext.hpp
	source/imgui/ScreenImContext.cpp
	source/imgui/ScreenImContext.hpp
	source/scene/Components.hpp
	source/scene/Scene.cpp
	source/scene/Scene.hpp
)

set_target_properties(AsteropeGame
	PROPERTIES ${DEFAULT_PROJECT_OPTIONS}
)

target_include_directories(AsteropeGame
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/source
		${CMAKE_CURRENT_BINARY_DIR}/source
	PUBLIC
		${DEFAULT_INCLUDE_DIRECTORIES}
)

target_link_libraries(AsteropeGame
	PRIVATE
		Json EnTT ImGUI

		Corrade::Containers
		Corrade::Utility
		Corrade::Main

		Magnum::Magnum
		Magnum::GlfwApplication
		Magnum::Primitives
		Magnum::MeshTools
		Magnum::Shaders
		Magnum::Trade
		Magnum::Text
		Magnum::GL
	PUBLIC
		${DEFAULT_LIBRARIES}
)

add_dependencies(AsteropeGame
		MagnumPlugins::TinyGltfImporter
		MagnumPlugins::StbImageImporter
		MagnumPlugins::StbTrueTypeFont
)

target_compile_definitions(AsteropeGame
	PRIVATE
	PUBLIC
		${DEFAULT_COMPILE_DEFINITIONS}
)

target_compile_options(AsteropeGame
	PRIVATE
	PUBLIC
		${DEFAULT_COMPILE_OPTIONS}
)

target_link_libraries(AsteropeGame
	PRIVATE
	PUBLIC
		${DEFAULT_LINKER_OPTIONS}
)
