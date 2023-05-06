local Utils = require("utils")


print("load  ***   main.lua")


function testeBatch()
    assets.loadGraph("wabbit", "assets/wabbit_alpha.png")
    local w = scene.createGameObject("Main",0)
    w:addScript("mainWabbit.lua")
end

function factorial(n)
    if n == 0 then
      return 1
    else
      return n * factorial(n-1)
    end
  end
  
  function fibonacci(n)
    if n <= 1 then
      return n
    else
      return fibonacci(n-1) + fibonacci(n-2)
    end
  end
  

function teste_lua()
    
print("LUA")
    local num_loops = 10000

-- Medir o tempo de execução da função factorial
local start_time_factorial = os.clock()
for i = 1, num_loops do
  factorial(10)
end
local end_time_factorial = os.clock()
local time_factorial = end_time_factorial - start_time_factorial

-- Medir o tempo de execução da função fibonacci
local start_time_fibonacci = os.clock()
for i = 1, num_loops do
  fibonacci(10)
end
local end_time_fibonacci = os.clock()
local time_fibonacci = end_time_fibonacci - start_time_fibonacci

local start_time_distance = os.clock()
for i = 1, num_loops do
  Utils.point_distance(10,10,2000,2000)
  Utils.point_direction(10,10,2000,2000)
 local a =  (Utils.lengthdir_x(10,10) + Utils.lengthdir_y(10,10))
end
local end_time_distance = os.clock()
local time_distance = end_time_distance - start_time_distance

-- Imprimir os resultados
print("Time for factorial: " .. time_factorial)
print("Time for fibonacci: " .. time_fibonacci)
print("Time for distance: " .. time_distance)



end


function teste_c()
    

print("CPP")
    local num_loops = 10000

    -- Medir o tempo de execução da função factorial
    local start_time_factorial = os.clock()
    for i = 1, num_loops do
      cfactorial(10)
    end
    local end_time_factorial = os.clock()
    local time_factorial = end_time_factorial - start_time_factorial
    
    -- Medir o tempo de execução da função fibonacci
    local start_time_fibonacci = os.clock()
    for i = 1, num_loops do
      cfibonacci(10)
    end
    local end_time_fibonacci = os.clock()
    local time_fibonacci = end_time_fibonacci - start_time_fibonacci


    local start_time_distance = os.clock()
for i = 1, num_loops do
  point_distance(10,10,2000,2000)
  point_direction(10,10,2000,2000)
  local a =  (lengthdir_x(10,10) + lengthdir_y(10,10))
end
local end_time_distance = os.clock()
local time_distance = end_time_distance - start_time_distance
    
    -- Imprimir os resultados
    print("Time for factorial: " .. time_factorial)
    print("Time for fibonacci: " .. time_fibonacci)
    print("Time for distance: " .. time_distance)
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



function OnCreate()
    
  assets.loadGraph("orb","assets/Orb.png")
  assets.loadGraph("tiles","assets/tiles/atlas.png")
    -- assets.loadGraph("wabbit","assets/wabbit_alpha.png")
    -- assets.loadGraph("player_run", "assets/Player/Run.png")

    -- local p = scene.createGameObject("Player",0)
    -- p:setPosition(400,200)
    -- local s = p:addSprite("player_run")
    -- s:setColor(255,0,0,255)

    -- local a = p:addAnimator()
    -- printTable(a)
    -- a:setMode(1)
    -- a:setAnimation("run")
    -- a:addRow("run","player_run",10,11)
    -- a:play()

 




      -- local w = scene.createGameObject("Main",0)
     -- w:addScript("mainWabbit.lua")


--     local w = scene.createGameObject("wabbit",0)
--     w:setPosition(400,200)
--    --printTable(w)
--     local s = w:addSprite("wabbit")

--     w:addScript("wabbit.lua")
  
    -- local w = scene.createGameObject("Main",0)
     -- w:addScript("mainWabbit.lua")

    -- local w = scene.createGameObject("Main",0)
    -- w:addScript("mainOvni.lua")

--   local w = scene.createGameObject("Main",0)
--    w:addScript("mainPlataforma.lua")

--testeBatch()

--testeTileMap()


--scene.load("assets/tmp.json")


end


function render()


end

-- function update(dt)
   
-- end

function OnClose()
  scene.save("assets/tmp.json")
end