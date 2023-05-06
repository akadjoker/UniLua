
local Object = {}
Object.__index = Object

function Object:OnReady()
    
 
    self.player = scene.findGameObject("Player")
    self:centerPivot()
    self:centerOrigin()
    self.panic = false
    self.speed = randf(0.2,0.8)
    self.size  = randf(0.5,1.0)
    self.anim =0
    self.pump=0
    self.energia=5
    self.timer =5
    
    


    
end


function Object:update(dt)
     
    self.anim= self.anim  + dt *1.2
    self.pump = ping_pong(self.anim,0.2)

    self:faceTo(self.player)
    self:advance(self.speed)
    if (self.panic) then
        self:setSpriteColor(255,0,0,0)
        self.panic=false
        self.timer = self.timer  -1 * (dt *100)
    else
        self:setSpriteColor(255,255,255,255)
        if (self.timer <= 0) then
            self.timer = 0;
            self.panic = false
        end
    end

    if self:place_meeting(self:getX(),self:getY(),"bullet") then
        self:setSpriteColor(255,0,0,255)
        self.panic=true
        self.energia = self.energia - 1
        self.timer = 5
    end

    
 

 self:setScale(self.size + self.pump,self.size + self.pump)
 if (self.energia <= 0) then
    self:kill()
    points = points + 1

    local explode = scene.createGameObject("explosion",5, self:getX(),self:getY())
    explode:addComponent("Sprite","explosion")
    explode:addScript("assets/scripts/shooter/explode.lua")
    explode:setPosition(self:getX(),self:getY())
    explode:setState("Prefab",true)
 end
 
end


function Object:OnCollision(other)

    if (other.name == "bullet") then
        other:kill()
    end
    

end


-- function Object:render()

--     canvas.drawText(" x ".. math.floor(self:getX()) .. " y ".. math.floor(self:getY()) ,10,10,22)
--     canvas.drawCircle(math.floor(self:getX()), math.floor(self:getY()) ,10)
    
  
--   end

return Object