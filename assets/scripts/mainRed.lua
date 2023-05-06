
local grid_width = 40
local grid_height = 32

local Object = {}
Object.__index = Object


function Object:OnReady()
    
   
    assets.loadGraph("tiles", "assets/levels/tiles.png")
    assets.loadGraph("orb", "assets/Orb.png")
    assets.loadGraph("bg", "assets/FloorTexture.png")
  

    assets.loadGraph("player_run", "assets/Player/Run.png")
    assets.loadGraph("player_jump", "assets/Player/Jump.png")
    assets.loadGraph("player_die", "assets/Player/Die.png")
    assets.loadGraph("player_idle", "assets/Player/Idle.png")
    assets.loadGraph("player_win", "assets/Player/Celebrate.png")
    

    local mapa = scene.createGameObject("Mapa",0)
    mapa:addScript("assets/scripts/red/map.lua")

    --scene.setCameraZoom(2)
    self:setSize(30*32,24*32)
    

    local p = scene.createGameObject("Player",1)
    p:addComponent("Sprite","player_idle")
    p:addScript("assets/scripts/red/player.lua")

    local orb1 = scene.createGameObject("Orb",1)
    orb1:addComponent("Sprite","orb")
    orb1:setPosition(20,20)

    local orb1 = scene.createGameObject("Orb",1)
    orb1:addComponent("Sprite","orb")
    orb1:setPosition(100,20)

    local orb1 = scene.createGameObject("Orb",1)
    orb1:addComponent("Sprite","orb")
    orb1:setPosition(200,24)
    
    local orb1 = scene.createGameObject("Orb",1)
    orb1:addComponent("Sprite","orb")
    orb1:setPosition(300,24)
    
    
end

function Object:OnRemove()

scene.save("assets/scenes/red.json")


end

function Object:update(dt)
    
   


end

function Object:render()
    canvas.drawGraphTiled("bg", 0,0,WindowWidth,WindowHeight, 0,0,112,128,0,0,0,1.0)
   
    
end

return Object