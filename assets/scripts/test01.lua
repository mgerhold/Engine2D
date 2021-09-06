counter = 1

print("test01.lua loaded!")

function update(entity)
    transform = getTransform(entity)
    transform.rotation = transform.rotation + 0.02
end