#include "Engine.hpp"
#include "wrapper.hpp"
#include <chrono>
#include <string>
#include <sstream>
#include "nlohmann/json.hpp"
using json = nlohmann::json;

ComponentID GetUniqueComponentID() noexcept
{
    static ComponentID lastID{0u};
    return lastID++;
}

//*********************************************************************************************************************
//**                         TransformComponent                                                                              **
//*********************************************************************************************************************

TransformComponent::TransformComponent(GameObject *parent) : object(parent)
{
    // Log(LOG_INFO, "TransformComponent::TransformComponent()");
    this->rotation = 0;
    this->position = Vec2(0.0f);
    this->scale = Vec2(1.0f);
    this->skew = Vec2(0.0f);
    transform.Identity();
}

TransformComponent::~TransformComponent()
{
    // Log(LOG_INFO, "TransformComponent::~TransformComponent()");
    object = nullptr;
}

void TransformComponent::pointToMouse(float speed, float angleDiff)
{
    Vector2 v = GetMousePosition();
    TurnTo(v.x, v.y, speed, angleDiff);
}

void TransformComponent::TurnTo(float x, float y, float speed, float angleDiff)
{

    rotation = lerpAngleDegrees(rotation, getAngle(position.x, position.y, x, y) + angleDiff, speed);
}

Matrix2D TransformComponent::GetLocalTrasformation()
{

    local_transform.Identity();
    if (skew.x == 0.0f && skew.y == 0.0f)
    {

        if (rotation == 0.0)
        {

            local_transform.Set(scale.x, 0.0, 0.0, scale.y, position.x - pivot.x * scale.x, position.y - pivot.y * scale.y);
        }
        else
        {
            float acos = cos(rotation * RAD);
            float asin = sin(rotation * RAD);
            float a = scale.x * acos;
            float b = scale.x * asin;
            float c = scale.y * -asin;
            float d = scale.y * acos;
            float tx = position.x - pivot.x * a - pivot.y * c;
            float ty = position.y - pivot.x * b - pivot.y * d;

            local_transform.Set(a, b, c, d, tx, ty);
        }
    }
    else
    {

        local_transform.Identity();
        local_transform.Scale(scale.x, scale.y);
        local_transform.Skew(skew.x, skew.y);
        local_transform.Rotate(rotation);
        local_transform.Translate(position.x, position.y);

        if (pivot.x != 0.0f || pivot.y != 0.0f)
        {

            local_transform.tx = position.x - local_transform.a * pivot.x - local_transform.c * pivot.y;
            local_transform.ty = position.y - local_transform.b * pivot.x - local_transform.d * pivot.y;
        }
    }

    return local_transform;
}

Matrix2D Matrix2DMult(const Matrix2D curr, const Matrix2D m)
{

    Matrix2D result;

    result.a = curr.a * m.a + curr.b * m.c;
    result.b = curr.a * m.b + curr.b * m.d;
    result.c = curr.c * m.a + curr.d * m.c;
    result.d = curr.c * m.b + curr.d * m.d;

    result.tx = curr.tx * m.a + curr.ty * m.c + m.tx;
    result.ty = curr.tx * m.b + curr.ty * m.d + m.ty;

    return result;
}

Matrix2D TransformComponent::GetWorldTransformation()
{

    local_transform = GetLocalTrasformation();
    if (object->parent != nullptr)
    {
        Matrix2D mat = object->parent->transform->GetWorldTransformation();
        wordl_transform = Matrix2DMult(local_transform, mat);
        return wordl_transform;
    }
    return local_transform;
}

//*********************************************************************************************************************
//**                         Component                                                                              **
//*********************************************************************************************************************
Component::Component()
{
    // Log(LOG_INFO, "Component Created");
    table_ref = LUA_NOREF;
    object = nullptr;
    depth = 0;
}
Component::~Component()
{
    if (table_ref != LUA_NOREF)
        luaL_unref(getState(), LUA_REGISTRYINDEX, table_ref);

    //  Log(LOG_INFO, "Component Destroyed");
}

//*********************************************************************************************************************
//**                         SpriteComponent                                                                              **
//*********************************************************************************************************************

SpriteComponent::SpriteComponent(const std::string &fileName) : Component()
{
    depth = 1;
    this->color = WHITE;
    FlipX = false;
    FlipY = false;
    clip.x = 0;
    clip.y = 0;
    clip.width = 1;
    clip.height = 1;
    graphID = fileName;

    graph = Assets::Instance().getGraph(fileName);
    if (graph)
    {
        clip.x = 0;
        clip.y = 0;
        clip.width = graph->width;
        clip.height = graph->height;
    }
}

void SpriteComponent::OnInit()
{
    object->centerOrigin();
    object->centerPivot();
}

void SpriteComponent::OnDebug()
{
    // if (object->parent)
    //     DrawText(TextFormat("%s : %s", object->name.c_str(), object->parent->name.c_str()), (int)object->getWorldX(), (int)object->getWorldY(), 5, RED);
    // else
    //     DrawText(TextFormat(" %s ", object->name.c_str()), (int)object->getWorldX(), (int)object->getWorldY(), 5, RED);

    // int x= (int)object->getWorldX() + clip.x - object->getPivotX();
    // int y= (int)object->getWorldY() + clip.y - object->getPivotY();

    //  DrawRectangleLines(x,y,clip.width,clip.height, BLUE);
}

void SpriteComponent::OnDraw()
{
    //  Log(LOG_INFO, "SpriteComponent::OnDraw");

    Matrix2D mat = object->transform->GetWorldTransformation();

    if (graph)
    {
        //  RenderTransformFlip(graph->texture, clip, FlipX, FlipY, color, &mat, 0);
        RenderTransformFlipClip(graph->texture, clip.width, clip.height, clip, FlipX, FlipY, color, &mat, 0);
    }
    else
    {

        DrawCircleLines((int)object->getX(), (int)object->getY(), 1, RED);
        //   Log(LOG_ERROR, "SpriteComponent::OnDraw  %s %f %f ",object->name.c_str() , (int)object->getX(), (int)object->getY());
    }
}

void SpriteComponent::SetClip(float x, float y, float width, float height)
{
    clip.x = x;
    clip.y = y;
    clip.width = width;
    clip.height = height;
}
void SpriteComponent::SetClip(Rectangle c)
{
    clip.x = c.x;
    clip.y = c.y;
    clip.width = c.width;
    clip.height = c.height;
}

namespace SpriteBind
{

    int SetSpriteColor(lua_State *L)
    {
        SpriteComponent *sprite = nullptr;
        if (lua_gettop(L) != 5)
        {
            Log(LOG_ERROR, "setSpriteColor function requires 4 arguments");
            return 0;
        }

        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "SpriteComponent");
            sprite = static_cast<SpriteComponent *>(lua_touserdata(L, -1));
            lua_pop(L, 1); // Remova o ponteiro do GameObject da pilha
        }
        else
        {
            Log(LOG_ERROR, "[setSpriteColor] argument is not a table");
            return 0;
        }

        if (sprite != nullptr)
        {
            int r = lua_tointeger(L, 2);
            int g = lua_tointeger(L, 3);
            int b = lua_tointeger(L, 4);
            int a = lua_tointeger(L, 5);
            sprite->color.r = r;
            sprite->color.g = g;
            sprite->color.b = b;
            sprite->color.a = a;
        }

        return 0;
    }

}
void SpriteComponent::BindLua(lua_State *L)
{

    using namespace SpriteBind;
    lua_newtable(L);                              // Cria uma nova tabela vazia
    table_ref = luaL_ref(L, LUA_REGISTRYINDEX);   // Armazena o referenciador da tabela no registro do Lua
    lua_rawgeti(L, LUA_REGISTRYINDEX, table_ref); // Recupera a tabela do objeto
    lua_pushlightuserdata(L, this);               // Empilha o ponteiro do GameObject
    lua_setfield(L, -2, "SpriteComponent");

    lua_pushstring(L, graphID.c_str());
    lua_setfield(L, -2, "graph");

    lua_pushcfunction(L, &SetSpriteColor);
    lua_setfield(L, -2, "setColor");
    lua_pop(L, 1);
    lua_rawgeti(L, LUA_REGISTRYINDEX, table_ref);
}

TileLayerComponent::TileLayerComponent(int width, int height, int tileWidth, int tileHeight, int spacing, int margin, const std::string &fileName) : tileWidth(tileWidth), tileHeight(tileHeight), spacing(spacing), margin(margin), width(width), height(height)
{

    graph = Assets::Instance().getGraph(fileName.c_str());
    if (!graph)
    {
        Log(LOG_ERROR, "TileLayerComponent::TileLayerComponent  %s ", fileName.c_str());
        isLoad = false;
    }
    isLoad = true;
    graphID = fileName;
    worldWidth = width * tileWidth;
    worldHeight = height * tileHeight;
    // tileMap.reserve(width * height + 1);
    for (int i = 0; i < width * height; i++)
    {
        tileMap.push_back(-1);
    }
}

namespace TileBind
{

    int SetTile(lua_State *L)
    {
        TileLayerComponent *tile = nullptr;
        if (lua_gettop(L) != 4)
        {
            Log(LOG_ERROR, "setTile function requires 3 arguments");
            return 0;
        }

        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "TileComponent");
            tile = static_cast<TileLayerComponent *>(lua_touserdata(L, -1));
            lua_pop(L, 1); // Remova o ponteiro do GameObject da pilha
        }
        else
        {
            Log(LOG_ERROR, "[setTile] argument is not a table");
            return 0;
        }

        if (tile != nullptr)
        {
            int x = lua_tointeger(L, 2);
            int y = lua_tointeger(L, 3);
            int tileID = lua_tointeger(L, 4);
            tile->setTile(x, y, tileID);
        }

        return 0;
    }

    int GetTile(lua_State *L)
    {
        TileLayerComponent *tile = nullptr;
        if (lua_gettop(L) != 3)
        {
            Log(LOG_ERROR, "getTile function requires 2 arguments");
            return 0;
        }

        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "TileComponent");
            tile = static_cast<TileLayerComponent *>(lua_touserdata(L, -1));
            lua_pop(L, 1); // Remova o ponteiro do GameObject da pilha
        }
        else
        {
            Log(LOG_ERROR, "[getTile] argument is not a table");
            return 0;
        }

        if (tile != nullptr)
        {
            int x = lua_tointeger(L, 2);
            int y = lua_tointeger(L, 3);
            int tileID = tile->getTile(x, y);
            lua_pushinteger(L, tileID);
            return 1;
        }

        return 0;
    }

    int LoadStringTiles(lua_State *L)
    {
        TileLayerComponent *tile = nullptr;
        if (lua_gettop(L) != 2 && lua_gettop(L) != 3)
        {
            Log(LOG_ERROR, "loadStringTiles function requires 1/2 arguments");
            return 0;
        }

        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "TileComponent");
            tile = static_cast<TileLayerComponent *>(lua_touserdata(L, -1));
            lua_pop(L, 1); // Remova o ponteiro do GameObject da pilha
        }
        else
        {
            Log(LOG_ERROR, "[loadStringTiles] argument is not a table");
            return 0;
        }

        if (tile != nullptr)
        {
            const char *str = lua_tostring(L, 2);
            if (str==nullptr)
            {
                Log(LOG_ERROR, "[loadStringTiles] argument is not a string");
                return 0;
            }
            int shift = 0;
            if (lua_gettop(L) == 3)
            {
                shift = lua_tointeger(L, 3);
            }
            tile->loadFromString(str,shift);
        }

        return 0;
    }

    int BuildSolids(lua_State *L)
    {
        TileLayerComponent *tile = nullptr;
        if (lua_gettop(L) != 1)
        {
            Log(LOG_ERROR, "buildSolids function requires GameObject argument");
            return 0;
        }

        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "TileComponent");
            tile = static_cast<TileLayerComponent *>(lua_touserdata(L, -1));
            lua_pop(L, 1); 
        }
        else
        {
            Log(LOG_ERROR, "[buildSolids] argument is not a table");
            return 0;
        }

        if (tile != nullptr)
        {
            
            tile->createSolids();
        }

        return 0;
    }


}
void TileLayerComponent::BindLua(lua_State *L)
{

    using namespace TileBind;
    lua_newtable(L);                              // Cria uma nova tabela vazia
    table_ref = luaL_ref(L, LUA_REGISTRYINDEX);   // Armazena o referenciador da tabela no registro do Lua
    lua_rawgeti(L, LUA_REGISTRYINDEX, table_ref); // Recupera a tabela do objeto
    lua_pushlightuserdata(L, this);               // Empilha o ponteiro do GameObject
    lua_setfield(L, -2, "TileComponent");

    lua_pushcfunction(L, &SetTile);
    lua_setfield(L, -2, "setTile");

    lua_pushcfunction(L, &GetTile);
    lua_setfield(L, -2, "getTile");

    lua_pushcfunction(L, &LoadStringTiles);
    lua_setfield(L, -2, "loadString");

    lua_pushcfunction(L, &BuildSolids);
    lua_setfield(L, -2, "buildSolids");

    // lua_pushstring(L, graphID.c_str());
    // lua_setfield(L, -2, "graph");

    // lua_pushcfunction(L, &SetSpriteColor);
    // lua_setfield(L, -2, "setColor");

    lua_pop(L, 1);
    lua_rawgeti(L, LUA_REGISTRYINDEX, table_ref);
}

void TileLayerComponent::PaintRectangle(int x, int y, int w, int h, int id)
{
    if (!isLoad || !isWithinBounds(x, y))
        return;
    for (int i = x; i < x + w; ++i)
    {
        for (int j = y; j < y + h; ++j)
        {
            setTile(i, j, id);
        }
    }
}

void TileLayerComponent::PaintCircle(int x, int y, int radius, int id)
{
    if (!isLoad || !isWithinBounds(x, y))
        return;

    int rsq = radius * radius;
    for (int i = x - radius; i <= x + radius; ++i)
    {
        for (int j = y - radius; j <= y + radius; ++j)
        {
            int dx = i - x;
            int dy = j - y;
            if (dx * dx + dy * dy <= rsq)
            {
                setTile(i, j, id);
            }
        }
    }
}
void TileLayerComponent::OnInit()
{
            object->originX=0;
            object->originY=0;
            object->width=width*tileWidth;
            object->height=height*tileHeight;
            object->bound.x=0;
            object->bound.y=0;
            object->bound.width=width*tileWidth;
            object->bound.height=height*tileHeight;
           // object->setDebug( SHOW_BOX |  SHOW_BOUND );

}

void TileLayerComponent::OnDebug()
{
    Log(LOG_INFO, "TileLayerComponent::OnDebug");

    if (!isLoad)
        return;

    DrawRectangle(0,0,width*tileWidth,height*tileHeight, RED);
}

void TileLayerComponent::OnDraw()
{
    //  Log(LOG_INFO, "TileLayerComponent::OnDraw");
    if (!isLoad || !graph)
        return;
    if (width == 0 || height == 0 || tileWidth == 0 || tileHeight == 0)
    {
        Log(LOG_ERROR, "TileLayerComponent::OnDraw %d %d  %d %d", width, height, tileWidth, tileHeight);
        isLoad = false;
        return;
    }

    Scene *scene = Scene::Instance();


    // auto startTime = std::chrono::high_resolution_clock::now();

    


//loop in view
float zoom = scene->camera.zoom;
Vector2 offset = scene->camera.offset;
Vector2 target = scene->camera.target;
Rectangle cameraView = {
    -offset.x/zoom + target.x - (scene->windowSize.x/2.0f/zoom),
    -offset.y/zoom + target.y - (scene->windowSize.y/2.0f/zoom),
    (float)scene->windowSize.x/zoom + (offset.x/zoom),
    (float)scene->windowSize.y/zoom + (offset.y/zoom)
};

int startX = (int)(cameraView.x / tileWidth);
int startY = (int)(cameraView.y / tileHeight);
int endX = (int)((cameraView.x + cameraView.width) / tileWidth) + 1;
int endY = (int)((cameraView.y + cameraView.height) / tileHeight) + 1;

    startX = Clamp(startX, 0, width);
    startY = Clamp(startY, 0, height);
    endX = Clamp(endX, 0, width);
    endY = Clamp(endY, 0, height);

    for (int i = startY; i < endY; i++)
    {
        for (int j = startX; j < endX; j++)
        {
            float posX = (float)(j * tileWidth);
            float posY = (float)(i * tileHeight);
            Rectangle tileRect = {posX, posY, tileWidth, tileHeight};
            if (!scene->inView(tileRect))
                    continue;

            int tile = getTile(j, i);
            if (tile != -1)
            {

                RenderTile(graph->texture,
                           posX, posY,
                           tileWidth, tileHeight,
                           getClip(tile),
                           false, false, 0);
                    
            }
            // 736
        }
    }
//  auto endTime = std::chrono::high_resolution_clock::now();
// auto deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() / 1000000.0f;
   
//     Log(LOG_INFO, "loop in view %f",deltaTime);


// //loop all

//   startTime = std::chrono::high_resolution_clock::now();


//     for (int i = 0; i < height; i++)
//     {
//         for (int j = 0; j < width; j++)
//         {
//                float  posX=(float)(j * tileWidth );
//                float  posY=(float)(i * tileHeight);
//                Rectangle tileRect = {posX, posY, tileWidth, tileHeight};
//                if (!scene->inView(tileRect))
//                     continue;
                
                

//                      int tile= getTile(j,i) ;
//                      if (tile!=-1)
//                      {

//                         RenderTile(graph->texture,
//                                     posX,posY,
//                                     tileWidth,tileHeight,
//                                     getClip(tile),
//                                     false,false,0);
                        
//                     }
//                     //115 ;)

//         }
//     }
//  endTime = std::chrono::high_resolution_clock::now();
//  deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() / 1000000.0f;

//     Log(LOG_INFO, "loop all %f",deltaTime);




  

    //  //   Log(LOG_INFO, "TileLayerComponent::OnDraw %d %d  %d %d", width, height,tileWidth, tileHeight);
}

void TileLayerComponent::loadFromArray(const int *tiles)
{
    tileMap.clear();
    int arraSize = (tileWidth * tileHeight);
    for (int i = 0; i < arraSize; i++)
    {
        tileMap.push_back(tiles[i]);
    }
}
void TileLayerComponent::loadFromCSVFile(const std::string &filename)
{
    //    m_tileMap.clear();
    //    std::string tmp;
    //    char delim = ','; // Ddefine the delimiter to split by
    //    std::ifstream myFile(filename);
    //    std::getline(myFile,tmp,delim);
    //    while (std::getline(myFile,tmp,delim))
    //   {
    //      int index = std::stoi(tmp);
    //      m_tileMap.push_back(index);
    //    }

    if (!FileInPath(filename))
    {
        Log(LOG_ERROR, "The file  %s dont exists ", filename.c_str());
        return;
    }
    std::string path = GetPath(filename);
    char *text = LoadFileText(path.c_str());

    if (text == nullptr)
    {
        Log(LOG_ERROR, " Reading  %s", filename.c_str());
        return;
    }

    std::istringstream file(text);
    tileMap.clear();
    tileMap.reserve(width * height);

    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream lineStream(line);
        std::string cell;
        while (std::getline(lineStream, cell, ','))
        {
            int tile = std::stoi(cell);
            tileMap.push_back(tile);
        }
    }

    UnloadFileText(text);
}

void TileLayerComponent::loadFromString(const std::string &text,int shift)
{
    std::istringstream file(text);
    tileMap.clear();
    tileMap.reserve(width * height);

    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream lineStream(line);
        std::string cell;
        while (std::getline(lineStream, cell, ','))
        {
            int tile = std::stoi(cell) + shift;
            tileMap.push_back(tile);
        }
    }
}

std::string TileLayerComponent::getCSV() const
{
    std::ostringstream csvStream;
    for (size_t i = 0; i < tileMap.size(); ++i)
    {
        csvStream << tileMap[i];

        if (i < tileMap.size() - 1)
        {
            csvStream << ",";
        }
    }
    return csvStream.str();
}

void TileLayerComponent::saveToCSVFile(const std::string &filename)
{
    std::ostringstream file;

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            int tile = tileMap[y * width + x];
            file << tile;
            if (x < width - 1)
            {
                file << ",";
            }
        }
        file << "\n";
    }

    std::string fileContent = file.str();

    bool success = SaveFileText(filename.c_str(), const_cast<char *>(fileContent.c_str()));

    if (!success)
    {
        Log(LOG_ERROR, "Saving file: %s", filename.c_str());
    }
}

void TileLayerComponent::setTile(int x, int y, int tile)
{
    if (!isLoad || !isWithinBounds(x, y))
        return;

    int index = (int)(x + y * width);
    tileMap[index] = tile;
}
int TileLayerComponent::getTile(int x, int y)
{
    if (!isLoad || !isWithinBounds(x, y))
        return -1;
    int index = (int)(x + y * width);
    return tileMap[index];
}

void TileLayerComponent::createSolids()
{
     int count=0;
     for (int x = 0; x < width; x++)
        {
            for (int y = 0; y < height; y++)
            {
                int tile= getTile(x,y) ;
                if (tile!=-1)
                {
                    if (tile>=1)
                    {

                     GameObject *solid = new GameObject("solid",2);
                    solid->solid=true;
                    solid->prefab=true;
                    solid->transform->position.x=x*tileWidth,
                    solid->transform->position.y=y*tileHeight;

                    solid->width = tileWidth;
                    solid->height= tileHeight;

                    solid->originX=0;// (tileWidth/2) + (tileWidth/2)  +14;
                    solid->originY=0;// (tileHeight/2) + (tileHeight/2)+14;
                    solid->transform->pivot.x=0;
                    solid->transform->pivot.y=0;

                    solid->UpdateWorld();
              

     

                    
//                    solid->setDebug( SHOW_BOX );

                    Scene::Instance()->AddGameObject(solid);

                    }

                    count++;
                }
            }
        }
        
}

Rectangle TileLayerComponent::getClip(int id)
{
    Rectangle clip;
    if (!graph || !isLoad)
    {
        Log(LOG_ERROR, "TileLayerComponent::getClip - Graph is null");
        return clip;
    }

    int columns = (int)floor(graph->width / tileWidth);
    // int rows = (int)floor(graph->height / tileHeight);

    int pRow = id / columns;
    int pFrame = id % columns;

    float sourcex = margin + (spacing + tileWidth) * pFrame;
    float sourcey = margin + (spacing + tileHeight) * pRow;

    clip.x = sourcex;
    clip.y = sourcey;
    clip.width = tileWidth;
    clip.height = tileHeight;

    return clip;
}

void TileLayerComponent::clear()
{
    tileMap.clear();
}

void TileLayerComponent::addTile(int index)
{
    tileMap.push_back(index);
}

//*********************************************************************************************************************
//**                         ANIMATION                                                                              **
//*********************************************************************************************************************

Animation::Animation(const std::string &graphID, int rows, int columns, int frameCount, float frameDuration) : frameCount(frameCount),
                                                                                                               currentFrame(0),
                                                                                                               frameDuration(frameDuration),
                                                                                                               currentTime(0),
                                                                                                               isReversed(false), rows(rows),
                                                                                                               columns(columns)

{
    this->graphID = graphID;
    graph = Assets::Instance().getGraph(graphID);
    if (graph)
    {
        imageWidth = graph->texture.width;
        imageHeight = graph->texture.height;
        // Log(LOG_INFO, "Animation::Animation() : graphID %s %d %d", graphID.c_str(), imageWidth, imageHeight);
    }
    else
    {
        Log(LOG_ERROR, "Animation::Animation() : graphID not found %s", graphID.c_str());
    }
}

Rectangle Animation::GetFrame()
{
    Rectangle rect;
    if (graph == nullptr)
    {
        Log(LOG_ERROR, "Animation::GetFrame() : graph is null");
        return rect;
    }
    rect.width = imageWidth / columns;
    rect.height = imageHeight / rows;
    rect.x = (currentFrame % columns) * rect.width;
    rect.y = (currentFrame / columns) * rect.height;
    return rect;
}

void Animation::Update(float deltaTime, AnimationMode mode)
{
    if (!graph)
        return;
    currentTime += deltaTime;

    if (mode == AnimationMode::Loop)
    {
        while (currentTime >= frameDuration)
        {
            currentFrame = (currentFrame + 1) % frameCount;
            currentTime -= frameDuration;
        }
    }
    else if (mode == AnimationMode::PingPong)
    {
        int frameIndex = currentFrame;
        int frameIncrement = isReversed ? -1 : 1;

        while (currentTime >= frameDuration)
        {
            frameIndex += frameIncrement;

            if (frameIndex < 0)
            {
                frameIndex = 1;
                isReversed = false;
            }
            else if (frameIndex >= frameCount)
            {
                frameIndex = frameCount - 2;
                isReversed = true;
            }

            currentTime -= frameDuration;
        }

        currentFrame = frameIndex;
    }
    else if (mode == AnimationMode::Stop)
    {
        if (currentFrame < frameCount - 1)
        {
            while (currentTime >= frameDuration)
            {
                currentFrame++;
                currentTime -= frameDuration;
            }
        }
    }
    else if (mode == AnimationMode::Once)
    {
        if (currentFrame < frameCount - 1)
        {
            while (currentTime >= frameDuration)
            {
                currentFrame++;
                currentTime -= frameDuration;
            }
        }
    }
}

//*********************************************************************************************************************
//**                         ANIMATOR                                                                               **
//*********************************************************************************************************************

Animator::Animator() : Component(), currentAnimation(""), nextAnimation(""),
                       isPlaying(true),
                       mode(AnimationMode::Loop)
{
    sprite = nullptr;
    object = nullptr;
    currentFrame = 0;
    frameCount = 0;
    frameDuration = 0;
    currentTime = 0;
    isReversed = false;
    isLoad = true;
}

void Animator::OnDestroy()
{
    for (auto &pair : animations)
    {
        delete pair.second;
    }
    animations.clear();
}

namespace BindAnimator
{
    static int SetAnimation(lua_State *L)
    {

        if (lua_gettop(L) <= 1)
        {
            return luaL_error(L, "setAnimation function requires 1/2 arguments");
        }
        Animator *animator = nullptr;

        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "AnimatorComponent");
            animator = static_cast<Animator *>(lua_touserdata(L, -1));
            lua_pop(L, 1);

            if (animator == nullptr)
            {
                return luaL_error(L, "[setAnimation] animator is null");
            }

            const char *name = lua_tostring(L, 2);
            bool now = false;

            if (lua_gettop(L) == 3)
            {
                now = lua_toboolean(L, 3);
            }

            animator->SetAnimation(name, now);
        }
        else
        {
            return luaL_error(L, "setAnimation Invalid argument type, expected table");
        }

        return 0;
    }

    static int SetMode(lua_State *L)
    {

        if (lua_gettop(L) != 2)
        {
            return luaL_error(L, "setMode function requires 1 arguments");
        }
        Animator *animator = nullptr;

        if (lua_istable(L, 1))
        {

            lua_getfield(L, 1, "AnimatorComponent");
            animator = static_cast<Animator *>(lua_touserdata(L, -1));
            lua_pop(L, 1);

            if (animator == nullptr)
            {
                return luaL_error(L, "[setMode] animator is null");
            }

            int m = lua_tointeger(L, 2);
            AnimationMode mode = AnimationMode::Loop;
            if (m == 0)
            {
                mode = AnimationMode::Loop;
            }
            else if (m == 1)
            {
                mode = AnimationMode::PingPong;
            }
            else if (m == 2)
            {
                mode = AnimationMode::Stop;
            }
            else if (m == 3)
            {
                mode = AnimationMode::Once;
            }
            animator->SetMode(mode);
        }
        else
        {
            return luaL_error(L, "setMode Invalid argument type, expected table");
        }

        return 0;
    }

    static int Play(lua_State *L)
    {

        Animator *animator = nullptr;

        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "AnimatorComponent");
            animator = static_cast<Animator *>(lua_touserdata(L, -1));
            lua_pop(L, 1);

            if (animator == nullptr)
            {
                return luaL_error(L, "[play] animator is null");
            }

            animator->Play();
        }
        else
        {
            return luaL_error(L, "play Invalid argument type, expected table");
        }
        return 0;
    }

    static int Stop(lua_State *L)
    {

        Animator *animator = nullptr;

        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "AnimatorComponent");
            animator = static_cast<Animator *>(lua_touserdata(L, -1));
            lua_pop(L, 1);

            if (animator == nullptr)
            {
                return luaL_error(L, "[stop] animator is null");
            }

            animator->Stop();
        }
        else
        {
            return luaL_error(L, "stop Invalid argument type, expected table");
        }
        return 0;
    }

    static int Pause(lua_State *L)
    {

        Animator *animator = nullptr;

        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "AnimatorComponent");
            animator = static_cast<Animator *>(lua_touserdata(L, -1));
            lua_pop(L, 1);

            if (animator == nullptr)
            {
                return luaL_error(L, "[pause] animator is null");
            }

            animator->Pause();
        }
        else
        {
            return luaL_error(L, "pause Invalid argument type, expected table");
        }
        return 0;
    }

    static int Add(lua_State *L)
    {

        if (lua_gettop(L) != 7)
        {
            return luaL_error(L, "add animation function requires 7 arguments");
        }

        Animator *animator = nullptr;

        if (lua_istable(L, 1))
        {

            lua_getfield(L, 1, "AnimatorComponent");
            animator = static_cast<Animator *>(lua_touserdata(L, -1));
            lua_pop(L, 1);

            if (animator == nullptr)
            {
                return luaL_error(L, "[add] animator is null");
            }

            const char *name = lua_tostring(L, 2);
            const char *graph = lua_tostring(L, 3);
            int row = lua_tointeger(L, 4);
            int column = lua_tointeger(L, 5);
            int frameCount = lua_tointeger(L, 6);
            float framesPerSecond = lua_tonumber(L, 7);
            animator->Add(name, graph, row, column, frameCount, framesPerSecond);
        }
        else
        {
            return luaL_error(L, "add Invalid argument type, expected table");
        }
        return 0;
    }
    static int GetFrameCount(lua_State *L)
    {

        Animator *animator = nullptr;

        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "AnimatorComponent");
            animator = static_cast<Animator *>(lua_touserdata(L, -1));
            lua_pop(L, 1);

            if (animator == nullptr)
            {
                return luaL_error(L, "[getFrameCount] animator is null");
            }

            lua_pushinteger(L, animator->getFrameCount());
        }
        else
        {
            return luaL_error(L, "getFrameCount Invalid argument type, expected table");
        }
        return 1;
    }

    int IsPlaying(lua_State *L)
    {

        Animator *animator = nullptr;

        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "AnimatorComponent");
            animator = static_cast<Animator *>(lua_touserdata(L, -1));
            lua_pop(L, 1);

            if (animator == nullptr)
            {
                return luaL_error(L, "[isPlaying] animator is null");
            }

            lua_pushboolean(L, animator->IsPlaying());
        }
        else
        {
            return luaL_error(L, "isPlaying Invalid argument type, expected table");
        }
        return 1;
    }

    static int GetCurrentFrame(lua_State *L)
    {

        Animator *animator = nullptr;

        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "AnimatorComponent");
            animator = static_cast<Animator *>(lua_touserdata(L, -1));
            lua_pop(L, 1);

            if (animator == nullptr)
            {
                return luaL_error(L, "[getCurrentFrame] animator is null");
            }

            lua_pushinteger(L, animator->getCurrentFrame());
        }
        else
        {
            return luaL_error(L, "getCurrentFrame Invalid argument type, expected table");
        }
        return 1;
    }

    static int GetFrameDuration(lua_State *L)
    {

        Animator *animator = nullptr;

        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "AnimatorComponent");
            animator = static_cast<Animator *>(lua_touserdata(L, -1));
            lua_pop(L, 1);

            if (animator == nullptr)
            {
                return luaL_error(L, "[getFrameDuration] animator is null");
            }

            lua_pushnumber(L, animator->getFrameDuration());
        }
        else
        {
            return luaL_error(L, "getFrameDuration Invalid argument type, expected table");
        }
        return 1;
    }

    static int GetCurrentTime(lua_State *L)
    {

        Animator *animator = nullptr;

        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "AnimatorComponent");
            animator = static_cast<Animator *>(lua_touserdata(L, -1));
            lua_pop(L, 1);

            if (animator == nullptr)
            {
                return luaL_error(L, "[getCurrentTime] animator is null");
            }

            lua_pushnumber(L, animator->getCurrentTime());
        }
        else
        {
            return luaL_error(L, "getCurrentTime Invalid argument type, expected table");
        }
        return 1;
    }

    static int GetAnimationname(lua_State *L)
    {
        Animator *animator = nullptr;

        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "AnimatorComponent");
            animator = static_cast<Animator *>(lua_touserdata(L, -1));
            lua_pop(L, 1);

            if (animator == nullptr)
            {
                return luaL_error(L, "[getAnimation] animator is null");
            }

            lua_pushstring(L, animator->getName().c_str());
        }
        else
        {
            return luaL_error(L, "getAnimation Invalid argument type, expected table");
        }
        return 1;
    }

}

void Animator::BindLua(lua_State *L)
{

    lua_newtable(L);
    table_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    lua_rawgeti(L, LUA_REGISTRYINDEX, table_ref); // Recupera a tabela do objeto
    lua_pushlightuserdata(L, this);               // Empilha o ponteiro do GameObject
    lua_setfield(L, -2, "AnimatorComponent");

    lua_pushstring(L, object->name.c_str());
    lua_setfield(L, -2, "parent");

    lua_pushcfunction(L, &BindAnimator::Add);
    lua_setfield(L, -2, "add");

    lua_pushcfunction(L, &BindAnimator::Play);
    lua_setfield(L, -2, "play");

    lua_pushcfunction(L, &BindAnimator::Stop);
    lua_setfield(L, -2, "stop");

    lua_pushcfunction(L, &BindAnimator::Pause);
    lua_setfield(L, -2, "pause");

    lua_pushcfunction(L, &BindAnimator::SetAnimation);
    lua_setfield(L, -2, "setAnimation");

    lua_pushcfunction(L, &BindAnimator::SetMode);
    lua_setfield(L, -2, "setMode");

    lua_pushcfunction(L, &BindAnimator::GetFrameCount);
    lua_setfield(L, -2, "getFrameCount");

    lua_pushcfunction(L, &BindAnimator::IsPlaying);
    lua_setfield(L, -2, "isPlaying");

    lua_pushcfunction(L, &BindAnimator::GetCurrentFrame);
    lua_setfield(L, -2, "getCurrentFrame");

    lua_pushcfunction(L, &BindAnimator::GetFrameDuration);
    lua_setfield(L, -2, "getFrameDuration");

    lua_pushcfunction(L, &BindAnimator::GetCurrentTime);
    lua_setfield(L, -2, "getCurrentTime");

    lua_pushcfunction(L, &BindAnimator::GetAnimationname);
    lua_setfield(L, -2, "getName");

    lua_pop(L, 1);
    lua_rawgeti(L, LUA_REGISTRYINDEX, table_ref);
}

void Animator::OnUpdate(float deltaTime)
{
    if (animations.size() == 0 || !isLoad)
        return;

    if (!sprite)
    {
        sprite = object->GetComponent<SpriteComponent>();
        if (!sprite)
        {
            Log(LOG_ERROR, "Animator::OnUpdate() : SpriteComponent not found");
            isLoad = false;
            return;
        }
    }

    //   Log(LOG_INFO, "Animator::OnUpdate() : %s", object->name.c_str());

    if (isPlaying)
    {
        Animation *animation = GetAnimation();
        if (!animation)
            return;
        animation->Update(deltaTime, mode);

        currentFrame = animation->currentFrame;
        currentTime = animation->currentTime;
        frameCount = animation->frameCount;
        frameDuration = animation->frameDuration;
        isReversed = animation->isReversed;

        if (nextAnimation != "" && animation->currentFrame == animation->frameCount - 1)
        {
            currentAnimation = nextAnimation;
            nextAnimation = "";
            animation->currentFrame = 0;
            animation->currentTime = 0;
            Play();
        }
        // verifica se a animação atual terminou de ser reproduzida
        if (animation->currentFrame == animation->frameCount - 1)
        {
            if (mode == AnimationMode::Loop)
            {
                animation->currentFrame = 0; // reinicia a animação
                animation->currentTime = 0;
            }
            else if (mode == AnimationMode::PingPong)
            {
                animation->isReversed = true; // inverte a reprodução da animação
            }
        }

        // verifica se a animação atual terminou de ser reproduzida no modo PingPong
        if (animation->currentFrame == 0 && animation->isReversed)
        {
            if (mode == AnimationMode::PingPong)
            {
                animation->isReversed = false; // inverte a reprodução da animação de volta
            }
        }
    }

    Animation *animation = GetAnimation();
    if (!animation)
        return;

    if (sprite)
    {

        Rectangle frameRectangle = animation->GetFrame();
        sprite->clip.x = frameRectangle.x;
        sprite->clip.y = frameRectangle.y;
        sprite->clip.width = frameRectangle.width;
        sprite->clip.height = frameRectangle.height;
        sprite->graph = animation->graph;
    }
}

void Animator::Play()
{
    isPlaying = true;
}

void Animator::Pause()
{
    isPlaying = false;
}

void Animator::Stop()
{
    isPlaying = false;
    if (animations.size() == 0 || !isLoad)
        return;
    Animation *animation = GetAnimation();
    if (!animation)
        return;

    animation->currentFrame = 0;
    animation->currentTime = 0;
}

void Animator::SetAnimation(const std::string &name, bool now)
{
    if (animations.size() == 0 || !isLoad)
        return;
    if (now)
    {
        currentAnimation = name;
        nextAnimation = "";
        Play();
    }
    else if (currentAnimation != name)
    {
        if (nextAnimation == "")
        {
            nextAnimation = name; // marca a próxima animação a ser reproduzida
        }
        else
        {
            nextAnimation = name; // sobrescreve a próxima animação marcada
        }
    }
}
Animation *Animator::GetAnimation()
{
    auto it = std::find_if(animations.begin(), animations.end(),
                           [this](const std::pair<std::string, Animation *> &pair)
                           {
                               return pair.first == currentAnimation;
                           });
    if (it != animations.end())
    {
        return it->second;
    }
    else
    {

        return animations.front().second;
    }
}

void Animator::AddAnimation(const std::string &name, Animation *animation)
{
    animations.push_back(std::make_pair(name, animation));
    if (currentAnimation == "")
    {
        currentAnimation = name;
    }
    OnUpdate(0);
}

void Animator::Add(const std::string &name, const std::string &graph, int rows, int columns, int frameCount, float framesPerSecond)
{
    float frameDuration = 1.0f / framesPerSecond;
    Animation *animation = new Animation(graph, rows, columns, frameCount, frameDuration);
    AddAnimation(name, animation);
}

void Animator::SetMode(AnimationMode mode)
{
    this->mode = mode;
    if (animations.size() == 0 || !isLoad)
        return;
    Animation *animation = GetAnimation();
    if (!animation)
        return;
    animation->currentFrame = 0;
    animation->currentTime = 0;
    animation->isReversed = false;
}
void Animator::OnInit()
{
    if (object != nullptr)
    {
        if (object->HasComponent<SpriteComponent>())
        {
            sprite = object->GetComponent<SpriteComponent>();
        }
        else
        {
            Log(LOG_ERROR, "SpriteComponent::OnInit() : GameObject has no SpriteComponent");
            isLoad = false;
        }
    }
}

void Animator::OnDebug()
{
    // Log(LOG_INFO, "Animator::OnDebug()");
}

//*********************************************************************************************************************
//**                         GameObject                                                                              **
//*********************************************************************************************************************

static unsigned long NewGameObjectID()
{
    static unsigned long id = 0;
    return id++;
}

GameObject::GameObject() : name("GameObject"),
                           alive(true), visible(true), active(true),
                           layer(1), script(nullptr)
{
    // Log(LOG_INFO, "GameObject created");
    parent = nullptr;
    id = NewGameObjectID();
    transform = new TransformComponent(this);
    UpdateWorld();
    bound.x = 0;
    bound.y = 0;
    width = 1;
    height = 1;
    originX = 0;
    originY = 0;
    solid = false;
    prefab = false;
    persistent = false;
    collidable = true;
    pickable = false;

    word_position.x = transform->position.x;
    word_position.y = transform->position.y;

    layer = 0;
    scriptName = "";
    _x = 0;
    _y = 0;
    table_ref = LUA_NOREF;
    debugMask = 0;
}

GameObject::GameObject(const std::string &Name) : GameObject()
{

    name = Name;
}

GameObject::GameObject(const std::string &Name, int layer) : GameObject()
{
    this->layer = layer;
    name = Name;
}

void GameObject::OnReady()
{

    // Log(LOG_INFO, "GameObject::OnReady()");
    if (script != nullptr)
        script->callOnReady();
}
void GameObject::OnPause()
{
    if (script != nullptr)
        script->callOnPause();
}

void GameObject::OnRemove()
{
    if (script != nullptr)
        script->callOnRemove();
}

void GameObject::Encapsulate(float x, float y)
{
    if (bbReset)
    {
        x1 = x2 = x;
        y1 = y2 = y;
        bbReset = false;
    }
    else
    {
        if (x < x1)
            x1 = x;
        if (x > x2)
            x2 = x;
        if (y < y1)
            y1 = y;
        if (y > y2)
            y2 = y;

        bound.x = x1;
        bound.y = y1;
        bound.width = (x2 - x1);
        bound.height = (y2 - y1);
    }
}

void GameObject::LiveReload()
{
    if (script != nullptr)
    {
        long time = GetFileModTime(script->script.c_str());

        //   Log(LOG_INFO, "Script %s time now %d  old %d ", script->script.c_str(), time, script->timeLoad);

        if (script->timeLoad < time)
        {
            script->watch = true;
            Log(LOG_INFO, "Reloading script %s", script->script.c_str());
            script->Reload();
            //  script->timeLoad = GetFileModTime(script->script.c_str());
        }
    }
    for (auto &c : children)
    {
        c->LiveReload();
    }
}

Vector2 ApplyMatrixToPoint(const Matrix2D &matrix, float x, float y)
{
    Vector2 transformedPoint;
    transformedPoint.x = x * matrix.a + y * matrix.c + matrix.tx;
    transformedPoint.y = x * matrix.b + y * matrix.d + matrix.ty;

    return transformedPoint;
}

void GameObject::UpdateWorld()
{
    Matrix2D mat = transform->GetWorldTransformation();
    word_position = mat.TransformCoords();

    float w = width  *  transform->scale.x;
    float h = height *  transform->scale.y;
    radius = std::min(w, h) / 2.0f;

    bbReset = true;
    float newX = word_position.x;
    float newY = word_position.y;
    const auto tx1 = 0;
    const auto ty1 = 0;
    const auto tx2 = w;
    const auto ty2 = h;

    if (GetWorldAngle() != 0.0f)
    {

        const auto cost = cosf(-GetWorldAngle() * DEG2RAD);
        const auto sint = sinf(-GetWorldAngle() * DEG2RAD);

        Encapsulate(tx1 * cost - ty1 * sint + newX, tx1 * sint + ty1 * cost + newY);
        Encapsulate(tx2 * cost - ty1 * sint + newX, tx2 * sint + ty1 * cost + newY);
        Encapsulate(tx2 * cost - ty2 * sint + newX, tx2 * sint + ty2 * cost + newY);
        Encapsulate(tx1 * cost - ty2 * sint + newX, tx1 * sint + ty2 * cost + newY);
    }
    else
    {

        Encapsulate(tx1 + newX, ty1 + newY);
        Encapsulate(tx2 + newX, ty1 + newY);
        Encapsulate(tx2 + newX, ty2 + newY);
        Encapsulate(tx1 + newX, ty2 + newY);
    }

    for (auto &c : children)
    {
        c->UpdateWorld();
    }
}
void GameObject::OnCollision(GameObject *other)
{

    if (script != nullptr)
    {
        script->callOnCollide(other);
    }
    // Log(LOG_INFO, "OnColide %s with %s ", name.c_str(), other->name.c_str());
}

void GameObject::sendMensageAll()
{
    if (!scene)
        return;

    for (auto &c : scene->gameObjects)
    {
        if (c == this)
            continue;
        if (!c->alive)
            continue;
        if (c->script != nullptr)
        {
            c->script->callOnMessage();
        }
    }

    for (auto &c : children)
    {
        c->sendMensageAll();
    }
}

void GameObject::setDebug(int mask)
{
    debugMask = mask;
}

void GameObject::sendMensageTo(const std::string &name)
{
    if (!scene)
        return;

    for (auto &c : scene->gameObjects)
    {
        if (!c->alive)
            continue;
        if (c->name == name)
        {
            if (c->script != nullptr)
            {
                c->script->callOnMessage();
            }
        }
    }

    for (auto &c : children)
    {
        c->sendMensageTo(name);
    }
}

void GameObject::Update(float dt)
{
    if (script != nullptr)
    {
        if (!script->callOnReadyDone)
        {
            OnReady();
        }
    }
    if (solid)
        return;

    UpdateWorld();

    if (script != nullptr)
        script->callOnUpdate(dt);

    for (auto &c : m_components)
    {
        c->OnUpdate(dt);
    }

    for (auto &c : children)
    {
        c->Update(dt);
    }
}

bool GameObject::place_free(float x, float y)
{
    if (!scene)
        return true;
    return scene->place_free(this, x, y);
}

bool GameObject::place_meeting(float x, float y, const std::string &name)
{
    if (!scene)
        return false;
    return scene->place_meeting(this, x, y, name);
}

bool GameObject::place_meeting_layer(float x, float y, int layer)
{

    if (!scene)
        return false;
    return scene->place_meeting_layer(this, x, y, layer);
}

bool GameObject::collideWith(GameObject *e, float x, float y)
{
    if (!scene || !e)
        return false;
    if (e == this)
        return false;

    _x = this->transform->position.x;
    _y = this->transform->position.y;
    this->transform->position.x = x;
    this->transform->position.y = y;

    if (
        x - this->getWorldOriginX() + width > e->getWorldX() - e->getWorldOriginX() &&
        y - this->getWorldOriginY() + height > e->getWorldY() - e->getWorldOriginY() &&
        x - this->getWorldOriginX() < e->getWorldX() - e->getWorldOriginX() + e->width &&
        y - this->getWorldOriginY() < e->getWorldY() - e->getWorldOriginY() + e->height)
    {
        this->transform->position.x = _x;
        this->transform->position.y = _y;
        OnCollision(e);
        e->OnCollision(this);
        return true;
    }
    this->transform->position.x = _x;
    this->transform->position.y = _y;
    return false;
}

void GameObject::centerPivot()
{

    if (HasComponent<SpriteComponent>())
    {
        auto sprite = GetComponent<SpriteComponent>();
        transform->pivot.x = sprite->clip.width / 2.0f;
        transform->pivot.y = sprite->clip.height / 2.0;
        //  Log(LOG_INFO, "Pivot %s set to %f %f", name.c_str(), transform->pivot.x, transform->pivot.y);
        // Log(LOG_INFO, "Size %s set to %f %f", name.c_str(), sprite->clip.width, sprite->clip.height);
    }
    else
    {

        transform->pivot.x = (width / 2.0f);
        transform->pivot.y = (height / 2.0f);
        // Log(LOG_INFO, "Pivot %s set to %f %f", name.c_str(), transform->pivot.x, transform->pivot.y);
    }
}

void GameObject::centerOrigin()
{

    if (HasComponent<SpriteComponent>())
    {
        auto sprite = GetComponent<SpriteComponent>();
        width = (int)sprite->clip.width / 2 * transform->scale.x;
        height = (int)sprite->clip.height / 2 * transform->scale.y;
        originX = -(int)(width / 2.0f) + (width / 2.0f);
        originY = -(int)(height / 2.0f) + (height / 2.0f);
    }
    else
    {

        originX = (width / 2.0f);
        originY = (height / 2.0f);
        // Log(LOG_INFO, "Pivot %s set to %f %f", name.c_str(), transform->pivot.x, transform->pivot.y);
    }
}

void GameObject::Debug()
{

    if (!debugMask)
        return;

    bool isOriginEnabled = (debugMask & SHOW_ORIGIN) != 0;
    bool isBoxCollideEnabled = (debugMask & SHOW_BOX) != 0;

    float finalRad = radius / 4.0f;
    if (finalRad < 0.5f)
        finalRad = 0.5f;

    if (script != nullptr)
    {
        if (!script->callOnReadyDone)
            return;
    }

    int bX = (int)getX();
    int bY = (int)getY();

    int cx = bX + originX;
    int cy = bY + originY;
    int cw = width ;
    int ch = height ;

    if (isBoxCollideEnabled)
        DrawRectangleLines( cx,  cy, cw , ch , WHITE);
    if (isOriginEnabled)
        DrawCircle(cx, cy, finalRad, WHITE);

    if (!solid)
    {

        Vec2 p;
        if (!parent)
            p = GetLocalPoint(transform->pivot.x, transform->pivot.y);
        else
            p = GetWorldPoint(transform->pivot.x, transform->pivot.y);

        bool isPivotEnabled = (debugMask & SHOW_PIVOT) != 0;
        bool isTrasnformEnabled = (debugMask & SHOW_TRANSFORM) != 0;

        if (isPivotEnabled)
            DrawCircle((int)p.x, (int)p.y, finalRad, LIME);

        float newX = word_position.x;
        float newY = word_position.y;
        const auto tx1 = 0;
        const auto ty1 = 0;
        const auto tx2 = width  * 2.0f * transform->scale.x;
        const auto ty2 = height * 2.0f * transform->scale.y;

        if (isTrasnformEnabled)
        {
            if (GetWorldAngle() != 0.0f)
            {

                const auto cost = cosf(-GetWorldAngle() * DEG2RAD);
                const auto sint = sinf(-GetWorldAngle() * DEG2RAD);
                float x1 = tx1 * cost - ty1 * sint + newX;
                float y1 = tx1 * sint + ty1 * cost + newY;
                float x2 = tx2 * cost - ty1 * sint + newX;
                float y2 = tx2 * sint + ty1 * cost + newY;
                float x3 = tx2 * cost - ty2 * sint + newX;
                float y3 = tx2 * sint + ty2 * cost + newY;
                float x4 = tx1 * cost - ty2 * sint + newX;
                float y4 = tx1 * sint + ty2 * cost + newY;
                DrawLine(x1, y1, x2, y2, LIME);
                DrawLine(x1, y1, x4, y4, LIME);
                DrawLine(x3, y3, x4, y4, LIME);
                DrawLine(x2, y2, x3, y3, LIME);
            }
            else
            {
                DrawLine(tx1 + newX, ty1 + newY, tx2 + newX, ty1 + newY, LIME);
                DrawLine(tx1 + newX, ty1 + newY, tx1 + newX, ty2 + newY, LIME);
                DrawLine(tx2 + newX, ty2 + newY, tx1 + newX, ty2 + newY, LIME);
                DrawLine(tx2 + newX, ty2 + newY, tx2 + newX, ty1 + newY, LIME);
            }
        }
    }

    bool isComponentsEnable = (debugMask & SHOW_COMPONENTS) != 0;
    bool isBoundEnable = (debugMask & SHOW_BOUND) != 0;

    if (isBoundEnable)
        DrawRectangleLinesEx(bound, 1.5f, MAGENTA);

    if (isComponentsEnable)
    {
        for (auto &c : m_components)
        {
            c->OnDebug();
        }
    }

    for (auto &c : children)
    {
        c->Debug();
    }
}
void GameObject::Render()
{

    if (script != nullptr && !solid)
    {
        if (!script->callOnReadyDone)
            return;
     }


    for (auto &c : m_components)
    {
        c->OnDraw();
    }

    if (script != nullptr && !solid)
    {
        script->callOnRender();
    }
    for (auto &c : children)
    {
        c->Render();
    }

    // Log(LOG_INFO, "Render %s %f %f", name.c_str(),getX(),getY());
}

GameObject::~GameObject()
{
    // Log(LOG_INFO, "[CPP] GameObject (%s) destroyed", name.c_str());

    for (auto &c : m_components)
    {
        if (c)
        {
            c->OnDestroy();
            delete c;
        }
    }

    for (auto &c : children)
    {
        if (c)
        {
            delete c;
        }
    }
    children.clear();

    if (table_ref != LUA_NOREF)
    {
        luaL_unref(getState(), LUA_REGISTRYINDEX, table_ref);
    }

    if (script != nullptr)
    {
        delete script;
    }
}

Vec2 GameObject::GetWorldPoint(float _x, float _y)
{

    return transform->wordl_transform.TransformCoords(Vec2(_x, _y));
}

Vec2 GameObject::GetWorldPoint(Vec2 p)
{
    return transform->wordl_transform.TransformCoords(p);
}

Vec2 GameObject::GetLocalPoint(Vec2 p)
{
    return transform->local_transform.TransformCoords(p);
}

Vec2 GameObject::GetLocalPoint(float x, float y)
{
    return transform->local_transform.TransformCoords(Vec2(x, y));
}

void GameObject::createScript(const char *lua, lua_State *L)
{
    scriptName = lua;
    std::string path = "";

    if (table_ref == LUA_NOREF)
    {
        BindLua(L);
    }

    //
    if (FileExists(lua))
    {
        path = lua;
    }
    else if (FileExists(TextFormat("%s.lua", lua)))
    {
        path = TextFormat("%s.lua", lua);
    }
    else if (FileExists(TextFormat("assets/%s.lua", lua)))
    {
        path = TextFormat("assets/%s.lua", lua);
    }
    else if (FileExists(TextFormat("../assets/%s.lua", lua)))
    {
        path = TextFormat("../assets/%s.lua", lua);
    }
    else if (FileExists(TextFormat("assets/scripts/%s", lua)))
    {
        path = TextFormat("assets/scripts/%s", lua);
    }
    else if (FileExists(TextFormat("../assets/scripts/%s", lua)))
    {
        path = TextFormat("../assets/scripts/%s", lua);
    }
    else if (FileExists(TextFormat("assets/scripts/%s.lua", lua)))
    {
        path = TextFormat("../assets/scripts/%s.lua", lua);
    }
    else if (FileExists(TextFormat("../assets/scripts/%s.lua", lua)))
    {
        path = TextFormat("../assets/scripts/%s.lua", lua);
    }
    else
    {
        Log(LOG_ERROR, "Script %s not found", lua);
        return;
    }

    new ScriptComponent(this, path.c_str(), L);
}

GameObject *GameObject::addChild(GameObject *e)
{
    children.push_back(e);
    e->parent = this;
    return e;
}

namespace BinGameObject
{

    static int SetPosition(lua_State *L)
    {
        GameObject *gameObject = nullptr;
        float x = 0, y = 0;

        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1); // Remova o ponteiro do GameObject da pilha
            x = lua_tonumber(L, 2);
            y = lua_tonumber(L, 3);
        }
        else
        {
            return luaL_error(L, "[setPosition] The First argument must be a table");
        }

        if (gameObject == nullptr)
        {
            return luaL_error(L, "[setPosition] gameObject is null");
        }

        gameObject->transform->position.x = x;
        gameObject->transform->position.y = y;
        

        return 0;
    }

    static int PointToMouse(lua_State *L)
    {

        GameObject *gameObject = nullptr;
        float x = 0, y = 0;

        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1); // Remova o ponteiro do GameObject da pilha
            x = lua_tonumber(L, 2);
            y = lua_tonumber(L, 3);
        }
        else
        {
            return luaL_error(L, "[PointToMouse] The First argument must be a table");
        }

        if (gameObject == nullptr)
        {
            return luaL_error(L, "[PointToMouse] gameObject is null");
        }

        gameObject->transform->pointToMouse(x, y);

        return 0;
    }

    static int SetScale(lua_State *L)
    {
        GameObject *gameObject = nullptr;
        float x = 0, y = 0;

        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1); // Remova o ponteiro do GameObject da pilha
            x = lua_tonumber(L, 2);
            y = lua_tonumber(L, 3);
        }
        else
        {
            return luaL_error(L, "[setScale] The First argument must be a table");
        }

        if (gameObject == nullptr)
        {
            return luaL_error(L, "[setScale] gameObject is null");
        }

        gameObject->transform->scale.x = x;
        gameObject->transform->scale.y = y;

        return 0;
    }

    static int SetPivot(lua_State *L)
    {
        GameObject *gameObject = nullptr;
        float x = 0, y = 0;

        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1); // Remova o ponteiro do GameObject da pilha
            x = lua_tonumber(L, 2);
            y = lua_tonumber(L, 3);
        }
        else
        {
            return luaL_error(L, "[setPivot] The First argument must be a table");
        }
        if (gameObject == nullptr)
        {
            return luaL_error(L, "[setPivot] gameObject is null");
        }

        gameObject->transform->pivot.x = x; // (x * gameObject->transform->scale.x);
        gameObject->transform->pivot.y = y; // (y * gameObject->transform->scale.y);

        return 0;
    }

    static int SetOrigin(lua_State *L)
    {
        GameObject *gameObject = nullptr;
        float x = 0, y = 0;

        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1); // Remova o ponteiro do GameObject da pilha
            x = lua_tonumber(L, 2);
            y = lua_tonumber(L, 3);
        }
        else
        {
            return luaL_error(L, "[SetOrigin] The First argument must be a table");
        }

        if (gameObject == nullptr)
        {
            return luaL_error(L, "[SetPivot] gameObject is null");
        }

        gameObject->originX = x;
        gameObject->originY = y;

        return 0;
    }
    static int SetSize(lua_State *L)
    {
        GameObject *gameObject = nullptr;
        float x = 0, y = 0;

        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1); // Remova o ponteiro do GameObject da pilha
            x = lua_tonumber(L, 2);
            y = lua_tonumber(L, 3);
        }
        else
        {
            return luaL_error(L, "[SetSize] The First argument must be a table");
        }

        if (gameObject == nullptr)
        {
            return luaL_error(L, "[SetSize] gameObject is null");
        }

        gameObject->width = x;
        gameObject->height = y;

        return 0;
    }

    static int SetCenterPivot(lua_State *L)
    {
        GameObject *gameObject = nullptr;

        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1); // Remova o ponteiro do GameObject da pilha
        }
        else
        {
            return luaL_error(L, "[SetCenterPivot] The First argument must be a table");
        }

        if (gameObject == nullptr)
        {
            return luaL_error(L, "[SetCenterPivot] gameObject is null");
        }

        gameObject->centerPivot();

        return 0;
    }

    static int SetRotation(lua_State *L)
    {
        GameObject *gameObject = nullptr;
        float angle = 0;

        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1); // Remova o ponteiro do GameObject da pilha
            angle = lua_tonumber(L, 2);
        }
        else
        {
            return luaL_error(L, "[SetRotation] The First argument must be a table");
        }

        if (gameObject == nullptr)
        {
            return luaL_error(L, "[SetRotation] gameObject is null");
        }

        gameObject->transform->rotation = angle;

        return 0;
    }

    static int TurnTo(lua_State *L)
    {
        GameObject *gameObject = nullptr;
        float x, y, speed, angleDiff = 0;

        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1); // Remova o ponteiro do GameObject da pilha
            x = lua_tonumber(L, 2);
            y = lua_tonumber(L, 3);
            speed = lua_tonumber(L, 4);
            angleDiff = lua_tonumber(L, 5);
        }
        else
        {
            return luaL_error(L, "[TurnTo] The First argument must be a table");
        }

        if (gameObject == nullptr)
        {
            return luaL_error(L, "[TurnTo] gameObject is null");
        }

        gameObject->transform->TurnTo(x, y, speed, angleDiff);

        return 0;
    }


  static int FaceTo(lua_State *L)
    {
        GameObject *gameObject = nullptr;
        GameObject *otherObject = nullptr;

        

        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1); // Remova o ponteiro do GameObject da pilha
        }
        else
        {
            return luaL_error(L, "[FaceTo] The First argument must be a table");
        }

        if (lua_istable(L, 2))
        {
            lua_getfield(L, 2, "gameObject");
            otherObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 2); // Remova o ponteiro do GameObject da pilha
        }
        else
        {
            return luaL_error(L, "[FaceTo] The Second argument must be a table");
        }

        if (gameObject == nullptr )
        {
            return luaL_error(L, "[FaceTo] gameObject is null");
        }

        if (otherObject == nullptr )
        {
            return luaL_error(L, "[FaceTo] target is null");
        }

        gameObject->transform->rotation = getAngle(gameObject->transform->position.x, gameObject->transform->position.y, otherObject->transform->position.x, otherObject->transform->position.y);

        return 0;
    }

    static int GetWorldPosition(lua_State *L)
    {
        // return luaL_error(L, "[CPP] getX %d ", lua_gettop(L));
        GameObject *gameObject = nullptr;
        float x, y = 0;
        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1); // Remova o ponteiro do GameObject da pilha
            x = lua_tonumber(L, 2);
            y = lua_tonumber(L, 3);
        }
        else
        {
            return luaL_error(L, "[GetWorldPosition] The First argument must be a table");
        }

        if (gameObject == nullptr)
        {
            return luaL_error(L, "[GetWorldPosition] gameObject is null");
        }

        Vec2 v = gameObject->GetWorldPoint(x, y);

        lua_pushnumber(L, v.x);
        lua_pushnumber(L, v.y);

        return 2;
    }

    static int GetLocalPosition(lua_State *L)
    {
        // return luaL_error(L, "[CPP] getX %d ", lua_gettop(L));
        GameObject *gameObject = nullptr;
        float x, y = 0;
        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1); // Remova o ponteiro do GameObject da pilha
            x = lua_tonumber(L, 2);
            y = lua_tonumber(L, 3);
        }
        else
        {
            return luaL_error(L, "[GetLocalPosition] The First argument must be a table");
        }

        if (gameObject == nullptr)
        {
            return luaL_error(L, "[GetLocalPosition] gameObject is null");
        }

        Vec2 v = gameObject->GetLocalPoint(x, y);

        lua_pushnumber(L, v.x);
        lua_pushnumber(L, v.y);

        return 2;
    }

    static int GetPosition(lua_State *L)
    {
        // return luaL_error(L, "[CPP] getX %d ", lua_gettop(L));
        GameObject *gameObject = nullptr;
        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1); // Remova o ponteiro do GameObject da pilha
        }
        else
        {
            return luaL_error(L, "[GetPosition] The First argument must be a table");
        }

        if (gameObject == nullptr)
        {
            return luaL_error(L, "[GetPosition] gameObject is null");
        }

        float x, y;
        x = gameObject->transform->position.x;
        y = gameObject->transform->position.y;

        lua_pushnumber(L, x);
        lua_pushnumber(L, y);

        return 2;
    }

    static int GetScale(lua_State *L)
    {
        // return luaL_error(L, "[CPP] getX %d ", lua_gettop(L));
        GameObject *gameObject = nullptr;
        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1); // Remova o ponteiro do GameObject da pilha
        }
        else
        {
            return luaL_error(L, "[GetScale] The First argument must be a table");
        }

        if (gameObject == nullptr)
        {
            return luaL_error(L, "[GetScale] gameObject is null");
        }

        float x, y;
        x = gameObject->transform->scale.x;
        y = gameObject->transform->scale.y;

        lua_pushnumber(L, x);
        lua_pushnumber(L, y);

        return 2;
    }

    static int GetPivot(lua_State *L)
    {
        // return luaL_error(L, "[CPP] getX %d ", lua_gettop(L));
        GameObject *gameObject = nullptr;
        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1); // Remova o ponteiro do GameObject da pilha
        }
        else
        {
            return luaL_error(L, "[GetPivot] The First argument must be a table");
        }

        if (gameObject == nullptr)
        {
            return luaL_error(L, "[GetPivot] gameObject is null");
        }

        float x, y;
        x = gameObject->transform->pivot.x;
        y = gameObject->transform->pivot.y;

        lua_pushnumber(L, x);
        lua_pushnumber(L, y);

        return 2;
    }

    static int GetRotation(lua_State *L)
    {
        // return luaL_error(L, "[CPP] getX %d ", lua_gettop(L));
        GameObject *gameObject = nullptr;
        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1); // Remova o ponteiro do GameObject da pilha
        }
        else
        {
            return luaL_error(L, "[GetRotation] The First argument must be a table");
        }

        if (gameObject == nullptr)
        {
            return luaL_error(L, "[GetRotation] gameObject is null");
        }

        float x;
        x = gameObject->transform->rotation;

        lua_pushnumber(L, x);

        return 1;
    }

    static int GetWorldRotation(lua_State *L)
    {
        // return luaL_error(L, "[CPP] getX %d ", lua_gettop(L));
        GameObject *gameObject = nullptr;
        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1); // Remova o ponteiro do GameObject da pilha
        }
        else
        {
            return luaL_error(L, "[GetWorldRotation] The First argument must be a table");
        }

        if (gameObject == nullptr)
        {
            return luaL_error(L, "[GetWorldRotation] gameObject is null");
        }

        float x;
        x = gameObject->GetWorldAngle();

        lua_pushnumber(L, x);

        return 1;
    }

    static int GetX(lua_State *L)
    {

        GameObject *gameObject = nullptr;
        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1); // Remova o ponteiro do GameObject da pilha
        }
        else
        {
            return luaL_error(L, "[GetX] The First argument must be a table");
        }

        if (gameObject == nullptr)
        {
            return luaL_error(L, "[GetX] gameObject is null");
        }

        float x;
        x = gameObject->transform->position.x;

        lua_pushnumber(L, x);

        return 1;
    }

    static int GetY(lua_State *L)
    {
        GameObject *gameObject = nullptr;

        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1); // Remova o ponteiro do GameObject da pilha
        }
        else
        {
            return luaL_error(L, "[GetY] The First argument must be a table");
        }

        if (gameObject == nullptr)
        {
            return luaL_error(L, "[GetY] gameObject is null");
        }

        float y;
        y = gameObject->transform->position.y;

        lua_pushnumber(L, y);

        return 1;
    }

    static int GetWorldX(lua_State *L)
    {

        GameObject *gameObject = nullptr;
        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1); // Remova o ponteiro do GameObject da pilha
        }
        else
        {
            return luaL_error(L, "[GetWorldX] The First argument must be a table");
        }

        if (gameObject == nullptr)
        {
            return luaL_error(L, "[GetWorldX] gameObject is null");
        }

        float x = gameObject->getWorldX();

        lua_pushnumber(L, x);

        return 1;
    }

    static int GetWorldY(lua_State *L)
    {

        GameObject *gameObject = nullptr;
        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1); // Remova o ponteiro do GameObject da pilha
        }
        else
        {
            return luaL_error(L, "[GetWorldY] The First argument must be a table");
        }

        if (gameObject == nullptr)
        {
            return luaL_error(L, "[GetWorldY] gameObject is null");
        }

        float y = gameObject->getWorldY();

        lua_pushnumber(L, y);

        return 1;
    }

    static int Kill(lua_State *L)
    {
        GameObject *gameObject = nullptr;

        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1); // Remova o ponteiro do GameObject da pilha
        }
        else
        {
            return luaL_error(L, "kill Invalid argument type, expected table");
        }

        if (gameObject == nullptr)
        {
            return luaL_error(L, "[Kill] gameObject is null");
        }

        gameObject->alive = false;

        return 0;
    }

    static int CenterPivot(lua_State *L)
    {
        GameObject *gameObject = nullptr;

        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1); // Remova o ponteiro do GameObject da pilha
        }
        else
        {
            return luaL_error(L, "centerPivor Invalid argument type, expected table");
        }

        if (gameObject == nullptr)
        {
            return luaL_error(L, "[centerPivot] gameObject is null");
        }

        gameObject->centerPivot();

        return 0;
    }
    static int CenterOrigin(lua_State *L)
    {
        GameObject *gameObject = nullptr;

        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1); // Remova o ponteiro do GameObject da pilha
        }
        else
        {
            return luaL_error(L, "centerOrigin Invalid argument type, expected table");
        }

        if (gameObject == nullptr)
        {
            return luaL_error(L, "[centerOrigin] gameObject is null");
        }

        gameObject->centerOrigin();

        return 0;
    }

    static int Advance(lua_State *L)
    {
        GameObject *gameObject = nullptr;
        float speed, off = 0;

        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1); // Remova o ponteiro do GameObject da pilha
            speed = lua_tonumber(L, 2);
            if (lua_gettop(L) == 3)
            {
                off = lua_tonumber(L, 3);
            }
        }
        else
        {
            return luaL_error(L, "advance Invalid argument type, expected table");
        }

        if (gameObject == nullptr)
        {
            return luaL_error(L, "[Advance] gameObject is null");
        }

        gameObject->transform->position.x += cos((gameObject->transform->rotation + off) * RAD) * speed;
        gameObject->transform->position.y += sin((gameObject->transform->rotation + off) * RAD) * speed;

        return 0;
    }

    static int XAdvance(lua_State *L)
    {
        GameObject *gameObject = nullptr;
        float speed, to = 0;

        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1); // Remova o ponteiro do GameObject da pilha
            speed = lua_tonumber(L, 2);
            to = lua_tonumber(L, 3);
        }
        else
        {
            return luaL_error(L, "xAdvance Invalid argument type, expected table");
        }

        if (gameObject == nullptr)
        {
            return luaL_error(L, "[XAdvance] gameObject is null");
        }

        gameObject->transform->position.x += cos(to * RAD) * speed;
        gameObject->transform->position.y += sin(to * RAD) * speed;

        return 0;
    }

    static int AddBoxCollider(lua_State *L)
    {
        GameObject *gameObject = nullptr;
        float x = 0, y = 0, w = 0, h = 0;

        if (lua_gettop(L) != 5)
        {
            return luaL_error(L, "addBoxCollider function requires 4 arguments");
        }

        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1);
            x = lua_tonumber(L, 2);
            y = lua_tonumber(L, 3);
            w = lua_tonumber(L, 4);
            h = lua_tonumber(L, 5);
        }
        else
        {
            return luaL_error(L, "addBoxCollider Invalid argument type, expected table");
        }

        if (gameObject == nullptr)
        {
            return luaL_error(L, "[addBoxCollider] gameObject is null");
        }

        gameObject->AddComponent<BoxColiderComponent>(x, y, w, h);

        return 0;
    }

    static int AddCircleCollider(lua_State *L)
    {
        GameObject *gameObject = nullptr;
        float x = 0, y = 0, r = 0;

        if (lua_gettop(L) != 4)
        {
            return luaL_error(L, "addCircleCollider function requires 3 arguments");
        }

        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1);
            x = lua_tonumber(L, 2);
            y = lua_tonumber(L, 3);
            r = lua_tonumber(L, 4);
        }
        else
        {
            return luaL_error(L, "addCircleCollider Invalid argument type, expected table");
        }

        if (gameObject == nullptr)
        {
            return luaL_error(L, "[addCircleCollider] gameObject is null");
        }

        gameObject->AddComponent<CircleColiderComponent>(x, y, r);

        return 0;
    }

    static int SetSpriteColor(lua_State *L)
    {
        GameObject *gameObject = nullptr;
        if (lua_gettop(L) != 5)
        {
            return luaL_error(L, "setSpriteColor function requires 4 arguments");
        }

        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1);

            if (gameObject == nullptr)
            {
                return luaL_error(L, "[setSpriteColor] gameObject is null");
            }
            if (gameObject->HasComponent<SpriteComponent>() == false)
            {
                return luaL_error(L, "[setSpriteColor] gameObject has no sprite component");
            }
            SpriteComponent *sprite = gameObject->GetComponent<SpriteComponent>();
            sprite->color.r = lua_tonumber(L, 2);
            sprite->color.g = lua_tonumber(L, 3);
            sprite->color.b = lua_tonumber(L, 4);
            sprite->color.a = lua_tonumber(L, 5);
        }
        else
        {
            return luaL_error(L, "setSpriteColor Invalid argument type, expected table");
        }

        return 0;
    }

    static int SetSpriteFlip(lua_State *L)
    {
        GameObject *gameObject = nullptr;
        if (lua_gettop(L) != 3)
        {
            return luaL_error(L, "setSpriteFlip function requires 3 arguments");
        }

        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1);

            if (gameObject == nullptr)
            {
                return luaL_error(L, "[setSpriteFlip] gameObject is null");
            }
            if (gameObject->HasComponent<SpriteComponent>() == false)
            {
                return luaL_error(L, "[setSpriteFlip] gameObject has no sprite component");
            }
            SpriteComponent *sprite = gameObject->GetComponent<SpriteComponent>();
            sprite->FlipX = lua_toboolean(L, 2);
            sprite->FlipY = lua_toboolean(L, 3);
        }
        else
        {
            return luaL_error(L, "setSpriteColor Invalid argument type, expected table");
        }

        return 0;
    }

    static int SetSpriteClip(lua_State *L)
    {
        GameObject *gameObject = nullptr;
        if (lua_gettop(L) != 5)
        {
            return luaL_error(L, "setSpriteClip function requires 4 arguments");
        }
        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1);

            if (gameObject == nullptr)
            {
                return luaL_error(L, "[setSpriteClip] gameObject is null");
            }
            if (gameObject->HasComponent<SpriteComponent>() == false)
            {
                return luaL_error(L, "[setSpriteClip] gameObject has no sprite component");
            }
            SpriteComponent *sprite = gameObject->GetComponent<SpriteComponent>();
            sprite->clip.x = lua_tonumber(L, 2);
            sprite->clip.y = lua_tonumber(L, 3);
            sprite->clip.width = lua_tonumber(L, 4);
            sprite->clip.height = lua_tonumber(L, 5);
        }
        else
        {
            return luaL_error(L, "setSpriteColor Invalid argument type, expected table");
        }

        return 0;
    }

    static int SetSpriteGraph(lua_State *L)
    {
        GameObject *gameObject = nullptr;
        if (lua_gettop(L) != 2)
        {
            return luaL_error(L, "setSpriteGraph function requires 1 argument");
        }
        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1);

            if (gameObject == nullptr)
            {
                return luaL_error(L, "[setSpriteGraph] gameObject is null");
            }
            if (gameObject->HasComponent<SpriteComponent>() == false)
            {
                return luaL_error(L, "[setSpriteGraph] gameObject has no sprite component");
            }
            SpriteComponent *sprite = gameObject->GetComponent<SpriteComponent>();
            const char *graph = lua_tostring(L, 2);
            if (Assets::Instance().hasGraph(graph))
            {
                sprite->graph = Assets::Instance().getGraph(graph);
            }
            else
            {
                return luaL_error(L, "[setSpriteGraph] graph %s not found", graph);
            }
        }
        else
        {
            return luaL_error(L, "setSpriteGraph Invalid argument type, expected table");
        }

        return 0;
    }
    //*********************************************************************************************************************
    static int AddAnimator(lua_State *L)
    {
        GameObject *gameObject = nullptr;

        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1);

            if (gameObject == nullptr)
            {
                return luaL_error(L, "[addAnimator] gameObject is null");
            }
            if (gameObject->HasComponent<Animator>())
            {
                return luaL_error(L, "[addAnimation] gameObject has already a animator component");
            }
            Animator *an = gameObject->AddComponent<Animator>();
            ///  an->AddRow("run", "player_run", 10,12);
            // an->SetAnimation("run");
            // an->Play();
            an->BindLua(L);
            return 1;
        }
        else
        {
            return luaL_error(L, "addAnimator Invalid argument type, expected table");
        }

        lua_pushnil(L);
        return 1;
    }

    static int getAnimator(lua_State *L)
    {
        GameObject *gameObject = nullptr;

        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1);

            if (gameObject == nullptr)
            {
                return luaL_error(L, "[getAnimator] gameObject is null");
            }
            if (gameObject->HasComponent<Animator>() == false)
            {
                return luaL_error(L, "[getAnimator] gameObject has no animator component");
            }
            Animator *animation = gameObject->GetComponent<Animator>();

            if (animation->table_ref == LUA_NOREF)
            {
                animation->BindLua(L);
            }
            lua_rawgeti(L, LUA_REGISTRYINDEX, animation->table_ref);
            return 1;
        }
        else
        {
            return luaL_error(L, "getAnimator Invalid argument type, expected table");
        }

        lua_pushnil(L);
        return 1;
    }

    static int SetAnimation(lua_State *L)
    {

        if (lua_gettop(L) <= 1)
        {
            return luaL_error(L, "setAnimation function requires 1/2 arguments");
        }
        GameObject *gameObject = nullptr;

        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1);

            if (gameObject == nullptr)
            {
                return luaL_error(L, "[setAnimation] gameObject is null");
            }
            if (gameObject->HasComponent<Animator>() == false)
            {
                return luaL_error(L, "[setAnimation] gameObject has no animator component");
            }
            Animator *animation = gameObject->GetComponent<Animator>();
            const char *name = lua_tostring(L, 2);
            bool now = false;

            if (lua_gettop(L) == 3)
            {
                now = lua_toboolean(L, 3);
            }

            animation->SetAnimation(name, now);
            if (now)
            {
                // return luaL_error(L, "play animation %s", name);
                animation->Play();
            }
        }
        else
        {
            return luaL_error(L, "setAnimation Invalid argument type, expected table");
        }

        return 0;
    }

    static int PlayAnimation(lua_State *L)
    {
        GameObject *gameObject = nullptr;
        if (lua_gettop(L) != 2)
        {
            return luaL_error(L, "playAnimation function requires 1 argument");
        }
        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1);

            if (gameObject == nullptr)
            {
                return luaL_error(L, "[playAnimation] gameObject is null");
                return 0;
            }
            if (gameObject->HasComponent<Animator>() == false)
            {
                return luaL_error(L, "[playAnimation] gameObject has no animator component");
            }
            Animator *animation = gameObject->GetComponent<Animator>();
            animation->Play();
        }
        else
        {
            return luaL_error(L, "playAnimation Invalid argument type, expected table");
        }

        return 0;
    }

    static int StopAnimation(lua_State *L)
    {
        GameObject *gameObject = nullptr;

        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1);

            if (gameObject == nullptr)
            {
                return luaL_error(L, "[stopAnimation] gameObject is null");
            }
            if (gameObject->HasComponent<Animator>() == false)
            {
                return luaL_error(L, "[stopAnimation] gameObject has no animator component");
            }
            Animator *animation = gameObject->GetComponent<Animator>();
            animation->Stop();
        }
        else
        {
            return luaL_error(L, "stopAnimation Invalid argument type, expected table");
        }

        return 0;
    }
    static int PauseAnimation(lua_State *L)
    {
        GameObject *gameObject = nullptr;

        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1);

            if (gameObject == nullptr)
            {
                return luaL_error(L, "[pauseAnimation] gameObject is null");
            }
            if (gameObject->HasComponent<Animator>() == false)
            {
                return luaL_error(L, "[pauseAnimation] gameObject has no animator component");
            }
            Animator *animation = gameObject->GetComponent<Animator>();
            animation->Pause();
        }
        else
        {
            return luaL_error(L, "pauseAnimation Invalid argument type, expected table");
        }

        return 0;
    }

    static int SetAnimationMode(lua_State *L)
    {
        GameObject *gameObject = nullptr;

        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1);

            if (gameObject == nullptr)
            {
                return luaL_error(L, "[setAnimationMode] gameObject is null");
            }
            if (gameObject->HasComponent<Animator>() == false)
            {
                return luaL_error(L, "[setAnimationMode] gameObject has no animator component");
            }
            Animator *animation = gameObject->GetComponent<Animator>();
            int mode = lua_tointeger(L, 2);
            animation->SetMode((AnimationMode)mode);
        }
        else
        {
            return luaL_error(L, "setAnimationMode Invalid argument type, expected table");
        }

        return 0;
    }

    static int SetTable(lua_State *L)
    {
        GameObject *gameObject = nullptr;
        if (lua_gettop(L) != 2)
        {
            return luaL_error(L, "SetTable function requires 1 argument");
        }
        // PrintLuaTable(L);

        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1);

            if (gameObject == nullptr)
            {
                return luaL_error(L, "[setTable] gameObject is null");
            }

            if (!lua_istable(L, 2))
            {
                luaL_error(L, "[setTable] Argument must be a table");
            }

            ///  PrintLuaTable(L);

            // float x = 0, y = 0, rotation = 0, scaleX = 1, scaleY = 1, pivotX = 0, pivotY = 0;
            // int width = 0, height = 0, r = 0, g = 0, b = 0, a = 0, originX = 0, originY = 0;
            // bool flipX = false, flipY = false;
            SpriteComponent *sprite = nullptr;
            bool useSprite = false;
            if (gameObject->HasComponent<SpriteComponent>())
            {
                sprite = gameObject->GetComponent<SpriteComponent>();
                useSprite = true;
            }

            if (lua_istable(L, -1))
            {
                lua_pushnil(L);
                while (lua_next(L, -2) != 0)
                {
                    const char *property_name = lua_tostring(L, -2);
                    //    const char *property_value = lua_tostring(L, -1);

                    //   return luaL_error(L,     "Key:%s - Value:%s", property_name, property_value);

                    if (strcmp(property_name, "rotation") == 0)
                    {
                        gameObject->transform->rotation = lua_tonumber(L, -1);
                        lua_pop(L, 1);
                        continue;
                    }

                    if (useSprite && sprite != nullptr)
                    {
                        if (strcmp(property_name, "red") == 0)
                        {
                            sprite->color.r = lua_tointeger(L, -1);
                        }
                        else if (strcmp(property_name, "green") == 0)
                        {
                            sprite->color.g = lua_tointeger(L, -1);
                        }
                        else if (strcmp(property_name, "blue") == 0)
                        {
                            sprite->color.b = lua_tointeger(L, -1);
                        }
                        else if (strcmp(property_name, "alpha") == 0)
                        {
                            sprite->color.a = lua_tointeger(L, -1);
                        }
                    }

                    lua_pop(L, 1);
                }
            }

            lua_getfield(L, 1, "x");
            if (lua_isnumber(L, -1))
            {
                gameObject->transform->position.x = lua_tonumber(L, -1);
                lua_pop(L, 1);
            }

            lua_getfield(L, 1, "y");
            if (lua_isnumber(L, -1))
            {
                gameObject->transform->position.y = lua_tonumber(L, -1);
                lua_pop(L, 1);
            }
        }
        else
        {
            return luaL_error(L, "setTable Invalid argument type, expected table");
        }
        return 0;
    }

    static int sendMessageData(lua_State *L)
    {
        GameObject *gameObject = nullptr;
        if (lua_gettop(L) != 2)
        {
            return luaL_error(L, "[sendMessage]  function requires 1 arguments");
        }
        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1);
            if (gameObject == nullptr)
            {
                return luaL_error(L, "[sendMessage] gameObject is null");
                return 0;
            }

            if (gameObject->script)
            {
                gameObject->sendMensageAll();
            }
        }
        else
        {
            return luaL_error(L, "[sendMessage]  Invalid argument type, expected table");
        }

        return 0;
    }

    static int sendMessageDataTo(lua_State *L)
    {
        GameObject *gameObject = nullptr;
        const char *name = nullptr;

        if (lua_gettop(L) != 3)
        {
            return luaL_error(L, "[sendMessageTo]  function requires 2 arguments");
        }
        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1);
            if (gameObject == nullptr)
            {
                return luaL_error(L, "[sendMessageTo] gameObject is null");
            }
            if (lua_isstring(L, -1))
            {
                name = lua_tostring(L, -1);
                if (gameObject->script && name != nullptr)
                {
                    //  return luaL_error(L, "sendMensageTo %s", name);
                    gameObject->sendMensageTo(name);
                }
            }
            else
            {
                return luaL_error(L, "[sendMessageTo]  Invalid argument type, expected string");
            }
        }
        else
        {
            return luaL_error(L, "[sendMessageTo]  Invalid argument type, expected table");
        }

        return 0;
    }

    static int PlaceFree(lua_State *L)
    {
        GameObject *gameObject = nullptr;
        if (lua_gettop(L) != 3)
        {
            return luaL_error(L, "[place_free]  function requires 2 arguments");
        }
        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1);
            if (gameObject == nullptr)
            {
                return luaL_error(L, "[place_free] gameObject is null");
            }

            float x = lua_tonumber(L, 2);
            float y = lua_tonumber(L, 3);

            bool isFree = gameObject->place_free(x, y);
            lua_pushboolean(L, isFree);
        }
        else
        {
            return luaL_error(L, "[place_free]  Invalid argument type, expected table");
        }

        return 1;
    }

    static int PlaceMeeting(lua_State *L)
    {
        GameObject *gameObject = nullptr;
        if (lua_gettop(L) != 4)
        {
            return luaL_error(L, "[place_meeting]  function requires 3 arguments");
        }
        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1);
            if (gameObject == nullptr)
            {
                return luaL_error(L, "[place_meeting] gameObject is null");
            }

            float x = lua_tonumber(L, 2);
            float y = lua_tonumber(L, 3);
            const char *name = lua_tostring(L, 4);

            bool isFree = gameObject->place_meeting(x, y, name);
            lua_pushboolean(L, isFree);

            return 1;
        }
        else
        {
            return luaL_error(L, "[place_meeting]  Invalid argument type, expected table");
        }

        return 0;
    }

    static int LayerPlaceMeeting(lua_State *L)
    {
        GameObject *gameObject = nullptr;
        if (lua_gettop(L) != 4)
        {
            return luaL_error(L, "[layer_place_meeting]  function requires 3 arguments");
        }
        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1);
            if (gameObject == nullptr)
            {
                return luaL_error(L, "[layer_place_meeting] gameObject is null");
            }

            float x = lua_tonumber(L, 2);
            float y = lua_tonumber(L, 3);
            int layer = (int)lua_tonumber(L, 4);

            bool isFree = gameObject->place_meeting_layer(x, y, layer);

            lua_pushboolean(L, isFree);
            return 1;
        }
        else
        {
            return luaL_error(L, "[place_meeting]  Invalid argument type, expected table");
        }

        return 0;
    }

    static int LoadSprite(lua_State *L)
    {

        GameObject *gameObject = nullptr;
        const char *graph = nullptr;

        if (lua_istable(L, 1) && lua_isstring(L, 2))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1);
            graph = lua_tostring(L, 2);
        }
        else
        {
            luaL_error(L, "[addSprite] argument is not a table");
            lua_pushnil(L);
            return 1;
        }

        if (gameObject != nullptr)
        {
            if (gameObject->HasComponent<SpriteComponent>())
            {
                luaL_error(L, "[addSprite] gameObject already has a SpriteComponent");
                lua_pushnil(L);
                return 1;
            }

            SpriteComponent *spr = gameObject->AddComponent<SpriteComponent>(graph);
            spr->BindLua(L);
            return 1;
        }
        else
        {
            luaL_error(L, "[addSprite] gameObject is null");
        }

        lua_pushnil(L);
        return 1;
    }

    static int LoadTileLayer(lua_State *L)
    {

        GameObject *gameObject = nullptr;

        if (lua_gettop(L) < 6 || lua_gettop(L) > 9)
        {

            luaL_error(L, "addTiles function requires 5/7 arguments");
            lua_pushnil(L);
            return 1;
        }

        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1);
        }
        else
        {
            luaL_error(L, "[addTiles] argument is not a table");
            lua_pushnil(L);
            return 1;
        }

        if (gameObject != nullptr)
        {
            if (gameObject->HasComponent<TileLayerComponent>())
            {
                luaL_error(L, "[addTiles] gameObject already has a TileLayerComponent");
                lua_pushnil(L);
                return 1;
            }
            //  TileLayerComponent(int width, int height, int tileWidth, int tileHeight, int spacing, int margin, const std::string &fileName);

            int width = (int)lua_tonumber(L, 2);
            int height = (int)lua_tonumber(L, 3);
            int tileWidth = (int)lua_tonumber(L, 4);
            int tileHeight = (int)lua_tonumber(L, 5);

            const char *fileName = lua_tostring(L, 6);

            int spacing = 0;
            int margin = 0;

            if (lua_gettop(L) == 9)
            {
                spacing = (int)lua_tonumber(L, 8);
                margin = (int)lua_tonumber(L, 9);
            }
            //   return luaL_error(L, "addTiles %d %d %d %d %d %d %s", width, height, tileWidth, tileHeight, spacing, margin, fileName);

            TileLayerComponent *spr = gameObject->AddComponent<TileLayerComponent>(
                width, height,
                tileWidth, tileHeight,
                spacing, margin, fileName);


            spr->BindLua(L);

            return 1;
        }
        else
        {
            Log(LOG_ERROR, "[addTiles] gameObject is null");
        }

        lua_pushnil(L);
        return 1;
    }
    static int GetSprite(lua_State *L)
    {

        GameObject *gameObject = nullptr;

        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1);
        }
        else
        {
            luaL_error(L, "[getSprite] argument is not a table");
            lua_pushnil(L);
        }

        if (gameObject != nullptr)
        {
            if (!gameObject->HasComponent<SpriteComponent>())
            {
                Log(LOG_ERROR, "[getSprite] gameObject has no SpriteComponent");
                lua_pushnil(L);
            }
            else
            {
                SpriteComponent *spr = gameObject->GetComponent<SpriteComponent>();
                if (spr->table_ref == LUA_NOREF)
                {
                    spr->BindLua(L);
                }
                lua_rawgeti(L, LUA_REGISTRYINDEX, spr->table_ref);
                return 1;
            }
        }
        else
        {
            luaL_error(L, "[getSprite] gameObject is null");
        }

        lua_pushnil(L);
        return 1;
    }

    static int LoadScript(lua_State *L)
    {

        GameObject *gameObject = nullptr;
        const char *script = nullptr;

        if (lua_istable(L, 1) && lua_isstring(L, 2))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1);
            script = lua_tostring(L, 2);
        }
        else
        {
            luaL_error(L, "[addScript] argument is not a table");
            lua_pushboolean(L, false);
            return 1;
        }

        if (gameObject != nullptr)
        {
            if (gameObject->script != nullptr)
            {
                luaL_error(L, "[addScript] gameObject already has a ScriptComponent");
                lua_pushboolean(L, false);
                return 1;
            }

            gameObject->createScript(script, L);
            lua_pushboolean(L, true);

            return 1;
        }
        else
        {
            luaL_error(L, "[addSprite] gameObject is null");
        }

        lua_pushboolean(L, false);
        return 1;
    }

    static int GameAddChild(lua_State *L)
    {
        GameObject *gameObject = nullptr;
        GameObject *gameChild = nullptr;

        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1); // Remova o ponteiro do GameObject da pilha
        }
        else
        {
            return luaL_error(L, "[addChild] The First argument must be a table");
        }

        if (gameObject == nullptr)
        {
            return luaL_error(L, "[addChild] gameObject is null");
        }

        if (lua_istable(L, 2))
        {
            lua_getfield(L, 2, "gameObject");
            gameChild = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 2);
        }
        else
        {
            return luaL_error(L, "[addChild] The Second argument must be a table");
        }

        if (gameChild == nullptr)
        {
            return luaL_error(L, "[addChild] gameChild is null");
        }
        gameObject->addChild(gameChild);

        return 0;
    }

    static int SetDebug(lua_State *L)
    {
        GameObject *gameObject = nullptr;

        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1); // Remova o ponteiro do GameObject da pilha
        }
        else
        {
            return luaL_error(L, "SetDebug Invalid argument type, expected table");
        }

        if (gameObject == nullptr)
        {
            return luaL_error(L, "[setDebug] gameObject is null");
        }

        int mask = lua_tointeger(L, 2);
        gameObject->setDebug(mask);

        return 0;
    }

    static int SetSolid(lua_State *L)
    {
        GameObject *gameObject = nullptr;

        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1);

            if (gameObject == nullptr)
            {
                return luaL_error(L, "[setSolid] gameObject is null");
            }

            bool mode = lua_toboolean(L, 2);
            gameObject->solid = mode;
        }
        else
        {
            return luaL_error(L, "setSolid Invalid argument type, expected table");
        }

        return 0;
    }

    static int SetActive(lua_State *L)
    {
        GameObject *gameObject = nullptr;

        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1);

            if (gameObject == nullptr)
            {
                return luaL_error(L, "[setActive] gameObject is null");
            }

            bool mode = lua_toboolean(L, 2);
            gameObject->active = mode;
        }
        else
        {
            return luaL_error(L, "setActive Invalid argument type, expected table");
        }

        return 0;
    }
    static int SetVisible(lua_State *L)
    {
        GameObject *gameObject = nullptr;

        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1);

            if (gameObject == nullptr)
            {
                return luaL_error(L, "[setVisible] gameObject is null");
            }

            bool mode = lua_toboolean(L, 2);
            gameObject->visible = mode;
        }
        else
        {
            return luaL_error(L, "setVisible Invalid argument type, expected table");
        }

        return 0;
    }

    static int SetPersistent(lua_State *L)
    {
        GameObject *gameObject = nullptr;

        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1);

            if (gameObject == nullptr)
            {
                return luaL_error(L, "[setPersistent] gameObject is null");
            }

            bool mode = lua_toboolean(L, 2);
            gameObject->persistent = mode;
        }
        else
        {
            return luaL_error(L, "setPersistent Invalid argument type, expected table");
        }

        return 0;
    }
    static int SetPrefab(lua_State *L)
    {
        GameObject *gameObject = nullptr;

        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1);

            if (gameObject == nullptr)
            {
                return luaL_error(L, "[SetPrefab] gameObject is null");
            }

            bool mode = lua_toboolean(L, 2);
            gameObject->prefab = mode;
        }
        else
        {
            return luaL_error(L, "SetPrefab Invalid argument type, expected table");
        }

        return 0;
    }

    static int SetCollidable(lua_State *L)
    {
        GameObject *gameObject = nullptr;

        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1);

            if (gameObject == nullptr)
            {
                return luaL_error(L, "[SetCollidable] gameObject is null");
            }

            bool mode = lua_toboolean(L, 2);
            gameObject->collidable = mode;
        }
        else
        {
            return luaL_error(L, "SetCollidable Invalid argument type, expected table");
        }

        return 0;
    }

    static int GetState(lua_State *L)
    {

        GameObject *gameObject = nullptr;
        const char *state = nullptr;
        if (lua_istable(L, 1) && lua_isstring(L, 2))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1);
            state = lua_tostring(L, 2);
        }
        else
        {
            luaL_error(L, "[getState] Invalid argument type, expected table");
            lua_pushboolean(L, false);
            return 1;
        }

        if (strcmp(state, "Visible") == 0)
        {
            lua_pushboolean(L, gameObject->visible);
            return 1;
        }
        else if (strcmp(state, "Active") == 0)
        {
            lua_pushboolean(L, gameObject->active);
            return 1;
        }
        else if (strcmp(state, "Persistent") == 0)
        {
            lua_pushboolean(L, gameObject->persistent);
            return 1;
        }
        else if (strcmp(state, "Prefab") == 0)
        {
            lua_pushboolean(L, gameObject->prefab);
            return 1;
        }
        else if (strcmp(state, "Collidable") == 0)
        {
            lua_pushboolean(L, gameObject->collidable);
            return 1;
        }
        else if (strcmp(state, "Pickable") == 0)
        {
            lua_pushboolean(L, gameObject->pickable);
            return 1;
        }
        else if (strcmp(state, "Solid") == 0)
        {
            lua_pushboolean(L, gameObject->solid);
            return 1;
        }

        luaL_error(L, "[getState] Unknown state");
        lua_pushboolean(L, false);
        return 1;
    }

    static int SetState(lua_State *L)
    {

        GameObject *gameObject = nullptr;
        const char *state = nullptr;
        bool mode = false;
        if (lua_istable(L, 1) && lua_isstring(L, 2))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1);
            state = lua_tostring(L, 2);
            mode = lua_toboolean(L, 3);
        }
        else
        {
            luaL_error(L, "[SetState] Invalid argument type, expected table");
            lua_pushboolean(L, false);
            return 1;
        }

        if (strcmp(state, "Visible") == 0)
        {
            gameObject->visible = mode;
        }
        else if (strcmp(state, "Active") == 0)
        {
            gameObject->active = mode;
        }
        else if (strcmp(state, "Persistent") == 0)
        {
            gameObject->persistent = mode;
        }
        else if (strcmp(state, "Prefab") == 0)
        {
            gameObject->prefab = mode;
        }
        else if (strcmp(state, "Solid") == 0)
        {
            gameObject->solid = mode;
        }
        else if (strcmp(state, "Collidable") == 0)
        {
            gameObject->collidable = mode;
        }
        else if (strcmp(state, "Pickable") == 0)
        {
            gameObject->pickable = mode;
        }
        else
        {
            luaL_error(L, "[setState] Unknown state");
            lua_pushboolean(L, false);
            return 1;
        }

        lua_pushboolean(L, true);
        return 1;
    }

    static int lAddComponent(lua_State *L)
    {

        GameObject *gameObject = nullptr;
        const char *type = nullptr;
        if (lua_istable(L, 1) && lua_isstring(L, 2))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1);
            type = lua_tostring(L, 2);
        }
        else
        {
            luaL_error(L, "[addComponent] Invalid argument type, expected table");
            lua_pushnil(L);
            return 1;
        }

        if (gameObject != nullptr && type != nullptr)
        {

            if (strcmp(type, "Sprite") == 0)
            {

                if (gameObject->HasComponent<SpriteComponent>())
                {
                    luaL_error(L, "[addComponent] gameObject already has a SpriteComponent");
                    lua_pushnil(L);
                    return 1;
                }
                const char *graph = lua_tostring(L, 3);
                SpriteComponent *spr = gameObject->AddComponent<SpriteComponent>(graph);
                spr->BindLua(L);
                return 1;
            }
            else if (strcmp(type, "Tiles") == 0)
            {
                if (gameObject->HasComponent<TileLayerComponent>())
                {
                    luaL_error(L, "[addComponent] gameObject already has a TileLayerComponent");
                    lua_pushnil(L);
                    return 1;
                }

                int width = (int)lua_tonumber(L, 3);
                int height = (int)lua_tonumber(L, 4);
                int tileWidth = (int)lua_tonumber(L, 5);
                int tileHeight = (int)lua_tonumber(L, 6);

                const char *fileName = lua_tostring(L, 7);
                if (fileName == nullptr)
                {
                    luaL_error(L, "[addComponent] Invalid argument type, expected string");
                    lua_pushnil(L);
                    return 1;
                }

                int spacing = 0;
                int margin = 0;

                if (lua_gettop(L) == 10)
                {
                    spacing = (int)lua_tonumber(L, 8);
                    margin = (int)lua_tonumber(L, 9);
                }
                //   return luaL_error(L, "addTiles %d %d %d %d %d %d %s", width, height, tileWidth, tileHeight, spacing, margin, fileName);

                TileLayerComponent *spr = gameObject->AddComponent<TileLayerComponent>(
                    width, height, tileWidth, tileHeight, spacing, margin, fileName);
                spr->BindLua(L);

                return 1;
            }
            else if (strcmp(type, "Animator") == 0)
            {
                if (gameObject->HasComponent<Animator>())
                {
                    luaL_error(L, "[addComponent] gameObject already has a AnimatorComponent");
                    lua_pushnil(L);
                    return 1;
                }

                Animator *an = gameObject->AddComponent<Animator>();
                an->BindLua(L);
                return 1;
            }
            else
            {
                luaL_error(L, "[addComponent] invalid component type");
                lua_pushnil(L);
                return 1;
            }
        }
        else
        {
            luaL_error(L, "[AddComponent] gameObject is null");
            lua_pushnil(L);
            return 1;
        }

        lua_pushnil(L);
        return 1;
    }

    static int lGetComponent(lua_State *L)
    {
        GameObject *gameObject = nullptr;

        const char *type = nullptr;
        if (lua_istable(L, 1) && lua_isstring(L, 2))
        {
            lua_getfield(L, 1, "gameObject");
            gameObject = static_cast<GameObject *>(lua_touserdata(L, -1));
            lua_pop(L, 1);
            type = lua_tostring(L, 2);
        }
        else
        {
            luaL_error(L, "[getComponent] Invalid argument type, expected table");
            lua_pushnil(L);
            return 1;
        }

        if (gameObject == nullptr)
        {
            luaL_error(L, "[getComponent] gameObject is null");
            lua_pushnil(L);
            return 1;
        }

        if (strcmp(type, "Sprite") == 0)
        {
            if (gameObject->HasComponent<SpriteComponent>() == false)
            {
                return luaL_error(L, "[getComponent] gameObject has no SpriteComponent");
            }
            SpriteComponent *spr = gameObject->GetComponent<SpriteComponent>();
            if (spr->table_ref == LUA_NOREF)
            {
                spr->BindLua(L);
            }
            lua_rawgeti(L, LUA_REGISTRYINDEX, spr->table_ref);
            return 1;
        }
        else if (strcmp(type, "Tiles") == 0)
        {
            if (gameObject->HasComponent<TileLayerComponent>() == false)
            {
                return luaL_error(L, "[getComponent] gameObject has no TileLayerComponent");
            }
            TileLayerComponent *tile = gameObject->GetComponent<TileLayerComponent>();
            if (tile->table_ref == LUA_NOREF)
            {
                tile->BindLua(L);
            }
            lua_rawgeti(L, LUA_REGISTRYINDEX, tile->table_ref);
            return 1;
        }
        else if (strcmp(type, "Animator") == 0)
        {
            if (gameObject->HasComponent<Animator>() == false)
            {
                return luaL_error(L, "[getComponent] gameObject has no AnimatorComponent");
            }
            Animator *an = gameObject->GetComponent<Animator>();
            if (an->table_ref == LUA_NOREF)
            {
                an->BindLua(L);
            }
            lua_rawgeti(L, LUA_REGISTRYINDEX, an->table_ref);
            return 1;
        }
        else
        {
            luaL_error(L, "[getComponent] invalid component type");
            lua_pushnil(L);
            return 1;
        }

        lua_pushnil(L);
        return 1;
    }

} // namespace BinGameObject

void GameObject::BindLua(lua_State *state)
{
    // Crie a tabela do objeto
    lua_newtable(state);

    table_ref = luaL_ref(state, LUA_REGISTRYINDEX);
    lua_rawgeti(state, LUA_REGISTRYINDEX, table_ref); // Empilha a tabela do objeto
    lua_pushlightuserdata(state, this);
    lua_setfield(state, -2, "gameObject"); // Armazena o ponteiro do GameObject na tabela

    using namespace BinGameObject;

    lua_pushstring(state, name.c_str());
    lua_setfield(state, -2, "name");

    lua_pushcfunction(state, &SetDebug);
    lua_setfield(state, -2, "setDebug");

    lua_pushcfunction(state, &GameAddChild);
    lua_setfield(state, -2, "addChild");

    lua_pushcfunction(state, &SetPosition);
    lua_setfield(state, -2, "setPosition"); // Armazena a função setPosition na tabela

    lua_pushcfunction(state, &GetPosition);
    lua_setfield(state, -2, "getPosition"); // Armazena a função getPosition na tabela

    lua_pushcfunction(state, &GetLocalPosition);
    lua_setfield(state, -2, "getLocalPoint"); // Armazena a função getPosition na tabela

    lua_pushcfunction(state, &GetWorldPosition);
    lua_setfield(state, -2, "getWorldPoint"); // Armazena a função getPosition na tabela

    lua_pushcfunction(state, &GetX);
    lua_setfield(state, -2, "getX");

    lua_pushcfunction(state, &GetY);
    lua_setfield(state, -2, "getY");

    lua_pushcfunction(state, &GetWorldX);
    lua_setfield(state, -2, "getWorldX");

    lua_pushcfunction(state, &GetWorldY);
    lua_setfield(state, -2, "getWorldY");

    lua_pushcfunction(state, &Kill);
    lua_setfield(state, -2, "kill");

    lua_pushcfunction(state, &CenterPivot);
    lua_setfield(state, -2, "centerPivot");

    lua_pushcfunction(state, &CenterOrigin);
    lua_setfield(state, -2, "centerOrigin");

    lua_pushcfunction(state, &SetScale);
    lua_setfield(state, -2, "setScale");

    lua_pushcfunction(state, &GetScale);
    lua_setfield(state, -2, "getScale");

    lua_pushcfunction(state, &SetRotation);
    lua_setfield(state, -2, "setRotation");

    lua_pushcfunction(state, &GetRotation);
    lua_setfield(state, -2, "getRotation");

    lua_pushcfunction(state, &GetWorldRotation);
    lua_setfield(state, -2, "getWorldRotation");

    lua_pushcfunction(state, &SetPivot);
    lua_setfield(state, -2, "setPivot");

    lua_pushcfunction(state, &SetOrigin);
    lua_setfield(state, -2, "setOrigin");

    lua_pushcfunction(state, &SetSize);
    lua_setfield(state, -2, "setSize");

    lua_pushcfunction(state, &SetCenterPivot);
    lua_setfield(state, -2, "centerPivot");

    lua_pushcfunction(state, &GetPivot);
    lua_setfield(state, -2, "getPivot");

    lua_pushcfunction(state, &TurnTo);
    lua_setfield(state, -2, "turnTo");

    lua_pushcfunction(state, &PointToMouse);
    lua_setfield(state, -2, "pointToMouse");

    lua_pushcfunction(state, &Advance);
    lua_setfield(state, -2, "advance");

    lua_pushcfunction(state, &XAdvance);
    lua_setfield(state, -2, "advanceTo");

    lua_pushcfunction(state, &AddBoxCollider);
    lua_setfield(state, -2, "addBoxCollider");

    lua_pushcfunction(state, &AddCircleCollider);
    lua_setfield(state, -2, "addCircleCollider");

    lua_pushcfunction(state, &LoadSprite);
    lua_setfield(state, -2, "addSprite");

    lua_pushcfunction(state, &LoadTileLayer);
    lua_setfield(state, -2, "addTiles");

    lua_pushcfunction(state, &AddAnimator);
    lua_setfield(state, -2, "addAnimator");

    lua_pushcfunction(state, &getAnimator);
    lua_setfield(state, -2, "getAnimator");

    lua_pushcfunction(state, &GetSprite);
    lua_setfield(state, -2, "getSprite");

    lua_pushcfunction(state, &LoadScript);
    lua_setfield(state, -2, "addScript");

    lua_pushcfunction(state, &SetSpriteClip);
    lua_setfield(state, -2, "setSpriteClip");

    lua_pushcfunction(state, &SetSpriteColor);
    lua_setfield(state, -2, "setSpriteColor");

    lua_pushcfunction(state, &SetSpriteFlip);
    lua_setfield(state, -2, "setSpriteFlip");

    lua_pushcfunction(state, &SetSpriteGraph);
    lua_setfield(state, -2, "setSpriteGraph");

    lua_pushcfunction(state, &SetTable);
    lua_setfield(state, -2, "setTable");

    lua_pushcfunction(state, &sendMessageData);
    lua_setfield(state, -2, "sendMessage");

    lua_pushcfunction(state, &sendMessageDataTo);
    lua_setfield(state, -2, "sendMessageTo");

    lua_pushcfunction(state, &PlaceFree);
    lua_setfield(state, -2, "place_free");

    lua_pushcfunction(state, &PlaceMeeting);
    lua_setfield(state, -2, "place_meeting");

    lua_pushcfunction(state, &LayerPlaceMeeting);
    lua_setfield(state, -2, "layer_place_meeting");

    lua_pushcfunction(state, &SetAnimation);
    lua_setfield(state, -2, "setAnimation");

    lua_pushcfunction(state, &PlayAnimation);
    lua_setfield(state, -2, "play");

    lua_pushcfunction(state, &StopAnimation);
    lua_setfield(state, -2, "stop");

    lua_pushcfunction(state, &SetAnimationMode);
    lua_setfield(state, -2, "mode");

    lua_pushcfunction(state, &PauseAnimation);
    lua_setfield(state, -2, "pause");

    lua_pushcfunction(state, &StopAnimation);
    lua_setfield(state, -2, "stop");

    lua_pushcfunction(state, &SetSolid);
    lua_setfield(state, -2, "setSolid");

    lua_pushcfunction(state, &SetVisible);
    lua_setfield(state, -2, "setVisible");

    lua_pushcfunction(state, &SetActive);
    lua_setfield(state, -2, "setActive");

    lua_pushcfunction(state, &SetCollidable);
    lua_setfield(state, -2, "setCollidable");

    lua_pushcfunction(state, &SetPrefab);
    lua_setfield(state, -2, "setPrefab");

    lua_pushcfunction(state, &SetPersistent);
    lua_setfield(state, -2, "setPersistent");

    lua_pushcfunction(state, &lAddComponent);
    lua_setfield(state, -2, "addComponent");

    lua_pushcfunction(state, &lGetComponent);
    lua_setfield(state, -2, "getComponent");

    lua_pushcfunction(state, &GetState);
    lua_setfield(state, -2, "getState");

    lua_pushcfunction(state, &SetState);
    lua_setfield(state, -2, "setState");

    lua_pushcfunction(state, &FaceTo);
    lua_setfield(state, -2, "faceTo");

    lua_pop(state, 1); // Remove a tabela do objeto da pilha
    lua_rawgeti(state, LUA_REGISTRYINDEX, table_ref);
}
//*********************************************************************************************************************
//**                         ScriptComponent                                                                          **
//*********************************************************************************************************************

ScriptComponent::ScriptComponent(GameObject *gameObject, const char *lua, lua_State *L)
    : gameObject(gameObject), state(L)
{
    script = lua;
    callOnReadyDone = false;
    watch = false;
    gameObject->script = this;

    lua_getglobal(L, "script_refs");
    lua_getfield(L, -1, lua);
    script_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    timeLoad = GetFileModTime(lua);
    if (script_ref == LUA_REFNIL) // O script ainda não está carregado
    {
        char *data = LoadFileText(lua);
        size_t size = strlen(data);

        if (luaL_loadbufferx(L, data, size, lua, nullptr) != LUA_OK)
        {
            const char *errMsg = lua_tostring(L, -1);
            Log(LOG_ERROR, "Failed to load script %s : %s ", lua, errMsg);
            lua_pop(L, 1);
            free(data);
            panic = true;
            return;
        }

        free(data);

        if (lua_pcall(L, 0, LUA_MULTRET, 0) != LUA_OK)
        {
            const char *errMsg = lua_tostring(L, -1);
            Log(LOG_ERROR, "Failed to execute script %s : %s ", lua, errMsg);
            lua_pop(L, 1);
            panic = true;
            return;
        }

        if (!lua_istable(L, -1))
        {
            Log(LOG_ERROR, "Script %s must return a table.", lua);
            lua_pop(L, 1);
            panic = true;
            return;
        }

        // Store a reference to the script in the global table
        lua_getglobal(L, "script_refs");
        lua_pushvalue(L, -2);     // Push the script table
        lua_setfield(L, -2, lua); // Store a reference to the script table in the global table using 'lua' instead of 'GetFileNameWithoutExt(lua)'
        lua_pop(L, 1);            // Remove the global table from the stack

        // Get the script reference
        lua_getglobal(L, "script_refs");
        lua_getfield(L, -1, lua);
        script_ref = luaL_ref(L, LUA_REGISTRYINDEX);

        lua_pop(L, 1); // Remove the global table from the stack
    }

    LuaBind();

    panic = false;
}

ScriptComponent::~ScriptComponent()
{
    for (const auto &entry : luaFunctions)
    {
        luaL_unref(state, LUA_REGISTRYINDEX, entry.second);
    }
    luaL_unref(state, LUA_REGISTRYINDEX, script_ref);

    gameObject = nullptr;

    //  return luaL_error(L, "ScriptComponent destroyed");
}

bool ScriptComponent::isFunctionRegistered(const char *functionName)
{
    auto it = luaFunctions.find(functionName);
    return it != luaFunctions.end();
}

void ScriptComponent::registerFunction(const char *functionName)
{

    lua_rawgeti(state, LUA_REGISTRYINDEX, script_ref);
    lua_getfield(state, -1, functionName);
    if (!lua_isfunction(state, -1))
    {
        lua_pop(state, 2);
        return;
    }

    int ref = luaL_ref(state, LUA_REGISTRYINDEX);
    lua_pop(state, 1);
    luaFunctions[functionName] = ref;
}

void ScriptComponent::LuaBind()
{

    for (const auto &entry : luaFunctions)
    {
        luaL_unref(state, LUA_REGISTRYINDEX, entry.second);
    }

    luaFunctions.clear();
    registerFunction("OnReady");
    registerFunction("OnRemove");

    registerFunction("OnAnimationEnd");
    registerFunction("OnAnimationStart");
    registerFunction("OnAnimation");

    registerFunction("render");
    registerFunction("update");

    registerFunction("OnPause");

    registerFunction("OnMessage");
    registerFunction("OnCollision");
}

bool ScriptComponent::Reload()
{
    if (!state)
        return false;

    if (!watch)
        if (!panic)
            return false;
    //  if (gameObject->prefab)
    //      return false;

    Log(LOG_WARNING, "Relod %s", script.c_str());

    // Desreferenciar o script anterior, se houver
    if (script_ref != LUA_NOREF)
    {
        luaL_unref(state, LUA_REGISTRYINDEX, script_ref);
    }

    // Carregar o novo script
    char *data = LoadFileText(script.c_str());
    size_t size = strlen(data);

    if (luaL_loadbufferx(state, data, size, script.c_str(), nullptr) != LUA_OK)
    {
        const char *errMsg = lua_tostring(state, -1);
        Log(LOG_ERROR, "Failed to reload script %s : %s ", script.c_str(), errMsg);
        lua_pop(state, 1);
        free(data);
        panic = true;
        return false;
    }
    free(data);

    if (lua_pcall(state, 0, LUA_MULTRET, 0) != LUA_OK)
    {
        const char *errMsg = lua_tostring(state, -1);
        Log(LOG_ERROR, "Failed to execute reloaded script %s : %s ", script.c_str(), errMsg);
        lua_pop(state, 1);
        panic = true;
        return false;
    }

    if (!lua_istable(state, -1))
    {
        Log(LOG_ERROR, "Reloaded script %s must return a table.", script.c_str());
        lua_pop(state, 1);
        panic = true;
        return false;
    }

    // Store a reference to the script in the global table
    lua_getglobal(state, "script_refs");
    lua_pushvalue(state, -2);                // Push the script table
    lua_setfield(state, -2, script.c_str()); // Store a reference to the script table in the global table using 'lua' instead of 'GetFileNameWithoutExt(lua)'
    lua_pop(state, 1);                       // Remove the global table from the stack

    // Get the script reference
    lua_getglobal(state, "script_refs");
    lua_getfield(state, -1, script.c_str());
    script_ref = luaL_ref(state, LUA_REGISTRYINDEX);

    lua_pop(state, 1); // Remove the global table from the stack

    LuaBind();
    panic = false;
    watch = false;
    timeLoad = GetFileModTime(script.c_str());
    //callOnReadyDone = false;
    Log(LOG_INFO, "Reloaded script %s completed", script.c_str());
    return true;
}

void ScriptComponent::callOnAnimationFrame(int frame, const std::string &name)
{
}

void ScriptComponent::callOnAnimationEnd(const std::string &name)
{
}
void ScriptComponent::callOnAnimationStart(const std::string &name)
{
}

void ScriptComponent::callOnCollide(GameObject *other)
{
    if (panic || !callOnReadyDone)
        return;
    if (!isFunctionRegistered("OnCollision"))
        return;

    int ref = luaFunctions["OnCollision"];
    lua_rawgeti(state, LUA_REGISTRYINDEX, ref);
    lua_rawgeti(state, LUA_REGISTRYINDEX, gameObject->table_ref); // Empilha a tabela do objeto

    if (other->script)
    {
        lua_rawgeti(state, LUA_REGISTRYINDEX, other->table_ref);
    }
    else
        lua_pushstring(state, other->name.c_str());

    if (lua_pcall(state, 2, 0, 0) != LUA_OK)
    {
        const char *errMsg = lua_tostring(state, -1);
        Log(LOG_ERROR, "Failed to call 'OnCollision' function in script: %s", errMsg);
        lua_pop(state, 1);
    }
}

void ScriptComponent::callOnUpdate(float dt)
{
    if (IsKeyReleased(KEY_F5))
        Reload();

    if (panic || !callOnReadyDone)
        return;
    if (!isFunctionRegistered("update"))
        return;

    int ref = luaFunctions["update"];
    lua_rawgeti(state, LUA_REGISTRYINDEX, ref);
    lua_rawgeti(state, LUA_REGISTRYINDEX, gameObject->table_ref); // Empilha a tabela do objeto
    lua_pushnumber(state, dt);

    if (lua_pcall(state, 2, 0, 0) != LUA_OK)
    {
        const char *errMsg = lua_tostring(state, -1);
        Log(LOG_ERROR, "Failed to call 'update' function in script: %s", errMsg);
        lua_pop(state, 1);
        panic = true;
    }
}

void ScriptComponent::callOnPause()
{
    if (panic || !callOnReadyDone)
        return;
    if (!isFunctionRegistered("OnPause"))
        return;

    int ref = luaFunctions["OnPause"];
    lua_rawgeti(state, LUA_REGISTRYINDEX, ref);

    lua_rawgeti(state, LUA_REGISTRYINDEX, gameObject->table_ref); // Empilha a tabela do objeto

    lua_pushnumber(state, GetFrameTime());

    if (lua_pcall(state, 2, 0, 0) != LUA_OK)
    {
        const char *errMsg = lua_tostring(state, -1);
        Log(LOG_ERROR, "Failed to call 'OnPause' function in script: %s", errMsg);
        lua_pop(state, 1);
        panic = true;
    }
}
void ScriptComponent::callOnReady()
{
    if (panic)
        return;

    if (!isFunctionRegistered("OnReady"))
    {
        callOnReadyDone = true;
        return;
    }

    int ref = luaFunctions["OnReady"];
    lua_rawgeti(state, LUA_REGISTRYINDEX, ref);
    lua_rawgeti(state, LUA_REGISTRYINDEX, gameObject->table_ref); // Empilha a tabela do objeto
    if (lua_pcall(state, 1, 0, 0) != LUA_OK)
    {
        const char *errMsg = lua_tostring(state, -1);
        Log(LOG_ERROR, "Failed to call 'OnReady' function in script: %s", errMsg);
        lua_pop(state, 1);
        panic = true;
        callOnReadyDone = false;
        return;
    }
    callOnReadyDone = true;
}

void ScriptComponent::callOnRemove()
{
    if (panic || !callOnReadyDone)
        return;
    if (!isFunctionRegistered("OnRemove"))
        return;

    int ref = luaFunctions["OnRemove"];
    lua_rawgeti(state, LUA_REGISTRYINDEX, ref);
    lua_rawgeti(state, LUA_REGISTRYINDEX, gameObject->table_ref); // Empilha a tabela do objeto

    if (lua_pcall(state, 1, 0, 0) != LUA_OK)
    {
        const char *errMsg = lua_tostring(state, -1);
        Log(LOG_ERROR, "Failed to call 'OnRemove' function in script: %s", errMsg);
        lua_pop(state, 1);
        panic = true;
    }
}

void ScriptComponent::callOnRender()
{
    if (panic || !callOnReadyDone)
        return;
    if (!isFunctionRegistered("render"))
        return;

    int ref = luaFunctions["render"];
    lua_rawgeti(state, LUA_REGISTRYINDEX, ref);

    lua_rawgeti(state, LUA_REGISTRYINDEX, gameObject->table_ref); // Empilha a tabela do objeto

    if (lua_pcall(state, 1, 0, 0) != LUA_OK)
    {
        const char *errMsg = lua_tostring(state, -1);
        Log(LOG_ERROR, "Failed to call 'render' function in script: %s", errMsg);
        lua_pop(state, 1);
        panic = true;
    }
}

void ScriptComponent::callOnMessage()
{
    if (panic || !callOnReadyDone)
        return;

    if (!isFunctionRegistered("OnMessage"))
        return;

    int ref = luaFunctions["OnMessage"];
    lua_rawgeti(state, LUA_REGISTRYINDEX, ref);

    lua_rawgeti(state, LUA_REGISTRYINDEX, gameObject->table_ref); // Empilha a tabela do objeto

    lua_pushvalue(state, 2);

    // enviamos a table recebida do lua para os scripts 'OnMessage'
    if (lua_pcall(state, 2, 0, 0) != LUA_OK)
    {
        const char *errMsg = lua_tostring(state, -1);
        Log(LOG_ERROR, "Failed to call 'OnMessage' function in script: %s", errMsg);
        lua_pop(state, 1);
        panic = true;
    }
}

//*********************************************************************************************************************
//**                         Scene                                                                                  **
//*********************************************************************************************************************

bool compareGameObjectDepth(const GameObject *a, const GameObject *b)
{
    return a->layer < b->layer;
}
Scene *Scene::m_instance = nullptr;

Scene::Scene() : lastCheckTime(0), checkInterval(5)
{
    m_instance = this;
    timer.start();
    camera.target.x = 0;
    camera.target.y = 0;

    camera.offset.x = 0;
    camera.offset.y = 0;

    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    enableEditor = true;
    enableCollisions = true;
    enableLiveReload = true;
    showDebug = true;
    showStats = true;

    numObjectsRemoved = 0;
    currentMode = None;
    selectedObject = nullptr;
    prevMousePos = {0, 0};
    m_num_layers = 0;
    addLayers(2);
}

Scene::~Scene()
{
    m_instance = nullptr;
}

int Scene::addLayer()
{
    std::vector<GameObject *> l;
    l.reserve(4000);
    layers.emplace(layersCount(), l);
    ++m_num_layers;
    return layersCount();
}

void Scene::addToLayer(GameObject *e)
{
    if (e->layer > layersCount())
    {
        while (layersCount() <= e->layer)
        {
            addLayer();
        }
    }
    layers[e->layer].emplace_back(e);
}

int Scene::layersCount()
{
    return (int)layers.size();
}

int Scene::addLayers(int count)
{
    for (int i = 0; i < count; i++)
    {
        addLayer();
    }
    return layersCount();
}

void Scene::ClearScene()
{

    for (int layer = 0; layer < layersCount(); layer++)
    {

        layers[layer].erase(
            std::remove_if(std::begin(layers[layer]), std::end(layers[layer]),
                           [](GameObject *entity)
                           {
                               if (entity->persistent)
                                   return false;
                               return true;
                           }),
            std::end(layers[layer]));
    }

    for (auto gameObject : gameObjectsToAdd)
    {
        gameObject->OnRemove();
        delete gameObject;
        gameObject = nullptr;
    }
    gameObjectsToAdd.clear();

    for (auto gameObject : gameObjects)
    {

        if (!gameObject->persistent)
        {
            numObjectsRemoved++;
            gameObjectsToRemove.push_back(gameObject);
        }
    }

    // auto partition_point = std::partition(gameObjects.begin(), gameObjects.end(),
    //                                         [](GameObject *obj) { return !obj->persistent; });
    // for (auto it = partition_point; it != gameObjects.end(); ++it)
    // {
    //     delete *it;
    // }
    // gameObjects.erase(partition_point, gameObjects.end());

    Collect();
}

void Scene::ClearAndFree()
{
    Log(LOG_INFO, "Clearing and free scene GameObject");

    for (int layer = 0; layer < layersCount(); layer++)
    {
        layers[layer].clear();
    }
    layers.clear();

    for (auto gameObject : gameObjects)
    {
        gameObject->OnRemove();
        delete gameObject;
        gameObject = nullptr;
    }
    gameObjects.clear();

    for (auto gameObject : gameObjectsToRemove)
    {
        delete gameObject;
        gameObject = nullptr;
    }
    gameObjectsToRemove.clear();

    for (auto gameObject : gameObjectsToAdd)
    {
        delete gameObject;
        gameObject = nullptr;
    }
    gameObjectsToAdd.clear();
}

void Scene::Init(const std::string &title, float fps, int windowWidth, int windowHeight, bool fullscreen)
{
    this->title = title;
    this->fps = fps;
    this->windowSize.x = windowWidth;
    this->windowSize.y = windowHeight;
    this->fullscreen = fullscreen;
    cameraBounds.x = 0;
    cameraBounds.y = 0;
    cameraBounds.width = windowWidth;
    cameraBounds.height = windowHeight;

    lua_pushinteger(getState(), windowWidth);
    lua_setglobal(getState(), "WindowWidth");

    lua_pushinteger(getState(), windowHeight);
    lua_setglobal(getState(), "WindowHeight");
}

void Scene::SetWorld(float width, float height)
{
    this->worldSize.x = width;
    this->worldSize.y = height;

    lua_pushinteger(getState(), (int)width);
    lua_setglobal(getState(), "WorldWidth");

    lua_pushinteger(getState(), (int)height);
    lua_setglobal(getState(), "WorldHeight");
}

void Scene::SetBackground(int r, int g, int b)
{
    background.r = r;
    background.g = g;
    background.b = b;
}

json serializeTransformComponent(TransformComponent *transform)
{
    json transformJson = {
        {"position", {{"x", transform->position.x}, {"y", transform->position.y}}},
        {"scale", {{"x", transform->scale.x}, {"y", transform->scale.y}}},
        {"pivot", {{"x", transform->pivot.x}, {"y", transform->pivot.y}}},
        {"skew", {{"x", transform->skew.x}, {"y", transform->skew.y}}},
        {"rotation", transform->rotation}};
    return transformJson;
}

void deserializeTransformComponent(const json &transformJson, TransformComponent *transform)
{

    transform->position = Vec2(transformJson["position"]["x"], transformJson["position"]["y"]);
    transform->scale = Vec2(transformJson["scale"]["x"], transformJson["scale"]["y"]);
    transform->pivot = Vec2(transformJson["pivot"]["x"], transformJson["pivot"]["y"]);
    transform->skew = Vec2(transformJson["skew"]["x"], transformJson["skew"]["y"]);
    transform->rotation = transformJson["rotation"].get<float>();
}

json serializeGameObject(GameObject *obj)
{

    json objJson =
        {
            {"name", obj->name},
            {"script", obj->scriptName},
            {"components", json::array()},
            {"visible", obj->visible},
            {"prefab", obj->prefab},
            {"layer", obj->layer},
            {"width", obj->width},
            {"height", obj->height},
            {"originX", obj->originX},
            {"originY", obj->originY},
            {"solid", obj->solid},
            {"persistent", obj->persistent},
            {"collidable", obj->collidable},
            {"pickable", obj->pickable},

            {"transform", serializeTransformComponent(obj->transform)},
            {"children", json::array()}};

    for (GameObject *child : obj->children)
        objJson["children"].push_back(serializeGameObject(child));

    if (obj->HasComponent<SpriteComponent>())
    {
        json sprite;
        SpriteComponent *spriteComponent = obj->GetComponent<SpriteComponent>();
        sprite["type"] = "SpriteComponent";
        sprite["graph"] = spriteComponent->graphID;
        sprite["FlipX"] = spriteComponent->FlipX;
        sprite["FlipY"] = spriteComponent->FlipY;
        json rectJson;
        rectJson["x"] = spriteComponent->clip.x;
        rectJson["y"] = spriteComponent->clip.y;
        rectJson["width"] = spriteComponent->clip.width;
        rectJson["height"] = spriteComponent->clip.height;
        sprite["clip"] = rectJson;
        json colorJson;
        colorJson["r"] = spriteComponent->color.r;
        colorJson["g"] = spriteComponent->color.g;
        colorJson["b"] = spriteComponent->color.b;
        colorJson["a"] = spriteComponent->color.a;
        sprite["color"] = colorJson;
        objJson["components"].push_back(sprite);
    }

    if (obj->HasComponent<TileLayerComponent>())
    {
        json tile;
        TileLayerComponent *tileComponent = obj->GetComponent<TileLayerComponent>();
        tile["type"] = "TileLayerComponent";
        tile["graph"] = tileComponent->graphID;
        tile["tileWidth"] = tileComponent->tileWidth;
        tile["tileHeight"] = tileComponent->tileHeight;
        tile["spacing"] = tileComponent->spacing;
        tile["margin"] = tileComponent->margin;
        tile["width"] = tileComponent->width;
        tile["height"] = tileComponent->height;
        tile["tiles"] = json::array();
        json tiles;
        tile["tiles"] = tileComponent->getCSV();
        objJson["components"].push_back(tile);
    }

    if (obj->HasComponent<Animator>())
    {
        json animation;
        Animator *animComponent = obj->GetComponent<Animator>();
        animation["type"] = "AnimatorComponent";
        animation["animations"] = json::array();
        for (auto &pair : animComponent->animations)
        {
            Animation *anim = pair.second;
            std::string name = pair.first;
            json animJson;
            animJson["name"] = name;
            animJson["graph"] = anim->graphID;
            animJson["count"] = anim->frameCount;
            animJson["duration"] = anim->frameDuration;
            animJson["rows"] = anim->rows;
            animJson["columns"] = anim->columns;
            animation["animations"].push_back(animJson);
        }

        objJson["components"].push_back(animation);
    }

    return objJson;
}

GameObject *deserializeGameObject(const json &objJson)
{
    GameObject *obj = new GameObject();
    obj->name = objJson["name"].get<std::string>();
    obj->visible = objJson["visible"].get<bool>();

    deserializeTransformComponent(objJson["transform"], obj->transform);

    obj->layer = objJson["layer"].get<int>();
    obj->width = objJson["width"].get<float>();
    obj->height = objJson["height"].get<float>();
    obj->originX = objJson["originX"].get<float>();
    obj->originY = objJson["originY"].get<float>();
    obj->solid = objJson["solid"].get<bool>();
    obj->persistent = objJson["persistent"].get<bool>();
    obj->collidable = objJson["collidable"].get<bool>();
    obj->pickable = objJson["pickable"].get<bool>();
    std::string scriptName = objJson["script"].get<std::string>();

    if (objJson.contains("components"))
    {

        for (const auto &componentsJson : objJson["components"])
        {
            if (componentsJson.contains("type"))
            {
                std::string componentType = componentsJson["type"];

                if (componentType == "SpriteComponent")
                {

                    std::string graphID = componentsJson["graph"].get<std::string>();

                    SpriteComponent *sprite = obj->AddComponent<SpriteComponent>(graphID);
                    sprite->FlipX = componentsJson["FlipX"].get<bool>();
                    sprite->FlipY = componentsJson["FlipY"].get<bool>();

                    json rectJson = componentsJson["clip"];
                    sprite->clip.x = rectJson["x"].get<float>();
                    sprite->clip.y = rectJson["y"].get<float>();
                    sprite->clip.width = rectJson["width"].get<float>();
                    sprite->clip.height = rectJson["height"].get<float>();

                    json colorJson = componentsJson["color"];
                    sprite->color.r = colorJson["r"].get<float>();
                    sprite->color.g = colorJson["g"].get<float>();
                    sprite->color.b = colorJson["b"].get<float>();
                    sprite->color.a = colorJson["a"].get<float>();
                }
                else if (componentType == "AnimatorComponent")
                {
                    Animator *anim = obj->AddComponent<Animator>();

                    for (const auto &animJson : componentsJson["animations"])
                    {
                        std::string name = animJson["name"].get<std::string>();
                        std::string graphID = animJson["graph"].get<std::string>();
                        int frameCount = animJson["count"].get<int>();
                        float duration = animJson["duration"].get<float>();
                        int rows = animJson["rows"].get<int>();
                        int columns = animJson["columns"].get<int>();

                        Animation *animation = new Animation(graphID, rows, columns, frameCount, duration);
                        anim->AddAnimation(name, animation);

                        //              Log(LOG_INFO, "Adding animation %s %s %d %f %d %d", name.c_str(),graphID.c_str(),frameCount,duration,rows,columns);
                    }
                    if (anim->animations.size() > 0)
                    {
                        anim->SetAnimation(anim->animations.begin()->first);
                        anim->Play();
                    }
                }
                else if (componentType == "TileLayerComponent")
                {

                    std::string graphID = componentsJson["graph"].get<std::string>();
                    int tileWidth = componentsJson["tileWidth"].get<int>();
                    int tileHeight = componentsJson["tileHeight"].get<int>();
                    int spacing = componentsJson["spacing"].get<int>();
                    int margin = componentsJson["margin"].get<int>();
                    int width = componentsJson["width"].get<int>();
                    int height = componentsJson["height"].get<int>();

                    TileLayerComponent *tileLayer = obj->AddComponent<TileLayerComponent>(width, height, tileWidth, tileHeight, spacing, margin, graphID);
                    tileLayer->loadFromString(componentsJson["tiles"].get<std::string>(),0);
                }
                else
                {
                    Log(LOG_ERROR, "Unknown component type %s", componentType.c_str());
                }
            }
        }
    }

    for (const auto &childJson : objJson["children"])
    {
        GameObject *child = deserializeGameObject(childJson);
        obj->addChild(child);
    }

    if (!scriptName.empty())
    {

        obj->createScript(scriptName.c_str(), getState());
    }

    return obj;
}

bool Scene::LoadTiled(const char *filename)
{

    Log(LOG_INFO, "Loading Tiled from %s", filename);
    char *sceneData = LoadFileText(filename);
    if (sceneData == nullptr)
    {
        Log(LOG_ERROR, "Failed to load tile map from %s", filename);
        return false;
    }
    json tileMap = json::parse(sceneData);
    free(sceneData);

    int width = tileMap["width"].get<int>();
    int height = tileMap["height"].get<int>();
    int tileWidth = tileMap["tilewidth"].get<int>();
    int tileHeight = tileMap["tileheight"].get<int>();
    std::string type = tileMap["type"].get<std::string>();

    SetWorld(width * tileWidth, height * tileHeight);

    Log(LOG_INFO, "Map size: %d x %d  Map tiles : %d x %d , type: %s ", width, height, tileWidth, tileHeight, type.c_str());

    bool isMap = (type == "map");

    if (isMap)
    {
        std::vector<Tileset> tiles;
        // const auto &tilesetJson = tileMap["tilesets"];
        for (const auto &tilesetJson : tileMap["tilesets"])
        {
            Tileset tileset;

            tileset.firstgid = tilesetJson["firstgid"];
            tileset.name = tilesetJson["name"];
            tileset.tileWidth = tilesetJson["tilewidth"];
            tileset.tileHeight = tilesetJson["tileheight"];
            tileset.spacing = tilesetJson["spacing"];
            tileset.margin = tilesetJson["margin"];
            tileset.imageSource = tilesetJson["image"];
            /// tileset.imageWidth = tilesetJson["imagewidth"];
            // tileset.imageHeight = tilesetJson["imageheight"];

            std::string path = tileset.imageSource;
            Assets::Instance().loadGraph(tileset.name, path.c_str());

            Log(LOG_INFO, "Tileset  %s", tileset.name.c_str());
            Log(LOG_INFO, "Tile Image %s", tileset.imageSource.c_str());
            Log(LOG_INFO, "Tile Width %d,  Tile Height %d", tileset.tileWidth, tileset.tileHeight);
            tiles.push_back(tileset);
        }

        std::vector<TileLayer> layers;
        // const auto &layersJson = tileMap["layers"];
        int index = 0;
        for (const auto &layersJson : tileMap["layers"])
        {
            TileLayer tileLayer;
            tileLayer.width = layersJson["width"].get<int>();
            tileLayer.height = layersJson["height"].get<int>();
            tileLayer.name = layersJson["name"].get<std::string>();
            tileLayer.opacity = layersJson["opacity"].get<int>();
            tileLayer.visible = layersJson["visible"].get<int>();
            tileLayer.x = layersJson["x"].get<int>();
            tileLayer.y = layersJson["y"].get<int>();
            tileLayer.type = layersJson["type"].get<std::string>();

            GameObject *layer = new GameObject(tileLayer.name.c_str(), index);
            Tileset tileset = tiles[0];
            TileLayerComponent *tm = layer->AddComponent<TileLayerComponent>(width, height, tileset.tileWidth, tileset.tileHeight, tileset.spacing, tileset.margin, tileset.name.c_str());

            layer->transform->position.x = 0;
            layer->transform->position.y = 0;
            layer->setDebug(SHOW_ALL);
            layer->radius = 0.1;
            layer->solid = true;
            layer->persistent = false;
            layer->width = width;
            layer->height = height;

            layer->originX = 0;
            layer->originY = 0;
            layer->collidable = false;

            if (layersJson.contains("encoding") && layersJson["encoding"] == "base64")
            {

                std::string data = layersJson["data"].get<std::string>();
                std::string decoded = base64_decode(data);
                Log(LOG_INFO, "Decoded %s", decoded.c_str());
            }
            else if (layersJson.contains("encoding") && layersJson["encoding"] == "csv")
            {

                std::string csvData = layersJson["data"].get<std::string>();

                Log(LOG_INFO, "Decoded %s", csvData.c_str());
            }
            else
            {
                std::vector<int> tiles = layersJson["data"].get<std::vector<int>>();
                for (int i = 0; i < (int)tiles.size(); i++)
                {
                    int tile = tiles[i];
                    tm->tileMap[i] = tile - tileset.firstgid;
                }
            }

            Log(LOG_INFO, "Layer Width %d Layer Height ", tileLayer.width, tileLayer.height);
            AddQueueObject(layer);
            // AddGameObject(layer);
        }
    }

    return true;
}
bool Scene::Save(const char *filename)
{

    Log(LOG_INFO, "Saving scene to %s", filename);
    json sceneJson;

    sceneJson["SceneName"] = "Scene";
    sceneJson["SceneVersion"] = "0.1";
    sceneJson["NumObjects"] = static_cast<int>(gameObjects.size());
    sceneJson["Developer"] = "Luis Santos";

    json SceneDebugJson =
        {
            {"showDebug", static_cast<bool>(showDebug)},
            {"showStats", static_cast<bool>(showStats)},
            {"enableCollisions", static_cast<bool>(enableCollisions)},
            {"enableLiveReload", static_cast<bool>(enableLiveReload)},
            {"enableEdit", static_cast<bool>(enableEditor)},
            {"gcObjectsRemoved", static_cast<int>(numObjectsRemoved)},
            {"reloadInterval", static_cast<int>(checkInterval)},

            

            

        };

    sceneJson["Config"] = SceneDebugJson;

    json WindowJson =
        {
            {"width", static_cast<int>(windowSize.x)},
            {"heigh", static_cast<int>(windowSize.y)},
            {"fps", static_cast<int>(fps)},
            {"r", static_cast<int>(background.r)},
            {"g", static_cast<int>(background.g)},
            {"b", static_cast<int>(background.b)},
            {"fullscreen", static_cast<bool>(fullscreen)},
            {"title", title},
        };
    sceneJson["Window"] = WindowJson;

    json cameraJson =
        {
            {"x", camera.target.x},
            {"y", camera.target.y},
            {"offsetX", camera.offset.x},
            {"offsetY", camera.offset.y},
            {"zoom", camera.zoom},
            {"rotation", camera.rotation},

        };
    sceneJson["Camera"] = cameraJson;

    json gameObjectsJson = json::array();
    for (GameObject *obj : gameObjects)
    {
        if (!obj->prefab)
            gameObjectsJson.push_back(serializeGameObject(obj));
    }
    sceneJson["GameObjects"] = gameObjectsJson;

    json imagesJson = json::array();
    for (auto &graph : Assets::Instance().graphs)
    {
        json imgJson =
            {
                {"key", graph.first},
                {"filename", graph.second->filename}};
        imagesJson.push_back(imgJson);
    }
    sceneJson["Images"] = imagesJson;

    std::string sceneData;

    try
    {
        sceneData = sceneJson.dump(4); // O parâmetro '4' indica a quantidade de espaços para indentação.
    }
    catch (const json::exception &e)
    {
        Log(LOG_ERROR, " converting JSON to string: %s", e.what());
        return false;
    }
    catch (...)
    {
        Log(LOG_ERROR, "converting JSON  objeto JSON to string.");
        return false;
    }

    unsigned int dataSize = sceneData.size() + 1; // Adiciona 1 para incluir o caractere nulo.
    char *data = new char[dataSize];
    memcpy(data, sceneData.c_str(), dataSize);

    if (!SaveFileText(filename, data))
    {
        Log(LOG_ERROR, "Failed to save scene to %s", filename);
        delete[] data;
        return false;
    }

    delete[] data;
    return true;
    // return SaveFileText(filename, (char *)sceneJson.dump(4).c_str());
}

bool Scene::Load(const char *filename)
{
    char *sceneData = LoadFileText(filename);
    if (sceneData == nullptr)
    {
        TraceLog(LOG_ERROR, "Failed to load scene from %s", filename);
        return false;
    }

    json sceneJson;

    try
    {
        sceneJson = json::parse(sceneData);
    }
    catch (const json::parse_error &e)
    {
        Log(LOG_ERROR, " parsing  JSON: %s ", e.what());
        UnloadFileText(sceneData);
        return false;
    }
    catch (const std::exception &e)
    {
        Log(LOG_ERROR, " parsingJSON: %s ", e.what());
        UnloadFileText(sceneData);
        return false;
    }
    catch (...)
    {
        Log(LOG_ERROR, " parsing JSON.");
        UnloadFileText(sceneData);
        return false;
    }

    UnloadFileText(sceneData);

    std::string sceneName = sceneJson["SceneName"].get<std::string>();
    sceneJson["NumObjects"].get<int>();
    sceneJson["SceneVersion"].get<std::string>();
    sceneJson["Developer"].get<std::string>();

    const auto &SceneDebugJson = sceneJson["Config"];
    showDebug = SceneDebugJson["showDebug"].get<bool>();
    showStats = SceneDebugJson["showStats"].get<bool>();
    enableCollisions = SceneDebugJson["enableCollisions"].get<bool>();
    enableLiveReload = SceneDebugJson["enableLiveReload"].get<bool>();
    enableEditor = SceneDebugJson["enableEdit"].get<bool>();
    numObjectsRemoved = SceneDebugJson["gcObjectsRemoved"].get<int>();
    checkInterval = SceneDebugJson["reloadInterval"].get<int>();

    sceneJson["Config"] = SceneDebugJson;

    const auto &WindowJson = sceneJson["Window"];
    windowSize.x = WindowJson["width"].get<int>();
    windowSize.y = WindowJson["heigh"].get<int>();
    fps = WindowJson["fps"].get<int>();
    background.r = WindowJson["r"].get<int>();
    background.g = WindowJson["g"].get<int>();
    background.b = WindowJson["b"].get<int>();
    fullscreen = WindowJson["fullscreen"].get<bool>();
    title = WindowJson["title"].get<std::string>();

    const auto &cameraJson = sceneJson["Camera"];
    camera.target.x = cameraJson["x"].get<float>();
    camera.target.y = cameraJson["y"].get<float>();
    camera.offset.x = cameraJson["offsetX"].get<float>();
    camera.offset.y = cameraJson["offsetY"].get<float>();
    camera.zoom = cameraJson["zoom"].get<float>();
    camera.rotation = cameraJson["rotation"].get<float>();

    const auto &imagesJson = sceneJson["Images"];
    for (const auto &imgJson : imagesJson)
    {

        std::string key = imgJson["key"].get<std::string>();
        std::string filename = imgJson["filename"].get<std::string>();
        Assets::Instance().loadGraph(key, filename);
    }

    const auto &objectsJson = sceneJson["GameObjects"];
    for (const auto &objJson : objectsJson)
    {
        GameObject *obj = deserializeGameObject(objJson);
        AddGameObject(obj);
    }

    return true;
}

float Distance(const Vec2 &a, const Vec2 &b)
{
    return sqrt(pow(b.x - a.x, 2) + pow(b.y - a.y, 2));
}
GameObject *Scene::CirclePick(float x, float y, float radius)
{

    for (GameObject *obj : gameObjects)
    {
        if (!obj->pickable)
            continue;
        if (CheckCollisionCircleRec({x, y}, radius, obj->bound))
            return obj;
    }
    return nullptr;
}

GameObject *Scene::MousePick()
{

    for (GameObject *obj : gameObjects)
    {
        if (!obj->pickable)
            continue;
        if (CheckCollisionPointRec(GetMousePosition(), obj->bound))
            return obj;
    }
    return nullptr;
}

GameObject *Scene::RectanglePick(float x, float y, float width, float height)
{

    for (GameObject *obj : gameObjects)
    {
        if (!obj->pickable)
            continue;
        if (CheckCollisionRecs({x, y, width, height}, obj->bound))
            return obj;
    }
    return nullptr;
}

void Scene::Editor()
{

    Vec2 mousePosition = Vec2(GetMouseX(), GetMouseY());
    Vector2 vmousePosition = GetMousePosition();

    if (selectedObject != nullptr)
    {
        if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
        {
            selectedObject->OnReady();
            selectedObject = nullptr;
            return;
        }
    }

    if (IsKeyPressed(KEY_S))
        currentMode = Scale;
    else if (IsKeyPressed(KEY_M))
        currentMode = Move;
    else if (IsKeyPressed(KEY_R))
        currentMode = Rotate;
    else if (IsKeyReleased(KEY_S) || IsKeyReleased(KEY_M) || IsKeyReleased(KEY_R))
        currentMode = None;

    DrawText("S - Scale", GetScreenWidth() - 150, 10, 20, (currentMode == Scale ? RED : WHITE));
    DrawText("M - Move", GetScreenWidth() - 150, 30, 20, (currentMode == Move ? RED : WHITE));
    DrawText("R - Rotate", GetScreenWidth() - 150, 50, 20, (currentMode == Rotate ? RED : WHITE));
    if (selectedObject != nullptr)
    {
        DrawText(TextFormat("Select %s", selectedObject->name.c_str()), GetScreenWidth() - 150, 70, 20, RED);
    }

    for (auto gameObject : gameObjects)
    {

        for (auto gameObject2 : gameObject->children)
        {
            Rectangle rect2 = gameObject2->bound;

            if (CheckCollisionPointRec(vmousePosition, rect2) && !selectedObject)
            {
                DrawRectangleLinesEx(rect2, 2, RED);
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                {
                    initialObjectPosition = gameObject2->transform->position;
                    initialMousePosition = mousePosition;
                    selectedObject = gameObject2;
                    break;
                }
            }
        }

        Rectangle rect = gameObject->bound;

        if (CheckCollisionPointRec(vmousePosition, rect) && !selectedObject)
        {
            DrawRectangleLinesEx(rect, 2, RED);
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                initialObjectPosition = gameObject->transform->position;
                initialMousePosition = mousePosition;

                selectedObject = gameObject;

                break;
            }
        }
    }

    if (selectedObject != nullptr)
    {

        Vector2 objectCenter = {
            selectedObject->getX() + selectedObject->bound.width / 2,
            selectedObject->getY() + selectedObject->bound.height / 2};

        DrawRectangleLinesEx(selectedObject->bound, 2, GREEN);

        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
        {

            switch (currentMode)
            {
            case Move:
            {

                selectedObject->transform->position.x += mousePosition.x - prevMousePos.x;
                selectedObject->transform->position.y += mousePosition.y - prevMousePos.y;
            }
            break;
            case Scale:
            {

                float distanceDelta = mousePosition.x - prevMousePos.x;
                float scaleIncrement = distanceDelta / 100.0f;
                selectedObject->transform->scale += scaleIncrement;
            }
            break;
            case Rotate:
            {
                float prevAngle = atan2(prevMousePos.y - objectCenter.y, prevMousePos.x - objectCenter.x);
                float currAngle = atan2(mousePosition.y - objectCenter.y, mousePosition.x - objectCenter.x);
                selectedObject->transform->rotation += (currAngle - prevAngle) * RAD2DEG;
            }
            break;
            case None:
            default:
                break;
            }

            if (selectedObject->transform->scale.x < 0.1f)
                selectedObject->transform->scale.x = 0.1f;
            if (selectedObject->transform->scale.y < 0.1f)
                selectedObject->transform->scale.y = 0.1f;

            selectedObject->UpdateWorld();
        }
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
        {
        }
    }
    prevMousePos = mousePosition;
}

std::string formatSize(size_t size)
{
    constexpr double KB = 1024.0;
    constexpr double MB = 1024.0 * 1024.0;

    std::string formattedSize;

    if (size < KB)
    {
        formattedSize = std::to_string(size) + " B";
    }
    else if (size < MB)
    {
        formattedSize = std::to_string(round(size / KB * 100.0) / 100.0) + " KB";
    }
    else
    {
        formattedSize = std::to_string(round(size / MB * 100.0) / 100.0) + " MB";
    }

    return formattedSize;
}

bool Scene::inView(const  Rectangle& r )
{
    return true;//CheckCollisionRecs(r, cameraView);
}

void Scene::Render()
{



    ClearBackground(background);
    BeginMode2D(camera);
    cameraPoint.x = (camera.offset.x - camera.target.x) ;
    cameraPoint.y = (camera.offset.y - camera.target.y) ;

    for (int i = 0; i < (int)layers.size(); i++)
    {
        for (auto &e : layers[i])
        {
            if (e->alive && e->visible && inView(e->bound))
            {
                e->Render();
                objectRender++;
            }
        }
    }

    if (showDebug)
    {
        for (auto gameObject : gameObjects)
        {
            if (gameObject->visible && gameObject->active)
                gameObject->Debug();
        }
    }

    if (timer.isPaused())
    {

        for (auto gameObject : gameObjects)
        {
            if (gameObject->alive && gameObject->active)
            {
                gameObject->OnPause();
            }
        }

        if (enableEditor)
            Editor();
    }

   






    EndMode2D();
    // int lastLayerKey = layers.rbegin()->first;
    // for (auto &e : layers[lastLayerKey])
    // {
    //     if (e->alive && e->visible)
    //     {
    //         e->Render();
    //     }
    // }

  
    

    if (showStats)
    {
        int Y = GetScreenHeight() - 40;
        int index = 0;
        for (int i = 0; i < layersCount(); i++)
        {
            if (layers[i].size() > 0)
            {
                Y = GetScreenHeight() - 24 - index * 22;
                index++;
            }
        }
        DrawRectangle(10, Y , 170, index * 22, Fade(SKYBLUE, 0.5f));
        DrawRectangleLines(11, Y+1, 168, index * 22-2, BLUE);
        Y = GetScreenHeight() - 40;
        index = 0;
        for (int i = 0; i < layersCount(); i++)
        {
            if (layers[i].size() > 0)
            {
                Y = GetScreenHeight() - 20 - index * 22;
                DrawText(TextFormat("Layer [%d]  Objects [%d] ", i, layers[i].size()), 28, Y, 10, LIME);
                index++;
            }
        }

        float x = 15;
        float y = 18;
        float s = 18;

        DrawRectangle(10, 10, 220, 100, BLACK);
        DrawRectangle(10, 10, 220, 100, Fade(SKYBLUE, 0.5f));
        DrawRectangleLines(10, 10, 220, 100, BLUE);

        DrawFPS(x, y);
        DrawText(TextFormat("Objects: %i/%d", gameObjects.size(),objectRender), x, y + 1 * s, s, LIME);
        DrawText(TextFormat("Elapsed time: %.2f", timer.getElapsedTime()), x, y + 2 * s, s, LIME);
        DrawText(TextFormat("Delta time: %.2f", timer.getDeltaTime()), x, y + 3 * s, s, LIME);
        DrawText(TextFormat("GC: %s", formatSize(getLuaMemoryUsage()).c_str()), x, y + 4 * s, s, LIME);
      //  DrawText(TextFormat("View: %f %f %f %f", cameraView.x,cameraView.y,cameraView.width,cameraView.height), x, y + 5 * s, s, LIME);
     //   DrawText(TextFormat("Camera: %f %f %f %f", camera.target.x,camera.target.y,camera.offset.x,camera.offset.y), x, y + 6 * s, s, LIME);



    }
}

void Scene::LiveReload()
{

    if (!enableLiveReload)
        return;
    std::time_t currentTime = std::time(nullptr);

    if (currentTime - lastCheckTime >= checkInterval)
    {
        lastCheckTime = currentTime;
        //  Log(LOG_INFO, "Checking for script changes");
        for (auto gameObject : gameObjects)
        {
            if (!gameObject->script)
            {
                continue;
            }
            gameObject->LiveReload();
        }
    }
}

GameObject *Scene::GetGameObjectByName(const std::string &name)
{
    for (auto gameObject : gameObjects)
    {
        if (gameObject->name == name)
        {
            return gameObject;
        }
    }
    Log(LOG_WARNING, "GameObject %s not found", name.c_str());
    return nullptr;
}

void Scene::Update()
{
    timer.update();

    objectRender=0;
    cameraView.x= (-camera.offset.x/camera.zoom) + camera.target.x - (windowSize.x/2.0f/camera.zoom);
    cameraView.y= (-camera.offset.y/camera.zoom) + camera.target.y - (windowSize.y/2.0f/camera.zoom);
    cameraView.width = (float)(windowSize.x/camera.zoom)+(camera.offset.x/camera.zoom) ;
    cameraView.height= (float)(windowSize.y/camera.zoom)+(camera.offset.y/camera.zoom) ;
 

    if (IsKeyReleased(KEY_F1))
    {
        showDebug = !showDebug;
    }

    if (IsKeyPressed(KEY_P))
    {
        if (selectedObject != nullptr)
        {
            selectedObject->OnReady();
            selectedObject = nullptr;
        }
        if (timer.isPaused())
        {
            timer.resume();
        }
        else
        {
            timer.pause();
        }
    }

    if (IsKeyReleased(KEY_F4))
    {
        Collect();
    }

    if (IsKeyReleased(KEY_F6))
    {
        enableLiveReload = !enableLiveReload;
        if (enableLiveReload)
            Log(LOG_INFO, "Live reload enabled");
        else
            Log(LOG_INFO, "Live reload disabled");
    }

    if (IsKeyReleased(KEY_F3))
    {
        enableCollisions = !enableCollisions;
        if (enableCollisions)
            Log(LOG_INFO, "Collisions enabled");
        else
            Log(LOG_INFO, "Collisions disabled");
    }

    // if (needSort)
    // {
    //     std::sort(gameObjects.begin(), gameObjects.end(), compareGameObjectDepth);
    //     needSort = false;
    // }

    if (!timer.isPaused())
    {
        for (auto gameObject : gameObjects)
        {
            if (gameObject->alive && gameObject->active)
            {
                gameObject->Update(timer.getDeltaTime());
            }
            if (!gameObject->alive)
            {
                numObjectsRemoved++;
                gameObjectsToRemove.push_back(gameObject);
            }
        }
    }

    // auto partition_point = std::partition(gameObjects.begin(), gameObjects.end(),
    //                                         [](GameObject *obj) { return obj->alive; });
    // for (auto it = partition_point; it != gameObjects.end(); ++it)
    // {
    //     delete *it;
    // }
    // gameObjects.erase(partition_point, gameObjects.end());

    for (auto gameObject : gameObjectsToRemove)
    {
        int layerKey = gameObject->layer;
        if (layers.find(layerKey) != layers.end())
        {
            std::vector<GameObject *> &objectsInLayer = layers[layerKey];
            auto it = std::find(objectsInLayer.begin(), objectsInLayer.end(), gameObject);
            if (it != objectsInLayer.end())
            {
                objectsInLayer.erase(it);
            }
        }

        auto it = std::find(gameObjects.begin(), gameObjects.end(), gameObject);
        if (it != gameObjects.end())
        {
            gameObject->OnRemove();
            gameObjects.erase(it);
            gameObject->scene = nullptr;
            delete gameObject;
            gameObject = nullptr;
        }
    }
    gameObjectsToRemove.clear();

    for (auto gameObject : gameObjectsToAdd)
    {
        gameObject->UpdateWorld();
        AddGameObject(gameObject);
    }
    gameObjectsToAdd.clear();
    LiveReload();
    if (enableCollisions)
        Collision();

    if (numObjectsRemoved > MAX_OBJECTS_REMOVE_TO_COLECT)
    {
        Collect();
        numObjectsRemoved = 0;
    }
    if (IsKeyReleased(KEY_F2))
    {
        ClearScene();
    }
}
bool Scene::place_meeting_layer(GameObject *obj, float x, float y, int layer)
{
    if (!obj->collidable)
        return false;

    for (auto other : layers[layer])
    {
        if (!other->collidable)
            continue;
        if (obj->collideWith(other, x, y))
            return true;
    }
    return false;
}
bool Scene::place_meeting(GameObject *obj, float x, float y, const std::string &objname)
{
    if (!obj->collidable )
        return false;
    
    for (auto other : gameObjects)
    {
        if ( (!other->collidable) && !inView(other->bound))
            continue;

        if (strcmp(other->name.c_str(), objname.c_str())==0)
        {
            if (obj->collideWith(other, x, y))
                return true;
        } else
            continue;        
    }
    return false;

    // for (auto gameObject : gameObjects)
    // {
    //     if (gameObject->name == name)
    //     {
    //         if (gameObject->HasComponent<BoxColiderComponent>())
    //         {
    //             auto boxColiderComponent = gameObject->GetComponent<BoxColiderComponent>();
    //             if (boxColiderComponent != nullptr)
    //             {
    //                 if (CheckCollisionPointRec({x, y}, boxColiderComponent->rect))
    //                 {
    //                     return true;
    //                 }
    //             }
    //         }

    //         if (gameObject->HasComponent<CircleColiderComponent>())
    //         {
    //             auto circleColiderComponent = gameObject->GetComponent<CircleColiderComponent>();
    //             if (circleColiderComponent != nullptr)
    //             {
    //                 if (CheckCollisionPointCircle({x, y}, circleColiderComponent->center, circleColiderComponent->radius))
    //                 {
    //                     return true;
    //                 }
    //             }
    //         }
    //     }
    // }
}

bool Scene::place_free(GameObject *obj, float x, float y)
{ 
    if (!obj->collidable )
        return true;
    
    for (auto other : gameObjects)
    {
        
        if ( (other->collidable))
        {
            if (obj->collideWith(other, x, y))
                return false;
        } else
             continue;
        
    }
    return true;

    // for (auto gameObject : gameObjects)
    // {
    //     if (gameObject->HasComponent<BoxColiderComponent>())
    //     {
    //         auto boxColiderComponent = gameObject->GetComponent<BoxColiderComponent>();
    //         if (boxColiderComponent != nullptr)
    //         {
    //             if (CheckCollisionPointRec({x, y}, boxColiderComponent->rect))
    //             {
    //                 return false;
    //             }
    //         }
    //     }

    //     if (gameObject->HasComponent<CircleColiderComponent>())
    //     {

    //         auto circleColiderComponent = gameObject->GetComponent<CircleColiderComponent>();
    //         if (circleColiderComponent != nullptr)
    //         {
    //             if (CheckCollisionPointCircle({x, y}, circleColiderComponent->center, circleColiderComponent->radius))
    //             {
    //                 return false;
    //             }
    //         }
    //     }
    // }
    return true;
}

void Scene::SetPositionCamera(float x, float y)
{
    camera.target.x = x;
    camera.target.y = y;
    cameraBounds.x = x;
    cameraBounds.y = y;
}

void Scene::SetCamera(float x, float y)
{

    camera.target.x = x;
    camera.target.y = y;
    cameraBounds.x = x - camera.offset.x * camera.zoom;
    cameraBounds.y = y - camera.offset.y * camera.zoom;

    //    camera.target.x  = 2 * camera.offset.x - x;
    //    camera.target.y  = 2 * camera.offset.y - y;
    //    cameraBounds.x = camera.target.x;
    //    cameraBounds.y = camera.target.y;

    // //local targetX = 2 * (WindowWidth /2)- self.x
    // //local targetY = 2 * (WindowHeight/2)- self.y
}

void Scene::Collision()
{
    for (int i = 0; i < (int)gameObjects.size(); i++)
    {
        GameObject *a = gameObjects[i];
        GameObject *parentA = a->parent;
        if (!a->collidable)
            continue;

        for (int j = i + 1; j < (int)gameObjects.size(); j++)
        {

            GameObject *b = gameObjects[j];

            if (!b->collidable)
                continue;

            if (a == b || b == parentA || a == b->parent)
            {
                continue;
            }

            // Log(LOG_INFO, "Collision between %s and %s", a->name.c_str(), b->name.c_str());

            if (a->HasComponent<BoxColiderComponent>() && b->HasComponent<BoxColiderComponent>())
            {
                BoxColiderComponent *colliderA = a->GetComponent<BoxColiderComponent>();
                BoxColiderComponent *colliderB = b->GetComponent<BoxColiderComponent>();

                if (colliderA->IsColide(colliderB))
                {
                    colliderA->OnColide(colliderB);
                    colliderB->OnColide(colliderA);
                    //          Log(LOG_INFO, "Collision between A %s and B %s", a->name.c_str(), b->name.c_str());
                    return;
                }
                if (colliderB->IsColide(colliderA))
                {
                    colliderB->OnColide(colliderA);
                    colliderA->OnColide(colliderB);
                    //   Log(LOG_INFO, "Collision between B %s and A %s", b->name.c_str(), a->name.c_str());
                    return;
                }
            }
            else if (a->HasComponent<CircleColiderComponent>() && b->HasComponent<CircleColiderComponent>())
            {
                CircleColiderComponent *colliderA = a->GetComponent<CircleColiderComponent>();
                CircleColiderComponent *colliderB = b->GetComponent<CircleColiderComponent>();

                if (colliderA->IsColide(colliderB))
                {
                    colliderA->OnColide(colliderB);
                    colliderB->OnColide(colliderA);
                    //          Log(LOG_INFO, "Collision between A %s and B %s", a->name.c_str(), b->name.c_str());
                    return;
                }
                if (colliderB->IsColide(colliderA))
                {
                    colliderB->OnColide(colliderA);
                    colliderA->OnColide(colliderB);
                    //   Log(LOG_INFO, "Collision between B %s and A %s", b->name.c_str(), a->name.c_str());
                    return;
                }
            }
            else if (a->HasComponent<BoxColiderComponent>() && b->HasComponent<CircleColiderComponent>())
            {
                BoxColiderComponent *colliderA = a->GetComponent<BoxColiderComponent>();
                CircleColiderComponent *colliderB = b->GetComponent<CircleColiderComponent>();
                if (colliderA->IsColide(colliderB))
                {
                    colliderA->OnColide(colliderB);
                    colliderB->OnColide(colliderA);
                    //          Log(LOG_INFO, "Collision between A %s and B %s", a->name.c_str(), b->name.c_str());
                    return;
                }
                if (colliderB->IsColide(colliderA))
                {
                    colliderB->OnColide(colliderA);
                    colliderA->OnColide(colliderB);
                    //   Log(LOG_INFO, "Collision between B %s and A %s", b->name.c_str(), a->name.c_str());
                    return;
                }
            }
            else if (a->HasComponent<CircleColiderComponent>() && b->HasComponent<BoxColiderComponent>())
            {
                CircleColiderComponent *colliderA = a->GetComponent<CircleColiderComponent>();
                BoxColiderComponent *colliderB = b->GetComponent<BoxColiderComponent>();
                if (colliderA->IsColide(colliderB))
                {
                    colliderA->OnColide(colliderB);
                    colliderB->OnColide(colliderA);
                    //          Log(LOG_INFO, "Collision between A %s and B %s", a->name.c_str(), b->name.c_str());
                    return;
                }
                if (colliderB->IsColide(colliderA))
                {
                    colliderB->OnColide(colliderA);
                    colliderA->OnColide(colliderB);
                    //   Log(LOG_INFO, "Collision between B %s and A %s", b->name.c_str(), a->name.c_str());
                    return;
                }
            }
        }
    }
}

Vector2 ColideComponent::GetWorldPosition()
{
    Vector2 pos;
    pos.x = object->getWorldX();
    pos.y = object->getWorldY();
    return pos;
}

Vector2 BoxColiderComponent::GetWorldPosition()
{
    Vector2 pos;
    pos.x = object->getWorldX() + rect.x;
    pos.y = object->getWorldY() + rect.y;
    return pos;
}

Vector2 CircleColiderComponent::GetWorldPosition()
{
    Vector2 pos;
    pos.x = object->getWorldX() + center.x;
    pos.y = object->getWorldY() + center.y;
    return pos;
}

bool BoxColiderComponent::IsColide(ColideComponent *other)
{
    if (other->type == ColliderType::Box)
    {
        BoxColiderComponent *box = (BoxColiderComponent *)other;
        return CheckCollisionRecs(GetWorldRect(), box->GetWorldRect());
    }
    else if (other->type == ColliderType::Circle)
    {
        CircleColiderComponent *circle = (CircleColiderComponent *)other;
        return CheckCollisionCircleRec(circle->center, circle->radius, GetWorldRect());
    }
    return false;
}

void BoxColiderComponent::OnColide(ColideComponent *other)
{
    other->object->OnCollision(this->object);
    object->OnCollision(other->object);
    //  Log(LOG_INFO, "BoxColiderComponent OnColide");
}

void BoxColiderComponent::OnDebug()
{
    DrawRectangleLinesEx(GetWorldRect(), 2, LIME);
}

Rectangle BoxColiderComponent::GetWorldRect()
{
    Rectangle worldRect;
    worldRect.x = object->getWorldX() + rect.x;
    worldRect.y = object->getWorldY() + rect.y;
    worldRect.width = rect.width;
    worldRect.height = rect.height;
    return worldRect;
}

bool CircleColiderComponent::IsColide(ColideComponent *other)
{
    if (other->type == ColliderType::Box)
    {
        BoxColiderComponent *box = (BoxColiderComponent *)other;
        return CheckCollisionCircleRec(GetWorldPosition(), radius, box->GetWorldRect());
    }
    else if (other->type == ColliderType::Circle)
    {
        CircleColiderComponent *circle = (CircleColiderComponent *)other;
        return CheckCollisionCircles(GetWorldPosition(), radius, circle->GetWorldPosition(), circle->radius);
    }
    return false;
}

void CircleColiderComponent::OnColide(ColideComponent *other)
{
    other->object->OnCollision(this->object);
    object->OnCollision(other->object);
    // Log(LOG_INFO, "CircleColiderComponent OnColide");
}

void CircleColiderComponent::OnDebug()
{
    Vector2 p = GetWorldPosition();
    DrawCircleLines(p.x, p.y, radius, RED);
}

void CircleColiderComponent::OnInit()
{
    if (object)
    {
        object->collidable = true;
    }
}

void BoxColiderComponent::OnInit()
{
    if (object)
    {
        object->collidable = true;
    }
}

//**************************************************************************************************
//  ColideComponent
//**************************************************************************************************

bool Contains(Rectangle src, Rectangle obj)
{
    float r1 = src.x + src.width;
    float r2 = obj.x + obj.width;
    float b1 = src.y + src.height;
    float b2 = obj.y + obj.height;

    if (src.x < r2 && r1 > obj.x && src.y < b2 && b1 > obj.y)
    {
        return true;
    }
    return false;
}

QuadtreeNode::QuadtreeNode(int level, const AABB &bounds) : level(level), bounds(bounds)
{
    for (int i = 0; i < 4; i++)
    {
        children[i] = nullptr;
    }
}

QuadtreeNode::~QuadtreeNode()
{
    for (int i = 0; i < 4; i++)
    {
        delete children[i];
    }
}

void QuadtreeNode::split()
{
    float subWidth = bounds.m_w / 2.0f;
    float subHeight = bounds.m_h / 2.0f;
    float x = bounds.m_x;
    float y = bounds.m_y;

    children[0] = new QuadtreeNode(level + 1, AABB(x + subWidth, y, subWidth, subHeight));
    children[1] = new QuadtreeNode(level + 1, AABB(x, y, subWidth, subHeight));
    children[2] = new QuadtreeNode(level + 1, AABB(x, y + subHeight, subWidth, subHeight));
    children[3] = new QuadtreeNode(level + 1, AABB(x + subWidth, y + subHeight, subWidth, subHeight));
}

int QuadtreeNode::getIndex(const AABB &aabb) const
{
    int index = -1;
    float verticalMidpoint = bounds.m_x + (bounds.m_w / 2.0f);
    float horizontalMidpoint = bounds.m_y + (bounds.m_h / 2.0f);

    bool topQuadrant = aabb.m_y < horizontalMidpoint && (aabb.m_y + aabb.m_y) < horizontalMidpoint;
    bool bottomQuadrant = aabb.m_y > horizontalMidpoint;

    if (aabb.m_x < verticalMidpoint && (aabb.m_x + aabb.m_w) < verticalMidpoint)
    {
        if (topQuadrant)
        {
            index = 1;
        }
        else if (bottomQuadrant)
        {
            index = 2;
        }
    }
    else if (aabb.m_x > verticalMidpoint)
    {
        if (topQuadrant)
        {
            index = 0;
        }
        else if (bottomQuadrant)
        {
            index = 3;
        }
    }

    return index;
}

void QuadtreeNode::insert(GameObject *obj)
{
    if (children[0] != nullptr)
    {
        int index = getIndex(obj->GetAABB());

        if (index != -1)
        {
            children[index]->insert(obj);
            return;
        }
    }

    objects.push_back(obj);

    if (objects.size() > MAX_OBJECTS && level < MAX_LEVELS)
    {
        if (children[0] == nullptr)
        {
            split();
        }

        auto it = objects.begin();
        while (it != objects.end())
        {
            int index = getIndex((*it)->GetAABB());
            if (index != -1)
            {
                children[index]->insert(*it);
                it = objects.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
}
void QuadtreeNode::remove(GameObject *obj)
{
    int index = getIndex(obj->GetAABB());
    if (children[0] != nullptr && index != -1)
    {
        children[index]->remove(obj);
        return;
    }

    for (auto it = objects.begin(); it != objects.end(); ++it)
    {
        if (*it == obj)
        {
            objects.erase(it);
            break;
        }
    }

    // Combina os quadrantes filhos, se necessário
    if (children[0] != nullptr)
    {
        int totalObjects = 0;
        for (int i = 0; i < 4; i++)
        {
            totalObjects += children[i]->objects.size();
        }

        if (totalObjects <= MAX_OBJECTS)
        {
            for (int i = 0; i < 4; i++)
            {
                objects.insert(objects.end(), children[i]->objects.begin(), children[i]->objects.end());
                delete children[i];
                children[i] = nullptr;
            }
        }
    }
}
void QuadtreeNode::draw()
{
    DrawRectangleLines(bounds.m_x, bounds.m_y, bounds.m_w, bounds.m_h, RAYWHITE);

    if (children[0] != nullptr)
    {
        for (int i = 0; i < 4; i++)
        {
            children[i]->draw();
        }
    }
}
void QuadtreeNode::retrieve(std::vector<GameObject *> &returnObjects, const Vec2 &point)
{
    if (bounds.contains(point))
    {
        returnObjects.insert(returnObjects.end(), objects.begin(), objects.end());
        //   DrawRectangleLines(bounds.m_x, bounds.m_y, bounds.m_w, bounds.m_h, RED);

        if (children[0] != nullptr)
        {
            for (int i = 0; i < 4; i++)
            {
                children[i]->retrieve(returnObjects, point);
            }
        }
    }
}
void QuadtreeNode::retrieve(std::vector<GameObject *> &returnObjects, const AABB &queryAABB)
{
    if (bounds.intersects(queryAABB))
    {
        for (GameObject *obj : objects)
        {
            if (obj->GetAABB().intersects(queryAABB))
            {
                // AABB aabb = obj->GetAABB();
                // DrawRectangle(aabb.m_x, aabb.m_y, aabb.m_w, aabb.m_h, BLUE);
                returnObjects.push_back(obj);
            }
        }

        if (children[0] != nullptr)
        {
            for (int i = 0; i < 4; i++)
            {
                children[i]->retrieve(returnObjects, queryAABB);
            }
        }
    }
}
void QuadtreeNode::retrieve(const Vec2 &center, float radius, std::vector<GameObject *> &resultObjects) const
{
    if (AABB::IntersectsCircle(bounds, center, radius))
    {
        for (const GameObject *obj : objects)
        {
            if (AABB::IntersectsCircle(obj->GetAABB(), center, radius))
            {
                // resultObjects.push_back(obj);
                resultObjects.push_back(const_cast<GameObject *>(obj));
            }
        }

        if (children[0] != nullptr)
        {
            for (int i = 0; i < 4; i++)
            {
                children[i]->retrieve(center, radius, resultObjects);
            }
        }
    }
}

int QuadtreeNode::countObjects() const
{
    int count = objects.size();

    if (children[0] != nullptr)
    {
        for (int i = 0; i < 4; i++)
        {
            count += children[i]->countObjects();
        }
    }

    return count;
}
void QuadtreeNode::clear()
{
    objects.clear();

    if (children[0] != nullptr)
    {
        for (int i = 0; i < 4; i++)
        {
            children[i]->clear();
            delete children[i];
            children[i] = nullptr;
        }
    }
}

//**********************************************************************************************//
//                                                                                              //
//  Quadtree.h                                                                                  //
//************************************************************************************************
Quadtree::Quadtree(float x, float y, float width, float height)
{
    root = new QuadtreeNode(0, AABB(x, y, width, height));
}

Quadtree::~Quadtree()
{
    delete root;
}

void Quadtree::insert(GameObject *obj)
{
    root->insert(obj);
}

void Quadtree::remove(GameObject *obj)
{
    root->remove(obj);
}
void Quadtree::draw()
{
    root->draw();
}
std::vector<GameObject *> Quadtree::getObjectsAtPoint(const Vec2 &point)
{
    std::vector<GameObject *> returnObjects;
    root->retrieve(returnObjects, point);
    return returnObjects;
}
std::vector<GameObject *> Quadtree::getObjectsInAABB(const AABB &queryAABB)
{
    std::vector<GameObject *> returnObjects;
    root->retrieve(returnObjects, queryAABB);
    return returnObjects;
}
std::vector<GameObject *> Quadtree::getObjectsInCircle(const Vec2 &center, float radius)
{
    std::vector<GameObject *> resultObjects;
    root->retrieve(center, radius, resultObjects);
    return resultObjects;
}

int Quadtree::countObjects() const
{
    return root->countObjects();
}
void Quadtree::clear()
{
    root->clear();
}