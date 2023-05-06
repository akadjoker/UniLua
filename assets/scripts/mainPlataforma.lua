
local grid_width = 40
local grid_height = 32

local Object = {}
Object.__index = Object
local object = setmetatable({}, Object)

function Object:addObject(x, y, w ,h )
    local solid = scene.createGameObject("solid", 0)
    solid:setPosition(x, y)
    solid:setSize(w,h)
    solid:setOrigin(w/2,h/2)
    solid:setSolid(true)
    solid:setPrefab(true)
    solid:addScript("assets/scripts/plataforma/object.lua") 
end

function Object:addTile(spr,x, y) 
    local tile = scene.createGameObject("solid", 1)
    tile:addSprite(spr)
    tile:setPosition(x, y)
    tile:setSize(40,32)
    tile:setOrigin(40/2,32/2)
    tile:setPivot(20,16) 
    tile:setSolid(false)
    tile:setPrefab(false)

end

function Object:addTileDepth(spr,x, y) 
    local tile = scene.createGameObject("decor", 1)
    tile:addSprite(spr)
    tile:setPosition(x, y)
    tile:setSize(40,32)
    tile:setOrigin(40/2,32/2)
    tile:setPivot(20,16) 
    tile:setSolid(false)
    tile:setPrefab(false)
    tile:setCollidable(false)
    

end

function Object:OnReady()
    
   scene.load("assets/scenes/plataforma.json")
    assets.loadGraph("tile0", "assets/Tiles/BlockA0.png")
    assets.loadGraph("tile1", "assets/Tiles/BlockA3.png")
    assets.loadGraph("tile2", "assets/Tiles/BlockB0.png")
    assets.loadGraph("exit", "assets/Tiles/Exit.png")
    assets.loadGraph("platform", "assets/Tiles/Platform.png")


    assets.loadGraph("player_run", "assets/Player/Run.png")
    assets.loadGraph("player_jump", "assets/Player/Jump.png")
    assets.loadGraph("player_die", "assets/Player/Die.png")
    assets.loadGraph("player_idle", "assets/Player/Idle.png")
    assets.loadGraph("player_win", "assets/Player/Celebrate.png")
    

    self.object_x=0
    self.object_y=0
      
    -- object:addObject("tile0",400, 390,100,40)  
    -- object:addObject("tile0",400, 390,100,40)  
    -- object:addObject(200, 400,100,40)  
    -- object:addObject(250, 320,20,100)  
     --object:addTile("tile0",1020/2, 740,1020,40)  
  
    object:addObject(1020/2, 740,1020,40)  
    object:addObject(10, 740/2,20,740)  
    object:addObject(1010, 740/2,20,740)  



     local p = scene.createGameObject("Player",0)
     p:setPosition(400,100)
     p:setPrefab(true)
     p:addSprite("player_idle")
     p:addScript("assets/scripts/plataforma/player.lua")


        

 
    -- p:addScript("assets/scripts/shooter/player.lua")


    -- wabbit->AddComponent<SpriteComponent>("player_idle");
    -- Animator *animation = wabbit->AddComponent<Animator>();
    -- animation->AddRow("run", "player_run", 10,12);
    -- animation->AddRow("win", "player_win", 11,10);
    -- animation->AddRow("jump", "player_jump", 11,8);
    -- animation->AddRow("idle", "player_idle", 1,1);
    -- animation->SetAnimation("jump");
    -- animation->Play();
    


    
end

function Object:OnRemove()

scene.save("assets/scenes/plataforma.json")


end

function Object:update(dt)
    
    -- print("   main update") 

    local mouse_x, mouse_y = Mouse.getLocal()
    self.object_x = math.floor(mouse_x/grid_width) * grid_width 
    self.object_y = math.floor(mouse_y/grid_height) * grid_height 

    if (Mouse.pressed(0)) then

        if (Key.down(KEY.ONE)) then
            object:addTile("tile0",self.object_x, self.object_y)
        end
        if (Key.down(KEY.TWO)) then
            object:addTile("tile1",self.object_x, self.object_y)
        end
        if (Key.down(KEY.THREE)) then
            object:addTile("tile2",self.object_x, self.object_y)
        end

        if (Key.down(KEY.Q)) then
            
           object:addTileDepth("exit",self.object_x, self.object_y)
        end
        if (Key.down(KEY.W)) then
           object:addTileDepth("platform",self.object_x, self.object_y)
        end
        
    end

    if (Mouse.pressed(1)) then
       
        local ob =scene.mousePick()
        if (ob ~= nil) then
    
            ob:kill()
            ob=nil
        end
    end




end

function Object:render()
    canvas.drawRectangle(self.object_x-20,self.object_y-(32/2),40,32)
end

return Object