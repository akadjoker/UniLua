print("[LUA]  load object.lua");




local Object = {}
Object.__index = Object

function Object:OnReady()
    
        
    self.data={}
    self.data.x = math.random(20,400)
    self.data.y = math.random(20,400)
    self:setPosition( self.data.x,self.data.y) 
    
end


function Object:update(dt)
    
    
end

return Object