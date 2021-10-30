require("flappy_bird_guids")
require("flappy_bird_types")

gameState = GameState.STARTING

backgroundLayers = {
    ground = BackgroundLayer.new(0.1, groundTextureGUID, 100, 0, 1.0),
    trees = BackgroundLayer.new(-0.2, treeTextureGUID, 40, 100, 0.2),
    skyline = BackgroundLayer.new(-0.3, skylineTextureGUID, 60, 100 + 40 / 3, 0.1),
    clouds = BackgroundLayer.new(-0.4, cloudTextureGUID, 90, 100 + 40 / 2, 0.06)
}

bird = Bird.new(80, 30)

cheatMode = false

screenSize = Vec2.new()
screenSize.x = 800
screenSize.y = 600

digitSize = 64

pipeWidth = 100
verticalPipeGap = 150
horizontalPipeGap = 400
availablePipeHeight = screenSize.y - backgroundLayers.ground.height - verticalPipeGap

initialHorizontalMovementSpeed = 200
horizontalMovementSpeed = initialHorizontalMovementSpeed

fireworksEntities = {}
fireworksVerticalOffset = 15
score = 0

function checkPipeCollision(pipePosition)
    local left = pipePosition.x - pipeWidth / 2 - bird.collisionRadius
    local right = pipePosition.x + pipeWidth / 2 + bird.collisionRadius
    local top = pipePosition.y + pipeHeight / 2 + bird.collisionRadius
    local bottom = pipePosition.y - pipeHeight / 2 - bird.collisionRadius
    if bird.transform.position.x < left or bird.transform.position.x > right or
            bird.transform.position.y > top or bird.transform.position.y < bottom then
        return false
    end
    local v = Vec2.new()
    if bird.transform.position.y >= top - bird.collisionRadius and bird.transform.position.y <= top then
        v.y = top - bird.collisionRadius - bird.transform.position.y
        if bird.transform.position.x >= left and bird.transform.position.x <= left + bird.collisionRadius then
            -- left top sub-square
            v.x = left + bird.collisionRadius - bird.transform.position.x
            return v.x * v.x + v.y * v.y <= bird.collisionRadius * bird.collisionRadius
        elseif bird.transform.position.x >= right - bird.collisionRadius and bird.transform.position.x <= right then
            -- right top sub-square
            v.x = right - bird.collisionRadius - bird.transform.position.x
            return v.x * v.x + v.y * v.y <= bird.collisionRadius * bird.collisionRadius
        end
    elseif bird.transform.position.y <= bottom + bird.collisionRadius and bird.transform.position.y >= bottom then
        v.y = bottom + bird.collisionRadius - bird.transform.position.y
        if bird.transform.position.x >= left and bird.transform.position.x <= left + bird.collisionRadius then
            -- left bottom sub-square
            v.x = left + bird.collisionRadius - bird.transform.position.x
            return v.x * v.x + v.y * v.y <= bird.collisionRadius * bird.collisionRadius
        elseif bird.transform.position.x >= right - bird.collisionRadius and bird.transform.position.x <= right then
            -- right bottom sub-square
            v.x = right - bird.collisionRadius - bird.transform.position.x
            return v.x * v.x + v.y * v.y <= bird.collisionRadius * bird.collisionRadius
        end
    end
    return true
end

function setupDigits()
    digitsSpriteSheet = c2k.assets.spriteSheet(digitsSpriteSheetGUID)
    digitsTexture = c2k.assets.texture(digitsTextureGUID)
    digitEntities = {}
end

function showNumber(number)
    for i = 1, #digitEntities do
        digitEntities[i]:destroy()
    end
    digitEntities = {}
    local digits = {}
    if number == 0 then
        digits[1] = 0
    else
        while number > 0 do
            digits[#digits + 1] = math.fmod(number, 10)
            number = math.floor(number / 10)
        end
    end
    for i = #digits, 1, -1 do
        local entityIndex = #digits - i + 1
        digitEntities[entityIndex] = Entity.new()
        digitEntities[entityIndex]:attachRoot()
        local transform = digitEntities[entityIndex]:attachTransform()
        transform.scale.x = digitSize / 2
        transform.scale.y = digitSize / 2
        transform.position.x = -(#digits / 2 - 0.5) * digitSize + (entityIndex - 1) * digitSize
        transform.position.y = screenSize.y / 4 + digitSize / 2
        transform.position.z = 0.5
        local sprite = digitEntities[entityIndex]:attachDynamicSprite()
        sprite.texture = digitsTexture
        sprite.textureRect = digitsSpriteSheet.frames[digits[i] + 1].rect
    end
end

function setupBackgroundLayer(layerTransforms, depth, textureGUID, spriteHeight, heightOffset)
    -- returns spriteWidth
    local texture = c2k.assets.texture(textureGUID)
    local textureAspect = texture.width / texture.height
    local spriteWidth = spriteHeight * textureAspect
    local numberOfTiles = math.floor(screenSize.x / spriteWidth) + 2
    for i = 1, numberOfTiles do
        local entity = Entity.new()
        entity:attachRoot()
        layerTransforms[i] = entity:attachTransform()
        layerTransforms[i].scale.x = spriteWidth / 2
        layerTransforms[i].scale.y = spriteHeight / 2
        layerTransforms[i].position.x = (-screenSize.x + spriteWidth) / 2 + (i - 1) * spriteWidth
        layerTransforms[i].position.y = (-screenSize.y + spriteHeight) / 2 + heightOffset
        layerTransforms[i].position.z = depth
        local sprite = entity:attachDynamicSprite()
        sprite.texture = texture
    end
    return spriteWidth
end

function setupPipeData()
    topPipeTexture = c2k.assets.texture(topPipeTextureGUID)
    bottomPipeTexture = c2k.assets.texture(bottomPipeTextureGUID)
    pipeTextureAspect = topPipeTexture.width / topPipeTexture.height
    pipeHeight = pipeWidth / pipeTextureAspect
    nextPipeSpawnTime = time.elapsed + 1
    pipeEntities = {}
end

function setStartingBirdTransform()
    bird.transform.scale.x = bird.width / 2
    bird.transform.scale.y = birdHeight / 2
    bird.transform.position.x = -screenSize.x / 4
    bird.transform.position.y = backgroundLayers.ground.height / 2
    bird.transform.rotation = 0
    bird.verticalVelocity = 0
end

function setupBird()
    local birdTexture = c2k.assets.texture(birdTextureGUID)
    local birdTextureAspect = birdTexture.width / birdTexture.height
    birdHeight = bird.width / birdTextureAspect
    local birdEntity = Entity.new()
    birdEntity:attachRoot()
    bird.transform = birdEntity:attachTransform()
    setStartingBirdTransform()
    local birdSprite = birdEntity:attachDynamicSprite()
    birdSprite.texture = birdTexture
    gravity = -9.81 * bird.transform.scale.y * 7.52
    bird.verticalVelocityOnClick = 18 * bird.transform.scale.y
end

function setupBackground()
    for _, layer in pairs(backgroundLayers) do
        layer.width = setupBackgroundLayer(layer.transforms, layer.depth, layer.textureGUID, layer.height, layer.heightOffset)
    end
end

function onAttach(_)
    input = c2k.getInput()
    time = c2k.getTime()

    setupDigits()
    showNumber(0)
    setupPipeData()
    setupBackground()
    setupBird()
end

function restartGame()
    setStartingBirdTransform()
    gameState = GameState.STARTING
    for i = 1, #pipeEntities do
        pipeEntities[i]:destroy()
    end
    pipeEntities = {}
    horizontalMovementSpeed = initialHorizontalMovementSpeed
    score = 0
    showNumber(0)
end

function updateBackgroundLayer(layerTransforms, tileWidth, movementSpeedFactor)
    local leftMostTileIndex = 0
    local minTilePositionX = 0
    for i = 1, #layerTransforms do
        layerTransforms[i].position.x = layerTransforms[i].position.x - horizontalMovementSpeed * movementSpeedFactor * time.delta
        if leftMostTileIndex == 0 or layerTransforms[i].position.x < minTilePositionX then
            leftMostTileIndex = i
            minTilePositionX = layerTransforms[i].position.x
        end
    end
    if layerTransforms[leftMostTileIndex].position.x + tileWidth / 2 < -screenSize.x / 2 then
        local rightMostTileIndex = leftMostTileIndex - 1
        if rightMostTileIndex == 0 then
            rightMostTileIndex = #layerTransforms
        end
        layerTransforms[leftMostTileIndex].position.x = layerTransforms[rightMostTileIndex].position.x + tileWidth
    end
end

function updateBackground()
    for _, layer in pairs(backgroundLayers) do
        updateBackgroundLayer(layer.transforms, layer.width, layer.movementSpeedFactor)
    end
end

function spawnPipes()
    local verticalPipeGapPosition = availablePipeHeight * math.random()
            + backgroundLayers.ground.height + (verticalPipeGap - screenSize.y) / 2
    local index = #pipeEntities + 1
    for pipeNumber = 1, 2 do
        pipeEntities[index] = Entity.new()
        pipeEntities[index]:attachRoot()
        local pipeTransform = pipeEntities[index]:attachTransform()
        pipeTransform.position.x = (screenSize.x + pipeWidth) / 2
        pipeTransform.position.z = -0.1
        pipeTransform.scale.x = pipeWidth / 2
        pipeTransform.scale.y = pipeHeight / 2
        local pipeSprite = pipeEntities[index]:attachDynamicSprite()
        if pipeNumber == 1 then
            pipeSprite.texture = bottomPipeTexture
            pipeTransform.position.y = verticalPipeGapPosition - (pipeHeight + verticalPipeGap) / 2
        else
            pipeSprite.texture = topPipeTexture
            pipeTransform.position.y = verticalPipeGapPosition + (pipeHeight + verticalPipeGap) / 2
        end
        index = index + 1
    end
    nextPipeSpawnTime = time.elapsed + (horizontalPipeGap + pipeWidth) / horizontalMovementSpeed
    horizontalPipeGap = horizontalPipeGap - 5
    if horizontalPipeGap < 2 * pipeWidth then
        horizontalPipeGap = 2 * pipeWidth
    end
end

function updatePipes()
    local birdPassedPipePair = false
    local passedPipe = 0
    for i = 1, #pipeEntities do
        local pipeTransform = pipeEntities[i]:getTransform()
        local passedBeforeMove = bird.transform.position.x > pipeTransform.position.x + pipeWidth / 2
        pipeTransform.position.x = pipeTransform.position.x - horizontalMovementSpeed * time.delta
        if not birdPassedPipePair then
            local passedAfterMove = bird.transform.position.x > pipeTransform.position.x + pipeWidth / 2
            if not passedBeforeMove and passedAfterMove then
                birdPassedPipePair = true
                passedPipe = i
            end
        end
    end
    if birdPassedPipePair and gameState == GameState.PLAYING then
        score = score + 1
        showNumber(score)
        local topPipeIndex = passedPipe + 1
        local transform = pipeEntities[topPipeIndex]:getTransform()
        local position = Vec3.new()
        position.x = transform.position.x
        position.y = transform.position.y - pipeHeight / 2 + fireworksVerticalOffset
        position.z = -0.2
        spawnFireworks(position)
    end

    if #pipeEntities > 1 then
        local leftMostPipeTransform = pipeEntities[1]:getTransform()
        if leftMostPipeTransform.position.x + pipeWidth / 2 < -screenSize.x then
            pipeEntities[1]:destroy()
            pipeEntities[2]:destroy()
            for i = 1, #pipeEntities - 2 do
                pipeEntities[i] = pipeEntities[i + 2]
            end
            pipeEntities[#pipeEntities] = nil
            pipeEntities[#pipeEntities] = nil
        end
    end
end

function lerp(from, to, t)
    if t < 0 then
        return from
    elseif t > 1 then
        return to
    else
        return (1 - t) * from + t * to
    end
end

function smoothStep(from, to, t)
    if t < 0 then
        return from
    elseif t > 1 then
        return to
    else
        return (3 * t * t - 2 * t * t * t) * (to - from) + from
    end
end

function jumpPressed()
    return input:mousePressed(MouseButton.Left) or input:keyPressed(Key.Space)
end

function doBirdJump()
    bird.verticalVelocity = bird.verticalVelocityOnClick
end

function updateBird()
    if gameState == GameState.PLAYING and cheatMode then
        local leftIndex = 0
        local rightIndex = 0
        local leftX = 0
        local rightX = 0
        for i = 1, #pipeEntities, 2 do
            local transform = pipeEntities[i]:getTransform()
            if transform.position.x < bird.transform.position.x and (leftIndex == 0 or leftX < transform.position.x) then
                leftIndex = i
                leftX = transform.position.x
            elseif transform.position.x > bird.transform.position.x and (rightIndex == 0 or rightX > transform.position.x) then
                rightIndex = i
                rightX = transform.position.x
            end
        end
        if leftIndex == 0 and rightIndex == 0 then
            return
        end
        local targetIndex = 0
        local targetY = 0
        if leftIndex == 0 then
            targetIndex = rightIndex
            local targetTransform1 = pipeEntities[targetIndex]:getTransform()
            local targetTransform2 = pipeEntities[targetIndex + 1]:getTransform()
            targetY = (targetTransform1.position.y + targetTransform2.position.y) / 2
        else
            local targetTransformLeft1 = pipeEntities[leftIndex]:getTransform()
            local targetTransformLeft2 = pipeEntities[leftIndex + 1]:getTransform()
            local targetTransformRight1 = pipeEntities[rightIndex]:getTransform()
            local targetTransformRight2 = pipeEntities[rightIndex + 1]:getTransform()
            local targetLeftY = (targetTransformLeft1.position.y + targetTransformLeft2.position.y) / 2
            local targetRightY = (targetTransformRight1.position.y + targetTransformRight2.position.y) / 2
            local interpolationParameter = (bird.transform.position.x - leftX) / (rightX - leftX)
            targetY = smoothStep(targetLeftY, targetRightY, interpolationParameter)
        end
        bird.transform.position.y = targetY
        bird.transform.rotation = 0
        return
    end
    local wasJumpPressed = jumpPressed()
    if gameState == GameState.BIRD_DOWN then
        if wasJumpPressed then
            restartGame()
        end
        return
    end
    if gameState == GameState.STARTING then
        bird.transform.position.y = math.sin(time.elapsed * 4) * 14
        if wasJumpPressed then
            gameState = GameState.PLAYING
            doBirdJump()
        end
        return
    end
    if wasJumpPressed and gameState ~= GameState.COLLIDED and gameState ~= GameState.BIRD_DOWN then
        doBirdJump()
    end
    bird.verticalVelocity = bird.verticalVelocity + gravity * time.delta
    bird.transform.position.y = bird.transform.position.y + bird.verticalVelocity * time.delta
    targetRotation = math.atan(bird.verticalVelocity / horizontalMovementSpeed)
    bird.transform.rotation = lerp(bird.transform.rotation, targetRotation, time.delta * 3.5)
    if bird.transform.position.y + birdHeight / 2 > screenSize.y / 2 then
        bird.transform.position.y = (screenSize.y - birdHeight) / 2
        bird.verticalVelocity = 0
    end
    if bird.transform.position.y <= -screenSize.y / 2 + backgroundLayers.ground.height then
        bird.transform.position.y = -screenSize.y / 2 + backgroundLayers.ground.height
        gameState = GameState.BIRD_DOWN
        horizontalMovementSpeed = 0
    end
end

function checkCollisions()
    for i = 1, #pipeEntities do
        local transform = pipeEntities[i]:getTransform()
        local collided = checkPipeCollision(transform.position)
        if collided then
            gameState = GameState.COLLIDED
            break
        end
    end
end

function spawnFireworks(position)
    local particleSystem = c2k.assets.particleSystem(fireworksParticleSystemGUID)
    local particleEmitterEntity = Entity.new()
    particleEmitterEntity:attachRoot()
    local transform = particleEmitterEntity:attachTransform()
    transform.position.x = position.x
    transform.position.y = position.y
    transform.position.z = position.z
    particleEmitterEntity:attachParticleEmitter(particleSystem)
    table.insert(fireworksEntities, particleEmitterEntity)
end

function handleFireworks()
    local entitiesToDelete = 0
    for i = 1, #fireworksEntities do
        local transform = fireworksEntities[i]:getTransform()
        transform.position.x = transform.position.x - horizontalMovementSpeed * time.delta
        if transform.position.x + pipeWidth * 2 < -screenSize.x / 2 then
            fireworksEntities[i]:destroy()
            entitiesToDelete = entitiesToDelete + 1
        end
    end
    -- this implementation only works as long the oldest entities occupy the
    -- lowest indices
    for _ = 1, entitiesToDelete do
        table.remove(fireworksEntities, 1)
    end
end

function update(_)
    if input:keyPressed(Key.Escape) then
        c2k.application.quit()
    end
    if input:keyPressed(Key.C) then
        cheatMode = not cheatMode
    end
    updateBackground()
    if gameState == GameState.PLAYING and time.elapsed >= nextPipeSpawnTime then
        spawnPipes()
    end
    updatePipes()
    updateBird()
    if gameState == GameState.PLAYING then
        checkCollisions()
    end
    handleFireworks()
end