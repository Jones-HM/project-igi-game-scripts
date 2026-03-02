--[[
Project I.G.I FreeCamera
Enables Free Roam Camera in IGI
Execute in Cheat Engine Lua Script

Controls:
Right / Left  = X Axis
Up / Down     = Y Axis
Space / Alt   = Z Axis
Mouse         = Rotation
Backspace     = Recalibrate
HOME          = Exit

Written by IGI-ResearchDevs - HM
Improved using new research 2026.

Improvements (2026):

The engine originally used:
    mov ecx, 42
    rep movsd

This instruction copies 42 DWORDs (168 bytes) from the
internal camera structure to the viewport buffer each frame.
That includes rotation, position, and additional camera state,
causing our manual position changes to be overwritten.

We modified it to:
    mov ecx, 9
    rep movsd

Now only the first 9 DWORDs (rotation matrix) are copied.
Rotation remains engine-controlled, while position is no
longer overwritten, allowing stable free movement.
--]]

OpenProcess("igi.exe")

-- Patch address
local patchAddress = getAddress("IGI.exe+97E82")

-- Save original bytes (B9 2A 00 00 00)
local originalBytes = readBytes(patchAddress, 5, true)

-- =====================================
-- ENABLE PATCH (mov ecx,9) Copy only Rotation Matrix from Camera System.
-- =====================================
writeBytes(patchAddress, 0xB9, 0x09, 0x00, 0x00, 0x00)

-- Enable 3rd person
writeInteger("[[[[0056E210]+08]+7CC]+14]+4F0", 3)

-- Viewport position (double)
local viewport_x = 0x00BCAB08
local viewport_y = viewport_x + 8
local viewport_z = viewport_x + 16

local speed = 1000

-- =========================
-- Main Loop
-- =========================
while not isKeyPressed(VK_HOME) do

    if isKeyPressed(VK_RIGHT) then
        writeDouble(viewport_x, readDouble(viewport_x) + speed)
    end

    if isKeyPressed(VK_LEFT) then
        writeDouble(viewport_x, readDouble(viewport_x) - speed)
    end

    if isKeyPressed(VK_UP) then
        writeDouble(viewport_y, readDouble(viewport_y) + speed)
    end

    if isKeyPressed(VK_DOWN) then
        writeDouble(viewport_y, readDouble(viewport_y) - speed)
    end

    if isKeyPressed(VK_SPACE) then
        writeDouble(viewport_z, readDouble(viewport_z) + speed)
    end

    if isKeyPressed(VK_MENU) then
        writeDouble(viewport_z, readDouble(viewport_z) - speed)
    end

    sleep(1)
end

-- =====================================
-- DISABLE (restore original bytes)
-- =====================================
writeBytes(patchAddress, table.unpack(originalBytes))

-- Restore normal camera
writeInteger("[[[[0056E210]+08]+7CC]+14]+4F0", 1)
