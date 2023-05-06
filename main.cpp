#include "src/wrapper.hpp"

extern int screenWidth;
extern int screenHeight;

extern Scene scene;

// void *operator new(std::size_t size)
// {
//   allocatedMemory += size;
//   return std::malloc(size);
// }

// void operator delete(void *ptr, std::size_t size) noexcept
// {
//   allocatedMemory -= size;
//   std::free(ptr);
// }

// void testeBemshmark()
// {
// Assets::Instance().loadGraph("wabbit", "assets/wabbit_alpha.png");

//    GameObject *gamePlayer = new GameObject("scene");
//    gamePlayer->createScript("mainWabbit.lua", L);
//    gamePlayer->persistent=true;
//    scene.AddGameObject(gamePlayer);


//   GameObject *wabbit = new GameObject("wabbit");
//   wabbit->createScript("wabbit.lua", L);
//   SpriteComponent *spr = wabbit->AddComponent<SpriteComponent>("wabbit");
//   wabbit->persistent=true;
//   scene.AddGameObject(wabbit);
// }

// void testeShooter()
// {

//  Assets::Instance().loadGraph("nave", "assets/playerShip1_orange.png");
//   Assets::Instance().loadGraph("wabbit", "assets/wabbit_alpha.png");
//   Assets::Instance().loadGraph("bala", "assets/texture.png");
//   Assets::Instance().loadGraph("lazer", "assets/laserRed08.png");
//   Assets::Instance().loadGraph("cano", "assets/gun02.png");
//   Assets::Instance().loadGraph("ufo", "assets/ufoRed.png");


//   GameObject *gamePlayer = new GameObject("player");
//   gamePlayer->createScript("player.lua", L);
//   gamePlayer->AddComponent<SpriteComponent>("nave");
//   scene.AddGameObject(gamePlayer);

//   {
//     GameObject *gameCano = new GameObject("cano");
//     gameCano->AddComponent<SpriteComponent>("cano");
//     gameCano->createScript("cano", L);
//     gameCano->transform->position.x = 20;
//     gameCano->transform->position.y = 30;
//     gamePlayer->addChild(gameCano);
//   }
//   {
//     GameObject *gameCano = new GameObject("cano");
//     gameCano->AddComponent<SpriteComponent>("cano");
//     gameCano->createScript("cano.lua", L);
//     gameCano->transform->position.x = 80;
//     gameCano->transform->position.y = 30;
//     gamePlayer->addChild(gameCano);
//   }

//   {
//     GameObject *gameOvni = new GameObject("ufo");
//     gameOvni->AddComponent<SpriteComponent>("ufo");
//         gameOvni->createScript("ovni.lua", L);
//     gameOvni->transform->position.x = 200;
//     gameOvni->transform->position.y = 200;
//     scene.AddGameObject(gameOvni);
//   }

//     {
//     GameObject *gameOvni = new GameObject("ufo");
//     gameOvni->AddComponent<SpriteComponent>("ufo");
//     gameOvni->createScript("ovni.lua", L);
//     gameOvni->transform->position.x = 700;
//     gameOvni->transform->position.y = 200;
//     scene.AddGameObject(gameOvni);
//   }

// }



// void testeMovements()
// {
//   Assets::Instance().loadGraph("wabbit", "assets/wabbit_alpha.png");
//   Assets::Instance().loadGraph("player_run", "assets/Player/Run.png");
//   Assets::Instance().loadGraph("player_jump", "assets/Player/Jump.png");
//   Assets::Instance().loadGraph("player_die", "assets/Player/Die.png");
//   Assets::Instance().loadGraph("player_idle", "assets/Player/Idle.png");
//   Assets::Instance().loadGraph("player_win", "assets/Player/Celebrate.png");


//    GameObject *gamePlayer = new GameObject("main");
//    gamePlayer->createScript("assets/scripts/moves/main.lua", L);
//    gamePlayer->persistent=true;
//    scene.AddGameObject(gamePlayer);


//   GameObject *wabbit = new GameObject("player");
//   wabbit->createScript("assets/scripts/moves/player.lua", L);
//   wabbit->AddComponent<SpriteComponent>("player_idle");
//   Animator *animation = wabbit->AddComponent<Animator>();
//   animation->AddRow("run", "player_run", 10,12);
//   animation->AddRow("win", "player_win", 11,10);
//   animation->AddRow("jump", "player_jump", 11,8);
//   animation->AddRow("idle", "player_idle", 1,1);
//   animation->SetAnimation("jump");
//   animation->Play();
//   wabbit->transform->position.x = 200;
//   wabbit->transform->position.y = 100;
//   //animation->SetMode(AnimationMode::Once);

  
//   scene.AddGameObject(wabbit);
// }

int main()
{

  LoadLua();


  InitWindow(screenWidth, screenHeight, "2D Engine");
  
  InitEngine();

  scene.Init("Moves",60,screenWidth, screenHeight, false);
  scene.SetBackground(0,0,0);
  scene.SetWorld(screenWidth, screenHeight);

  //scene.Load("assets/scene.json");

SetWindowSize((int)scene.windowSize.x, (int)scene.windowSize.y);
SetWindowTitle(scene.title.c_str());
SetTargetFPS(scene.fps);


//testeMovements();
//testeBemshmark();
//testeShooter();

//    GameObject *gamePlayer = new GameObject("scene");
//    gamePlayer->createScript("wabbitTeste.lua", L);
//   // gamePlayer->AddComponent<SpriteComponent>("wabbit");
//    gamePlayer->persistent=true;
//    scene.AddGameObject(gamePlayer);


  // GameObject *gamePlayer = new GameObject("wabbit");
  // gamePlayer->createScript("wabbit.lua", L);
  // gamePlayer->AddComponent<SpriteComponent>("wabbit");
  // gamePlayer->persistent=true;
  // scene.AddGameObject(gamePlayer);

//ovni

  //SetTargetFPS(6000);
  //SetTargetFPS(60);

  while (!WindowShouldClose())
  {



    BeginDrawing();

    EngineRender();

    EndDrawing();
  }

 // scene.Save("assets/tmp.json");

  CloseLua();
  
  CloseWindow();

  return 0;
}