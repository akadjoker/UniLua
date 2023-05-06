function sign(x)
  return x > 0 and 1 or x < 0 and -1 or 0
end


local Object = {}
Object.__index = Object

function Object:OnReady()
    
  local animation = self:addAnimator()
  animation:addRow("run", "player_run", 10,12)
  animation:addRow("win", "player_win", 11,10)
  animation:addRow("jump", "player_jump", 11,8)
  animation:addRow("idle", "player_idle", 1,1)
  animation:setAnimation("idle")
  animation:play()

    self.x = self:getX()
    self.y = self:getY()
    self.last_y=self.y
    self.last_x=self.x
    self.vspeed = 0
    self.hspeed = 0
    self.gravity = 0.5
    self.state = "idle"
    self:setPosition( self.x,self.y) 
    self:setSize(30,54)
    self:setOrigin(15,25)
    self:setPivot(32,32) 
    self:setRotation(0)
    self:setSpriteColor(255,255,255,255)
    

end


function Object:update(dt)
    
    
    local move = Key.check(KEY.right) - Key.check(KEY.left)

 --   self:setRotation(self:getRotation()+1)
  
    self.hspeed = move * 5;
    self.vspeed =self.vspeed + self.gravity;
 --   self.vspeed = movey *2;
    --self:setSpriteColor(255,255,255,255)


    if (self.hspeed == 0 and self.vspeed>=0.5 and self.state ~="jump") then
       
      self.state="idle"
    elseif (self.hspeed ~= 0 and self.vspeed>=0.5 ) then
      self.state="run"
    elseif (self.vspeed < 0.5) then
      self.state="jump" 
    end
  

    -- if (self.hspeed > 0) then
    --   self:setSpriteFlip(true,false)
    -- elseif (self.hspeed < 0) then
    --   self:setSpriteFlip(false,false)
    -- end

    if (Key.check(KEY.ONE)==1) then
     self:setAnimation("run")
    
      
    end

    if (Key.check(KEY.TWO) ==1)  then
      self:setAnimation("jump")
     
    end
    
  

   if (Key.check(KEY.space)==1 and  not self:place_free(self.x, self.y + 1)) 
   then
     self.vspeed = -10
     self.state = "jump"
     self:setAnimation("jump")
   end

  -- print(  self.vspeed .. "," .. self.hspeed)



    if (self:place_meeting(self.x + self.hspeed, self.y, "solid")) 
    then
      while ( not self:place_meeting(self.x + sign(self.hspeed), self.y,"solid") ) 
      do
         self.x =self.x + sign(self.hspeed)
     end
      
      self.hspeed = 0
    end

    --print( sign(self.vspeed))

   if (self:place_meeting(self.x, self.y + self.vspeed, "solid")) 
   then
       while (not self:place_meeting(self.x, self.y + sign(self.vspeed), "solid"))
       do
           self.y =self.y+ sign(self.vspeed)
       end
     self.vspeed = 0
     if (self.hspeed==0) then
       self.state="idle"
     end
   end 


   if (self.state=="run") then
     self:setAnimation("run")
     if (self.hspeed > 0) then
       self:setSpriteFlip(true,false)
      elseif (self.hspeed < 0) then
        self:setSpriteFlip(false,false)
      end
    
    elseif (self.state=="idle") then
      self:setAnimation("idle",true)
    end
  


    ---self.x= max(20, math.min(1020,self.x))
    if (self.x > 1000) then
      self.x = 1000
      self.hspeed = 0
    end
    if (self.x < 20) then
      self.x = 20
      self.hspeed=0
    end



    self.y=self.y + self.vspeed
    self.x=self.x + self.hspeed

  self:setPosition(self.x,self.y)



 


end

function Object:render()
    canvas.drawText("state:" .. self.state .. " : velx:"  .. self.hspeed .. " vely:" .. self.vspeed, 400, 10, 22)
    --print("x:" .. self.x .. " y:" .. self.y)
end



return Object