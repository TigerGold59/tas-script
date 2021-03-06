#include "lua_hiddbg.hpp"

void registerHIDDBG(lua_State* L)
{
    lua_register(L, "hiddbg_AttachController", lua_hiddbg_AttachController);
    lua_register(L, "hiddbg_DetachController", lua_hiddbg_DetachController);
    lua_register(L, "hiddbg_IsControllerAttached", lua_hiddbg_IsControllerAttached);
    lua_register(L, "hiddbg_SetButtons", lua_hiddbg_SetButtons);
    lua_register(L, "hiddbg_SetJoystick", lua_hiddbg_SetJoystick);
}

// Returns a pro controller connected using bluetooth, accepting arguments for the bodyColor then buttonsColor then gripLColor then gripRcolor
int lua_hiddbg_AttachController(lua_State* L)
{
    u32 gripRColor = lua_tointeger(L, -1);
    u32 gripLColor = lua_tointeger(L, -2);
    u32 buttonsColor = lua_tointeger(L, -3);
    u32 bodyColor = lua_tointeger(L, -4);

    int n = lua_gettop(L);
    lua_pop(L, n);

    HiddbgHdlsDeviceInfo device = { 0 };
    device.deviceType = HidDeviceType_FullKey3;
    device.npadInterfaceType = NpadInterfaceType_Bluetooth;
    device.singleColorBody = bodyColor;
    device.singleColorButtons = buttonsColor;
    device.colorLeftGrip = gripLColor;
    device.colorRightGrip = gripRColor;

    Controller* controller = reinterpret_cast<Controller *>(lua_newuserdata(L, sizeof(Controller)));
    std::memset(controller, 0, sizeof(Controller));
    
    Result rc = hiddbgAttachHdlsVirtualDevice(&controller->handle, &device);
    if (R_FAILED(rc))
    {
        std::size_t len = std::snprintf(nullptr, 0, "Error attaching controller: %#x", rc);
        char error[len+1];
        std::sprintf(error, "Error attaching controller: %#x", rc);
        lua_pushstring(L, error);
        lua_error(L);
    }

    controller->state.batteryCharge = 4;

    rc = hiddbgSetHdlsState(controller->handle, &controller->state);
    if(R_FAILED(rc))
    {
        std::size_t len = std::snprintf(nullptr, 0, "Error setting controller state: %#x", rc);
        char error[len+1];
        std::sprintf(error, "Error setting controller state: %#x", rc);
        lua_pushstring(L, error);
        lua_error(L);
    }

    return 1;
}

// Takes a controller userdata and detaches it
int lua_hiddbg_DetachController(lua_State* L)
{
    Controller* controller = reinterpret_cast<Controller *>(lua_touserdata(L, -1));

    int n = lua_gettop(L);
    lua_pop(L, n);

    Result rc = hiddbgDetachHdlsVirtualDevice(controller->handle);
    if (R_FAILED(rc))
    {
        std::size_t len = std::snprintf(nullptr, 0, "Error disconnecting controller: %#x", rc);
        char error[len+1];
        std::sprintf(error, "Error disconnecting controller: %#x", rc);
        lua_pushstring(L, error);
        lua_error(L);
    }

    return 0;
}

// Takes a controller userdata and returns whether the handle is connected
int lua_hiddbg_IsControllerAttached(lua_State* L)
{
    Controller* controller = reinterpret_cast<Controller *>(lua_touserdata(L, -1));

    int n = lua_gettop(L);
    lua_pop(L, n);

    if(controller == nullptr)
    {
        lua_pushboolean(L, false);

        return 1;
    }
    
    bool isAttached;

    Result rc = hiddbgIsHdlsVirtualDeviceAttached(controller->handle, &isAttached);
    if(R_FAILED(rc))
    {
        std::size_t len = std::snprintf(nullptr, 0, "Error checking if controller attached: %#x", rc);
        char error[len+1];
        std::sprintf(error, "Error checking if controller attached: %#x", rc);
        lua_pushstring(L, error);
        lua_error(L);
    }

    lua_pushboolean(L, isAttached);

    return 1;
}

// Takes a controller userdata then a button field and updates the buttons
int lua_hiddbg_SetButtons(lua_State* L)
{
    u64 buttons = lua_tointeger(L, -1);

    Controller* controller = reinterpret_cast<Controller *>(lua_touserdata(L, -2));

    int n = lua_gettop(L);
    lua_pop(L, n);

    controller->state.buttons = buttons;

    Result rc = hiddbgSetHdlsState(controller->handle, &controller->state);
    if(R_FAILED(rc))
    {
        std::size_t len = std::snprintf(nullptr, 0, "Error setting button state: %#x", rc);
        char error[len+1];
        std::sprintf(error, "Error setting button state: %#x", rc);
        lua_pushstring(L, error);
        lua_error(L);
    }

    return 0;
}

// Takes a controller userdata then a joystick index then an x position and y position and updates the joystick in question
int lua_hiddbg_SetJoystick(lua_State* L)
{
    s32 y = lua_tointeger(L, -1);
    s32 x = lua_tointeger(L, -2);

    int index = lua_tointeger(L, -3);

    Controller* controller = reinterpret_cast<Controller *>(lua_touserdata(L, -4));

    int n = lua_gettop(L);
    lua_pop(L, n);

    controller->state.joysticks[index-1].dx = x;
    controller->state.joysticks[index-1].dy = y;

    Result rc = hiddbgSetHdlsState(controller->handle, &controller->state);
    if(R_FAILED(rc))
    {
        std::size_t len = std::snprintf(nullptr, 0, "Error setting joystick state: %#x", rc);
        char error[len+1];
        std::sprintf(error, "Error setting joystick state: %#x", rc);
        lua_pushstring(L, error);
        lua_error(L);
    }

    return 0;
}