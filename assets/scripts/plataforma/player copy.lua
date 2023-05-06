function sign(x)
  return x > 0 and 1 or x < 0 and -1 or 0
end


local Object = {}
Object.__index = Object

function Object:OnReady()
    

    self.x = 200
    self.y = 200
    self.last_y=self.y
    self.last_x=self.x
    self.vspeed = 0
    self.hspeed = 0
    self:centerPivot()

    self:setPosition( self.x,self.y) 
    self:setSize(25,25)
    self:setOrigin(12,12)

 

    

end


function Object:update(dt)
    
    local speed = 2 * (dt *100)
    local move = Key.check(KEY.right) - Key.check(KEY.left)

  


  

    self.hspeed = move * speed;
    self.x = self.x + self.hspeed    *  speed

   
    if (self:place_free(self.x + self.hspeed,self.y )==false) then
    
       -- while(self:place_free(self.x + sign(self.hspeed),self.y )==true) do
       --     self.x = self.x + sign(self.hspeed)
       -- end
        self.hspeed=0
        self.x=self.last_x
    end



  --  if (self.y <= 400) then
      self.vspeed = self.vspeed + 0.1 * speed
 --   else
  --    self.vspeed = 0
 --   end

    if (self:place_free(self.x,self.y + self.vspeed ) ==false) then
       -- while(self:place_free(self.x,self.y + sign(self.vspeed) )) do
       --     self.y = self.y - sign(self.vspeed)
       -- end
        self.vspeed = 0
        self.y=self.last_y
    end

    if (Key.check(KEY.space)==1 and self.vspeed == 0) then
      self.vspeed = -2 * speed
    end

    self.y =  self.y + self.vspeed   * speed 

   

    ---self.x= max(20, math.min(1020,self.x))
    if (self.x > 1000) then
      self.x = 1000
      self.hspeed = 0
    end
    if (self.x < 20) then
      self.x = 20
      self.hspeed=0
    end


 --  self.x=Mouse.getX()
 --  self.y=Mouse.getY()




   if (free==true) 
   then
       self:setSpriteColor(255,0,0,255)
     --  print("free")
    else
        self:setSpriteColor(255,255,255,255)
      --  print("not free")
    end



   --print(free) 

  -- self:setPosition(self.x, self.y)
  self:setPosition(self.x,self.y)
  self.last_y=self.y
  self.last_x=self.x

 


end



return Object