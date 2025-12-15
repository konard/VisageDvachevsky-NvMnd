#include "NovelMind/core/application.hpp"
#include "NovelMind/core/logger.hpp"

namespace NovelMind::core {

Application::Application() : m_running(false) {}

Application::~Application() { shutdown(); }

Result<void> Application::initialize(const EngineConfig &config) {
  m_config = config;

  if (m_config.debug) {
    Logger::instance().setLevel(LogLevel::Debug);
  }

  NOVELMIND_LOG_INFO("Initializing NovelMind engine...");

  m_window = platform::createWindow();
  auto windowResult = m_window->create(m_config.window);
  if (windowResult.isError()) {
    NOVELMIND_LOG_ERROR("Failed to create window");
    return windowResult;
  }

  m_timer.reset();
  m_running = true;

  onInitialize();

  NOVELMIND_LOG_INFO("Engine initialized successfully");
  return Result<void>::ok();
}

void Application::shutdown() {
  if (!m_running) {
    return;
  }

  NOVELMIND_LOG_INFO("Shutting down engine...");

  onShutdown();

  if (m_window) {
    m_window->destroy();
    m_window.reset();
  }

  m_running = false;

  NOVELMIND_LOG_INFO("Engine shutdown complete");
}

void Application::run() {
  if (!m_running) {
    NOVELMIND_LOG_ERROR("Cannot run: engine not initialized");
    return;
  }

  NOVELMIND_LOG_INFO("Starting main loop...");
  mainLoop();
}

void Application::quit() { m_running = false; }

bool Application::isRunning() const { return m_running; }

f64 Application::getDeltaTime() const { return m_timer.getDeltaTime(); }

f64 Application::getElapsedTime() const { return m_timer.getElapsedSeconds(); }

platform::IWindow *Application::getWindow() { return m_window.get(); }

const platform::IWindow *Application::getWindow() const {
  return m_window.get();
}

void Application::onInitialize() {
  // Override in derived class
}

void Application::onShutdown() {
  // Override in derived class
}

void Application::onUpdate(f64 /*deltaTime*/) {
  // Override in derived class
}

void Application::onRender() {
  // Override in derived class
}

void Application::mainLoop() {
  while (m_running && !m_window->shouldClose()) {
    m_timer.tick();
    f64 deltaTime = m_timer.getDeltaTime();

    m_window->pollEvents();

    onUpdate(deltaTime);
    onRender();

    m_window->swapBuffers();
  }
}

} // namespace NovelMind::core
