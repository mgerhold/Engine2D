-- the next two lines are called once for this file (not for every entity!)
math.randomseed(42)
rotationSpeeds = {}

-- awake is called when this script is attached onto an entity
function awake(entity)
    rotationSpeeds[entity] = math.random(10, 60) -- create random rotation speeds for the current entity
end

-- update gets called every frame for every entity with this script
function update(entity)
    transform = getTransform(entity)
    transform.rotation = transform.rotation + math.rad(rotationSpeeds[entity]) * getTime().delta
end