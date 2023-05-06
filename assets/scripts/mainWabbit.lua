
local utils = require("assets/scripts/utils")


local Object = {}
Object.__index = Object

function Object:OnReady()
    
   -- for i=1, 5000 do
        local layer =math.floor(math.random()*6)
        local bullet = scene.createGameObject("wabbit",layer)
        bullet:addSprite("wabbit")
        bullet:addScript("assets/scripts/wabbit/wabbit.lua") 
  --  end
    
    
end


function Object:update(dt)
    
    if (Mouse.down(0)) then
        local x = Mouse.getX()
        local y = Mouse.getY()
        local layer =math.floor(math.random()*8)
        local bullet = scene.createGameObject("wabbit",layer)
        bullet:addSprite("wabbit")
        bullet:addScript("assets/scripts/wabbit/wabbit.lua") 

    
    end
    
end



return Object