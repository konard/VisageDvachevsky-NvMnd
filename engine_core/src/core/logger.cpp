#include "NovelMind/core/logger.hpp"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace NovelMind::core {

Logger &Logger::instance() {
  static Logger instance;
  return instance;
}

Logger::Logger() : m_level(LogLevel::Info), m_useColors(true) {}

Logger::~Logger() { closeOutputFile(); }

void Logger::setLevel(LogLevel level) {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_level = level;
}

LogLevel Logger::getLevel() const { return m_level; }

void Logger::setOutputFile(const std::string &path) {
  std::lock_guard<std::mutex> lock(m_mutex);
  closeOutputFile();
  m_fileStream.open(path, std::ios::out | std::ios::app);
}

void Logger::closeOutputFile() {
  if (m_fileStream.is_open()) {
    m_fileStream.close();
  }
}

void Logger::log(LogLevel level, std::string_view message) {
  if (level < m_level || m_level == LogLevel::Off) {
    return;
  }

  std::lock_guard<std::mutex> lock(m_mutex);

  std::string timestamp = getCurrentTimestamp();
  const char *levelStr = levelToString(level);

  std::ostringstream oss;
  oss << "[" << timestamp << "] [" << levelStr << "] " << message;
  std::string logLine = oss.str();

  if (m_fileStream.is_open()) {
    m_fileStream << logLine << std::endl;
  }

  std::ostream &out = (level >= LogLevel::Warning) ? std::cerr : std::cout;

  if (m_useColors) {
    const char *colorCode = "";
    const char *resetCode = "\033[0m";

    switch (level) {
    case LogLevel::Trace:
      colorCode = "\033[90m";
      break;
    case LogLevel::Debug:
      colorCode = "\033[36m";
      break;
    case LogLevel::Info:
      colorCode = "\033[32m";
      break;
    case LogLevel::Warning:
      colorCode = "\033[33m";
      break;
    case LogLevel::Error:
      colorCode = "\033[31m";
      break;
    case LogLevel::Fatal:
      colorCode = "\033[35m";
      break;
    default:
      break;
    }

    out << colorCode << logLine << resetCode << std::endl;
  } else {
    out << logLine << std::endl;
  }
}

void Logger::trace(std::string_view message) { log(LogLevel::Trace, message); }

void Logger::debug(std::string_view message) { log(LogLevel::Debug, message); }

void Logger::info(std::string_view message) { log(LogLevel::Info, message); }

void Logger::warning(std::string_view message) {
  log(LogLevel::Warning, message);
}

void Logger::error(std::string_view message) { log(LogLevel::Error, message); }

void Logger::fatal(std::string_view message) { log(LogLevel::Fatal, message); }

const char *Logger::levelToString(LogLevel level) const {
  switch (level) {
  case LogLevel::Trace:
    return "TRACE";
  case LogLevel::Debug:
    return "DEBUG";
  case LogLevel::Info:
    return "INFO ";
  case LogLevel::Warning:
    return "WARN ";
  case LogLevel::Error:
    return "ERROR";
  case LogLevel::Fatal:
    return "FATAL";
  default:
    return "?????";
  }
}

std::string Logger::getCurrentTimestamp() const {
  auto now = std::chrono::system_clock::now();
  auto time = std::chrono::system_clock::to_time_t(now);
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()) %
            1000;

  std::ostringstream oss;
  oss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
  oss << '.' << std::setfill('0') << std::setw(3) << ms.count();
  return oss.str();
}

} // namespace NovelMind::core
