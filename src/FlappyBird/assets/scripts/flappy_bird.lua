groundTextureGUID = "6e105d9e-5b8b-44ae-8b21-03f8fee5357c"
topPipeTextureGUID = "aedbb58b-f2d8-44d4-b046-4e27ed36141c"
bottomPipeTextureGUID = "1129191c-9108-4293-823a-e3e8f13f32c4"
birdTextureGUID = "15a01f9b-c970-43e1-bbe8-79a5b04a58e6"

screenSize = Vec2.new()
screenSize.x = 800
screenSize.y = 600

groundHeight = 100

pipeWidth = 100
verticalPipeGap = 150
horizontalPipeGap = 400
availablePipeHeight = screenSize.y - groundHeight - verticalPipeGap

birdWidth = 80
verticalBirdVelocity = 0
birdRotation = 0

horizontalMovementSpeed = 200

function setupGroundTiles()
    local groundTexture = c2k.assets.texture(groundTextureGUID)
    local groundTextureAspect = groundTexture.width / groundTexture.height
    groundWidth = groundTextureAspect * groundHeight
    numberOfGroundTiles = math.floor(screenSize.x / groundWidth) + 2
    groundTransforms = {}
    for i = 1, numberOfGroundTiles do
        local groundEntity = Entity.new()
        groundEntity:attachRoot()
        groundTransforms[i] = groundEntity:attachTransform()
        groundTransforms[i].scale.x = groundWidth / 2
        groundTransforms[i].scale.y = groundHeight / 2
        groundTransforms[i].position.x = (-screenSize.x + groundWidth) / 2 + (i - 1) * groundWidth
        groundTransforms[i].position.y = (-screenSize.y + groundHeight) / 2
        local groundSprite = groundEntity:attachDynamicSprite()
        groundSprite.texture = groundTexture
    end
end

function setupPipeData()
    topPipeTexture = c2k.assets.texture(topPipeTextureGUID)
    bottomPipeTexture = c2k.assets.texture(bottomPipeTextureGUID)
    pipeTextureAspect = topPipeTexture.width / topPipeTexture.height
    pipeHeight = pipeWidth / pipeTextureAspect
    nextPipeSpawnTime = time.elapsed + 1
    pipeEntities = {}
end

function setupBird()
    local birdTexture = c2k.assets.texture(birdTextureGUID)
    local birdTextureAspect = birdTexture.width / birdTexture.height
    birdHeight = birdWidth / birdTextureAspect
    local birdEntity = Entity.new()
    birdEntity:attachRoot()
    birdTransform = birdEntity:attachTransform()
    birdTransform.scale.x = birdWidth / 2
    birdTransform.scale.y = birdHeight / 2
    birdTransform.position.x = -screenSize.x / 4
    local birdSprite = birdEntity:attachDynamicSprite()
    birdSprite.texture = birdTexture
    gravity = -9.81 * birdTransform.scale.y * 7.52
    verticalBirdVelocityOnClick = 18 * birdTransform.scale.y
end

function onAttach(entity)
    input = c2k.getInput()
    time = c2k.getTime()

    setupPipeData()
    setupGroundTiles()
    setupBird()
end

function handlePlayerInput()
    if input:keyPressed(Key.Escape) then
        c2k.application.quit()
    end
    applyForce = input:mousePressed(MouseButton.Left) or input:keyPressed(Key.Space)
end

function updateGroundTiles()
    local leftMostGroundIndex = 0
    local minGroundPositionX = 0
    for i = 1, numberOfGroundTiles do
        groundTransforms[i].position.x = groundTransforms[i].position.x - horizontalMovementSpeed * time.delta
        if leftMostGroundIndex == 0 or groundTransforms[i].position.x < minGroundPositionX then
            leftMostGroundIndex = i
            minGroundPositionX = groundTransforms[i].position.x
        end
    end
    if groundTransforms[leftMostGroundIndex].position.x + groundWidth / 2 < -screenSize.x / 2 then
        local rightMostGroundIndex = leftMostGroundIndex - 1
        if rightMostGroundIndex == 0 then
            rightMostGroundIndex = numberOfGroundTiles
        end
        groundTransforms[leftMostGroundIndex].position.x = groundTransforms[rightMostGroundIndex].position.x + groundWidth
    end
end

function spawnPipes()
    local verticalPipeGapPosition = availablePipeHeight * math.random() + groundHeight + (verticalPipeGap - screenSize.y) / 2
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
    for i = 1, #pipeEntities do
        local pipeTransform = pipeEntities[i]:getTransform()
        pipeTransform.position.x = pipeTransform.position.x - horizontalMovementSpeed * time.delta
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
    return (1 - t) * from + t * to
end

function updateBird()
    if applyForce then
        verticalBirdVelocity = verticalBirdVelocityOnClick
    end
    verticalBirdVelocity = verticalBirdVelocity + gravity * time.delta
    birdTransform.position.y = birdTransform.position.y + verticalBirdVelocity * time.delta
    targetRotation = math.atan(verticalBirdVelocity / horizontalMovementSpeed)
    birdTransform.rotation = lerp(birdTransform.rotation, targetRotation, time.delta * 3.5)
end

function update(entity)
    handlePlayerInput()

    updateGroundTiles()
    if time.elapsed >= nextPipeSpawnTime then
        spawnPipes()
    end
    updatePipes()
    updateBird()
end