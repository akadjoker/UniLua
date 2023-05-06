points =0 

local Object = {}
Object.__index = Object

function Object:OnReady()
    
    assets.loadGraph("nave", "assets/playerShip1_orange.png")
    assets.loadGraph("wabbit", "assets/wabbit_alpha.png")
    assets.loadGraph("bala", "assets/texture.png")
    assets.loadGraph("lazer", "assets/laserRed08.png")
    assets.loadGraph("cano", "assets/gun02.png")
    assets.loadGraph("ufo", "assets/ufoRed.png")
    assets.loadGraph("explosion", "assets/exp2.png")
    assets.loadGraph("bg", "assets/FloorTexture.png")


    local p = scene.createGameObject("Player",1)
    p:setPosition(400,200)
    p:addSprite("nave")
    p:addScript("assets/scripts/shooter/player.lua")

    local a = scene.createGameObject("canoA",0,20,30,0,p)
    a:addSprite("cano")
    a:addScript("assets/scripts/shooter/cano.lua")


    local b = scene.createGameObject("canoB",0,80,30,0,p)
    b:addSprite("cano")
    b:addScript("assets/scripts/shooter/cano.lua")



    
    scene.setState("Collisions",false)
    scene.setState("Stats",false)


    
end


function Object:update(dt)
    
    if (randi(0,200) <= 5) then
        local state = randi(0,3)

        if (state==0) then
           local ovni = scene.createGameObject("ufo",state, -20, randi(0,WindowWidth))
            ovni:addComponent("Sprite","ufo")
            ovni:addScript("assets/scripts/shooter/ovni.lua")
            ovni:setState("Prefab",true)
        elseif (state==1) then
            local ovni = scene.createGameObject("ufo",state,WindowWidth+20,randi(0,WindowHeight))
            ovni:addComponent("Sprite","ufo")
            ovni:addScript("assets/scripts/shooter/ovni.lua")
            ovni:setState("Prefab",true)

        end
    end
    
end

function Object:render()
    canvas.drawGraphTiled("bg", 0,0,WindowWidth,WindowHeight, 0,0,112,128,0,0,0,1.0)
    canvas.drawText("SCORE: "..points,WindowWidth/2-70,20,22)
    
end



return Object