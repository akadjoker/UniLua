
  
  local Object = {}
  Object.__index = Object
  

  local imagem = [[
  ....................
  ....................
  ....................
  ....................
  ....................
  ....................
  ....................
  .........GGG........
  .........###........
  ....................
  ....GGG.......GGG...
  ....###.......###...
  ....................
  .1................X.
  ####################
  ]]
  


  -- define as dimensões da imagem
  local width = 24
  local height = 16
  

-- cria uma matriz para armazenar os pixels
local pixels = {}

  function Object:OnReady()
    self.tiles = self:addComponent("Tiles")
   self.tiles = self:addTiles( width, height, 40, 32,"tiles",1,1)
   for y = 0, height-1 do
    for x = 0, width-1 do
        self.tiles:setTile(x, y, 0) -- Tile vazio
      
      end
    end
  
  end
  
  
  function Object:update(dt)
      
    local x =Mouse.getX()
    local y =Mouse.getY()


    x = math.floor(x/40)
    y = math.floor(y/32)
    if (Mouse.down(0)) then
        local tile = self.tiles:getTile(x, y) +1
        if (tile > 10) then
            tile = 0
        end

      self.tiles:setTile(x, y, tile)
    end
  
  end
  
  function Object:render()
    canvas.drawGraph("tiles", 100,100)
    canvas.drawText("state:" , 400, 100, 22)
    
  

    
  end
 
  
  
  
  return Object