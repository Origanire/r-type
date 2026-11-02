#include "SfmlAudio.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <utility>
#include <vector>

#include <SFML/Audio/InputSoundFile.hpp>

#include "AudioError.hpp"

namespace engine::audio {

namespace {

constexpr std::size_t DECODE_CHUNK_SAMPLES = 4096;

} // namespace

void SfmlAudio::loadSound(const std::string &name, const std::string &path)
{
    sf::InputSoundFile input;
    if (!input.openFromFile(path))
        throw AudioError("SfmlAudio: cannot open sound " + path);
    std::vector<std::int16_t> samples;
    std::array<std::int16_t, DECODE_CHUNK_SAMPLES> chunk{};
    for (std::uint64_t read = input.read(chunk.data(), chunk.size()); read > 0;
        read = input.read(chunk.data(), chunk.size()))
        samples.insert(samples.end(), chunk.begin(), chunk.begin() + static_cast<std::ptrdiff_t>(read));
    sf::SoundBuffer buffer;
    if (samples.empty()
        || !buffer.loadFromSamples(samples.data(), samples.size(), input.getChannelCount(), input.getSampleRate(),
            input.getChannelMap()))
        throw AudioError("SfmlAudio: cannot decode sound " + path);
    m_buffers.insert_or_assign(name, std::move(buffer));
}

void SfmlAudio::loadMusic(const std::string &name, const std::string &path)
{
    sf::Music probe;
    if (!probe.openFromFile(path))
        throw AudioError("SfmlAudio: cannot open music " + path);
    m_musicPaths.insert_or_assign(name, path);
}

void SfmlAudio::playSound(const std::string &name)
{
    const auto found = m_buffers.find(name);
    if (found == m_buffers.end())
        throw AudioError("SfmlAudio: unknown sound " + name);
    m_playing.remove_if([](const sf::Sound &sound) { return sound.getStatus() == sf::Sound::Status::Stopped; });
    if (m_playing.size() >= MAX_PLAYING_SOUNDS)
        return;
    sf::Sound &sound = m_playing.emplace_back(found->second);
    sound.setVolume(m_soundVolume);
    sound.play();
}

void SfmlAudio::playMusic(const std::string &name)
{
    const auto found = m_musicPaths.find(name);
    if (found == m_musicPaths.end())
        throw AudioError("SfmlAudio: unknown music " + name);
    auto music = std::make_unique<sf::Music>();
    if (!music->openFromFile(found->second))
        throw AudioError("SfmlAudio: cannot open music " + found->second);
    music->setLooping(true);
    music->setVolume(m_musicVolume);
    music->play();
    m_music = std::move(music);
}

void SfmlAudio::stopMusic()
{
    if (m_music)
        m_music->stop();
}

void SfmlAudio::setSoundVolume(float volume)
{
    m_soundVolume = std::clamp(volume, 0.f, MAX_VOLUME);
}

void SfmlAudio::setMusicVolume(float volume)
{
    m_musicVolume = std::clamp(volume, 0.f, MAX_VOLUME);
    if (m_music)
        m_music->setVolume(m_musicVolume);
}

} // namespace engine::audio
