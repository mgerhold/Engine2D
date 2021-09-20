function onAttach(thisEntity)
    input = c2k.getInput()
end

function update(thisEntity)
    if input:keyPressed(Key.Num1) then
        local entity = Entity.new() -- create empty entity
        local script = c2k.assets.script("43e72b36-75d0-4ba7-93c7-d443d649bacc") -- retrieve handle to script asset
        entity:attachScript(script) -- attach script to newly created entity
    end
end