
local Object = {}
Object.__index = Object

function Object:OnReady()
    
  self.animation = self:addComponent("Animator")
  self.animation:add("boom","explosion",4,4,16,25)
  --self.animation:setAnimation("boom")
  self.animation:setMode(2)
  self.animation:play()
  
  self:centerPivot()
  self:centerOrigin()

    
end


function Object:update(dt)
     
  if (self.animation:getCurrentFrame() == self.animation:getFrameCount()-1) then
    self:kill()
  
  end
 
end


return Object