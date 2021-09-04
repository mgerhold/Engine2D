print("test02.lua loaded!")

function update(entity)
    print("updating entity " .. entity .. " in another script")
    subroutine()
end

function subroutine()
    print("this is another yellow subroutine")
end