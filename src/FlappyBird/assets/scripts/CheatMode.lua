function handleCheatMode(game)
    local leftIndex = 0
    local rightIndex = 0
    local leftX = 0
    local rightX = 0
    for i = 1, #game.pipeContainer.entities, 2 do
        local transform = game.pipeContainer.entities[i]:getTransform()
        if transform.position.x < game.bird.transform.position.x and (leftIndex == 0 or leftX < transform.position.x) then
            leftIndex = i
            leftX = transform.position.x
        elseif transform.position.x > game.bird.transform.position.x and (rightIndex == 0 or rightX > transform.position.x) then
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
        local targetTransform1 = game.pipeContainer.entities[targetIndex]:getTransform()
        local targetTransform2 = game.pipeContainer.entities[targetIndex + 1]:getTransform()
        targetY = (targetTransform1.position.y + targetTransform2.position.y) / 2
    else
        local targetTransformLeft1 = game.pipeContainer.entities[leftIndex]:getTransform()
        local targetTransformLeft2 = game.pipeContainer.entities[leftIndex + 1]:getTransform()
        local targetTransformRight1 = game.pipeContainer.entities[rightIndex]:getTransform()
        local targetTransformRight2 = game.pipeContainer.entities[rightIndex + 1]:getTransform()
        local targetLeftY = (targetTransformLeft1.position.y + targetTransformLeft2.position.y) / 2
        local targetRightY = (targetTransformRight1.position.y + targetTransformRight2.position.y) / 2
        local interpolationParameter = (game.bird.transform.position.x - leftX) / (rightX - leftX)
        targetY = smoothStep(targetLeftY, targetRightY, interpolationParameter)
    end
    game.bird.transform.position.y = targetY
    game.bird.transform.rotation = 0
end