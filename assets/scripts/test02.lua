function onAttach(entity)
    printWarning(entity)
end

function update(entity)
    printWarning(entity)
end

function printWarning(entity)
    print("Warning: Default script on entity " .. entity.id)
end