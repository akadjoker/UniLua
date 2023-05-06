/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   wrapper.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lrosa-do <lrosa-do@student.42lisboa>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/17 07:10:21 by lrosa-do          #+#    #+#             */
/*   Updated: 2023/04/29 07:05:31 by lrosa-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "wrapper.hpp"

int screenWidth = 1020;
int screenHeight = 750;

Scene scene;

static void LuaStartEnum(lua_State *L)
{

    lua_newtable(L);
}

static void LuaSetEnum(lua_State *L, const char *name, int value)
{

    LuaPush_int(L, value);
    lua_setfield(L, -2, name);
}

static void LuaEndEnum(lua_State *L, const char *name)
{

    lua_setglobal(L, name);
}

static void LuaPushFunction(lua_State *L, const char *name, lua_CFunction func)
{

    lua_pushcfunction(L, func);
    lua_setglobal(L, name);
}

static void LuaNewClass(lua_State *L, const char *name)
{

    lua_newtable(L);
    lua_setglobal(L, name);
}

static void LuaPushClassFuntion(lua_State *L, const char *nclass, const char *name, lua_CFunction func)
{

    lua_getglobal(L, nclass);
    lua_pushcfunction(L, func);
    lua_setfield(L, -2, name);
}

namespace nWindow
{

    static int lGetWidth(lua_State *L)
    {
        lua_pushinteger(L, GetScreenWidth());
        return 1;
    }

    static int lGetHeight(lua_State *L)
    {
        lua_pushinteger(L, GetScreenHeight());
        return 1;
    }

    static int lSetWindowSize(lua_State *L)
    {
        if (lua_isnumber(L, 1))
        {
            screenWidth = lua_tointeger(L, 1);
            screenHeight = lua_tointeger(L, 2);
            SetWindowSize(screenWidth, screenHeight);
        }
        return 0;
    }

    void RegisterWindow(lua_State *L)
    {
        LuaNewClass(L, "window");
        LuaPushClassFuntion(L, "window", "getWidth", lGetWidth);
        LuaPushClassFuntion(L, "window", "getHeight", lGetHeight);
        LuaPushClassFuntion(L, "window", "setWindowSize", lSetWindowSize);
    }
}

namespace nScene
{

    static int GetGameObjectByName(lua_State *L)
    {
        // Verifique se o argumento é uma string
        if (!lua_isstring(L, 1))
        {
            lua_pushnil(L);
            return 1;
        }

        const char *name = lua_tostring(L, 1);

        GameObject *gameObject = scene.GetGameObjectByName(name);

        if (gameObject == nullptr)
        {
            lua_pushnil(L);
        }
        else
        {
            if (gameObject->table_ref != LUA_NOREF)
            {
                // Recupere a tabela do objeto associada ao gameObject
                lua_rawgeti(L, LUA_REGISTRYINDEX, gameObject->table_ref);
            }
            else
            {
                lua_pushnil(L);
            }
        }

        // Retorne o valor na pilha (gameObject ou nil)
        return 1;
    }

    static int CreateGameObject(lua_State *L)
    {

        const char *name = luaL_checkstring(L, 1);
        int layer = 0;
        float x = 0, y = 0;
        float angle = 0;
        if (lua_gettop(L) >= 2)
        {
            layer = luaL_checkinteger(L, 2);
        }
        GameObject *obj = new GameObject(name, layer);
        if (lua_gettop(L) >= 4)
        {
            x = luaL_checknumber(L, 3);
            y = luaL_checknumber(L, 4);
            obj->transform->position.x = x;
            obj->transform->position.y = y;
        }
        if (lua_gettop(L) >= 5)
        {
            angle = luaL_checknumber(L, 5);
            obj->transform->rotation = angle;
        }
        bool isChild = false;
        bool add2Scene = true;
        if (lua_gettop(L) >= 6)
        {
            if (lua_isboolean(L, 6))
            {
                add2Scene = lua_toboolean(L, 6);
            }
        }

        if (lua_gettop(L) >= 6)
        {
            if (lua_istable(L, 6))
            {
                lua_getfield(L, 6, "gameObject");
                GameObject *parent = static_cast<GameObject *>(lua_touserdata(L, -1));
                lua_pop(L, 6);

                if (parent != nullptr)
                {
                    parent->addChild(obj);
                    isChild = true;
                }
                else
                {
                    Log(LOG_INFO, "[CreateGameObject] Parent is null");
                }
            }
        }

        // obj->prefab = true;
        obj->BindLua(L);
        if (!isChild && add2Scene)
        {
            scene.AddQueueObject(obj);
        }

        return 1;
    }

    static int Add(lua_State *L)
    {
        GameObject *gameObject = nullptr;

        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1);

            if (gameObject == nullptr)
            {
                return luaL_error(L, "[add] gameObject is null");
            }
            scene.AddQueueObject(gameObject);
            lua_pushboolean(L, 1);
            return 1;
        }
        else
        {
            return luaL_error(L, "add Invalid argument type, expected table");
        }

        lua_pushboolean(L, 0);
        return 1;
    }

    static int GetView(lua_State *L)
    {
        lua_pushnumber(L, scene.cameraPoint.x);
        lua_pushnumber(L, scene.cameraPoint.y);
        return 2;
    }
    

    static int GetWorldSpace(lua_State *L)
    {
        float x = luaL_checknumber(L, 1);
        float y = luaL_checknumber(L, 2);

        // float cameraPointx = (scene.camera.offset.x - scene.camera.target.x) ;
        // float cameraPointy = (scene.camera.offset.y - scene.camera.target.y) ;

        
        lua_pushnumber(L, (x *  scene.camera.zoom)- scene.cameraPoint.x);
        lua_pushnumber(L, (y *  scene.camera.zoom)- scene.cameraPoint.y);
        return 2;
    }
    
    
    static int GetViewSize(lua_State *L)
    {
        
        lua_pushnumber(L, scene.cameraView.height);
        lua_pushnumber(L, scene.cameraView.height);

        return 2;

        
    }

    static int Remove(lua_State *L)
    {
        GameObject *gameObject = nullptr;

        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1);

            if (gameObject == nullptr)
            {
                return luaL_error(L, "[remove] gameObject is null");
            }
            scene.AddQueueObject(gameObject);
            lua_pushboolean(L, 1);
            return 1;
        }
        else
        {
            return luaL_error(L, "remove Invalid argument type, expected table");
        }

        lua_pushboolean(L, 0);
        return 1;
    }

    int LoadScene(lua_State *L)
    {
        const char *name = luaL_checkstring(L, 1);
        scene.Load(name);
        return 0;
    }

    int SaveScene(lua_State *L)
    {
        const char *name = luaL_checkstring(L, 1);
        //    Log(LOG_INFO, "Saving scene %s", name);
        scene.Save(name);
        return 0;
    }
    int ClearScene(lua_State *L)
    {
        (void)L;
        scene.ClearScene();
        return 0;
    }

    int SetColisions(lua_State *L)
    {
        bool colisions = lua_toboolean(L, 1);
        scene.enableCollisions = colisions;
        return 0;
    }
    int MousePick(lua_State *L)
    {
        GameObject *obj = scene.MousePick();
        if (obj != nullptr)
        {
            if (obj->table_ref != LUA_NOREF)
            {
                // Recupere a tabela do objeto associada ao gameObject
                lua_rawgeti(L, LUA_REGISTRYINDEX, obj->table_ref);
            }
            else
            {
                lua_pushnil(L);
            }
        }
        else
        {
            lua_pushnil(L);
        }
        return 1;
    }
    int RectanglePick(lua_State *L)
    {

        float x = luaL_checknumber(L, 1);
        float y = luaL_checknumber(L, 2);
        float w = luaL_checknumber(L, 3);
        float h = luaL_checknumber(L, 4);
        GameObject *obj = scene.RectanglePick(x, y, w, h);

        if (obj != nullptr)
        {
            if (obj->table_ref != LUA_NOREF)
            {
                // Recupere a tabela do objeto associada ao gameObject
                lua_rawgeti(L, LUA_REGISTRYINDEX, obj->table_ref);
            }
            else
            {
                lua_pushnil(L);
            }
        }
        else
        {
            lua_pushnil(L);
        }
        return 1;
    }

    int CirclePick(lua_State *L)
    {

        float x = luaL_checknumber(L, 1);
        float y = luaL_checknumber(L, 2);
        float r = luaL_checknumber(L, 3);

        GameObject *obj = scene.CirclePick(x, y, r);

        if (obj != nullptr)
        {
            if (obj->table_ref != LUA_NOREF)
            {
                // Recupere a tabela do objeto associada ao gameObject
                lua_rawgeti(L, LUA_REGISTRYINDEX, obj->table_ref);
            }
            else
            {
                lua_pushnil(L);
            }
        }
        else
        {
            lua_pushnil(L);
        }
        return 1;
    }

    int LoadTiled(lua_State *L)
    {
        const char *name = luaL_checkstring(L, 1);
        if (scene.LoadTiled(name))
        {
            lua_pushboolean(L, 1);
        }
        else
        {
            lua_pushboolean(L, 0);
        }
        return 1;
    }

    int SetCameraPosition(lua_State *L)
    {
        if (lua_gettop(L) != 2)
        {
            return luaL_error(L, "setCameraPosition function requires 2 arguments");
        }
        float x = luaL_checknumber(L, 1);
        float y = luaL_checknumber(L, 2);
        scene.SetPositionCamera(x, y);
        return 0;
    }

    int SetCamera(lua_State *L)
    {
        if (lua_gettop(L) != 2)
        {
            return luaL_error(L, "setCamera function requires 2 arguments");
        }
        float x = luaL_checknumber(L, 1);
        float y = luaL_checknumber(L, 2);
        scene.SetCamera(x, y);
        return 0;
    }
    int SetCameraSize(lua_State *L)
    {
        if (lua_gettop(L) != 2)
        {
            return luaL_error(L, "setCameraSize function requires 2 arguments");
        }
        float x = luaL_checknumber(L, 1);
        float y = luaL_checknumber(L, 2);
        scene.cameraBounds.width = x;
        scene.cameraBounds.height = y;
        return 0;
    }

    int SetCameraPivot(lua_State *L)
    {
        if (lua_gettop(L) != 2)
        {
            return luaL_error(L, "setCameraPivot function requires 2 arguments");
        }
        float x = luaL_checknumber(L, 1);
        float y = luaL_checknumber(L, 2);
        scene.camera.offset.x = x;
        scene.camera.offset.y = y;
        return 0;
    }

    int SetCameraRotation(lua_State *L)
    {
        if (lua_gettop(L) != 1)
        {
            return luaL_error(L, "setCameraRotation function requires 1 argument");
        }
        float rotation = luaL_checknumber(L, 1);
        scene.camera.rotation = rotation;
        return 0;
    }

    int SetCameraZoom(lua_State *L)
    {
        if (lua_gettop(L) != 1)
        {
            return luaL_error(L, "setCameraZoom function requires 1 argument");
        }
        float zoom = luaL_checknumber(L, 1);
        scene.camera.zoom = zoom;
        return 0;
    }

    int SetState(lua_State *L)
    {
        if (lua_gettop(L) != 2)
        {
            return luaL_error(L, "setState function requires 2 arguments");
        }
        const char *state = luaL_checkstring(L, 1);
        bool mode = lua_toboolean(L, 2);

        if (strcmp(state, "Lines") == 0)
        {
            scene.showDebug = mode;
        }
        else if (strcmp(state, "Collisions") == 0)
        {
            scene.enableCollisions = mode;
        }
        else if (strcmp(state, "Reload") == 0)
        {
            scene.enableLiveReload = mode;
        }
        else if (strcmp(state, "Editor") == 0)
        {
            scene.enableEditor = mode;
        }
        else if (strcmp(state, "Stats") == 0)
        {
            scene.showStats = mode;
        }
        else
        {
            return luaL_error(L, "setState unknown state");
        }

        return 0;
    }

    void RegisterScene(lua_State *L)
    {

        LuaNewClass(L, "scene");

        LuaPushClassFuntion(L, "scene", "findGameObject", GetGameObjectByName);
        LuaPushClassFuntion(L, "scene", "createGameObject", CreateGameObject);
        LuaPushClassFuntion(L, "scene", "load", LoadScene);
        LuaPushClassFuntion(L, "scene", "save", SaveScene);
        LuaPushClassFuntion(L, "scene", "loadTiles", LoadTiled);
        LuaPushClassFuntion(L, "scene", "clear", ClearScene);
        LuaPushClassFuntion(L, "scene", "add", Add);
        LuaPushClassFuntion(L, "scene", "remove", Remove);
        LuaPushClassFuntion(L, "scene", "setCollisions", SetColisions);
        LuaPushClassFuntion(L, "scene", "mousePick", MousePick);
        LuaPushClassFuntion(L, "scene", "rectanglePick", RectanglePick);
        LuaPushClassFuntion(L, "scene", "circlePick", CirclePick);
        LuaPushClassFuntion(L, "scene", "setState", SetState);

        LuaPushClassFuntion(L, "scene", "setCamera", SetCamera);
        LuaPushClassFuntion(L, "scene", "setCameraPosition", SetCameraPosition);
        LuaPushClassFuntion(L, "scene", "setCameraPivot", SetCameraPivot);
        LuaPushClassFuntion(L, "scene", "setCameraRotation", SetCameraRotation);
        LuaPushClassFuntion(L, "scene", "setCameraZoom", SetCameraZoom);
        LuaPushClassFuntion(L, "scene", "setCameraSize", SetCameraSize);
        LuaPushClassFuntion(L, "scene", "getViewPosition", GetView);
        LuaPushClassFuntion(L, "scene", "getViewSize", GetViewSize);
        LuaPushClassFuntion(L, "scene", "getWorldSpace", GetWorldSpace);
        
    }

}

namespace nInput
{
    static int getKey(lua_State *L)
    {
        int key = luaL_checkinteger(L, 1);
        lua_pushinteger(L, IsKeyDown(key));
        return 1;
    }

    static int getKeyDown(lua_State *L)
    {
        int key = luaL_checkinteger(L, 1);
        lua_pushboolean(L, IsKeyDown(key));
        return 1;
    }
    static int getKeyPressed(lua_State *L)
    {
        int key = luaL_checkinteger(L, 1);
        lua_pushboolean(L, IsKeyPressed(key));
        return 1;
    }

    static int getKeyUp(lua_State *L)
    {
        int key = luaL_checkinteger(L, 1);
        lua_pushboolean(L, IsKeyReleased(key));
        return 1;
    }

    static int getMouse(lua_State *L)
    {
        int key = luaL_checkinteger(L, 1);
        lua_pushinteger(L, IsMouseButtonDown(key));
        return 1;
    }
    static int getMouseDown(lua_State *L)
    {
        int key = luaL_checkinteger(L, 1);
        lua_pushboolean(L, IsMouseButtonDown(key));
        return 1;
    }

    static int getMousePressed(lua_State *L)
    {
        int key = luaL_checkinteger(L, 1);
        lua_pushboolean(L, IsMouseButtonPressed(key));
        return 1;
    }

    static int getMouseUp(lua_State *L)
    {
        int key = luaL_checkinteger(L, 1);
        lua_pushboolean(L, IsMouseButtonReleased(key));
        return 1;
    }

    static int getMouseX(lua_State *L)
    {
        lua_pushinteger(L, GetMouseX());
        return 1;
    }
    static int getMouseY(lua_State *L)
    {
        lua_pushinteger(L, GetMouseY());
        return 1;
    }

    static int getMouseLocal(lua_State *L)
    {
        Vector2 mouse = GetMousePosition();

        lua_pushnumber(L, mouse.x);
        lua_pushnumber(L, mouse.y);
        return 2;
    }

    static int getMouseDelta(lua_State *L)
    {
        Vector2 mouse = GetMouseDelta();

        lua_pushnumber(L, mouse.x);
        lua_pushnumber(L, mouse.y);
        return 2;
    }

    static int getMouseWheel(lua_State *L)
    {
        lua_pushinteger(L, GetMouseWheelMove());
        return 1;
    }

    void RegisterInput(lua_State *L)
    {

        LuaStartEnum(L);
        LuaSetEnum(L, "space", 32);
        LuaSetEnum(L, "escape", 256);
        LuaSetEnum(L, "enter", 257);
        LuaSetEnum(L, "backspace", 259);
        LuaSetEnum(L, "right", 262);
        LuaSetEnum(L, "left", 263);
        LuaSetEnum(L, "down", 264);
        LuaSetEnum(L, "up", 265);
        LuaSetEnum(L, "F1", 290);
        LuaSetEnum(L, "F2", 291);
        LuaSetEnum(L, "F3", 292);
        LuaSetEnum(L, "F4", 293);
        LuaSetEnum(L, "F5", 294);
        LuaSetEnum(L, "F6", 295);
        LuaSetEnum(L, "F7", 296);
        LuaSetEnum(L, "F8", 297);
        LuaSetEnum(L, "F9", 298);
        LuaSetEnum(L, "F10", 299);
        LuaSetEnum(L, "LEFT_SHIFT", 340);
        LuaSetEnum(L, "LEFT_CONTROL", 341);
        LuaSetEnum(L, "LEFT_ALT", 342);
        LuaSetEnum(L, "RIGHT_SHIFT", 344);
        LuaSetEnum(L, "RIGHT_CONTROL", 345);
        LuaSetEnum(L, "RIGHT_ALT", 346);
        LuaSetEnum(L, "ZERO", 48);
        LuaSetEnum(L, "ONE", 49);
        LuaSetEnum(L, "TWO", 50);
        LuaSetEnum(L, "THREE", 51);
        LuaSetEnum(L, "FOUR", 52);
        LuaSetEnum(L, "FIVE", 53);
        LuaSetEnum(L, "SIX", 54);
        LuaSetEnum(L, "SEVEN", 55);
        LuaSetEnum(L, "EIGHT", 56);
        LuaSetEnum(L, "NINE", 57);
        LuaSetEnum(L, "A", 65);
        LuaSetEnum(L, "B", 66);
        LuaSetEnum(L, "C", 67);
        LuaSetEnum(L, "D", 68);
        LuaSetEnum(L, "E", 69);
        LuaSetEnum(L, "F", 70);
        LuaSetEnum(L, "G", 71);
        LuaSetEnum(L, "H", 72);
        LuaSetEnum(L, "I", 73);
        LuaSetEnum(L, "J", 74);
        LuaSetEnum(L, "K", 75);
        LuaSetEnum(L, "L", 76);
        LuaSetEnum(L, "M", 77);
        LuaSetEnum(L, "N", 78);
        LuaSetEnum(L, "O", 79);
        LuaSetEnum(L, "P", 80);
        LuaSetEnum(L, "Q", 81);
        LuaSetEnum(L, "R", 82);
        LuaSetEnum(L, "S", 83);
        LuaSetEnum(L, "T", 84);
        LuaSetEnum(L, "U", 85);
        LuaSetEnum(L, "V", 86);
        LuaSetEnum(L, "W", 87);
        LuaSetEnum(L, "X", 88);
        LuaSetEnum(L, "Y", 89);
        LuaSetEnum(L, "Z", 90);
        LuaEndEnum(L, "KEY");

        LuaStartEnum(L);
        LuaSetEnum(L, "LEFT", 0);
        LuaSetEnum(L, "RIGHT", 1);
        LuaSetEnum(L, "MIDDLE", 2);
        LuaEndEnum(L, "MOUSE");

        //**********************************
        LuaNewClass(L, "Key");
        LuaPushClassFuntion(L, "Key", "check", getKey);
        LuaPushClassFuntion(L, "Key", "down", getKeyDown);
        LuaPushClassFuntion(L, "Key", "pressed", getKeyPressed);
        LuaPushClassFuntion(L, "Key", "released", getKeyUp);

        //*********************************
        LuaNewClass(L, "Mouse");
        LuaPushClassFuntion(L, "Mouse", "check", getMouse);
        LuaPushClassFuntion(L, "Mouse", "down", getMouseDown);
        LuaPushClassFuntion(L, "Mouse", "pressed", getMousePressed);
        LuaPushClassFuntion(L, "Mouse", "released", getMouseUp);
        LuaPushClassFuntion(L, "Mouse", "getX", getMouseX);
        LuaPushClassFuntion(L, "Mouse", "getY", getMouseY);
        LuaPushClassFuntion(L, "Mouse", "getLocal", getMouseLocal);
        LuaPushClassFuntion(L, "Mouse", "getDelta", getMouseDelta);
        LuaPushClassFuntion(L, "Mouse", "getWheel", getMouseWheel);
    }

} // namespace name

namespace nUtils
{

    float max(float a, float b) { return (a > b) ? a : b; }
    float min(float a, float b) { return (a < b) ? a : b; }

    float lengthdir_x(float _length, float _direction)
    {
        return _length * cos(_direction * RAD);
    }

    float lengthdir_y(float _length, float _direction)
    {
        return _length * sin(_direction * RAD);
    }

    float point_distance(float _x1, float _y1, float _x2, float _y2)
    {
        return sqrt(pow((_x1 - _x2), 2) + pow((_y1 - _y2), 2));
    }

    float point_direction(float _x1, float _y1, float _x2, float _y2)
    {
        return atan2(_y2 - _y1, _x2 - _x1) * DEG;
    }

    float degtorad(float _degree) { return _degree * RAD; }
    float radtodeg(float _degree) { return _degree * DEG; }

    float hermite(float value1, float tangent1, float value2, float tangent2, float amount)
    {
        float v1 = value1;
        float v2 = value2;
        float t1 = tangent1;
        float t2 = tangent2;
        float s = amount;
        float result;
        float sCubed = s * s * s;
        float sSquared = s * s;

        if (amount == 0)
            result = value1;
        else if (amount == 1)
            result = value2;
        else
            result = (2 * v1 - 2 * v2 + t2 + t1) * sCubed +
                     (3 * v2 - 3 * v1 - 2 * t1 - t2) * sSquared +
                     t1 * s +
                     v1;
        return result;
    }

    float fget_angle(float x1, float y1, float x2, float y2)
    {
        float dx = x2 - x1;
        float dy = y2 - y1;
        float angle;

        if (dx == 0.0f)
            return dy > 0.0f ? 270.0f : 90.0f;

        angle = (atan(dy / dx) * 180.0f / PI);

        return dx > 0 ? -angle : -angle + 180;
    }

    float get_angle(float x1, float y1, float x2, float y2)
    {
        float a = atan2(y2 - y1, x2 - x1) * DEG;
        return a < 0.0f ? a + 360.0f : a;
    }

    float clamp(float value, float min, float max)
    {
        float result = (value < min) ? min : value;

        if (result > max)
            result = max;

        return result;
    }

    float lerp(float start, float end, float amount)
    {
        float result = start + amount * (end - start);

        return result;
    }

    float normalize(float value, float start, float end)
    {
        float result = (value - start) / (end - start);

        return result;
    }

    float lerp_angle(float a, float b, float lerpFactor)
    {
        float result;
        float diff = b - a;
        if (diff < -180.f)
        {
            b += 360.f;
            result = lerp(a, b, lerpFactor);
            if (result >= 360.f)
                result -= 360.f;
        }
        else if (diff > 180.f)
        {
            b -= 360.f;
            result = lerp(a, b, lerpFactor);
            if (result < 0.f)
                result += 360.f;
        }
        else
            result = lerp(a, b, lerpFactor);

        return result;
    }

    float smooth_step(float value1, float value2, float amount)
    {

        float result = clamp(amount, 0, 1);
        result = hermite(value1, 0, value2, 0, result);

        return result;
    }
    float repeat(float t, float length)
    {
        return clamp(t - floor(t / length) * length, 0.0f, length);
    }
    float ping_pong(float t, float length)
    {
        t = repeat(t, length * 2.0f);
        return length - abs(t - length);
    }

    int sign(float value)
    {
        return value < 0 ? -1 : (value > 0 ? 1 : 0);
    }

    static int lclamp(lua_State *L)
    {
        if (lua_gettop(L) != 3)
        {
            return luaL_error(L, "clamp function requires 3 arguments");
        }
        float value = lua_tonumber(L, 1);
        float min = lua_tonumber(L, 2);
        float max = lua_tonumber(L, 3);
        lua_pushnumber(L, clamp(value, min, max));
        return 1;
    }

    static int lhermite(lua_State *L)
    {
        if (lua_gettop(L) != 5)
        {
            return luaL_error(L, "hermite function requires 5 arguments");
        }
        float value1 = lua_tonumber(L, 1);
        float tangent1 = lua_tonumber(L, 2);
        float value2 = lua_tonumber(L, 3);
        float tangent2 = lua_tonumber(L, 4);
        float amount = lua_tonumber(L, 5);
        lua_pushnumber(L, hermite(value1, tangent1, value2, tangent2, amount));
        return 1;
    }

    static int llerp_angle(lua_State *L)
    {
        if (lua_gettop(L) != 3)
        {
            return luaL_error(L, "lerp_angle function requires 3 arguments");
        }
        float a = lua_tonumber(L, 1);
        float b = lua_tonumber(L, 2);
        float lerpFactor = lua_tonumber(L, 3);
        lua_pushnumber(L, lerp_angle(a, b, lerpFactor));
        return 1;
    }

    static int lget_angle(lua_State *L)
    {
        if (lua_gettop(L) != 4)
        {
            return luaL_error(L, "get_angle function requires 4 arguments");
        }
        float x1 = lua_tonumber(L, 1);
        float y1 = lua_tonumber(L, 2);
        float x2 = lua_tonumber(L, 3);
        float y2 = lua_tonumber(L, 4);
        lua_pushnumber(L, get_angle(x1, y1, x2, y2));
        return 1;
    }

    static int lfget_angle(lua_State *L)
    {
        if (lua_gettop(L) != 4)
        {
            return luaL_error(L, "fget_angle function requires 4 arguments");
        }
        float x1 = lua_tonumber(L, 1);
        float y1 = lua_tonumber(L, 2);
        float x2 = lua_tonumber(L, 3);
        float y2 = lua_tonumber(L, 4);
        lua_pushnumber(L, fget_angle(x1, y1, x2, y2));
        return 1;
    }

    static int llerp(lua_State *L)
    {
        if (lua_gettop(L) != 3)
        {
            return luaL_error(L, "lerp function requires 3 arguments");
        }
        float start = lua_tonumber(L, 1);
        float end = lua_tonumber(L, 2);
        float amount = lua_tonumber(L, 3);
        lua_pushnumber(L, lerp(start, end, amount));
        return 1;
    }

    static int lnormalize(lua_State *L)
    {
        if (lua_gettop(L) != 3)
        {
            return luaL_error(L, "normalize function requires 3 arguments");
        }
        float value = lua_tonumber(L, 1);
        float start = lua_tonumber(L, 2);
        float end = lua_tonumber(L, 3);
        lua_pushnumber(L, normalize(value, start, end));
        return 1;
    }

    static int lsmooth_step(lua_State *L)
    {
        if (lua_gettop(L) != 3)
        {
            return luaL_error(L, "smooth_step function requires 3 arguments");
        }
        float value1 = lua_tonumber(L, 1);
        float value2 = lua_tonumber(L, 2);
        float amount = lua_tonumber(L, 3);
        lua_pushnumber(L, smooth_step(value1, value2, amount));
        return 1;
    }

    static int lrepeat(lua_State *L)
    {
        if (lua_gettop(L) != 2)
        {
            return luaL_error(L, "repeat function requires 2 arguments");
        }
        float t = lua_tonumber(L, 1);
        float length = lua_tonumber(L, 2);
        lua_pushnumber(L, repeat(t, length));
        return 1;
    }

    static int lping_pong(lua_State *L)
    {
        if (lua_gettop(L) != 2)
        {
            return luaL_error(L, "ping_pong function requires 2 arguments");
        }
        float t = lua_tonumber(L, 1);
        float length = lua_tonumber(L, 2);
        lua_pushnumber(L, ping_pong(t, length));
        return 1;
    }

    static int lsign(lua_State *L)
    {
        if (lua_gettop(L) != 1)
        {
            return luaL_error(L, "sign function requires 1 argument");
        }
        float value = lua_tonumber(L, 1);
        lua_pushnumber(L, sign(value));
        return 1;
    }

    static int lmax(lua_State *L)
    {
        if (lua_gettop(L) != 2)
        {
            return luaL_error(L, "max function requires 2 arguments");
        }
        float a = lua_tonumber(L, 1);
        float b = lua_tonumber(L, 2);
        lua_pushnumber(L, max(a, b));
        return 1;
    }

    static int lmin(lua_State *L)
    {
        if (lua_gettop(L) != 2)
        {
            return luaL_error(L, "min function requires 2 arguments");
        }
        float a = lua_tonumber(L, 1);
        float b = lua_tonumber(L, 2);
        lua_pushnumber(L, min(a, b));
        return 1;
    }

    static int lengthdir_x(lua_State *L)
    {
        if (lua_gettop(L) != 2)
        {
            return luaL_error(L, "lengthdir_x function requires 2 arguments");
        }
        float _length = lua_tonumber(L, 1);
        float _direction = lua_tonumber(L, 2);
        lua_pushnumber(L, lengthdir_x(_length, _direction));
        return 1;
    }

    static int lengthdir_y(lua_State *L)
    {
        if (lua_gettop(L) != 2)
        {
            return luaL_error(L, "lengthdir_y function requires 2 arguments");
        }
        float _length = lua_tonumber(L, 1);
        float _direction = lua_tonumber(L, 2);
        lua_pushnumber(L, lengthdir_y(_length, _direction));
        return 1;
    }

    static int point_distance(lua_State *L)
    {
        if (lua_gettop(L) != 4)
        {
            return luaL_error(L, "point_distance function requires 4 arguments");
        }
        float _x1 = lua_tonumber(L, 1);
        float _y1 = lua_tonumber(L, 2);
        float _x2 = lua_tonumber(L, 3);
        float _y2 = lua_tonumber(L, 4);
        lua_pushnumber(L, point_distance(_x1, _y1, _x2, _y2));
        return 1;
    }

    static int point_direction(lua_State *L)
    {
        if (lua_gettop(L) != 4)
        {
            return luaL_error(L, "point_direction function requires 4 arguments");
        }
        float _x1 = lua_tonumber(L, 1);
        float _y1 = lua_tonumber(L, 2);
        float _x2 = lua_tonumber(L, 3);
        float _y2 = lua_tonumber(L, 4);
        lua_pushnumber(L, point_direction(_x1, _y1, _x2, _y2));
        return 1;
    }

    static int degtorad(lua_State *L)
    {
        if (lua_gettop(L) != 1)
        {
            return luaL_error(L, "degtorad function requires 1 argument");
        }
        float _degree = lua_tonumber(L, 1);
        lua_pushnumber(L, degtorad(_degree));
        return 1;
    }

    static int radtodeg(lua_State *L)
    {
        if (lua_gettop(L) != 1)
        {
            return luaL_error(L, "radtodeg function requires 1 argument");
        }
        float _degree = lua_tonumber(L, 1);
        lua_pushnumber(L, radtodeg(_degree));
        return 1;
    }
    std::random_device rd;

    std::mt19937 generator(rd());

    int random_int(int min, int max)
    {
        std::uniform_int_distribution<int> distribution(min, max); 
        return distribution(generator);
    }


    float random_float(float min, float max)
    {
        std::uniform_real_distribution<float> distribution(min, max); 
        return distribution(generator);
    }

    static int rand_float(lua_State *L)
    {
        if (lua_gettop(L) != 2)
        {
            return luaL_error(L, "randf function requires 2 arguments");
        }
        float _min = lua_tonumber(L, 1);
        float _max = lua_tonumber(L, 2);
        lua_pushnumber(L, random_float(_min, _max));
        return 1;
    }

    static int rand_int(lua_State *L)
    {
        if (lua_gettop(L) != 2)
        {
            return luaL_error(L, "randi function requires 2 arguments");
        }
        int _min = lua_tointeger(L, 1);
        int _max = lua_tointeger(L, 2);
        lua_pushinteger(L, random_int(_min, _max));
        return 1;
    }

    void RegisterUtils(lua_State *L)
    {

        LuaPushFunction(L, "max", lmax);
        LuaPushFunction(L, "min", lmin);
        LuaPushFunction(L, "repeat", lrepeat);
        LuaPushFunction(L, "ping_pong", lping_pong);
        LuaPushFunction(L, "sign", lsign);
        LuaPushFunction(L, "clamp", lclamp);
        LuaPushFunction(L, "lerp", llerp);
        LuaPushFunction(L, "smooth_step", lsmooth_step);
        LuaPushFunction(L, "normalize", lnormalize);
        LuaPushFunction(L, "hermite", lhermite);
        LuaPushFunction(L, "lerp_angle", llerp_angle);
        LuaPushFunction(L, "fget_angle", lfget_angle);
        LuaPushFunction(L, "get_angle", lget_angle);
        LuaPushFunction(L, "lengthdir_x", lengthdir_x);
        LuaPushFunction(L, "lengthdir_y", lengthdir_y);
        LuaPushFunction(L, "point_distance", point_distance);
        LuaPushFunction(L, "point_direction", point_direction);
        LuaPushFunction(L, "degtorad", degtorad);
        LuaPushFunction(L, "radtodeg", radtodeg);
        LuaPushFunction(L, "randf", rand_float);
        LuaPushFunction(L, "randi", rand_int);
    }

}

namespace nCanvas
{

    Color color = WHITE;
    Rectangle imageSrc;
    Rectangle imageDst;

    static int drawRectangle(lua_State *L)
    {
        if (lua_gettop(L) <= 3)
        {
            return luaL_error(L, "drawRectangle function requires 4/5 arguments");
        }
        int x = lua_tointeger(L, 1);
        int y = lua_tointeger(L, 2);
        int width = lua_tointeger(L, 3);
        int height = lua_tointeger(L, 4);
        bool fill = true;
        if (lua_gettop(L) >= 5)
            fill = lua_toboolean(L, 5);

        if (fill)
            DrawRectangle(x, y, width, height, color);
        else
            DrawRectangleLines(x, y, width, height, color);

        return 0;
    }

    static int drawCircle(lua_State *L)
    {
        if (lua_gettop(L) <= 2)
        {
            return luaL_error(L, "drawCircle function requires 3/4 arguments");
        }
        int x = lua_tointeger(L, 1);
        int y = lua_tointeger(L, 2);
        int radius = lua_tointeger(L, 3);
        bool fill = true;
        if (lua_gettop(L) >= 4)
            fill = lua_toboolean(L, 4);

        if (fill)
            DrawCircle(x, y, radius, color);
        else
            DrawCircleLines(x, y, radius, color);

        return 0;
    }

    static int drawLine(lua_State *L)
    {
        if (lua_gettop(L) != 4)
        {
            return luaL_error(L, "drawLine function requires 4 arguments");
        }
        int x1 = lua_tointeger(L, 1);
        int y1 = lua_tointeger(L, 2);
        int x2 = lua_tointeger(L, 3);
        int y2 = lua_tointeger(L, 4);
        DrawLine(x1, y1, x2, y2, color);
        return 0;
    }

    static int drawText(lua_State *L)
    {
        if (lua_gettop(L) < 4)
        {
            return luaL_error(L, "drawText function requires 4 arguments");
        }
        char *string = (char *)lua_tostring(L, 1);
        int x = lua_tointeger(L, 2);
        int y = lua_tointeger(L, 3);
        int s = lua_tointeger(L, 4);
        DrawText(string, x, y, s, color);
        return 0;
    }

    static int setColor(lua_State *L)
    {
        if (lua_gettop(L) != 3)
        {
            return luaL_error(L, "setColor function requires 3 arguments");
        }
        color.r = lua_tointeger(L, 1);
        color.g = lua_tointeger(L, 2);
        color.b = lua_tointeger(L, 3);
        return 0;
    }

    static int setAlpha(lua_State *L)
    {
        if (lua_gettop(L) != 1)
        {
            return luaL_error(L, "setAlpha function requires 1 arguments");
        }
        color.a = lua_tointeger(L, 1);
        return 0;
    }

    static int drawGraph(lua_State *L)
    {
        if (lua_gettop(L) != 3 && lua_gettop(L) != 7)
        {
            return luaL_error(L, "drawGraph function requires 3 or 7 Src(4) arguments");
        }

        const char *graphName = lua_tostring(L, 1);
        float x = lua_tonumber(L, 2);
        float y = lua_tonumber(L, 3);

        if (!Assets::Instance().hasGraph(graphName))
        {

            return luaL_error(L, "graph %s not found", graphName);
        }
        Graph *graph = Assets::Instance().getGraph(graphName);

        if (lua_gettop(L) == 7)
        {
            float sx = lua_tonumber(L, 4);
            float sy = lua_tonumber(L, 5);
            float sw = lua_tonumber(L, 6);
            float sh = lua_tonumber(L, 7);
            Rectangle source = (Rectangle){sx, sy, sw, sh};
            DrawTextureRec(graph->texture, source, (Vector2){x, y}, color);
        }
        else
            DrawTexture(graph->texture, (int)x, (int)y, color);

        return 0;
    }

    static int drawGraphTiled(lua_State *L)
    {
        if (lua_gettop(L) != 13)
        {
            return luaL_error(L, "drawGraphTiled function requires Graph(1) Src(4) Dst(4) Pivot(2) rotation(1) scale(1) arguments");
        }

        const char *graphName = lua_tostring(L, 1);

        if (!Assets::Instance().hasGraph(graphName))
        {
            return luaL_error(L, "graph %s not found", graphName);
        }
        Graph *graph = Assets::Instance().getGraph(graphName);

        float dx = lua_tonumber(L, 2);
        float dy = lua_tonumber(L, 3);
        float dw = lua_tonumber(L, 4);
        float dh = lua_tonumber(L, 5);

        float sx = lua_tonumber(L, 6);
        float sy = lua_tonumber(L, 7);
        float sw = lua_tonumber(L, 8);
        float sh = lua_tonumber(L, 9);

        float ox = lua_tonumber(L, 10);
        float oy = lua_tonumber(L, 11);
        float rotation = lua_tonumber(L, 12);
        float scale = lua_tonumber(L, 13);

        Rectangle source;
        source.x = sx;
        source.y = sy;
        source.width = sw;
        source.height = sh;

        Rectangle dest;
        dest.x = dx;
        dest.y = dy;
        dest.width = dw;
        dest.height = dh;

        Vector2 pivot;
        pivot.x = ox;
        pivot.y = oy;

        DrawTextureTiled(graph->texture, source, dest, pivot, rotation, scale, color);
        return 0;
    }

    static int drawGraphRotate(lua_State *L)
    {

        const char *graphName = lua_tostring(L, 1);

        if (!Assets::Instance().hasGraph(graphName))
        {
            return luaL_error(L, "graph %s not found", graphName);
        }
        Graph *graph = Assets::Instance().getGraph(graphName);

        float dx = lua_tonumber(L, 2);
        float dy = lua_tonumber(L, 3);
        float dw = lua_tonumber(L, 4);
        float dh = lua_tonumber(L, 5);

        float sx = lua_tonumber(L, 6);
        float sy = lua_tonumber(L, 7);
        float sw = lua_tonumber(L, 8);
        float sh = lua_tonumber(L, 9);

        float ox = lua_tonumber(L, 10);
        float oy = lua_tonumber(L, 11);
        float rotate = lua_tonumber(L, 12);

        Rectangle source;
        source.x = sx;
        source.y = sy;
        source.width = sw;
        source.height = sh;

        Rectangle dest;
        dest.x = dx;
        dest.y = dy;
        dest.width = dw;
        dest.height = dh;

        Vector2 pivot;
        pivot.x = ox;
        pivot.y = oy;

        DrawTexturePro(graph->texture, source, dest, pivot, rotate, color);
        return 0;
    }
    void RegisterCanvas(lua_State *L)
    {

        lua_newtable(L);

        lua_pushcfunction(L, setColor);
        lua_setfield(L, -2, "setColor");

        lua_pushcfunction(L, setAlpha);
        lua_setfield(L, -2, "setAlpha");

        lua_pushcfunction(L, drawRectangle);
        lua_setfield(L, -2, "drawRectangle");

        lua_pushcfunction(L, drawCircle);
        lua_setfield(L, -2, "drawCircle");

        lua_pushcfunction(L, drawLine);
        lua_setfield(L, -2, "drawLine");

        lua_pushcfunction(L, drawText);
        lua_setfield(L, -2, "drawText");

        lua_pushcfunction(L, drawGraph);
        lua_setfield(L, -2, "drawGraph");

        lua_pushcfunction(L, drawGraphTiled);
        lua_setfield(L, -2, "drawGraphTiled");

        lua_pushcfunction(L, drawGraphRotate);
        lua_setfield(L, -2, "drawGraphRotate");

        lua_setglobal(L, "canvas");
    }

}

namespace nEngine
{

    static int loadGraph(lua_State *L)
    {
        if (lua_gettop(L) != 2)
        {
            return luaL_error(L, "loadGraph function requires 2 argument (KEY, PATH)");
        }
        char *key = (char *)lua_tostring(L, 1);
        char *path = (char *)lua_tostring(L, 2);

        Assets::Instance().loadGraph(key, path);

        return 0;
    }

    static int HasGraph(lua_State *L)
    {
        if (lua_gettop(L) != 1)
        {
            return luaL_error(L, "hasGraph function requires 1 argument (KEY)");
        }
        char *key = (char *)lua_tostring(L, 1);

        bool b = Assets::Instance().hasGraph(key);
        lua_pushboolean(L, b);

        return 1;
    }

    void RegisterAssets(lua_State *L)
    {

        LuaNewClass(L, "assets");

        LuaPushClassFuntion(L, "assets", "loadGraph", loadGraph);
        LuaPushClassFuntion(L, "assets", "hasGraph", HasGraph);
    }

}

double getLuaMemoryUsage()
{
    lua_State *L = getState();
    int totalKilobytes = lua_gc(L, LUA_GCCOUNT, 0);
    int totalBytes = lua_gc(L, LUA_GCCOUNTB, 0);

    // Converte a quantidade de memória usada para double
    double memoryUsage = static_cast<double>(totalKilobytes) + (static_cast<double>(totalBytes) / 1024.0);
    return memoryUsage;
}
lua_State *L;

struct MainScript
{

    std::string path;
    bool isLoad;
    bool panic;
    long timeLoad;
    std::unordered_map<std::string, int> luaFunctions;

    MainScript()
    {

        isLoad = false;
        panic = false;
        timeLoad = 0;
    }
    bool isFunctionRegistered(const std::string &functionName)
    {

        auto it = luaFunctions.find(functionName);
        return it != luaFunctions.end();
    }

    void registerFunction(const char *functionName)
    {
        lua_getglobal(L, functionName);
        if (lua_isfunction(L, -1))
        {
            luaFunctions[functionName] = luaL_ref(L, LUA_REGISTRYINDEX);
        }
        else
        {
            lua_pop(L, 1); // Remover o valor não-função do topo da pilha
        }
    }
    void LuaBind()
    {
        luaFunctions.clear();
        registerFunction("OnCreate");
        registerFunction("OnClose");
        registerFunction("render");
        registerFunction("update");
    }
    void Load()
    {

        isLoad = false;
        if (FileExists("main.lua"))
        {
            this->path = "main.lua";
            Log(LOG_INFO, "Loading main.lua");
            isLoad = true;
        }
        else if (FileExists("scripts/main.lua"))
        {

            this->path = "scripts/main.lua";
            isLoad = true;
            Log(LOG_INFO, "Loading scripts/main.lua");
        }
        else if (FileExists("assets/main.lua"))
        {

            this->path = "assets/main.lua";
            isLoad = true;
            Log(LOG_INFO, "Loading assets/main.lua");
        }
        else if (FileExists("assets/scripts/main.lua"))
        {

            this->path = "assets/scripts/main.lua";
            isLoad = true;
            Log(LOG_INFO, "Loading assets/scripts/main.lua");
        }

        if (!isLoad)
        {
            Log(LOG_WARNING, "Failed to load script main.lua");
            return;
        }

        timeLoad = GetFileModTime(this->path.c_str());

        char *data = LoadFileText(this->path.c_str());
        size_t size = strlen(data);

        if (luaL_loadbufferx(L, data, size, this->path.c_str(), nullptr) != LUA_OK)
        {
            const char *errMsg = lua_tostring(L, -1);
            Log(LOG_ERROR, "Failed to load script %s : %s ", this->path.c_str(), errMsg);
            lua_pop(L, 1);
            free(data);
            panic = true;
            return;
        }

        free(data);

        if (lua_pcall(L, 0, LUA_MULTRET, 0) != LUA_OK)
        {
            const char *errMsg = lua_tostring(L, -1);
            Log(LOG_ERROR, "Failed to execute script %s : %s ", this->path.c_str(), errMsg);
            lua_pop(L, 1);
            panic = true;
            return;
        }
        panic = false;

        LuaBind();
    }

    void Update(float dt)
    {
        if (panic || !isLoad)
        {
            return;
        }

        if (!isFunctionRegistered("update"))
        {
            return;
        }

        int updateRef = luaFunctions["update"];
        lua_rawgeti(L, LUA_REGISTRYINDEX, updateRef);
        lua_pushnumber(L, dt);

        if (lua_pcall(L, 1, 0, 0) != LUA_OK)
        {
            const char *errMsg = lua_tostring(L, -1);
            Log(LOG_ERROR, "Failed to execute script %s [update] : %s ", path.c_str(), errMsg);
            lua_pop(L, 1);
            panic = true;
            return;
        }
    }
    void Render()
    {
        if (panic || !isLoad)
        {
            return;
        }

        if (!isFunctionRegistered("render"))
        {
            return;
        }

        int updateRef = luaFunctions["render"];
        lua_rawgeti(L, LUA_REGISTRYINDEX, updateRef);
        if (lua_pcall(L, 0, 0, 0) != LUA_OK)
        {
            const char *errMsg = lua_tostring(L, -1);
            Log(LOG_ERROR, "Failed to execute script %s [render] : %s ", path.c_str(), errMsg);
            lua_pop(L, 1);
            panic = true;
            return;
        }
    }

    void Create()
    {
        if (panic || !isLoad)
        {
            return;
        }

        if (!isFunctionRegistered("OnCreate"))
        {
            return;
        }

        int updateRef = luaFunctions["OnCreate"];
        lua_rawgeti(L, LUA_REGISTRYINDEX, updateRef);
        if (lua_pcall(L, 0, 0, 0) != LUA_OK)
        {
            const char *errMsg = lua_tostring(L, -1);
            Log(LOG_ERROR, "Failed to execute script %s [OnCreate] : %s ", path.c_str(), errMsg);
            lua_pop(L, 1);
            panic = true;
            return;
        }
    }
    void Close()
    {
        if (panic || !isLoad)
        {
            return;
        }

        if (!isFunctionRegistered("OnClose"))
        {
            return;
        }

        int updateRef = luaFunctions["OnClose"];
        lua_rawgeti(L, LUA_REGISTRYINDEX, updateRef);
        if (lua_pcall(L, 0, 0, 0) != LUA_OK)
        {
            const char *errMsg = lua_tostring(L, -1);
            Log(LOG_ERROR, "Failed to execute script %s [OnClose] : %s ", path.c_str(), errMsg);
            lua_pop(L, 1);
            panic = true;
            return;
        }
    }
};

MainScript mainScript;

// engine part

void InitEngine()
{
    mainScript.Create();
    mainScript.Render();
    mainScript.Update(0);
}

void FreeEngine()
{
    mainScript.Close();
    scene.ClearAndFree();
    Assets::Instance().clear();
}

void EngineRender()
{
    mainScript.Update(GetFrameTime());
    scene.Update();

    mainScript.Render();
    scene.Render();
}

void Collect()
{
    lua_State *L = getState();
    Log(LOG_INFO, "Collecting garbage");
    lua_gc(L, LUA_GCCOLLECT, 0);
}

// lua parte

lua_State *getState()
{
    return L;
}

void LoadLua()
{
    L = luaL_newstate();
    luaL_openlibs(L);

    nCanvas::RegisterCanvas(L);
    nInput::RegisterInput(L);
    nEngine::RegisterAssets(L);
    nUtils::RegisterUtils(L);
    nScene::RegisterScene(L);
    nWindow::RegisterWindow(L);

    // LuaPushFunction(L,"cfibonacci", l_fibonacci);
    // LuaPushFunction(L,"cfactorial", l_factorial);

#if defined(PLATFORM_ANDROID)
    luaL_dostring(L, "package.path = package.path .. ';?.lua;/sdcard/lua?.lua;/sdcard/lua/assets/?.lua'");
#endif

#if defined(PLATFORM_DESKTOP)
    luaL_dostring(L, "package.path = package.path .. ';assets/scripts/?.lua;assets/scripts/utils/?.lua'");
#endif

    lua_newtable(L);
    lua_setglobal(L, "script_refs");
    mainScript.Load();
}

void CloseLua()
{
    FreeEngine();
    lua_close(L);
}

void PrintLuaStack(lua_State *L)
{
    int top = lua_gettop(L);
    for (int i = 1; i <= top; i++)
    {
        int t = lua_type(L, i);
        switch (t)
        {
        case LUA_TSTRING:
            Log(LOG_INFO, "s:%s\n", lua_tostring(L, i));
            break;
        case LUA_TBOOLEAN:
            Log(LOG_INFO, "%s \n", lua_toboolean(L, i) ? "true" : "false");
            break;
        case LUA_TNUMBER:
            Log(LOG_INFO, "N: %g\n", lua_tonumber(L, i));
            break;
        default:
            Log(LOG_INFO, "N:%s\n", lua_typename(L, t));
            // PrintLuaTable(L);
            break;
        }
        printf(" ");
    }
    printf("\n");
}

void PrintLuaTable(lua_State *L)
{
    lua_pushnil(L);
    while (lua_next(L, -2) != 0)
    {
        // A chave está no índice -2 e o valor está no índice -1 da pilha
        if (lua_isstring(L, -2))
        {
            const char *key = lua_tostring(L, -2); // Lê a chave como uma string
            Log(LOG_INFO, "  %s", key);
        }
        lua_pop(L, 1); // Remove o valor, mas mantém a chave para a próxima iteração
    }
}

void PrintTopValue(lua_State *L)
{
    int n = lua_gettop(L); // Número de parâmetros
    for (int i = 1; i <= n; i++)
    {
        const char *tname = lua_typename(L, lua_type(L, i));
        Log(LOG_INFO, "  %d: %s", i, tname);
    }
}