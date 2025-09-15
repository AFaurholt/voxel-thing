#pragma once

#include <vector>
#include <string>
#include <gif.h>

class GifRecorder {
private:
    int m_width;
    int m_height;
    int m_fps;
    bool m_recording;
    std::vector<std::vector<uint8_t>> m_frames;
public:
    GifRecorder(int width, int height, int fps)
        : m_width(width), m_height(height), m_fps(fps), m_recording(false) {}

    void startRecording() {
        m_recording = true;
        m_frames.clear();
    }

    void stopRecording(const std::string& filename) {
        if (!m_recording) return;
        
        std::cout << "Creating writer...  " << std::endl;

        GifWriter writer;
        GifBegin(&writer, filename.c_str(), m_width, m_height, 100 / m_fps);
        
        std::cout << "Writing frames...  " << std::endl;

        size_t frameCount = 0;
        for (const auto& frame : m_frames) {
            std::cout << "Frame: " << frameCount << std::endl;
            frameCount++;
            GifWriteFrame(&writer, frame.data(), m_width, m_height, 100 / m_fps);
        }
        
        std::cout << "Frames written...  " << std::endl;


        GifEnd(&writer);
        m_frames.clear();
        m_recording = false;
    }

    void captureFrame() {
        if (!m_recording) return;

        std::vector<uint8_t> pixels(m_width * m_height * 4);
        glReadPixels(0, 0, m_width, m_height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
        // Convert RGBA to RGB and flip vertically
        std::vector<uint8_t> frame(m_width * m_height * 4);
        for (int y = 0; y < m_height; y++) {
            for (int x = 0; x < m_width; x++) {
                int srcIdx = (m_height - 1 - y) * m_width * 4 + x * 4;
                int dstIdx = y * m_width * 4 + x * 4;
                frame[dstIdx] = pixels[srcIdx];     // R
                frame[dstIdx + 1] = pixels[srcIdx + 1]; // G
                frame[dstIdx + 2] = pixels[srcIdx + 2]; // B
            }
        }

        m_frames.push_back(frame);
        std::cout << "Frames recorded:  " << m_frames.size() << std::endl;

    }

    const bool isRecording() { return m_recording; }
};