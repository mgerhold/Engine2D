spriteSheetGUID = "22171ab7-6b8b-474f-bf24-366724625641"
shaderGUID = "b520f0eb-1756-41e0-ac07-66c3338bc594"

-- onAttach is called once for every entity with this script
function onAttach(entity)
    --[[ this function attaches all required components
         to this entity to make it display the first sprite
         of a sprite sheet ]]--

    -- retrieve handle to sprite sheet
    spriteSheet = c2k.assets.spriteSheet(spriteSheetGUID)

    -- attach transform component to this entity
    local transform = entity:attachTransform()
    transform.scale.x = 120
    transform.scale.y = 120

    -- retrieve handle to shader program
    local shader = c2k.assets.shaderProgram(shaderGUID)

    -- attach dynamic sprite component to this entity
    sprite = entity:attachDynamicSprite() -- sprite handle is cached for later since "sprite" is global
    sprite.shaderProgram = shader
    sprite.texture = spriteSheet.texture
    currentIndex = 1
    sprite.textureRect = spriteSheet.frames[currentIndex].rect

    -- attach root component (this entity has no parent)
    entity:attachRoot()

    -- cache handle to input system
    input = c2k.getInput()
end

-- update is called every frame for this entity
function update(entity)
    if input:keyPressed(Key.Enter) then
        -- display the next sprite of the sprite sheet
        currentIndex = currentIndex + 1
        if currentIndex == #spriteSheet.frames + 1 then
            currentIndex = 1
        end
        sprite.textureRect = spriteSheet.frames[currentIndex].rect
    end
end