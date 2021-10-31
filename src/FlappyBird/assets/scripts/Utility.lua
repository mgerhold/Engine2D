function lerp(from, to, t)
    if t < 0 then
        return from
    elseif t > 1 then
        return to
    else
        return (1 - t) * from + t * to
    end
end

function smoothStep(from, to, t)
    if t < 0 then
        return from
    elseif t > 1 then
        return to
    else
        return (3 * t * t - 2 * t * t * t) * (to - from) + from
    end
end