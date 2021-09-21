-- the next lines are called once for this file (not for every entity!)
math.randomseed(42)
rotationSpeeds = {}
spawnedEntities = {}
transforms = {}

textureGUID = "9043b452-363c-4917-bfde-592a72077e37"
texture = c2k.assets.texture(textureGUID)
widthToHeightRatio = texture.width / texture.height

shaderGUID = "b520f0eb-1756-41e0-ac07-66c3338bc594"
shader = c2k.assets.shaderProgram(shaderGUID)

numSpawnedEntities = 0
currentlySpawning = true
maxNumSpawnedEntities = 0
speed = 100.0 -- rotation speed

-- awake is called when this script is attached onto an entity
function onAttach(entity)
    rotationSpeeds[entity.id] = math.random(10, 60) -- create random rotation speeds for the current entity
    maxNumSpawnedEntities = maxNumSpawnedEntities + 1
    transforms[entity.id] = entity:getTransform() -- cache handle to transform component
    time = c2k.getTime() -- cache time handle
    input = c2k.getInput() -- cache input handle
end

-- update gets called every frame for every entity with this script
function update(entity)
    if input:keyPressed(Key.G) then
        if numSpawnedEntities < maxNumSpawnedEntities and currentlySpawning then
            spawnedEntities[entity.id] = Entity.new()

            newTransform = spawnedEntities[entity.id]:attachTransform()
            newTransform.position.x = math.random(-200, 200)
            newTransform.position.y = math.random(-200, 200)
            newTransform.rotation = math.rad(math.random(0, 360))
            newTransform.scale.x = 15 * widthToHeightRatio
            newTransform.scale.y = 15

            newSprite = spawnedEntities[entity.id]:attachDynamicSprite()
            newSprite.texture = texture
            newSprite.shaderProgram = shader

            spawnedEntities[entity.id]:attachRoot()

            numSpawnedEntities = numSpawnedEntities + 1
            if numSpawnedEntities == maxNumSpawnedEntities then
                currentlySpawning = false
            end
        elseif not currentlySpawning then
            spawnedEntities[entity.id]:destroy()
            spawnedEntities[entity.id] = nil

            numSpawnedEntities = numSpawnedEntities - 1
            if numSpawnedEntities == 0 then
                currentlySpawning = true;
                spawnedEntities = {}
            end
        end
    end

    transforms[entity.id].rotation = transforms[entity.id].rotation + math.rad(rotationSpeeds[entity.id]) * time.delta
    if input:keyDown(Key.Left) then
        transforms[entity.id].position.x = transforms[entity.id].position.x - speed * time.delta
    end
    if input:keyDown(Key.Right) then
        transforms[entity.id].position.x = transforms[entity.id].position.x + speed * time.delta
    end
    if input:keyDown(Key.Up) then
        transforms[entity.id].position.y = transforms[entity.id].position.y + speed * time.delta
    end
    if input:keyDown(Key.Down) then
        transforms[entity.id].position.y = transforms[entity.id].position.y - speed * time.delta
    end
end