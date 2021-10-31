PipeContainer = {}
PipeContainer.__index = PipeContainer

function PipeContainer.new(topTextureGUID, bottomTextureGUID, pipeWidth, horizontalGap, verticalGap, groundHeight, screenSize)
    local topTexture = c2k.assets.texture(topTextureGUID)
    local textureAspect = topTexture.width / topTexture.height
    local result = {
        topTexture = topTexture,
        bottomTexture = c2k.assets.texture(bottomTextureGUID),
        textureAspect = textureAspect,
        pipeWidth = pipeWidth,
        pipeHeight = pipeWidth / textureAspect,
        nextSpawnTime = time.elapsed,
        availableHeight = screenSize.y - verticalGap - groundHeight,
        horizontalGap = horizontalGap,
        verticalGap = verticalGap,
        groundHeight = groundHeight,
        entities = {},
        screenSize = screenSize
    }
    setmetatable(result, PipeContainer)
    return result
end

function PipeContainer:spawn(horizontalMovementSpeed)
    local verticalGapPosition = self.availableHeight * math.random() + (self.verticalGap - self.screenSize.y) / 2 + self.groundHeight
    local index = #self.entities + 1
    for pipeNumber = 1, 2 do
        self.entities[index] = Entity.new()
        self.entities[index]:attachRoot()
        local pipeTransform = self.entities[index]:attachTransform()
        pipeTransform.position.x = (self.screenSize.x + self.pipeWidth) / 2
        pipeTransform.position.z = -0.1
        pipeTransform.scale.x = self.pipeWidth / 2
        pipeTransform.scale.y = self.pipeHeight / 2
        local pipeSprite = self.entities[index]:attachDynamicSprite()
        if pipeNumber == 1 then
            pipeSprite.texture = self.bottomTexture
            pipeTransform.position.y = verticalGapPosition - (self.pipeHeight + self.verticalGap) / 2
        else
            pipeSprite.texture = self.topTexture
            pipeTransform.position.y = verticalGapPosition + (self.pipeHeight + self.verticalGap) / 2
        end
        index = index + 1
    end
    self.nextSpawnTime = time.elapsed + (self.horizontalGap + self.pipeWidth) / horizontalMovementSpeed
    self.horizontalGap = self.horizontalGap - 5
    if self.horizontalGap < 2 * self.pipeWidth then
        self.horizontalGap = 2 * self.pipeWidth
    end
end

function PipeContainer:update(horizontalMovementSpeed, bird)
    local birdPassedPipePair = false
    local passedPipe = 0
    for i, entity in ipairs(self.entities) do
        local pipeTransform = entity:getTransform()
        local passedBeforeMove = bird.transform.position.x > pipeTransform.position.x + self.pipeWidth / 2
        pipeTransform.position.x = pipeTransform.position.x - horizontalMovementSpeed * time.delta
        if not birdPassedPipePair then
            local passedAfterMove = bird.transform.position.x > pipeTransform.position.x + self.pipeWidth / 2
            if not passedBeforeMove and passedAfterMove then
                birdPassedPipePair = true
                passedPipe = i
            end
        end
    end

    if #self.entities > 1 then
        local leftMostPipeTransform = self.entities[1]:getTransform()
        if leftMostPipeTransform.position.x + self.pipeWidth / 2 < -self.screenSize.x then
            self.entities[1]:destroy()
            self.entities[2]:destroy()
            for i = 1, #self.entities - 2 do
                self.entities[i] = self.entities[i + 2]
            end
            self.entities[#self.entities] = nil
            self.entities[#self.entities] = nil
        end
    end

    if birdPassedPipePair then
        return passedPipe
    else
        return nil
    end
end

function PipeContainer:clear()
    for _, entity in ipairs(self.entities) do
        entity:destroy()
    end
    self.entities = {}
end

function PipeContainer:collisionOfAnyPipeWithCircle(circleCenter, circleRadius)
    for _, pipeEntity in ipairs(self.entities) do
        if self:pipeCollisionWithCircle(pipeEntity, circleCenter, circleRadius) then
            return true
        end
    end
    return false
end

function PipeContainer:pipeCollisionWithCircle(pipeEntity, circleCenter, circleRadius)
    local pipePosition = pipeEntity:getTransform().position
    local left = pipePosition.x - self.pipeWidth / 2 - circleRadius
    local right = pipePosition.x + self.pipeWidth / 2 + circleRadius
    local top = pipePosition.y + self.pipeHeight / 2 + circleRadius
    local bottom = pipePosition.y - self.pipeHeight / 2 - circleRadius
    if circleCenter.x < left or circleCenter.x > right or
            circleCenter.y > top or circleCenter.y < bottom then
        return false
    end
    local v = Vec2.new()
    if circleCenter.y >= top - circleRadius and circleCenter.y <= top then
        v.y = top - circleRadius - circleCenter.y
        if circleCenter.x >= left and circleCenter.x <= left + circleRadius then
            -- left top sub-square
            v.x = left + circleRadius - circleCenter.x
            return v.x * v.x + v.y * v.y <= circleRadius * circleRadius
        elseif circleCenter.x >= right - circleRadius and circleCenter.x <= right then
            -- right top sub-square
            v.x = right - circleRadius - circleCenter.x
            return v.x * v.x + v.y * v.y <= circleRadius * circleRadius
        end
    elseif circleCenter.y <= bottom + circleRadius and circleCenter.y >= bottom then
        v.y = bottom + circleRadius - circleCenter.y
        if circleCenter.x >= left and circleCenter.x <= left + circleRadius then
            -- left bottom sub-square
            v.x = left + circleRadius - circleCenter.x
            return v.x * v.x + v.y * v.y <= circleRadius * circleRadius
        elseif circleCenter.x >= right - circleRadius and circleCenter.x <= right then
            -- right bottom sub-square
            v.x = right - circleRadius - circleCenter.x
            return v.x * v.x + v.y * v.y <= circleRadius * circleRadius
        end
    end
    return true
end