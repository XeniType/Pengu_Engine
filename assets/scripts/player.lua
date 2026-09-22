local Script = {}

function Script:onStart(entity)
    print("Player script started")
end

function Script:onUpdate(entity, dt)
    if IsKeyDown(Action.Arrow_Up) then
        local pos = entity:getPosition()
        pos.z = pos.z - 0.005 * dt
        entity:setPosition(pos)
    end
    if IsKeyDown(Action.Arrow_Down) then
        local pos = entity:getPosition()
        pos.z = pos.z + 0.005 * dt
        entity:setPosition(pos)
    end
    if IsKeyDown(Action.Arrow_Left) then
        local pos = entity:getPosition()
        pos.x = pos.x - 0.005 * dt
        entity:setPosition(pos)
    end
    if IsKeyDown(Action.Arrow_Right) then
        local pos = entity:getPosition()
        pos.x = pos.x + 0.005 * dt
        entity:setPosition(pos)
    end
    if IsKeyDown(Action.N) then
        local pos = entity:getPosition()
        pos.y = pos.y + 0.005 * dt
        entity:setPosition(pos)
    end
    if IsKeyDown(Action.M) then
        local pos = entity:getPosition()
        pos.y = pos.y - 0.005 * dt
        entity:setPosition(pos)
    end
end

return Script