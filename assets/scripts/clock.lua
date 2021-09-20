function onAttach(thisEntity)
    time = c2k.getTime()
    nextPrint = time.elapsed + 1.0
    counter = 0
end

function update(thisEntity)
    local elapsed = time.elapsed
    if elapsed >= nextPrint then
        print(counter .. " seconds elapsed")
        nextPrint = elapsed + 1.0
    end
end