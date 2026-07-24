#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <cmath>
#include <cstdlib>
#include <map>
#include <thread>
#include <chrono>
#include <random>

struct GameObject3D {
    int id;
    std::string name;
    float x, y, z;
    float r, g, b;
    bool active;
};

class DynaDuck {
private:
    std::map<std::string, std::string> configStore;
    std::vector<GameObject3D> sceneObjects;
    std::vector<std::string> diskList;
    bool webServerRunning = false;
    int nextId = 1;
    std::mt19937 rng{std::random_device{}()};

public:
    float x = 0.0f, y = 0.0f, z = 0.0f;

    std::string ExecuteShell(std::string cmd) {
        std::string result = "";
        FILE* pipe = popen(cmd.c_str(), "r");
        if (!pipe) {
            return "ERROR: Failed to open pipe.";
        }
        char buffer[128];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            result += buffer;
        }
        pclose(pipe);
        return result;
    }

    void GetDriveList() {
        diskList.clear();
#if defined(_WIN32)
        std::string output = ExecuteShell("wmic logicaldisk get deviceid,volumename,size");
#else
        std::string output = ExecuteShell("lsblk -o NAME,SIZE,TYPE | grep disk");
#endif
        diskList.push_back(output);
        std::cout << output;
    }

    bool WriteRawBytes(std::string targetDrive, std::string imagePath) {
        std::ifstream img(imagePath, std::ios::binary);
        if (!img.is_open()) {
            return false;
        }

        std::ofstream target(targetDrive, std::ios::binary);
        if (!target.is_open()) {
            return false;
        }

        char buffer[4096];
        while (img.read(buffer, sizeof(buffer))) {
            target.write(buffer, img.gcout());
        }
        target.write(buffer, img.gcout());
        
        return true;
    }

    std::string GetSystemMemory() {
        std::string memInfo = ExecuteShell("free -h");
        if (memInfo.empty()) memInfo = "Memory query not supported on this host platform.";
        return memInfo;
    }

    std::string GetCPUUsage() {
        std::string cpuInfo = ExecuteShell("top -b -n1 | grep 'Cpu(s)'");
        if (cpuInfo.empty()) cpuInfo = "CPU query active.";
        return cpuInfo;
    }

    std::vector<std::string> ListDirectory(std::string path) {
        std::vector<std::string> files;
        std::string cmd = "ls -p " + path;
        std::string rawList = ExecuteShell(cmd);
        std::stringstream ss(rawList);
        std::string item;
        while (std::getline(ss, item)) {
            if (!item.empty()) files.push_back(item);
        }
        return files;
    }

    bool CheckFileExists(std::string path) {
        std::ifstream file(path);
        return file.good();
    }

    void StartWebServer(int port) {
        webServerRunning = true;
    }

    void LoadUI(std::string htmlFilePath) {
        CheckFileExists(htmlFilePath);
    }

    void SendWebSocketEvent(std::string eventName, std::string data) {
        if (webServerRunning) {
            std::cout << "[WebSocket Event] " << eventName << ": " << data << "\n";
        }
    }

    std::string ReceiveWebAction() {
        return "IDLE";
    }

    void BundleAssets(std::string assetName) {
    }

    void Load2DImage(std::string type, std::string filepath) {
        CheckFileExists(filepath);
    }

    void SetLayerOrder(std::string type, int layerDepth) {
    }

    void DrawSprite(float posX, float posY, float scale, std::string textureName) {
    }

    void Play2DSound(std::string path) {
        CheckFileExists(path);
    }

    void Init3DSpace() {
        sceneObjects.clear();
    }

    int Spawn3DObject(std::string shape, float posX, float posY, float posZ) {
        GameObject3D obj;
        obj.id = nextId++;
        obj.name = shape;
        obj.x = posX;
        obj.y = posY;
        obj.z = posZ;
        obj.r = 255; obj.g = 255; obj.b = 255;
        obj.active = true;
        sceneObjects.push_back(obj);
        return obj.id;
    }

    void Move3DObject(int id, float addX, float addY, float addZ) {
        for (auto& obj : sceneObjects) {
            if (obj.id == id) {
                obj.x += addX;
                obj.y += addY;
                obj.z += addZ;
                return;
            }
        }
    }

    void Color3DObject(int id, int r, int g, int b) {
        for (auto& obj : sceneObjects) {
            if (obj.id == id) {
                obj.r = (float)r; obj.g = (float)g; obj.b = (float)b;
                return;
            }
        }
    }

    bool RaycastClick(float mouseX, float mouseY, float targetX, float targetY, float sizeX, float sizeY) {
        if (mouseX >= targetX && mouseX <= (targetX + sizeX) && mouseY >= targetY && mouseY <= (targetY + sizeY)) {
            return true;
        }
        return false;
    }

    void Attach3DUIBillboard(int id, std::string text) {
    }

    bool CheckAABBCollision(float x1, float y1, float w1, float h1, float x2, float y2, float w2, float h2) {
        return (x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2);
    }

    float CalculateDistance(float x1, float y1, float x2, float y2) {
        return std::sqrt(std::pow(x2 - x1, 2) + std::pow(y2 - y1, 2));
    }

    float Lerp(float start, float end, float alpha) {
        return start + alpha * (end - start);
    }

    void FetchURL(std::string endpoint) {
        std::string cmd = "curl -s " + endpoint;
        ExecuteShell(cmd);
    }

    void SendDiscordWebhook(std::string webhookUrl, std::string message) {
        std::string payload = "{\"content\": \"" + message + "\"}";
        std::string cmd = "curl -H \"Content-Type: application/json\" -d '" + payload + "' " + webhookUrl;
        ExecuteShell(cmd);
    }

    void ReadWriteJSON(std::string filepath, std::string key, std::string value) {
        configStore[key] = value;
        std::ofstream file(filepath);
        if (file.is_open()) {
            file << "{\n";
            for (auto const& [k, val] : configStore) {
                file << "  \"" << k << "\": \"" << val << "\"\n";
            }
            file << "}\n";
        }
    }

    void GenerateChiptuneWave(std::string waveType, int frequency, int durationMs) {
        std::this_thread::sleep_for(std::chrono::milliseconds(durationMs));
    }

    void LogInfo(std::string message) {
        std::cout << "[Log]: " << message << "\n";
    }

    void SleepApp(int milliseconds) {
        std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
    }

    void InitializeEngine() {}
    void ShutdownEngine() {}
    float GetDeltaTime() { return 0.016f; }
    float GetFPS() { return 60.0f; }
    void SetWindowSize(int width, int height) {}
    void SetWindowTitle(std::string title) {}
    void PollEvents() {}
    void ClearScreen(float r, float g, float b) {}
    void PresentFrame() {}
    void LoadFont(std::string filepath, int fontSize) {}
    void DrawText2D(std::string text, float x, float y, float r, float g, float b) {}
    void DrawRect(float x, float y, float w, float h, float r, float g, float b) {}
    void DrawCircle(float x, float y, float radius, float r, float g, float b) {}
    void DrawLine(float x1, float y1, float x2, float y2, float r, float g, float b) {}
    bool IsKeyPressed(int keycode) { return false; }
    bool IsMouseButtonPressed(int button) { return false; }
    float GetMouseX() { return 0.0f; }
    float GetMouseY() { return 0.0f; }
    void LoadAudioFile(std::string filepath) {}
    void StopAudio(std::string filepath) {}
    void SetAudioVolume(float volume) {}
    std::string HttpGet(std::string url) { return ExecuteShell("curl -s " + url); }
    std::string HttpPost(std::string url, std::string payload) { return ExecuteShell("curl -s -X POST -d '" + payload + "' " + url); }
    void ParseJSON(std::string rawJson) {}
    std::string GetJSONValue(std::string key) { return configStore[key]; }
    void SaveTextFile(std::string filepath, std::string content) {
        std::ofstream file(filepath);
        if (file.is_open()) file << content;
    }
    std::string ReadTextFile(std::string filepath) {
        std::ifstream file(filepath);
        std::stringstream buffer;
        if (file.is_open()) buffer << file.rdbuf();
        return buffer.str();
    }
    long long GetTimestamp() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    }
    int RandomInt(int min, int max) {
        std::uniform_int_distribution<int> dist(min, max);
        return dist(rng);
    }
    float RandomFloat(float min, float max) {
        std::uniform_real_distribution<float> dist(min, max);
        return dist(rng);
    }

    void QuickPrint(std::string text) { std::cout << text << "\n"; }
    void PrintError(std::string text) { std::cerr << "[ERROR] " << text << "\n"; }
    bool FileSave(std::string path, std::string data) { SaveTextFile(path, data); return CheckFileExists(path); }
    std::string FileRead(std::string path) { return ReadTextFile(path); }
    void OpenBrowser(std::string url) { ExecuteShell("xdg-open " + url); }
    void CopyToClipboard(std::string text) { ExecuteShell("echo '" + text + "' | xclip -selection clipboard"); }
    int Clamp(int val, int minVal, int maxVal) { return (val < minVal) ? minVal : (val > maxVal ? maxVal : val); }
    float ClampF(float val, float minVal, float maxVal) { return (val < minVal) ? minVal : (val > maxVal ? maxVal : val); }
    void WaitSeconds(int seconds) { std::this_thread::sleep_for(std::chrono::seconds(seconds)); }
};
