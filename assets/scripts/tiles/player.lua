function sign(x)
    return x > 0 and 1 or x < 0 and -1 or 0
  end
  
  
  local Object = {}
  Object.__index = Object
  
  function Object:OnReady()
      
    local spr = self:getComponent("Sprite")
    spr:setColor(255,0,0,255)


      self.x = WindowWidth /2
      self.y = WindowHeight /2
      self.vspeed = 0
      self.hspeed = 0
      self:setPosition( self.x,self.y) 
      self:setSize(30,54)
      self:setOrigin(15,25)
      self:setPivot(32,32) 
      self:setRotation(0)
      
      
      scene.setCameraPivot(WindowWidth/2,WindowHeight/2)
      scene.setCameraSize(WindowWidth,WindowHeight)
  
  end
  
  
  function Object:update(dt)
      
      
      local x_move = Key.check(KEY.right) - Key.check(KEY.left)
      local y_move = Key.check(KEY.down) - Key.check(KEY.up)
  
   --   self:setRotation(self:getRotation()+1)
    
      self.hspeed = x_move * 5;
      self.vspeed = y_move * 5;

  

    
      
  

  
      self.y=self.y + self.vspeed
      self.x=self.x + self.hspeed
  
    self:setPosition(self.x,self.y)

    scene.setCamera(self.x,self.y)
  
  
   
  
  
  end
  
  function Object:render()
      canvas.drawText( " : velx:"  .. self.hspeed .. " vely:" .. self.vspeed, 400, 10, 22)
      
  end
  
  
  
  return Object