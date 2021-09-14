spriteSheetGUID = "22171ab7-6b8b-474f-bf24-366724625641"
shaderGUID = "b520f0eb-1756-41e0-ac07-66c3338bc594"

function onAttach(entity)
    spriteSheet = c2k.assets.spriteSheet(spriteSheetGUID)
    print("There are " .. #spriteSheet.frames .. " sprites in this sprite sheet")
    print("It uses the texture with guid " .. spriteSheet.texture.guid)

    local transform = entity:attachTransform()
    transform.scale.x = 60
    transform.scale.y = 60
    local sprite = entity:attachDynamicSprite()
    local shader = c2k.assets.shaderProgram(shaderGUID)
    sprite.shaderProgram = shader
    sprite.texture = spriteSheet.texture
    currentIndex = 1
    sprite.textureRect = spriteSheet.frames[currentIndex].rect
    entity:attachRoot()
end

function update(entity)
    local input = c2k.getInput()
    if input:keyPressed(Key.Enter) then
        currentIndex = currentIndex + 1
        if currentIndex == #spriteSheet.frames + 1 then
            currentIndex = 1
        end
        sprite = entity:getDynamicSprite()
        sprite.textureRect = spriteSheet.frames[currentIndex].rect
    end
end