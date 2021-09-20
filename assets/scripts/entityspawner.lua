invalidEntity = Entity.invalid()
entity = invalidEntity

function onAttach(thisEntity)
    input = c2k.getInput()
end

function update(thisEntity)
    if entity == invalidEntity and input:keyPressed(Key.Num1) then
        entity = Entity.new() -- create empty entity
        local script = c2k.assets.script("43e72b36-75d0-4ba7-93c7-d443d649bacc") -- retrieve handle to script asset
        entity:attachScript(script) -- attach script to newly created entity
    end
    if entity ~= invalidEntity and input:keyPressed(Key.Num2) then
        entity:destroy()
        print("after destroying entity: " .. entity.id)
    end
end