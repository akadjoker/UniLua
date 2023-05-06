local Utils = require("utils")


local Object = {}
Object.__index = Object

function Object:OnReady()

   

    
    self.minX=0
    self.minY=0
    self.maxX=1020
    self.maxY=720
    self.live=100



    self.x = Mouse.getX()
    self.y = Mouse.getY()
    self.speedX=  (math.random() * 5) -2.5
    self.speedY=  (math.random() * 5) - 2.5 
    self:setPosition( self.x,self.y) 
    self.angle =0
    --self:setDebug( flags.SHOW_BOX | flags.SHOW_ORIGIN)

    

   -- self:setDebug(Utils.flags.SHOW_ALL)
    


    --155 154
    self.data={}
    self.data.rotation =math.floor( self.angle)
    self.data.x=self.x
    self.data.y=self.y

    self.data.id = id
  

    -- --self:sendMessage(self.data)
    -- --self:sendMessageTo(self.data, "scene")

    

end


function Object:update(dt)
    
    local speed = (dt *100)
    
    self.x = self.x + self.speedX    *  speed
    self.speedY = self.speedY + 0.01 * speed
    self.y =  self.y + self.speedY   * speed 

    if self.x > self.maxX then
        self.speedX = -0.9 
        self.x = self.maxX
    elseif self.x < self.minX then
        self.speedX = -0.9 
        self.x = self.minX
    end

    if self.y > self.maxY then
         self.speedY = self.speedY * -0.9
         self.y = self.maxY
    elseif self.y < self.minY then
        self.speedY = self.speedY * -0.9
        self.y = self.minY
    end

    --local x = Mouse.getX()
    --local y = Mouse.getY()
    self.angle = self.angle + 0.1
    self.data.rotation = self.angle


  self:setPosition(self.x, self.y)
  self:setRotation(self:getRotation() +5.1)
  
    

    self.live = self.live -1
    if self.live < 0 then
        self:kill()
    end

end



return Object