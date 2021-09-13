-- the next lines are called once for this file (not for every entity!)
math.randomseed(42)
rotationSpeeds = {}
spawnedEntities = {}

textureGUID = "9043b452-363c-4917-bfde-592a72077e37"
texture = c2k.assets.texture(textureGUID)
widthToHeightRatio = texture.width / texture.height

shaderGUID = "b520f0eb-1756-41e0-ac07-66c3338bc594"
shader = c2k.assets.shaderProgram(shaderGUID)

numSpawnedEntities = 0
currentlySpawning = true
maxNumSpawnedEntities = 0

-- awake is called when this script is attached onto an entity
function onAttach(entity)
    rotationSpeeds[entity.id] = math.random(10, 60) -- create random rotation speeds for the current entity
    maxNumSpawnedEntities = maxNumSpawnedEntities + 1
end

-- update gets called every frame for every entity with this script
function update(entity)
    input = c2k.getInput()

    if input:keyPressed(Key.G) then
        if numSpawnedEntities < maxNumSpawnedEntities and currentlySpawning then
            spawnedEntities[entity.id] = entity.new()

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

    time = c2k.getTime()
    transform = entity:getTransform()
    transform.rotation = transform.rotation + math.rad(rotationSpeeds[entity.id]) * time.delta
    speed = 100.0
    if input:keyDown(Key.Left) then
        transform.position.x = transform.position.x - speed * time.delta
    end
    if input:keyDown(Key.Right) then
        transform.position.x = transform.position.x + speed * time.delta
    end
    if input:keyDown(Key.Up) then
        transform.position.y = transform.position.y + speed * time.delta
    end
    if input:keyDown(Key.Down) then
        transform.position.y = transform.position.y - speed * time.delta
    end
end