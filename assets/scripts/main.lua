local Utils = require("utils")


print("load  ***   main.lua")


function testeBatch()
    assets.loadGraph("wabbit", "assets/wabbit_alpha.png")
    local w = scene.createGameObject("Main",0)
    w:addScript("mainWabbit.lua")
end



function testeTileMap()

  --scene.loadTiles("assets/levels/red2.tmj")
  assets.loadGraph("player","assets/Orb.png")
  assets.loadGraph("tiles","assets/tiles/atlas.png")
  assets.loadGraph("player_run", "assets/Player/Run.png")


  local mapa = scene.createGameObject("Mapa",0)
  mapa:addScript("assets/scripts/tiles/map.lua")
  

  local obj = scene.createGameObject("Player",1)
  local spr =obj:addComponent("Sprite","player")
  
  local anim = scene.createGameObject("Animation",1)
  anim:setPosition(400,200)
  anim:addComponent("Sprite","player_run")
  local animation = anim:addComponent("Animator")
  animation:add("run", "player_run",1, 10, 10,12)
  animation:setAnimation("run")
  animation:play()


  obj:addScript("assets/scripts/tiles/player.lua")


end

function testeShooter()
  
  local w = scene.createGameObject("Main",0)
  w:addScript("mainOvni.lua")

end

function testeRed()
  
  local w = scene.createGameObject("Main",0)
  w:addScript("mainRed.lua")

end

function OnCreate()
 
 

  --testeRed()

--testeBatch()

--testeTileMap()

testeShooter()

--scene.load("assets/tmp.json")


end


function render()


end

-- function update(dt)
   
-- end

function OnClose()
  scene.save("assets/tmp.json")
end