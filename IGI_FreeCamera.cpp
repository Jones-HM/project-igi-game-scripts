/*[[Project I.G.I FreeCamera. Enables Free Roam Camera in IGI must be executed in CheatEngine Lua Script.
Controls:
X-Axis = Right/Left
Y-Axis = Up/Down
Z-Axis = Space/Alt
Angle/Roation = Mouse.
Recalibrate View = Backspace.
Start/Exit key = 'END'
Written by IGI-ResearchDevs - HM.--]]*/

#include "GTLibc.hpp"
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>

using namespace GTLIBC;

#define GAME_PROCESS "igi"
#define CAMERA_COPY_OFFSET  0x00097E82   // IGI.exe + 97E82

#define VIEWPORT_X  0x00BCAB08
#define VIEWPORT_Y  (VIEWPORT_X + 8)
#define VIEWPORT_Z  (VIEWPORT_X + 16)

#define PLAYER_BASE_OFFSET 0x0016E210

class IGIFreeCamera
{
public:
    IGIFreeCamera() : gameLib(true) {}

    bool Initialize()
    {
        std::cout << "[INFO] Attaching to IGI...\n";

        if (!gameLib.FindGameProcess(GAME_PROCESS))
        {
            std::cout << "[ERROR] Game process not found.\n";
            return false;
        }

        gameBaseAddress = gameLib.GetGameBaseAddress();
        std::cout << "[INFO] Base Address: 0x"
                  << std::hex << gameBaseAddress << std::dec << "\n";

        PatchCameraCopy();
        EnableThirdPerson();

        return true;
    }

    void Run()
    {
        std::cout << "[INFO] FreeCam running. Press HOME to exit.\n";

        while (!gameLib.IsKeyPressed(VK_HOME))
        {
            UpdateCamera();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        Restore();
        std::cout << "[INFO] FreeCam exited cleanly.\n";
    }

private:
    GTLibc gameLib;
    DWORD gameBaseAddress{};
    std::vector<BYTE> originalBytes;
    const double movementOffset = 1000.0;

private:

    // ===============================
    // Patch mov ecx,42 → mov ecx,9
    // ===============================
    void PatchCameraCopy()
    {
        DWORD patchAddress = gameBaseAddress + CAMERA_COPY_OFFSET;

        originalBytes.clear();

        std::cout << "[DEBUG] Original Bytes: ";
        for (int i = 0; i < 5; ++i)
        {
            BYTE byte = gameLib.ReadAddress<BYTE>(patchAddress + i);
            originalBytes.push_back(byte);
            std::cout << std::hex << (int)byte << " ";
        }
        std::cout << std::dec << "\n";

        // Expected original: B9 2A 00 00 00

        BYTE patch[5] = { 0xB9, 0x09, 0x00, 0x00, 0x00 };

        for (int i = 0; i < 5; ++i)
        {
            gameLib.WriteAddress<BYTE>(patchAddress + i, patch[i]);
        }

        std::cout << "[INFO] Camera copy patched (ECX=9).\n";
    }

    void RestoreCameraCopy()
    {
        DWORD patchAddress = gameBaseAddress + CAMERA_COPY_OFFSET;

        if (originalBytes.size() != 5)
            return;

        for (int i = 0; i < 5; ++i)
        {
            gameLib.WriteAddress<BYTE>(patchAddress + i, originalBytes[i]);
        }

        std::cout << "[INFO] Camera copy restored.\n";
    }

    // ===============================
    // Player Base
    // ===============================
    DWORD GetPlayerBase()
    {
        DWORD basePtr =
            gameLib.ReadAddress<DWORD>(gameBaseAddress + PLAYER_BASE_OFFSET);

        if (!basePtr)
            return 0;

        return gameLib.ReadPointerOffsets<DWORD>(basePtr, { 0x8, 0x7CC, 0x14 });
    }

    void EnableThirdPerson()
    {
        DWORD playerBase = GetPlayerBase();
        if (!playerBase) return;

        gameLib.WriteAddress<int>(playerBase + 0x4F0, 3);
    }

    void DisableThirdPerson()
    {
        DWORD playerBase = GetPlayerBase();
        if (!playerBase) return;

        gameLib.WriteAddress<int>(playerBase + 0x4F0, 1);
    }

    // ===============================
    // Movement (Same as Lua)
    // ===============================
    void UpdateCamera()
    {
        double x = gameLib.ReadAddress<double>(VIEWPORT_X);
        double y = gameLib.ReadAddress<double>(VIEWPORT_Y);
        double z = gameLib.ReadAddress<double>(VIEWPORT_Z);

        if (gameLib.IsKeyPressed(VK_RIGHT))
            gameLib.WriteAddress<double>(VIEWPORT_X, x + movementOffset);

        if (gameLib.IsKeyPressed(VK_LEFT))
            gameLib.WriteAddress<double>(VIEWPORT_X, x - movementOffset);

        if (gameLib.IsKeyPressed(VK_UP))
            gameLib.WriteAddress<double>(VIEWPORT_Y, y + movementOffset);

        if (gameLib.IsKeyPressed(VK_DOWN))
            gameLib.WriteAddress<double>(VIEWPORT_Y, y - movementOffset);

        if (gameLib.IsKeyPressed(VK_SPACE))
            gameLib.WriteAddress<double>(VIEWPORT_Z, z + movementOffset);

        if (gameLib.IsKeyPressed(VK_MENU))
            gameLib.WriteAddress<double>(VIEWPORT_Z, z - movementOffset);
    }

    // ===============================
    // Cleanup
    // ===============================
    void Restore()
    {
        RestoreCameraCopy();
        DisableThirdPerson();
    }
};

int main()
{
    try
    {
        IGIFreeCamera freecam;

        if (!freecam.Initialize())
            return 0;

        freecam.Run();
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << "\n";
    }

    return 0;
}
