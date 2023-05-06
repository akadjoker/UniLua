local Utils = require("utils")

function sign(x)
    return x > 0 and 1 or x < 0 and -1 or 0
  end
  
  
  local Object = {}
  Object.__index = Object
  local object = setmetatable({}, Object)

  function Object:OnReady()
      
    local spr = self:getComponent("Sprite")
    local animation = self:addAnimator()
    animation:add("run", "player_run",1,10, 10,12)
    animation:add("win", "player_win",1,10, 11,10)
    animation:add("jump", "player_jump",1,11, 11,8)
    animation:add("idle", "player_idle",1,1, 1,1)
    animation:setAnimation("idle")
    animation:play()

    self.hit =0

      self.x =0
      self.y =0
      self.vspeed = 0
      self.hspeed = 0
      self.gravity = 0.5
      self:setPosition( self.x,self.y) 
      self:setScale(0.5,0.5)
      
      self:setOrigin(15*0.5,25*0.5)
      self:setPivot(32*0.5,32*0.5) 
      self:setRotation(0)
      
      self:setSize(14,24)
      self:setOrigin(0,0)
      scene.setCameraPivot(WindowWidth/2,WindowHeight/2)
      scene.setCameraSize(WindowWidth,WindowHeight)
      scene.setCameraZoom(2)

      --self:setDebug( 1 << 1 | 1 << 2) 
      
  
  end


function Object:update(dt)
    
  local x_move = Key.check(KEY.right) - Key.check(KEY.left)
  local y_move = Key.check(KEY.down) - Key.check(KEY.up)

--   self:setRotation(self:getRotation()+1)

  self.hspeed = x_move * 5;
  self.vspeed = y_move * 5;



if (self:place_meeting(self.x , self.y, "solid")) then
   self:setSpriteColor(255,0,0,255)
  self.hit=1
else
  self:setSpriteColor(255,255,255,255)
end
  



  self.y=self.y + self.vspeed
  self.x=self.x + self.hspeed

self:setPosition(self.x,self.y)

scene.setCamera(self.x,self.y)

end

  function Object:update_tmp(dt)

      
    
    local move = Key.check(KEY.right) - Key.check(KEY.left)

    --   self:setRotation(self:getRotation()+1)
     
       self.hspeed = move * 2 * (dt*100);
       self.vspeed =self.vspeed + self.gravity * (dt*100);
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
       
     
   
      if (Key.released(KEY.space))-- and   not self:place_free(self.x, self.y + 1)) 
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

    scene.setCamera(clamp(self.x,255,460),clamp(self.y,186,293) )
   
   
   
    
   
   
   end

  
  
  
  function Object:render()
      local x,y = scene.getViewPosition()
      local x,y = scene.getWorldSpace(200,20)
      local w,h = scene.getViewSize()

    --  print ("x: " .. x .. " y:".. y .. "w" .. w .." h " ..h)
      canvas.setColor(255,0,0)
      canvas.drawText( " : velx:"  .. self.hspeed .. " vely:" .. self.vspeed, x,y , 22)
      canvas.drawCircle(x ,y+100 ,10,true)
      canvas.setColor(255,255,255)

      
  end
  
  
  
  return Object