require("Game")

function onAttach(_)
    input = c2k.getInput()
    time = c2k.getTime()
    game = Game.new(Vec2.new(800, 600))
end

function update(_)
    game:update()
end