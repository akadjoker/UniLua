/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Engine.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lrosa-do <lrosa-do@student.42lisboa>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/17 07:10:02 by lrosa-do          #+#    #+#             */
/*   Updated: 2023/04/23 19:39:31 by lrosa-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <raylib.h>
#include <lua.hpp>
#include "Utils.hpp"

#include <utility>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <math.h>
#include <unordered_map>
#include <raylib.h>
#include <lua.hpp>
#include <utility>
#include <memory>
#include <string>
#include <bitset>
#include <cstring>
#include <ctime>

#define MAX_OBJECTS_REMOVE_TO_COLECT 250

#define LuaPush_int(L, value) lua_pushinteger(L, value)
#define LuaPush_float(L, value) lua_pushnumber(L, value);
#define LuaPush_bool(L, value) lua_pushboolean(L, value)
#define LuaPush_string(L, value) lua_pushstring(L, value)

class Component;
using ComponentID = size_t;
ComponentID GetUniqueComponentID() noexcept;
template <typename T>
inline ComponentID GetComponentTypeID() noexcept
{
    static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");

    static ComponentID typeID{GetUniqueComponentID()};
    return typeID;
}

constexpr size_t maxComponents{8};

using ComponentBitset = std::bitset<maxComponents>;
using ComponentArray = std::array<Component *, maxComponents>;

class Scene;

class Timer
{
public:
    Timer() : startTime(GetTime()), lastFrameTime(0), deltaTime(0), pausedTime(0), paused(false)
    {
    }

    void update()
    {
        if (!paused)
        {
            double currentTime = GetTime();
            deltaTime = currentTime - lastFrameTime;
            lastFrameTime = currentTime;
        }
        else
        {
            deltaTime = 0;
        }
    }

    void start()
    {
        startTime = GetTime();
        lastFrameTime = 0;
        deltaTime = 0;
        pausedTime = 0;
        paused = false;
    }

    void reset()
    {
        startTime = GetTime();
        pausedTime = 0;
        paused = false;
    }
    void pause()
    {
        if (!paused)
        {
            pausedTime = GetTime() - lastFrameTime;
            paused = true;
        }
    }

    void resume()
    {
        if (paused)
        {
            lastFrameTime = GetTime() - pausedTime;
            paused = false;
        }
    }

    double getDeltaTime() const { return deltaTime; }

    double getElapsedTime() const
    {
        if (paused)
        {
            return pausedTime - startTime;
        }
        else
        {
            return GetTime() - startTime;
        }
    }

    bool isPaused() const { return paused; }

private:
    double startTime;
    double lastFrameTime;
    double deltaTime;
    double pausedTime;
    bool paused;
};

class Graph
{
public:
    Graph() : width(0), height(0)
    {
    }
    Graph(const Graph &other)
        : texture(other.texture), width(other.width), height(other.height)
    {
    }
    Graph(const char *filepath)
    {
        texture = LoadTexture(filepath);
        width = texture.width;
        height = texture.height;
        filename = filepath;
        //  Log(LOG_INFO, "Graph %s loaded %d %d ", filepath, width, height);
    }

    std::string filename;
    std::string key;
    Texture2D texture;
    int width;
    int height;
};

class Assets
{
public:
    static Assets &Instance()
    {
        static Assets instance;
        return instance;
    }

    Graph *getGraph(const std::string &key)
    {
        auto it = graphs.find(key);
        if (it != graphs.end())
        {
            return it->second;
        }
        Log(LOG_WARNING, "Graph %s not found", key.c_str());
        return nullptr;
    }

    bool hasGraph(const std::string &key)
    {
        auto it = graphs.find(key);
        if (it != graphs.end())
        {
            return true;
        }
        return false;
    }

    Graph *loadGraph(const std::string &key, const std::string &filepath)
    {
        Graph *graph = getGraph(key);
        if (graph != nullptr)
        {
            return graph;
        }

        if (FileExists(filepath.c_str()))
        {
            graph = new Graph(filepath.c_str());
        }
        else if (FileExists((std::string("assets/") + filepath).c_str()))
        {
            graph = new Graph((std::string("assets/") + filepath).c_str());
        }
        else if (FileExists((std::string("assets/images/") + filepath).c_str()))
        {
            graph = new Graph((std::string("assets/images/") + filepath).c_str());
        }
        else if (FileExists((std::string("assets/textures/") + filepath).c_str()))
        {
            graph = new Graph((std::string("assets/textures/") + filepath).c_str());
        }
        else if (FileExists((std::string("assets/levels/") + filepath).c_str()))
        {
            graph = new Graph((std::string("assets/levels/") + filepath).c_str());
        }
        else if (FileExists((std::string("../assets/levels/") + filepath).c_str()))
        {
            graph = new Graph((std::string("../assets/levels/") + filepath).c_str());
        }
        else if (FileExists((std::string("../assets/images/") + filepath).c_str()))
        {
            graph = new Graph((std::string("..assets/images/") + filepath).c_str());
        }
        else if (FileExists((std::string("../assets/textures/") + filepath).c_str()))
        {
            graph = new Graph((std::string("../assets/textures/") + filepath).c_str());
        }
        else
        {
            Log(LOG_ERROR, "Failed to load  image %s", filepath.c_str());
            return nullptr;
        }

        // if (FileExists(filepath.c_str()) == false)
        // {
        //     Log(LOG_WARNING, "File %s not found", filepath.c_str());
        //     return nullptr;
        // }

        graph->key = key;
        graphs[key] = graph;
        return graph;
    }

    void unloadGraph(const std::string &key)
    {
        auto it = graphs.find(key);
        if (it != graphs.end())
        {
            UnloadTexture(it->second->texture);
            graphs.erase(it);
        }
    }
    void clear()
    {
        for (auto &graph : graphs)
        {
            Log(LOG_WARNING, " Unload image  %s ", graph.second->filename.c_str());
            UnloadTexture(graph.second->texture);
            delete graph.second;
        }
        graphs.clear();
    }

    Assets() {}
    Assets(const Assets &) = delete;
    Assets &operator=(const Assets &) = delete;

    std::unordered_map<std::string, Graph *> graphs;
};

class ScriptComponent;
class GameObject;

class Component
{
public:
    GameObject *object;
    int table_ref;
    int depth;

    Component();
    virtual ~Component();

    virtual void BindLua(lua_State *L) { (void)L; };
    virtual void OnInit() {}
    virtual void OnUpdate(float delta) { (void)delta; }
    virtual void OnDraw() {}
    virtual void OnDebug() {}
    virtual void OnDestroy() {}
};

//*********************************************************************************************************************
//**                         ColiderComponent                                                                       **
//*********************************************************************************************************************

enum ColliderType
{
    Box,
    Circle
};

class CircleColiderComponent;
class BoxColiderComponent;

class ColideComponent : public Component
{
public:
    ColliderType type;
    virtual bool IsColide(ColideComponent *other) = 0;
    virtual void OnColide(ColideComponent *other) = 0;
    virtual Vector2 GetWorldPosition();
};

class BoxColiderComponent : public ColideComponent
{
public:
    Rectangle rect;
    BoxColiderComponent(float x, float y, float w, float h)
    {
        this->rect.x = x;
        this->rect.y = y;
        this->rect.width = w;
        this->rect.height = h;

        type = ColliderType::Box;
    }

    Vector2 GetWorldPosition() override;
    Rectangle GetWorldRect();

    void OnDebug() override;
    bool IsColide(ColideComponent *other) override;
    void OnColide(ColideComponent *other) override;
    void OnInit() override;
};

class CircleColiderComponent : public ColideComponent
{
public:
    Vector2 center;
    float radius;
    CircleColiderComponent(float x, float y, float radius)
    {
        this->center.x = x;
        this->center.y = y;
        this->radius = radius;
        type = ColliderType::Circle;
    }
    void OnInit() override;

    Vector2 GetWorldPosition() override;

    void OnDebug() override;
    bool IsColide(ColideComponent *other) override;
    void OnColide(ColideComponent *other) override;
};

//*********************************************************************************************************************
//**                         TransformComponent                                                                       **
//*********************************************************************************************************************

class TransformComponent
{
public:
    TransformComponent(GameObject *parent);
    ~TransformComponent();
    GameObject *object;
    Vec2 position;
    Vec2 scale;
    Vec2 pivot;
    Vec2 skew;
    float rotation;
    Vec2 m_PreviousPosition;
    Matrix2D transform;
    Matrix2D local_transform;
    Matrix2D wordl_transform;
    Matrix2D GetLocalTrasformation();
    Matrix2D GetWorldTransformation();

    void TurnTo(float x, float y, float speed, float angleDiff);
    void pointToMouse(float speed, float angleDiff);
};

class SpriteComponent : public Component
{

public:
    Graph *graph;
    Color color;
    bool FlipX;
    bool FlipY;
    Rectangle clip;
    std::string graphID;

    SpriteComponent(const std::string &fileName);
    void OnDraw() override;
    void OnDebug() override;
    void OnInit() override;

    void SetClip(Rectangle clip);
    void SetClip(float x, float y, float width, float height);

    void BindLua(lua_State *L) override;

private:
    bool isLoad;
};

struct Tileset
{
    std::string name;
    std::string imageSource;
    int firstgid;
    int tileWidth;
    int tileHeight;
    int spacing;
    int margin;
};

struct TileLayer
{
    std::vector<int> data;
    int width;
    int height;
    int x;
    int y;
    int opacity;
    bool visible;
    std::string type;
    std::string name;
};

class TileLayerComponent : public Component
{

public:
    Graph *graph;
    Tileset *tileset;
    std::vector<int> tileMap;
    std::string graphID;
    int tileWidth;
    int tileHeight;
    int spacing;
    int margin;
    int width;
    int height;
    int worldWidth;
    int worldHeight;

    TileLayerComponent(int width, int height, int tileWidth, int tileHeight, int spacing, int margin, const std::string &fileName);
    void OnDraw() override;
    void OnDebug() override;
    void OnInit() override;
    void BindLua(lua_State *L);

    void loadFromArray(const int *tiles);
    void loadFromCSVFile(const std::string &filename);
    void saveToCSVFile(const std::string &filename);

    void createSolids();

    std::string  getCSV() const;
    void loadFromString(const std::string &str,int shift);

    void clear();
    void addTile(int index);

    void setTile(int x, int y, int tile);
    int getTile(int x, int y);
    Rectangle getClip(int id);

    bool isWithinBounds(int x, int y) const
    {
        return x >= 0 && x < width && y >= 0 && y < height;
    }

    void PaintRectangle(int x, int y, int w, int h, int id);
    void PaintCircle(int x, int y, int radius, int id);

private:
    bool isLoad;
};

//*********************************************************************************************************************
//**                         ANIMATION                                                                              **
//*********************************************************************************************************************

enum AnimationMode
{
    Loop,
    PingPong,
    Stop,
    Once
};
class Animation
{
public:
    Graph *graph;

    int rows;
    int columns;
    int imageWidth;
    int imageHeight;

    int frameCount;
    int currentFrame;
    float frameDuration;
    float currentTime;
    bool isReversed;
    std::string graphID;
    

    
    Animation(const std::string &graphID, int rows, int columns, int frameCount, float frameDuration);

    Rectangle GetFrame();
    void Update(float deltaTime, AnimationMode mode);
};

class Animator : public Component
{
public:
    std::vector<std::pair<std::string, Animation *>> animations;
    std::string currentAnimation;
    std::string nextAnimation;
    bool isPlaying;

    AnimationMode mode;
    SpriteComponent *sprite;

    int frameCount;
    int currentFrame;
    float frameDuration;
    float currentTime;
    bool isReversed;
    bool isLoad;
    

    Animator();

    void OnDestroy() override;
    void OnDebug() override;
    void OnInit() override;
    void OnUpdate(float delta) override;
    Animation *GetAnimation();

    
    void Add(const std::string &name, const std::string &graph, int rows, int columns, int frameCount, float framesPerSecond);
    
    void AddAnimation(const std::string& name, Animation *animation);

    void SetMode(AnimationMode mode);
    void SetAnimation(const std::string &name, bool now = true);
    void Play();
    void Pause();
    void Stop();

    void BindLua(lua_State *L) override;

    bool IsPlaying() { return isPlaying; }
    int getFrameCount() { return frameCount; }
    int getCurrentFrame() { return currentFrame; }
    float getFrameDuration() { return frameDuration; }
    float getCurrentTime() { return currentTime; }
    bool getIsReversed() { return isReversed; }
    std::string getName() { return currentAnimation; }
};

//*********************************************************************************************************************
//**                         GameObject                                                                              **
//*********************************************************************************************************************

const int SHOW_ORIGIN = 1 << 1;     // origin do collider
const int SHOW_BOX = 1 << 2;        // box do collider
const int SHOW_BOUND = 1 << 3;      // final bound
const int SHOW_PIVOT = 1 << 4;      // pivot do transform
const int SHOW_TRANSFORM = 1 << 5;  // transform
const int SHOW_COMPONENTS = 1 << 6; // componentes que podem dezenhar algo
const int SHOW_ALL = SHOW_ORIGIN | SHOW_BOX | SHOW_BOUND | SHOW_PIVOT | SHOW_TRANSFORM | SHOW_COMPONENTS;

class GameObject
{
public:
    std::string name;
    std::string scriptName;
    unsigned long id;
    bool alive;
    bool visible;
    bool active;
    bool prefab;
    bool persistent;
    long debugMask;
    int layer;

    Scene *scene;

    // hit point
    int width;
    int height;
    int originX;
    int originY;
    bool collidable;
    bool pickable;

    bool solid;

    // transform box
    float x1, y1, x2, y2;
    bool bbReset;

    Rectangle bound;

    float radius;
    GameObject *parent;
    TransformComponent *transform;
    friend class ScriptComponent;
    ScriptComponent *script;
    int table_ref;

    Vec2 word_position;
    std::vector<GameObject *> children;

    GameObject();
    GameObject(const std::string &Name);
    GameObject(const std::string &Name, int depth);

    virtual ~GameObject();

    void Update(float dt);
    void Render();
    void Debug();
    void OnReady();
    void OnPause();
    void OnRemove();
    void OnCollision(GameObject *other);

    void sendMensageAll();
    void sendMensageTo(const std::string &name);
    void setDebug(int mask);
    void LiveReload();
    void UpdateWorld();

    Vec2 GetWorldPoint(float _x, float _y);
    Vec2 GetWorldPoint(Vec2 p);
    Vec2 GetLocalPoint(Vec2 p);
    Vec2 GetLocalPoint(float x, float y);

    bool collideWith(GameObject *e, float x, float y);
    bool place_free(float x, float y);
    bool place_meeting(float x, float y, const std::string &name);
    bool place_meeting_layer(float x, float y, int layer);

    void BindLua(lua_State *L);

    void centerPivot();
    void centerOrigin();

    void generateUniqueID() 
    {
        // std::hash<std::string> stringHash;
        // size_t imageFileHash = stringHash(spriteComponent.imageFile);
        // size_t xHash = std::hash<int>()(spriteComponent.x);
        // size_t yHash = std::hash<int>()(spriteComponent.y);

        // id = imageFileHash ^ xHash ^ yHash;
    }

    

    float GetWorldAngle()
    {
        if (parent)
            return parent->transform->rotation - transform->rotation;
        return transform->rotation;
    }
    void Encapsulate(float x, float y);
    GameObject *addChild(GameObject *e);

    float getWorldX() const
    {
        if (parent != nullptr)
        {
            return (parent->getWorldX() - transform->position.x);
        }
        return transform->position.x;
    }

    float getWorldY() const
    {

        if (parent != nullptr)
            return (parent->getWorldY() + transform->position.y);
        return transform->position.y;
    }
    float getWorldScaleX() const
    {
        if (parent != nullptr)
        {
            return (parent->getWorldScaleX() - transform->scale.x);
        }
        return transform->scale.x;
    }

    float getWorldScaleY() const
    {

        if (parent != nullptr)
            return (parent->getWorldScaleY() + transform->scale.y);
        return transform->scale.y;
    }

    float getWorldOriginX() const
    {
        if (parent != nullptr)
            return (parent->getWorldOriginX() + originX);
        return originX;
    }

    float getWorldOriginY() const
    {
        if (parent != nullptr)
            return (parent->getWorldOriginX() + originY);
        return originY;
    }

    float getRealX() const
    {
        return getWorldOriginX() + getWorldX() - getWorldOriginX();
    }
    float getRealY() const
    {
        return getWorldOriginY() + getWorldY() - getWorldOriginY();
    }

    float getPivotX()
    {
        return transform->pivot.x;
    }

    float getPivotY()
    {
        return transform->pivot.y;
    }
    float getX()
    {
        return transform->position.x;
    }

    float getY()
    {
        return transform->position.y;
    }

    AABB GetAABB() const
    {
        float aabbX = bound.x;
        float aabbY = bound.y;
        float aabbWidth = bound.width;
        float aabbHeight = bound.height;

        return AABB(aabbX, aabbY, aabbWidth, aabbHeight);
    }

    void createScript(const char *lua, lua_State *L);

    void AddScriptComponent(ScriptComponent *component)
    {
        script = component;
    }

    std::vector<Component *> getComponents() { return m_components; };
    
    template <typename T>
    bool HasComponent() const
    {
        return m_componentBitset[GetComponentTypeID<T>()];
    }

    template <typename T>
    T *GetComponent() const
    {
        if (!HasComponent<T>())
        {
            return NULL;
        }
        auto ptr(m_componentArray[GetComponentTypeID<T>()]);
        return static_cast<T *>(ptr);
    }

    template <typename T, typename... TArgs>
    T *AddComponent(TArgs &&...args)
    {
        T *c = AddComponentManualInit<T>(std::forward<TArgs>(args)...);
        c->OnInit();
        return c;
    }

    template <typename T, typename... TArgs>
    T *AddComponentManualInit(TArgs &&...args)
    {
        if (HasComponent<T>())
        {
            return GetComponent<T>();
        }

        Component *c(new T(std::forward<TArgs>(args)...));
        c->object = this;

        m_components.emplace_back(std::move(c));

        m_componentArray[GetComponentTypeID<T>()] = c;
        m_componentBitset[GetComponentTypeID<T>()] = true;

        return static_cast<T *>(c);
    }

    template <typename T>
    void RemoveComponent()
    {
        if (!HasComponent<T>())
        {
            return;
        }

        m_componentArray[GetComponentTypeID<T>()]->OnDestroy();

        m_components.erase(
            std::remove_if(std::begin(m_components), std::end(m_components),
                           [this](const Component *component)
                           {
                               return m_componentArray[GetComponentTypeID<T>()] == component;
                           }),
            std::end(m_components));

        m_componentArray[GetComponentTypeID<T>()] = NULL;
        m_componentBitset[GetComponentTypeID<T>()] = false;
    }

protected:
    std::vector<Component *> m_components;

    ComponentArray m_componentArray;
    ComponentBitset m_componentBitset;
    float _x;
    float _y;
};

class ScriptComponent
{
public:
    friend class GameObject;
    std::unordered_map<std::string, int> luaFunctions;
    bool callOnReadyDone;
    int script_ref;

    std::string script;
    GameObject *gameObject;
    lua_State *state;
    bool panic;
    bool watch;
    long timeLoad;

    ScriptComponent(GameObject *gameObject, const char *lua, lua_State *L);
    virtual ~ScriptComponent();
    ScriptComponent(const ScriptComponent &other)
        : script(other.script)
    {
    }
    void callOnReady();
    void callOnRemove();
    void callOnPause();
    void callOnUpdate(float dt);
    void callOnRender();
    void callOnCollide(GameObject *other);
    void callOnMessage();

    void callOnAnimationFrame(int frame, const std::string &name);
    void callOnAnimationEnd(const std::string &name);
    void callOnAnimationStart(const std::string &name);

    void registerFunction(const char *functionName);
    bool isFunctionRegistered(const char *functionName);

    bool Reload();
    void LuaBind();
};

//*********************************************************************************************************************
//**                         Scene                                                                                   **
//*********************************************************************************************************************

class QuadtreeNode
{
public:
    static const int MAX_OBJECTS = 4;
    static const int MAX_LEVELS = 5;

    QuadtreeNode(int level, const AABB &bounds);
    ~QuadtreeNode();
    void split();

    int getIndex(const AABB &aabb) const;
    void insert(GameObject *obj);
    void remove(GameObject *obj);
    void draw();
    void retrieve(std::vector<GameObject *> &returnObjects, const Vec2 &point);

    void retrieve(std::vector<GameObject *> &returnObjects, const AABB &queryAABB);

    void retrieve(const Vec2 &center, float radius, std::vector<GameObject *> &resultObjects) const;

    int countObjects() const;
    void clear();

    int level;
    AABB bounds;
    std::vector<GameObject *> objects;
    QuadtreeNode *children[4];
};

class Quadtree
{
public:
    QuadtreeNode *root;

    Quadtree(float x, float y, float width, float height);

    ~Quadtree();

    void insert(GameObject *obj);

    void remove(GameObject *obj);

    void draw();

    std::vector<GameObject *> getObjectsAtPoint(const Vec2 &point);

    std::vector<GameObject *> getObjectsInAABB(const AABB &queryAABB);

    std::vector<GameObject *> getObjectsInCircle(const Vec2 &center, float radius);

    int countObjects() const;

    void clear();
};

class Scene
{
public:
    enum TransformMode
    {
        None,
        Scale,
        Move,
        Rotate
    };

    Scene();
    virtual ~Scene();

    static Scene *Instance() { return m_instance; }
    /*
    remove all gameobjects from scene  free memory
    */
    void ClearAndFree();
    /*
    remove all gameobjects from scene not the persistent objects
    */
    void ClearScene();
    void LiveReload();

    // create layers
    int addLayer();
    int addLayers(int count);
    void addToLayer(GameObject *e);
    int layersCount();

    bool Load(const char *filename);
    bool Save(const char *filename);

    bool LoadTiled(const char *filename);

    GameObject *CirclePick(float x, float y, float radius);
    GameObject *MousePick();
    GameObject *RectanglePick(float x, float y, float w, float h);

    void Editor();
    void SetCamera(float x, float y);
    void SetPositionCamera(float x, float y);

    void AddGameObject(GameObject *gameObject)
    {
        gameObject->scene = this;
        gameObject->Render();
        gameObjects.push_back(gameObject);
        addToLayer(gameObject);
    }

    void AddQueueObject(GameObject *gameObject)
    {
        gameObject->scene = this;
        
        gameObjectsToAdd.push_back(gameObject);
    }

    void RemoveGameObject(GameObject *gameObject)
    {
        gameObjectsToRemove.push_back(gameObject);
    }

    GameObject *GetGameObjectByName(const std::string &name);
    bool inView(const  Rectangle& r );

    void Update();
    void Render();
    void Collision();

    void Init(const std::string &title, float fps, int windowWidth, int windowHeight, bool fullscreen);
    void SetWorld(float width, float height);
    void SetBackground(int r, int g, int b);

    bool place_meeting(GameObject *obj, float x, float y, const std::string &name);
    bool place_meeting_layer(GameObject *obj, float x, float y, int layer);
    bool place_free(GameObject *obj, float x, float y);

    std::vector<GameObject *> gameObjects;
    std::vector<GameObject *> gameObjectsToRemove;
    std::vector<GameObject *> gameObjectsToAdd;
    std::vector<GameObject *> gameObjectsZoombies; // objects sem pai e sem estar na scene
    std::map<int, std::vector<GameObject *>> layers;
    int m_num_layers;

    int numObjectsRemoved;
    bool needSort;
    bool enableLiveReload;
    bool enableCollisions;
    bool showDebug;
    bool enableEditor;
    bool showStats;
    int objectRender;
    
 
    Camera2D camera;
    Rectangle cameraBounds;
    Rectangle cameraView;
    Vector2   cameraPoint; 
    Timer timer;
    std::time_t lastCheckTime;
    std::time_t checkInterval;
    TransformMode currentMode;
    GameObject *selectedObject;
    Vec2 initialObjectPosition;
    Vec2 initialMousePosition;
    Vec2 prevMousePos;
    Vec2 windowSize;
    Vec2 worldSize;
    float fps;
    std::string title;
    Color background;
    bool fullscreen;
    static Scene *m_instance;
};

//*********************************************************************************************************************
//**                         Canvas                                                                                  **
//*********************************************************************************************************************
