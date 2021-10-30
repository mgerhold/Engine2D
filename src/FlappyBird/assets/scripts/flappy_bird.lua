groundTextureGUID = "6e105d9e-5b8b-44ae-8b21-03f8fee5357c"
topPipeTextureGUID = "aedbb58b-f2d8-44d4-b046-4e27ed36141c"
bottomPipeTextureGUID = "1129191c-9108-4293-823a-e3e8f13f32c4"
birdTextureGUID = "15a01f9b-c970-43e1-bbe8-79a5b04a58e6"
treeTextureGUID = "fe8b71f6-1fcd-48fb-86af-d6815a3eb0fd"
skylineTextureGUID = "f7cf0445-f962-4d78-90ab-eede24e46960"
cloudTextureGUID = "9780697b-37d4-4223-9100-5ecfe6d2f490"
digitsSpriteSheetGUID = "092cb082-c71e-4b1e-9067-a555848d2211"
digitsTextureGUID = "aa9af365-c53f-4719-aff3-be3292a65636"
fireworksParticleSystemGUID = "4d77a770-2c5c-4c7d-973d-da19d0b9a856"

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

gameState = GameState.STARTING

backgroundLayers = {
    BackgroundLayer.new(0.1, groundTextureGUID, 100, 0, 1.0),
    BackgroundLayer.new(-0.2, treeTextureGUID, 40, 100, 0.2),
    BackgroundLayer.new(-0.3, skylineTextureGUID, 60, 100 + 40 / 3, 0.1),
    BackgroundLayer.new(-0.4, cloudTextureGUID, 90, 100 + 40 / 2, 0.06)
}

cheatMode = false

screenSize = Vec2.new()
screenSize.x = 800
screenSize.y = 600

digitSize = 64

pipeWidth = 100
verticalPipeGap = 150
horizontalPipeGap = 400
availablePipeHeight = screenSize.y - backgroundLayers[1].height - verticalPipeGap

birdWidth = 80
birdCollisionRadius = 30 / 80 * birdWidth
verticalBirdVelocity = 0
birdRotation = 0

initialHorizontalMovementSpeed = 200
horizontalMovementSpeed = initialHorizontalMovementSpeed

fireworksEntities = {}
fireworksVerticalOffset = 15
score = 0

function checkPipeCollision(pipePosition)
    local left = pipePosition.x - pipeWidth / 2 - birdCollisionRadius
    local right = pipePosition.x + pipeWidth / 2 + birdCollisionRadius
    local top = pipePosition.y + pipeHeight / 2 + birdCollisionRadius
    local bottom = pipePosition.y - pipeHeight / 2 - birdCollisionRadius
    if birdTransform.position.x < left or birdTransform.position.x > right or
            birdTransform.position.y > top or birdTransform.position.y < bottom then
        return false
    end
    local v = Vec2.new()
    if birdTransform.position.y >= top - birdCollisionRadius and birdTransform.position.y <= top then
        v.y = top - birdCollisionRadius - birdTransform.position.y
        if birdTransform.position.x >= left and birdTransform.position.x <= left + birdCollisionRadius then
            -- left top sub-square
            v.x = left + birdCollisionRadius - birdTransform.position.x
            return v.x * v.x + v.y * v.y <= birdCollisionRadius * birdCollisionRadius
        elseif birdTransform.position.x >= right - birdCollisionRadius and birdTransform.position.x <= right then
            -- right top sub-square
            v.x = right - birdCollisionRadius - birdTransform.position.x
            return v.x * v.x + v.y * v.y <= birdCollisionRadius * birdCollisionRadius
        end
    elseif birdTransform.position.y <= bottom + birdCollisionRadius and birdTransform.position.y >= bottom then
        v.y = bottom + birdCollisionRadius - birdTransform.position.y
        if birdTransform.position.x >= left and birdTransform.position.x <= left + birdCollisionRadius then
            -- left bottom sub-square
            v.x = left + birdCollisionRadius - birdTransform.position.x
            return v.x * v.x + v.y * v.y <= birdCollisionRadius * birdCollisionRadius
        elseif birdTransform.position.x >= right - birdCollisionRadius and birdTransform.position.x <= right then
            -- right bottom sub-square
            v.x = right - birdCollisionRadius - birdTransform.position.x
            return v.x * v.x + v.y * v.y <= birdCollisionRadius * birdCollisionRadius
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
    birdTransform.scale.x = birdWidth / 2
    birdTransform.scale.y = birdHeight / 2
    birdTransform.position.x = -screenSize.x / 4
    birdTransform.position.y = backgroundLayers[1].height / 2
    birdTransform.rotation = 0
    verticalBirdVelocity = 0
end

function setupBird()
    local birdTexture = c2k.assets.texture(birdTextureGUID)
    local birdTextureAspect = birdTexture.width / birdTexture.height
    birdHeight = birdWidth / birdTextureAspect
    local birdEntity = Entity.new()
    birdEntity:attachRoot()
    birdTransform = birdEntity:attachTransform()
    setStartingBirdTransform()
    local birdSprite = birdEntity:attachDynamicSprite()
    birdSprite.texture = birdTexture
    gravity = -9.81 * birdTransform.scale.y * 7.52
    verticalBirdVelocityOnClick = 18 * birdTransform.scale.y
end

function setupBackground()
    for i = 1, #backgroundLayers do
        backgroundLayers[i].width = setupBackgroundLayer(
                backgroundLayers[i].transforms,
                backgroundLayers[i].depth,
                backgroundLayers[i].textureGUID,
                backgroundLayers[i].height,
                backgroundLayers[i].heightOffset
        )
    end
end

function onAttach(entity)
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
    for i = 1, #backgroundLayers do
        updateBackgroundLayer(
                backgroundLayers[i].transforms,
                backgroundLayers[i].width,
                backgroundLayers[i].movementSpeedFactor
        )
    end
end

function spawnPipes()
    local verticalPipeGapPosition = availablePipeHeight * math.random()
            + backgroundLayers[1].height + (verticalPipeGap - screenSize.y) / 2
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
        local passedBeforeMove = birdTransform.position.x > pipeTransform.position.x + pipeWidth / 2
        pipeTransform.position.x = pipeTransform.position.x - horizontalMovementSpeed * time.delta
        if not birdPassedPipePair then
            local passedAfterMove = birdTransform.position.x > pipeTransform.position.x + pipeWidth / 2
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
    verticalBirdVelocity = verticalBirdVelocityOnClick
end

function updateBird()
    if gameState == GameState.PLAYING and cheatMode then
        local leftIndex = 0
        local rightIndex = 0
        local leftX = 0
        local rightX = 0
        for i = 1, #pipeEntities, 2 do
            local transform = pipeEntities[i]:getTransform()
            if transform.position.x < birdTransform.position.x and (leftIndex == 0 or leftX < transform.position.x) then
                leftIndex = i
                leftX = transform.position.x
            elseif transform.position.x > birdTransform.position.x and (rightIndex == 0 or rightX > transform.position.x) then
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
            local interpolationParameter = (birdTransform.position.x - leftX) / (rightX - leftX)
            targetY = smoothStep(targetLeftY, targetRightY, interpolationParameter)
        end
        birdTransform.position.y = targetY
        birdTransform.rotation = 0
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
        birdTransform.position.y = math.sin(time.elapsed * 4) * 14
        if wasJumpPressed then
            gameState = GameState.PLAYING
            doBirdJump()
        end
        return
    end
    if wasJumpPressed and gameState ~= GameState.COLLIDED and gameState ~= GameState.BIRD_DOWN then
        doBirdJump()
    end
    verticalBirdVelocity = verticalBirdVelocity + gravity * time.delta
    birdTransform.position.y = birdTransform.position.y + verticalBirdVelocity * time.delta
    targetRotation = math.atan(verticalBirdVelocity / horizontalMovementSpeed)
    birdTransform.rotation = lerp(birdTransform.rotation, targetRotation, time.delta * 3.5)
    if birdTransform.position.y + birdHeight / 2 > screenSize.y / 2 then
        birdTransform.position.y = (screenSize.y - birdHeight) / 2
        verticalBirdVelocity = 0
    end
    if birdTransform.position.y <= -screenSize.y / 2 + backgroundLayers[1].height then
        birdTransform.position.y = -screenSize.y / 2 + backgroundLayers[1].height
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
    for i = 1, entitiesToDelete do
        table.remove(fireworksEntities, 1)
    end
end

function update(entity)
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