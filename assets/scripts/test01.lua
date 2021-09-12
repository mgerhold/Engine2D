-- the next two lines are called once for this file (not for every entity!)
math.randomseed(42)
rotationSpeeds = {}

bjarneTexture = "9043b452-363c-4917-bfde-592a72077e37"
defaultShader = "b520f0eb-1756-41e0-ac07-66c3338bc594"

-- awake is called when this script is attached onto an entity
function onAttach(entity)
    rotationSpeeds[entity.id] = math.random(10, 60) -- create random rotation speeds for the current entity
end

-- update gets called every frame for every entity with this script
function update(entity)
    input = c2k.getInput()

    if input:keyPressed(Key.G) then
        newEntity = entity.new()

        newTransform = newEntity:attachTransform()
        newTransform.position.x = math.random(-200, 200)
        newTransform.position.y = math.random(-200, 200)
        newTransform.rotation = math.rad(math.random(0, 360))
        newTransform.scale.x = 15
        newTransform.scale.y = 15

        newSprite = newEntity:attachDynamicSprite()
        texture = c2k.assets.texture(bjarneTexture)
        newSprite.texture = texture
        shader = c2k.assets.shaderProgram(defaultShader)
        newSprite.shaderProgram = shader

        newEntity:attachRoot()
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