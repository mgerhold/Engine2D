counter = 1

function update(entity)
    counter = counter + 1
    print("updating entity " .. entity .. ", counter == " .. counter)
    subroutine()
end

function subroutine()
    print("this is a yellow subroutine")
end