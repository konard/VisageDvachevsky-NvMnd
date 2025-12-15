#include "NovelMind/audio/audio_manager.hpp"
#include <algorithm>
#include <cmath>

namespace NovelMind::audio {

// ============================================================================
// AudioSource Implementation
// ============================================================================

AudioSource::AudioSource() = default;
AudioSource::~AudioSource() = default;

void AudioSource::play() {
  if (m_state == PlaybackState::Paused) {
    m_state = PlaybackState::Playing;
  } else if (m_state == PlaybackState::Stopped) {
    m_state = PlaybackState::Playing;
    m_position = 0.0f;
  }
}

void AudioSource::pause() {
  if (m_state == PlaybackState::Playing || m_state == PlaybackState::FadingIn ||
      m_state == PlaybackState::FadingOut) {
    m_state = PlaybackState::Paused;
  }
}

void AudioSource::stop() {
  m_state = PlaybackState::Stopped;
  m_position = 0.0f;
  m_fadeTimer = 0.0f;
}

void AudioSource::update(f64 deltaTime) {
  if (m_state == PlaybackState::Stopped || m_state == PlaybackState::Paused) {
    return;
  }

  // Update fading
  if (m_state == PlaybackState::FadingIn ||
      m_state == PlaybackState::FadingOut) {
    m_fadeTimer += static_cast<f32>(deltaTime);

    if (m_fadeTimer >= m_fadeDuration) {
      m_volume = m_fadeTargetVolume;

      if (m_state == PlaybackState::FadingOut && m_stopAfterFade) {
        stop();
        return;
      } else {
        m_state = PlaybackState::Playing;
      }
    } else {
      f32 t = m_fadeTimer / m_fadeDuration;
      m_volume =
          m_fadeStartVolume + (m_fadeTargetVolume - m_fadeStartVolume) * t;
    }
  }

  // Update playback position
  m_position += static_cast<f32>(deltaTime);

  // Check for loop or end
  if (m_duration > 0.0f && m_position >= m_duration) {
    if (m_loop) {
      m_position = std::fmod(m_position, m_duration);
    } else {
      stop();
    }
  }
}

void AudioSource::setVolume(f32 volume) {
  m_volume = std::max(0.0f, std::min(1.0f, volume));
  m_targetVolume = m_volume;
}

void AudioSource::setPitch(f32 pitch) {
  m_pitch = std::max(0.1f, std::min(4.0f, pitch));
}

void AudioSource::setPan(f32 pan) {
  m_pan = std::max(-1.0f, std::min(1.0f, pan));
}

void AudioSource::setLoop(bool loop) { m_loop = loop; }

void AudioSource::fadeIn(f32 duration) {
  if (duration <= 0.0f) {
    m_volume = m_targetVolume;
    m_state = PlaybackState::Playing;
    return;
  }

  m_fadeStartVolume = 0.0f;
  m_fadeTargetVolume = m_targetVolume;
  m_fadeDuration = duration;
  m_fadeTimer = 0.0f;
  m_stopAfterFade = false;
  m_volume = 0.0f;
  m_state = PlaybackState::FadingIn;
}

void AudioSource::fadeOut(f32 duration, bool stopWhenDone) {
  if (duration <= 0.0f) {
    if (stopWhenDone) {
      stop();
    } else {
      m_volume = 0.0f;
    }
    return;
  }

  m_fadeStartVolume = m_volume;
  m_fadeTargetVolume = 0.0f;
  m_fadeDuration = duration;
  m_fadeTimer = 0.0f;
  m_stopAfterFade = stopWhenDone;
  m_state = PlaybackState::FadingOut;
}

// ============================================================================
// AudioManager Implementation
// ============================================================================

AudioManager::AudioManager() {
  // Initialize default channel volumes
  m_channelVolumes[AudioChannel::Master] = 1.0f;
  m_channelVolumes[AudioChannel::Music] = 0.8f;
  m_channelVolumes[AudioChannel::Sound] = 1.0f;
  m_channelVolumes[AudioChannel::Voice] = 1.0f;
  m_channelVolumes[AudioChannel::Ambient] = 0.7f;
  m_channelVolumes[AudioChannel::UI] = 0.8f;

  for (int i = 0; i < 6; ++i) {
    m_channelMuted[static_cast<AudioChannel>(i)] = false;
  }
}

AudioManager::~AudioManager() { shutdown(); }

Result<void> AudioManager::initialize() {
  if (m_initialized) {
    return Result<void>::ok();
  }

  // Audio backend initialization would go here
  // (SDL_mixer, miniaudio, etc.)

  m_initialized = true;
  return Result<void>::ok();
}

void AudioManager::shutdown() {
  if (!m_initialized) {
    return;
  }

  stopAll(0.0f);
  m_sources.clear();

  // Audio backend shutdown would go here

  m_initialized = false;
}

void AudioManager::update(f64 deltaTime) {
  if (!m_initialized) {
    return;
  }

  // Update master fade
  if (m_masterFadeDuration > 0.0f) {
    m_masterFadeTimer += static_cast<f32>(deltaTime);
    if (m_masterFadeTimer >= m_masterFadeDuration) {
      m_masterFadeVolume = m_masterFadeTarget;
      m_masterFadeDuration = 0.0f;
    } else {
      f32 t = m_masterFadeTimer / m_masterFadeDuration;
      m_masterFadeVolume += (m_masterFadeTarget - m_masterFadeVolume) * t;
    }
  }

  // Update ducking
  updateDucking(deltaTime);

  // Update all sources
  for (auto &source : m_sources) {
    if (source && source->isPlaying()) {
      source->update(deltaTime);
    }
  }

  // Remove stopped sources (but keep some pooled)
  m_sources.erase(std::remove_if(m_sources.begin(), m_sources.end(),
                                 [](const auto &s) {
                                   return s &&
                                          s->getState() ==
                                              PlaybackState::Stopped &&
                                          s->channel != AudioChannel::Music;
                                 }),
                  m_sources.end());

  // Check voice playback status
  if (m_voicePlaying) {
    auto *voiceSource = getSource(m_currentVoiceHandle);
    if (!voiceSource || !voiceSource->isPlaying()) {
      m_voicePlaying = false;
      m_targetDuckLevel = 1.0f;
      fireEvent(AudioEvent::Type::Stopped, m_currentVoiceHandle, "voice");
    }
  }
}

AudioHandle AudioManager::playSound(const std::string &id,
                                    const PlaybackConfig &config) {
  if (!m_initialized) {
    return {};
  }

  // Check source limit
  if (m_sources.size() >= m_maxSounds) {
    // Find and remove lowest priority sound
    auto lowest = std::min_element(
        m_sources.begin(), m_sources.end(),
        [](const auto &a, const auto &b) { return a->priority < b->priority; });

    if (lowest != m_sources.end() && (*lowest)->priority < config.priority) {
      m_sources.erase(lowest);
    } else {
      return {}; // Can't play
    }
  }

  AudioHandle handle = createSource(id, config.channel);
  auto *source = getSource(handle);
  if (!source) {
    return {};
  }

  source->setVolume(config.volume);
  source->setPitch(config.pitch);
  source->setPan(config.pan);
  source->setLoop(config.loop);
  source->priority = config.priority;

  if (config.fadeInDuration > 0.0f) {
    source->fadeIn(config.fadeInDuration);
  } else {
    source->play();
  }

  fireEvent(AudioEvent::Type::Started, handle, id);
  return handle;
}

AudioHandle AudioManager::playSound(const std::string &id, f32 volume,
                                    bool loop) {
  PlaybackConfig config;
  config.volume = volume;
  config.loop = loop;
  return playSound(id, config);
}

void AudioManager::stopSound(AudioHandle handle, f32 fadeDuration) {
  auto *source = getSource(handle);
  if (source) {
    if (fadeDuration > 0.0f) {
      source->fadeOut(fadeDuration, true);
    } else {
      source->stop();
      fireEvent(AudioEvent::Type::Stopped, handle, source->trackId);
    }
  }
}

void AudioManager::stopAllSounds(f32 fadeDuration) {
  for (auto &source : m_sources) {
    if (source && source->channel == AudioChannel::Sound) {
      if (fadeDuration > 0.0f) {
        source->fadeOut(fadeDuration, true);
      } else {
        source->stop();
      }
    }
  }
}

AudioHandle AudioManager::playMusic(const std::string &id,
                                    const MusicConfig &config) {
  if (!m_initialized) {
    return {};
  }

  // Stop current music
  if (m_currentMusicHandle.isValid()) {
    stopMusic(0.0f);
  }

  AudioHandle handle = createSource(id, AudioChannel::Music);
  auto *source = getSource(handle);
  if (!source) {
    return {};
  }

  source->setVolume(config.volume);
  source->setLoop(config.loop);

  if (config.fadeInDuration > 0.0f) {
    source->fadeIn(config.fadeInDuration);
  } else {
    source->play();
  }

  m_currentMusicHandle = handle;
  m_currentMusicId = id;

  fireEvent(AudioEvent::Type::Started, handle, id);
  return handle;
}

AudioHandle AudioManager::crossfadeMusic(const std::string &id, f32 duration,
                                         const MusicConfig &config) {
  if (!m_initialized) {
    return {};
  }

  // Fade out current music
  if (m_currentMusicHandle.isValid()) {
    auto *current = getSource(m_currentMusicHandle);
    if (current) {
      current->fadeOut(duration, true);
    }
  }

  // Start new music with fade in
  MusicConfig newConfig = config;
  newConfig.fadeInDuration = duration;
  return playMusic(id, newConfig);
}

void AudioManager::stopMusic(f32 fadeDuration) {
  if (!m_currentMusicHandle.isValid()) {
    return;
  }

  auto *source = getSource(m_currentMusicHandle);
  if (source) {
    if (fadeDuration > 0.0f) {
      source->fadeOut(fadeDuration, true);
    } else {
      source->stop();
      fireEvent(AudioEvent::Type::Stopped, m_currentMusicHandle,
                m_currentMusicId);
    }
  }

  m_currentMusicHandle.invalidate();
  m_currentMusicId.clear();
}

void AudioManager::pauseMusic() {
  auto *source = getSource(m_currentMusicHandle);
  if (source) {
    source->pause();
    fireEvent(AudioEvent::Type::Paused, m_currentMusicHandle, m_currentMusicId);
  }
}

void AudioManager::resumeMusic() {
  auto *source = getSource(m_currentMusicHandle);
  if (source) {
    source->play();
    fireEvent(AudioEvent::Type::Resumed, m_currentMusicHandle,
              m_currentMusicId);
  }
}

bool AudioManager::isMusicPlaying() const {
  if (!m_currentMusicHandle.isValid()) {
    return false;
  }

  for (const auto &source : m_sources) {
    if (source && source->handle.id == m_currentMusicHandle.id) {
      return source->isPlaying();
    }
  }
  return false;
}

const std::string &AudioManager::getCurrentMusicId() const {
  return m_currentMusicId;
}

f32 AudioManager::getMusicPosition() const {
  for (const auto &source : m_sources) {
    if (source && source->handle.id == m_currentMusicHandle.id) {
      return source->getPlaybackPosition();
    }
  }
  return 0.0f;
}

void AudioManager::seekMusic(f32 /*position*/) {
  // Implementation depends on audio backend
}

AudioHandle AudioManager::playVoice(const std::string &id,
                                    const VoiceConfig &config) {
  if (!m_initialized) {
    return {};
  }

  // Stop current voice
  if (m_currentVoiceHandle.isValid()) {
    stopVoice(0.0f);
  }

  AudioHandle handle = createSource(id, AudioChannel::Voice);
  auto *source = getSource(handle);
  if (!source) {
    return {};
  }

  source->setVolume(config.volume);
  source->setLoop(false);
  source->play();

  m_currentVoiceHandle = handle;
  m_voicePlaying = true;

  // Apply ducking
  if (config.duckMusic && m_autoDuckingEnabled) {
    m_duckVolume = config.duckAmount;
    m_duckFadeDuration = config.duckFadeDuration;
    m_targetDuckLevel = m_duckVolume;
  }

  fireEvent(AudioEvent::Type::Started, handle, id);
  return handle;
}

void AudioManager::stopVoice(f32 fadeDuration) {
  if (!m_currentVoiceHandle.isValid()) {
    return;
  }

  auto *source = getSource(m_currentVoiceHandle);
  if (source) {
    if (fadeDuration > 0.0f) {
      source->fadeOut(fadeDuration, true);
    } else {
      source->stop();
    }
  }

  m_voicePlaying = false;
  m_targetDuckLevel = 1.0f;
  m_currentVoiceHandle.invalidate();
}

bool AudioManager::isVoicePlaying() const { return m_voicePlaying; }

void AudioManager::skipVoice() { stopVoice(0.0f); }

void AudioManager::setChannelVolume(AudioChannel channel, f32 volume) {
  m_channelVolumes[channel] = std::max(0.0f, std::min(1.0f, volume));
}

f32 AudioManager::getChannelVolume(AudioChannel channel) const {
  auto it = m_channelVolumes.find(channel);
  return (it != m_channelVolumes.end()) ? it->second : 1.0f;
}

void AudioManager::setMasterVolume(f32 volume) {
  setChannelVolume(AudioChannel::Master, volume);
}

f32 AudioManager::getMasterVolume() const {
  return getChannelVolume(AudioChannel::Master);
}

void AudioManager::setChannelMuted(AudioChannel channel, bool muted) {
  m_channelMuted[channel] = muted;
}

bool AudioManager::isChannelMuted(AudioChannel channel) const {
  auto it = m_channelMuted.find(channel);
  return (it != m_channelMuted.end()) ? it->second : false;
}

void AudioManager::muteAll() { m_allMuted = true; }

void AudioManager::unmuteAll() { m_allMuted = false; }

void AudioManager::fadeAllTo(f32 targetVolume, f32 duration) {
  m_masterFadeTarget = std::max(0.0f, std::min(1.0f, targetVolume));
  m_masterFadeDuration = duration;
  m_masterFadeTimer = 0.0f;
}

void AudioManager::pauseAll() {
  for (auto &source : m_sources) {
    if (source && source->isPlaying()) {
      source->pause();
    }
  }
}

void AudioManager::resumeAll() {
  for (auto &source : m_sources) {
    if (source && source->getState() == PlaybackState::Paused) {
      source->play();
    }
  }
}

void AudioManager::stopAll(f32 fadeDuration) {
  for (auto &source : m_sources) {
    if (source && source->isPlaying()) {
      if (fadeDuration > 0.0f) {
        source->fadeOut(fadeDuration, true);
      } else {
        source->stop();
      }
    }
  }

  m_currentMusicHandle.invalidate();
  m_currentMusicId.clear();
  m_currentVoiceHandle.invalidate();
  m_voicePlaying = false;
}

AudioSource *AudioManager::getSource(AudioHandle handle) {
  if (!handle.isValid()) {
    return nullptr;
  }

  for (auto &source : m_sources) {
    if (source && source->handle.id == handle.id) {
      return source.get();
    }
  }
  return nullptr;
}

bool AudioManager::isPlaying(AudioHandle handle) const {
  if (!handle.isValid()) {
    return false;
  }

  for (const auto &source : m_sources) {
    if (source && source->handle.id == handle.id) {
      return source->isPlaying();
    }
  }
  return false;
}

std::vector<AudioHandle> AudioManager::getActiveSources() const {
  std::vector<AudioHandle> handles;
  for (const auto &source : m_sources) {
    if (source && source->isPlaying()) {
      handles.push_back(source->handle);
    }
  }
  return handles;
}

size_t AudioManager::getActiveSourceCount() const {
  return static_cast<size_t>(
      std::count_if(m_sources.begin(), m_sources.end(),
                    [](const auto &s) { return s && s->isPlaying(); }));
}

void AudioManager::setEventCallback(AudioCallback callback) {
  m_eventCallback = std::move(callback);
}

void AudioManager::setMaxSounds(size_t max) { m_maxSounds = max; }

void AudioManager::setAutoDuckingEnabled(bool enabled) {
  m_autoDuckingEnabled = enabled;
  if (!enabled) {
    m_targetDuckLevel = 1.0f;
  }
}

void AudioManager::setDuckingParams(f32 duckVolume, f32 fadeDuration) {
  m_duckVolume = std::max(0.0f, std::min(1.0f, duckVolume));
  m_duckFadeDuration = std::max(0.0f, fadeDuration);
}

AudioHandle AudioManager::createSource(const std::string &trackId,
                                       AudioChannel channel) {
  auto source = std::make_unique<AudioSource>();

  AudioHandle handle;
  handle.id = m_nextHandleId++;
  handle.valid = true;

  source->handle = handle;
  source->trackId = trackId;
  source->channel = channel;

  m_sources.push_back(std::move(source));
  return handle;
}

void AudioManager::releaseSource(AudioHandle handle) {
  m_sources.erase(std::remove_if(m_sources.begin(), m_sources.end(),
                                 [&handle](const auto &s) {
                                   return s && s->handle.id == handle.id;
                                 }),
                  m_sources.end());
}

void AudioManager::fireEvent(AudioEvent::Type type, AudioHandle handle,
                             const std::string &trackId) {
  if (m_eventCallback) {
    AudioEvent event;
    event.type = type;
    event.handle = handle;
    event.trackId = trackId;
    m_eventCallback(event);
  }
}

void AudioManager::updateDucking(f64 deltaTime) {
  if (std::abs(m_currentDuckLevel - m_targetDuckLevel) < 0.001f) {
    m_currentDuckLevel = m_targetDuckLevel;
    return;
  }

  f32 speed = m_duckFadeDuration > 0.0f ? 1.0f / m_duckFadeDuration : 10.0f;
  if (m_currentDuckLevel < m_targetDuckLevel) {
    m_currentDuckLevel =
        std::min(m_targetDuckLevel,
                 m_currentDuckLevel + static_cast<f32>(deltaTime) * speed);
  } else {
    m_currentDuckLevel =
        std::max(m_targetDuckLevel,
                 m_currentDuckLevel - static_cast<f32>(deltaTime) * speed);
  }
}

f32 AudioManager::calculateEffectiveVolume(const AudioSource &source) const {
  if (m_allMuted || isChannelMuted(source.channel)) {
    return 0.0f;
  }

  f32 volume = getChannelVolume(AudioChannel::Master);
  volume *= getChannelVolume(source.channel);
  volume *= m_masterFadeVolume;

  // Apply ducking to music
  if (source.channel == AudioChannel::Music) {
    volume *= m_currentDuckLevel;
  }

  return volume;
}

} // namespace NovelMind::audio
