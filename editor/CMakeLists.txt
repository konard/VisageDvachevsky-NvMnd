# NovelMind Visual Editor - Qt 6 Based GUI
#
# This is a complete remake of the GUI using Qt 6 Widgets,
# replacing the previous ImGui/SDL2 implementation.

# Find Qt6
find_package(Qt6 COMPONENTS Widgets Core Gui Svg QUIET)

if(NOT Qt6_FOUND)
    message(STATUS "NovelMind Editor: Qt6 not found - Editor will not be built")
    message(STATUS "  Install Qt6 development packages to enable the editor:")
    message(STATUS "    Ubuntu/Debian: sudo apt-get install qt6-base-dev qt6-svg-dev")
    message(STATUS "    Fedora: sudo dnf install qt6-qtbase-devel qt6-qtsvg-devel")
    message(STATUS "    macOS: brew install qt@6")
    message(STATUS "    Windows: Download Qt6 from qt.io")

    # Disable editor build flag so tests know not to link against it
    set(NOVELMIND_BUILD_EDITOR OFF PARENT_SCOPE)
    return()
endif()

message(STATUS "NovelMind Editor: Qt6 found - Building editor with Qt6 GUI")

# Enable Qt MOC, UIC, RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Qt6 Editor Library
add_library(novelmind_editor STATIC
    # Core editor systems (backend - retained from previous implementation)
    src/editor_runtime_host.cpp
    src/asset_pipeline.cpp
    src/editor_settings.cpp
    src/voice_manager.cpp
    src/timeline_editor.cpp
    src/curve_editor.cpp
    src/selection_system.cpp
    src/event_bus.cpp
    src/project_manager.cpp
    src/inspector_binding.cpp
    src/asset_preview.cpp
    src/timeline_playback.cpp
    src/editor_state.cpp
    src/error_reporter.cpp
    src/hotkeys_manager.cpp

    # Qt6 GUI implementation
    src/qt/qt_event_bus.cpp
    src/qt/qt_selection_manager.cpp
    src/qt/nm_style_manager.cpp
    src/qt/nm_main_window.cpp
    src/qt/nm_dock_panel.cpp
    src/qt/nm_welcome_dialog.cpp
    src/qt/nm_undo_manager.cpp
    src/qt/nm_icon_manager.cpp

    # Phase 0 & 1 panels
    src/qt/panels/nm_scene_view_panel.cpp
    src/qt/panels/nm_story_graph_panel.cpp
    src/qt/panels/nm_inspector_panel.cpp
    src/qt/panels/nm_console_panel.cpp
    src/qt/panels/nm_asset_browser_panel.cpp
    src/qt/panels/nm_hierarchy_panel.cpp

    # Phase 3 & 4 panels
    src/qt/panels/nm_timeline_panel.cpp
    src/qt/panels/nm_curve_editor_panel.cpp
    src/qt/panels/nm_voice_manager_panel.cpp
    src/qt/panels/nm_localization_panel.cpp
    src/qt/panels/nm_diagnostics_panel.cpp
    src/qt/panels/nm_build_settings_panel.cpp

    # Phase 5 - Play-In-Editor
    src/qt/nm_play_mode_controller.cpp
    src/qt/panels/nm_play_toolbar_panel.cpp
    src/qt/panels/nm_debug_overlay_panel.cpp

    # Qt6 Headers (required for AUTOMOC to work)
    include/NovelMind/editor/qt/qt_event_bus.hpp
    include/NovelMind/editor/qt/qt_selection_manager.hpp
    include/NovelMind/editor/qt/nm_style_manager.hpp
    include/NovelMind/editor/qt/nm_main_window.hpp
    include/NovelMind/editor/qt/nm_dock_panel.hpp
    include/NovelMind/editor/qt/nm_welcome_dialog.hpp
    include/NovelMind/editor/qt/nm_undo_manager.hpp
    include/NovelMind/editor/qt/nm_icon_manager.hpp
    include/NovelMind/editor/qt/nm_play_mode_controller.hpp

    # Panel headers
    include/NovelMind/editor/qt/panels/nm_scene_view_panel.hpp
    include/NovelMind/editor/qt/panels/nm_story_graph_panel.hpp
    include/NovelMind/editor/qt/panels/nm_inspector_panel.hpp
    include/NovelMind/editor/qt/panels/nm_console_panel.hpp
    include/NovelMind/editor/qt/panels/nm_asset_browser_panel.hpp
    include/NovelMind/editor/qt/panels/nm_hierarchy_panel.hpp
    include/NovelMind/editor/qt/panels/nm_timeline_panel.hpp
    include/NovelMind/editor/qt/panels/nm_curve_editor_panel.hpp
    include/NovelMind/editor/qt/panels/nm_voice_manager_panel.hpp
    include/NovelMind/editor/qt/panels/nm_localization_panel.hpp
    include/NovelMind/editor/qt/panels/nm_diagnostics_panel.hpp
    include/NovelMind/editor/qt/panels/nm_build_settings_panel.hpp
    include/NovelMind/editor/qt/panels/nm_play_toolbar_panel.hpp
    include/NovelMind/editor/qt/panels/nm_debug_overlay_panel.hpp

    # Qt Resources
    resources/editor_resources.qrc
)

# Alias for backward compatibility
add_library(editor ALIAS novelmind_editor)

target_include_directories(novelmind_editor
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(novelmind_editor
    PUBLIC
        engine_core
        Qt6::Widgets
        Qt6::Core
        Qt6::Gui
        Qt6::Svg
    PRIVATE
        novelmind_compiler_options
)

target_compile_definitions(novelmind_editor PRIVATE
    NOVELMIND_QT6_GUI
)

# Editor executable
add_executable(novelmind_editor_app
    src/qt/main.cpp
)

target_link_libraries(novelmind_editor_app
    PRIVATE
        novelmind_editor
        engine_core
        Qt6::Widgets
        Qt6::Core
        Qt6::Gui
        Qt6::Svg
        novelmind_compiler_options
)

# Set output name to novelmind_editor
set_target_properties(novelmind_editor_app PROPERTIES
    OUTPUT_NAME "novelmind_editor"
)

# Copy Qt DLLs on Windows (for development convenience)
if(WIN32)
    # Add Qt bin directory to PATH for running
    get_target_property(_qmake_executable Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)

    # Use windeployqt for deployment
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")
    if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET novelmind_editor_app POST_BUILD
            COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                --no-translations
                --no-system-d3d-compiler
                "$<TARGET_FILE:novelmind_editor_app>"
            COMMENT "Running windeployqt..."
        )
    endif()
endif()
