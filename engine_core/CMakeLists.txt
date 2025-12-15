add_library(engine_core STATIC
    # Core
    src/core/logger.cpp
    src/core/application.cpp
    src/core/timer.cpp
    src/core/profiler.cpp
    src/core/debug_overlay.cpp
    src/core/property_system.cpp

    # Platform
    src/core/platform_sdl.cpp

    # VFS (Legacy)
    src/vfs/virtual_fs.cpp
    src/vfs/memory_fs.cpp
    src/vfs/pack_reader.cpp

    # VFS (Enhanced)
    src/vfs/file_handle.cpp
    src/vfs/resource_id.cpp
    src/vfs/file_system_backend.cpp
    src/vfs/resource_cache.cpp
    src/vfs/virtual_file_system.cpp
    src/vfs/pack_security.cpp

    # Renderer
    src/renderer/renderer.cpp
    src/renderer/texture.cpp
    src/renderer/sprite.cpp
    src/renderer/font.cpp

    # Scripting
    src/scripting/interpreter.cpp
    src/scripting/vm.cpp
    src/scripting/vm_security.cpp
    src/scripting/lexer.cpp
    src/scripting/parser.cpp
    src/scripting/compiler.cpp
    src/scripting/validator.cpp
    src/scripting/script_runtime.cpp
    src/scripting/ir.cpp

    # Renderer (Text)
    src/renderer/text_layout.cpp

    # Scene
    src/scene/scene_manager.cpp
    src/scene/scene_object.cpp
    src/scene/character_sprite.cpp
    src/scene/dialogue_box.cpp
    src/scene/choice_menu.cpp
    src/scene/transition.cpp
    src/scene/scene_graph.cpp
    src/scene/scene_inspector.cpp

    # Input
    src/input/input_manager.cpp

    # Audio
    src/audio/audio_manager.cpp

    # Save
    src/save/save_manager.cpp

    # UI Framework
    src/ui/ui_framework.cpp

    # Localization
    src/localization/localization_manager.cpp

    # VFS Multi-Pack
    src/vfs/multi_pack_manager.cpp
)

target_include_directories(engine_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(engine_core
    PRIVATE
        novelmind_compiler_options
)

# Find SDL2 (optional for now, will be required later)
find_package(SDL2 QUIET)
if(SDL2_FOUND)
    target_link_libraries(engine_core PRIVATE SDL2::SDL2)
    target_compile_definitions(engine_core PRIVATE NOVELMIND_HAS_SDL2)
endif()

# Define version
target_compile_definitions(engine_core
    PUBLIC
        NOVELMIND_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
        NOVELMIND_VERSION_MINOR=${PROJECT_VERSION_MINOR}
        NOVELMIND_VERSION_PATCH=${PROJECT_VERSION_PATCH}
)
