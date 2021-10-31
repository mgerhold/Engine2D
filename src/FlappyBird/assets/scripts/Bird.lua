Bird = {}
Bird.__index = Bird

function Bird.new(width, collisionRadius, spriteSheetGUID, screenSize, groundHeight)
    local result = {
        width = width,
        collisionRadius = collisionRadius,
        verticalVelocity = 0,
        screenSize = screenSize,
        groundHeight = groundHeight,
        spriteSheet = c2k.assets.spriteSheet(spriteSheetGUID),
        wingAnimationUpdateInterval = 0.12,
        currentAnimationFrame = 1
    }
    setmetatable(result, Bird)
    local birdTexture = result.spriteSheet.texture
    local birdTextureAspect = result.spriteSheet.frames[1].sourceWidth / result.spriteSheet.frames[1].sourceHeight
    result.height = result.width / birdTextureAspect
    result.entity = Entity.new()
    result.entity:attachRoot()
    result.transform = result.entity:attachTransform()
    Bird.resetTransform(result, screenSize, groundHeight)
    local birdSprite = result.entity:attachDynamicSprite()
    birdSprite.texture = birdTexture
    result.gravity = -9.81 * result.transform.scale.y * 7.52
    result.verticalVelocityOnClick = 18 * result.transform.scale.y
    result:showAnimationFrame(1)
    result.nextWingUpdateTime = time.elapsed + result.wingAnimationUpdateInterval
    return result
end

function Bird:resetTransform()
    self.transform.scale.x = self.width / 2
    self.transform.scale.y = self.height / 2
    self.transform.position.x = -self.screenSize.x / 4
    self.transform.position.y = self.groundHeight / 2
    self.transform.rotation = 0
    self.verticalVelocity = 0
end

function Bird:jump()
    self.verticalVelocity = self.verticalVelocityOnClick
end

function Bird:roam()
    self.transform.position.y = math.sin(time.elapsed * 4) * 14
    self:updateAnimation()
end

function Bird:update(wasJumpPressed, horizontalMovementSpeed)
    if wasJumpPressed then
        self:jump()
    end
    self.verticalVelocity = self.verticalVelocity + self.gravity * time.delta
    self.transform.position.y = self.transform.position.y + self.verticalVelocity * time.delta
    targetRotation = math.atan(self.verticalVelocity / horizontalMovementSpeed)
    self.transform.rotation = lerp(self.transform.rotation, targetRotation, time.delta * 3.5)
    if self.transform.position.y + self.height / 2 > self.screenSize.y / 2 then
        self.transform.position.y = (self.screenSize.y - self.height) / 2
        self.verticalVelocity = 0
    end
    self:updateAnimation()
end

function Bird:hasHitTheGround()
    return self.transform.position.y <= -self.screenSize.y / 2 + self.groundHeight
end

function Bird:updateAnimation()
    if time.elapsed > self.nextWingUpdateTime then
        self.nextWingUpdateTime = time.elapsed + self.wingAnimationUpdateInterval
        local nextAnimationFrame = self.currentAnimationFrame + 1
        if nextAnimationFrame == 4 then
            nextAnimationFrame = 1
        end
        self:showAnimationFrame(nextAnimationFrame)
    end
end

function Bird:showAnimationFrame(number)
    local sprite = self.entity:getDynamicSprite()
    sprite.textureRect = self.spriteSheet.frames[number].rect
    self.currentAnimationFrame = number
end 