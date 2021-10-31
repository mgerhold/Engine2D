BackgroundLayer = {}
BackgroundLayer.__index = BackgroundLayer

function BackgroundLayer.new(depth, textureGUID, height, heightOffset, movementSpeedFactor)
    local result = {
        transforms = {},
        depth = depth,
        textureGUID = textureGUID,
        height = height,
        width = 0,
        heightOffset = heightOffset,
        movementSpeedFactor = movementSpeedFactor
    }
    return result
end

BackgroundLayerContainer = {}
BackgroundLayerContainer.__index = BackgroundLayerContainer

function BackgroundLayerContainer.new(screenSize, backgroundLayers)
    for _, layer in pairs(backgroundLayers) do
        local texture = c2k.assets.texture(layer.textureGUID)
        local textureAspect = texture.width / texture.height
        layer.width = layer.height * textureAspect
        local numberOfTiles = math.floor(screenSize.x / layer.width) + 2
        for i = 1, numberOfTiles do
            local entity = Entity.new()
            entity:attachRoot()
            layer.transforms[i] = entity:attachTransform()
            layer.transforms[i].scale.x = layer.width / 2
            layer.transforms[i].scale.y = layer.height / 2
            layer.transforms[i].position.x = (-screenSize.x + layer.width) / 2 + (i - 1) * layer.width
            layer.transforms[i].position.y = (-screenSize.y + layer.height) / 2 + layer.heightOffset
            layer.transforms[i].position.z = layer.depth
            local sprite = entity:attachDynamicSprite()
            sprite.texture = texture
        end
    end
    local result = {
        layers = backgroundLayers,
        screenSize = screenSize
    }
    setmetatable(result, BackgroundLayerContainer)
    return result
end

function BackgroundLayerContainer:update(horizontalMovementSpeed)
    for _, layer in pairs(self.layers) do
        local leftMostTileIndex = 0
        local minTilePositionX = 0
        for i = 1, #layer.transforms do
            layer.transforms[i].position.x = layer.transforms[i].position.x - horizontalMovementSpeed * layer.movementSpeedFactor * time.delta
            if leftMostTileIndex == 0 or layer.transforms[i].position.x < minTilePositionX then
                leftMostTileIndex = i
                minTilePositionX = layer.transforms[i].position.x
            end
        end
        if layer.transforms[leftMostTileIndex].position.x + layer.width / 2 < -self.screenSize.x / 2 then
            local rightMostTileIndex = leftMostTileIndex - 1
            if rightMostTileIndex == 0 then
                rightMostTileIndex = #layer.transforms
            end
            layer.transforms[leftMostTileIndex].position.x = layer.transforms[rightMostTileIndex].position.x + layer.width
        end
    end
end