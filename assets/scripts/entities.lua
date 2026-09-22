local Script = {}

Script.rotspeed = math.random() * 0.005

function Script:onStart(entity)
    Log.info("Entity Started")
end

function Script:onUpdate(entity, dt)
    local pos = entity:getPosition()
    local rot = entity:getRotation()

    rot.z = rot.z - Script.rotspeed * dt
    
    local origin = vec3(0, 0, 0)
    local dist = glm.distance(pos, origin)

    print(origin + origin)
    print(origin + dist)
    print(dist + origin)

    print(origin - origin)
    print(origin - dist)
    print(dist - origin)

    print(origin * origin)
    print(origin * dist)
    print(dist * origin)

    Log.info(tostring(dist))

    entity:setRotation(rot)
end

return Script