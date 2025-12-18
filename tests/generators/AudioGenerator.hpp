#pragma once

#include <cmath>
#include <cstdint>
#include <fstream>
#include <vector>

class AudioGenerator {
   public:
    static std::vector<int16_t> generateSineWave(float frequency = 440.0f, float duration = 1.0f,
                                                 int sampleRate = 4400) {
        std::vector<int16_t> samples;
        int numSamples = static_cast<int>(duration * sampleRate);
        samples.reserve(numSamples);

        float amplitude = 0.5f * 32767.0f;
        float angularFreq = 2.0f * M_PI * frequency / sampleRate;

        for (int i = 0; i < numSamples; ++i) {
            float sample = amplitude * std::sin(angularFreq * i);
            samples.push_back(static_cast<int16_t>(sample));
        }

        return samples;
    }

    static std::vector<int16_t> generateSilence(float duration = 1.0f, int sampleRate = 44100) {
        int numSamples = static_cast<int>(duration * sampleRate);
        return std::vector<int16_t>(numSamples, 0);
    }

    static std::vector<int16_t> generateWhiteNoise(float duration = 1.0f, int sampleRate = 44100) {
        std::vector<int16_t> samples;
        int numSamples = static_cast<int>(duration * sampleRate);
        samples.reserve(numSamples);

        for (int i = 0; i < numSamples; ++i) {
            int16_t sample = static_cast<int16_t>(rand() % 65535 - 32767);
            samples.push_back(sample);
        }

        return samples;
    }

    static bool saveAsWAV(const std::vector<int16_t>& samples, const std::string& filename,
                          int sampleRate = 44100, int numChannels = 1) {
        std::ofstream file(filename, std::ios::binary);
        if (!file)
            return false;

        // Header WAV
        struct WAVHeader {
            char riff[4] = {'R', 'I', 'F', 'F'};
            uint32_t chunkSize;
            char wave[4] = {'W', 'A', 'V', 'E'};
            char fmt[4] = {'f', 'm', 't', ' '};
            uint32_t subchunk1Size = 16;
            uint16_t audioFormat = 1;  // PCM
            uint16_t numChannels;
            uint32_t sampleRate;
            uint32_t byteRate;
            uint16_t blockAlign;
            uint16_t bitsPerSample = 16;
            char data[4] = {'d', 'a', 't', 'a'};
            uint32_t dataSize;
        };

        WAVHeader header;
        header.numChannels = numChannels;
        header.sampleRate = sampleRate;
        header.byteRate = sampleRate * numChannels * sizeof(int16_t);
        header.blockAlign = numChannels * sizeof(int16_t);
        header.dataSize = samples.size() * sizeof(int16_t);
        header.chunkSize = 36 + header.dataSize;

        file.write(reinterpret_cast<const char*>(&header), sizeof(header));
        file.write(reinterpret_cast<const char*>(samples.data()), samples.size() * sizeof(int16_t));

        return true;
    }
};
