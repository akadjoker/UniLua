  


local Object = {}
Object.__index = Object

function Object:OnReady()
   
    self.x = self:getX()
    self.y = self:getY()
    self.life=90
    self.speed =0.20
    self:setScale(0.5,0.5)
    self:addCircleCollider(0,0,5)
    
    --self:setSize(25,25)
    --self:setOrigin(0,0)
    self:centerPivot()
    --self:setPivot(0,0)
      
  
end
        

function Object:update(dt)

    self.life=self.life-1
    self.speed = self.speed + 0.2 * (dt*100)
    self:advance( self.speed * (dt*100)) 
    
    
  --  self:setPosition( self.x, self.y) 

  --  print("bullet life: " .. self.life)
  
    if (self.life <=0) then
        self:kill()
   end

end

function Object:OnCollision(other)


end

return Object
