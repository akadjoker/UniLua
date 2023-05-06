function printTable(t, indent)
    indent = indent or 0
    for k, v in pairs(t) do
        if type(v) == "table" then
            print(string.rep("  ", indent) .. tostring(k) .. " = {")
            printTable(v, indent + 1)
            print(string.rep("  ", indent) .. "}")
        else
            print(string.rep("  ", indent) .. tostring(k) .. " = " .. tostring(v))
        end
    end
  end

local Object = {}
Object.__index = Object

function Object:OnReady()
    
    
    
    self.data={}
    self.angle=self:getRotation()
    self.firing_delay=0
    self.bounce =-5.0
    self.y = self:getY()
    self.x = self:getX()
    self.recoil=0
    self:centerPivot()
   -- self:setOrigin(20,40)
    --self.parent=scene.getGameObjectByName("player")
    

    print("Script cano reload .")
    self.flipX=true
    self.red=255
    self.green=0

--    printTable(self)


    
end


function Object:update(dt)
    
    
    self.firing_delay=self.firing_delay-1
    self.recoil = max(0, self.recoil -1)
  
   
    --print(self:getPosition())
    if (Mouse.down(0) and  self.firing_delay<0 ) then
        self.firing_delay=12
        self.recoil =8

        

        local bullet = scene.createGameObject("bullet",0)
        local x,y =self:getWorldPoint(5,-10)

    
        bullet:setRotation(self:getWorldRotation()+90)
        bullet:setPosition(x,y)
        bullet:addSprite("bala")
        bullet:addScript("assets/scripts/shooter/bullet.lua") 
       
    end

    self.y = 30  -  lengthdir_y(self.recoil, 90)

    self:setPosition( self.x, self.y)

  
    

end

return Object