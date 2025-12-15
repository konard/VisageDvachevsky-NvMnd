/**
 * @file main.cpp
 * @brief NovelMind Editor Qt6 Application Entry Point
 *
 * This is the main entry point for the NovelMind Visual Novel Editor.
 * The editor is built with Qt 6 Widgets and provides:
 * - Visual scene editing with WYSIWYG preview
 * - Node-based story graph editor
 * - Asset management and import pipeline
 * - Project build and export system
 *
 * Version: 0.3.0
 */

#include "NovelMind/core/logger.hpp"
#include "NovelMind/editor/qt/nm_main_window.hpp"
#include "NovelMind/editor/qt/nm_style_manager.hpp"
#include "NovelMind/editor/qt/nm_welcome_dialog.hpp"

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QSettings>
#include <iostream>

using namespace NovelMind;
using namespace NovelMind::editor::qt;

/**
 * @brief Print application version and build info
 */
void printVersion() {
  std::cout << "NovelMind Editor v0.3.0\n";
  std::cout << "Built with Qt " << QT_VERSION_STR << " and C++20\n";
  std::cout << "Copyright (c) 2024 NovelMind Contributors\n";
  std::cout << "Licensed under MIT License\n";
}

/**
 * @brief Main entry point
 */
int main(int argc, char *argv[]) {
  // Enable High-DPI scaling
  QApplication::setHighDpiScaleFactorRoundingPolicy(
      Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

  // Create the Qt application
  QApplication app(argc, argv);
  app.setApplicationName("NovelMind Editor");
  app.setApplicationVersion("0.3.0");
  app.setOrganizationName("NovelMind");
  app.setOrganizationDomain("novelmind.io");

  // Set up command line parser
  QCommandLineParser parser;
  parser.setApplicationDescription("NovelMind Visual Novel Editor");
  parser.addHelpOption();
  parser.addVersionOption();

  QCommandLineOption newProjectOption(QStringList() << "n" << "new",
                                      "Create a new project at <path>", "path");
  parser.addOption(newProjectOption);

  QCommandLineOption openProjectOption(QStringList() << "o" << "open",
                                       "Open an existing project", "path");
  parser.addOption(openProjectOption);

  QCommandLineOption noWelcomeOption("no-welcome", "Skip the welcome screen");
  parser.addOption(noWelcomeOption);

  QCommandLineOption resetLayoutOption("reset-layout",
                                       "Reset panel layout to defaults");
  parser.addOption(resetLayoutOption);

  QCommandLineOption scaleOption("scale", "Set UI scale factor (0.5-3.0)",
                                 "factor", "1.0");
  parser.addOption(scaleOption);

  // Add positional argument for project file
  parser.addPositionalArgument("project", "Project file to open");

  parser.process(app);

  // Initialize Qt resources from static library
  Q_INIT_RESOURCE(editor_resources);

  // Initialize logger
  core::Logger::instance().setLevel(core::LogLevel::Info);
  core::Logger::instance().info("NovelMind Editor starting...");

  // Initialize style manager and apply dark theme
  NMStyleManager &styleManager = NMStyleManager::instance();
  styleManager.initialize(&app);

  // Apply scale factor if specified
  if (parser.isSet(scaleOption)) {
    bool ok;
    double scale = parser.value(scaleOption).toDouble(&ok);
    if (ok && scale >= 0.5 && scale <= 3.0) {
      styleManager.setUiScale(scale);
    }
  }

  // Create main window
  NMMainWindow mainWindow;

  if (!mainWindow.initialize()) {
    core::Logger::instance().error("Failed to initialize main window");
    return 1;
  }

  // Reset layout if requested
  if (parser.isSet(resetLayoutOption)) {
    mainWindow.resetToDefaultLayout();
  }

  // Handle project opening
  QString projectPath;
  bool skipWelcome = parser.isSet(noWelcomeOption);

  if (parser.isSet(openProjectOption)) {
    projectPath = parser.value(openProjectOption);
    skipWelcome = true;
  } else if (!parser.positionalArguments().isEmpty()) {
    projectPath = parser.positionalArguments().first();
    skipWelcome = true;
  }

  // Show welcome dialog if needed
  if (!skipWelcome) {
    QSettings settings("NovelMind", "Editor");
    bool skipWelcomeInFuture =
        settings.value("skipWelcomeScreen", false).toBool();

    if (!skipWelcomeInFuture) {
      NMWelcomeDialog welcomeDialog;
      if (welcomeDialog.exec() == QDialog::Accepted) {
        if (welcomeDialog.shouldCreateNewProject()) {
          core::Logger::instance().info(
              "User requested new project with template: " +
              welcomeDialog.selectedTemplate().toStdString());
          // TODO: Open new project dialog
        } else if (!welcomeDialog.selectedProjectPath().isEmpty()) {
          projectPath = welcomeDialog.selectedProjectPath();
        }

        if (welcomeDialog.shouldSkipInFuture()) {
          settings.setValue("skipWelcomeScreen", true);
        }
      } else {
        // User closed welcome dialog - exit application
        core::Logger::instance().info("User closed welcome dialog");
        return 0;
      }
    }
  }

  if (!projectPath.isEmpty()) {
    mainWindow.updateWindowTitle(projectPath);
    // TODO: Actually open the project
  }

  // Show the main window
  mainWindow.show();

  core::Logger::instance().info("Editor initialized successfully");

  // Run the application event loop
  int result = app.exec();

  // Shutdown
  mainWindow.shutdown();
  core::Logger::instance().info("NovelMind Editor shut down cleanly");

  return result;
}
