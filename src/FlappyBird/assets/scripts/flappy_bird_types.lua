GameState = {
    STARTING = 1,
    PLAYING = 2,
    COLLIDED = 3,
    BIRD_DOWN = 4
}

BackgroundLayer = {}

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

Bird = {}

function Bird.new(width, collisionRadius)
    local result = {
        width = width,
        collisionRadius = collisionRadius,
        verticalVelocity = 0,
        transform = 0
    }
    return result
end