FireworksContainer = {}
FireworksContainer.__index = FireworksContainer

function FireworksContainer.new(particleSystemGUID, screenSize, offscreenDespawnOffset)
    local result = {
        particleSystemGUID = particleSystemGUID,
        entities = {},
        offscreenDespawnOffset = offscreenDespawnOffset,
        screenSize = screenSize
    }
    setmetatable(result, FireworksContainer)
    return result
end

function FireworksContainer:spawn(position)
    local particleSystem = c2k.assets.particleSystem(self.particleSystemGUID)
    local particleEmitterEntity = Entity.new()
    particleEmitterEntity:attachRoot()
    local transform = particleEmitterEntity:attachTransform()
    transform.position = position
    particleEmitterEntity:attachParticleEmitter(particleSystem)
    table.insert(self.entities, particleEmitterEntity)
end

function FireworksContainer:update(horizontalMovementSpeed)
    local numEntitiesToDelete = 0
    for _, entity in ipairs(self.entities) do
        local transform = entity:getTransform()
        transform.position.x = transform.position.x - horizontalMovementSpeed * time.delta
        if transform.position.x + self.offscreenDespawnOffset < -self.screenSize.x / 2 then
            entity:destroy()
            numEntitiesToDelete = numEntitiesToDelete + 1
        end
    end
    -- this implementation only works as long the oldest entities occupy the
    -- lowest indices
    for _ = 1, numEntitiesToDelete do
        table.remove(self.entities, 1)
    end
end

function FireworksContainer:clear()
    for _, entity in ipairs(self.entities) do
        entity:destroy()
    end
    self.entities = {}
end