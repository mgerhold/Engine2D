NumberDisplay = {}
NumberDisplay.__index = NumberDisplay

function NumberDisplay.new(spriteSheetGUID, textureGUID, centerPosition, digitSize, initialValue)
    if initialValue == nil then
        initialValue = 0
    end
    local result = {
        spriteSheet = c2k.assets.spriteSheet(spriteSheetGUID),
        texture = c2k.assets.texture(textureGUID),
        entities = {},
        centerPosition = centerPosition,
        digitSize = digitSize
    }
    setmetatable(result, NumberDisplay)
    result:show(initialValue)
    return result
end

function NumberDisplay:show(number)
    for _, entity in ipairs(self.entities) do
        entity:destroy()
    end
    self.entities = {}
    local numberString = tostring(number)
    for i = 1, #numberString do
        local digit = string.byte(numberString:sub(i, i)) - string.byte("0")
        self.entities[i] = Entity.new()
        self.entities[i]:attachRoot()
        local transform = self.entities[i]:attachTransform()
        transform.scale = Vec2.new(self.digitSize / 2, self.digitSize / 2)
        transform.position = Vec3.new(self.centerPosition.x + self.digitSize * (-#numberString / 2 + i - 0.5),
                self.centerPosition.y, self.centerPosition.z)
        local sprite = self.entities[i]:attachDynamicSprite()
        sprite.texture = self.texture
        sprite.textureRect = self.spriteSheet.frames[digit + 1].rect
    end
end