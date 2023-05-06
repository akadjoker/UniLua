

local Object = {}
Object.__index = Object

function Object:OnReady()
    
        

    self.angle=self:getRotation()
    self:setPosition( window.getWidth()/2,window.getHeight()/2+100)
    self:centerPivot()
    self:addCircleCollider(0,0,50)

    self.flipX = true
    self.flipY=true
    
    --self:setSpriteColor(255,0,0,255)
    --self:setSpriteFlip(false,true)


   -- self:setPosition(99/2,75/2) 
     --self:setPivot(99/2,75/2)
     --self:setOrigin(99/2,75/2)
     --self:setSize(99,75)
      
           
end


function Object:update(dt)
  self.flipX = true
  self.flipY=true
  
    
    self:pointToMouse(0.9 * (dt *5.0),-90)
    self:advanceTo( 15 * (dt *5.0) ,self:getRotation()+90) 

--local x_move = Key.check(KEY.right) - Key.check(KEY.left)

  if (Key.pressed(KEY.Q)) then
    local explode = scene.createGameObject("explosion",5, self:getX(),self:getY())
    explode:addComponent("Sprite","explosion")
    explode:addScript("assets/scripts/shooter/explode.lua")
    explode:setPosition(self:getX(),self:getY())
    explode:setState("Prefab",true)

  end

  
end



function Object:OnPause(dt)


  canvas.drawText("PAUSE",800/2-50,640/2,22)

  
  
end

-- function Object:render()

--   canvas.drawText(" x ".. math.floor(self:getX()) .. " y ".. math.floor(self:getY()) ,10,10,22)
--   canvas.drawCircle(math.floor(self:getX()), math.floor(self:getY()) ,10)
  

-- end

return Object
