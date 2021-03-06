
function Egg.init(monster, min)
    data = monster.data
    monster:setScale(math.random() * 0.2 + 0.8)

    monster.moveSpeed = 0
    monster.moveAngle =  math.random() * math.pi * 2.0
    
    monster:playAnimation("stand", math.random())
    
    monster:modifyDrawLevel(2)
    monster.colorMultiplier:addBuff(Vec4.new(0.6, 0.6, 0.6, 1.0))
    
    data.spawnInterval = 3.0 + math.random() * 0.5
    data.spawnTime = data.spawnInterval * (3.0 + math.random())
    
    data.cannotBecomeBoss = true
    data.cannotShootBullets = true
    
    data.monsterType = "Alien"
    if math.random() < 0.07 then
        data.monsterType = "Spider"
    end
    monster.knockbackResistance:addBuff(0.1)
    
    if missionData.eggCount == nil then
        missionData.eggCount = 0
    end
    missionData.eggCount = missionData.eggCount + 1
    calcRandomSpawns()
end

function Egg.spawnChanceInMission(missionData, min)
    if missionData.isSurvival ~= true  then
        return 0.0
    end
    if missionData.eggCount ~= nil and missionData.eggCount > 1 + math.min(min, 4.0) then
        return 0.0
    end
    return 0.03 + clamp(min * 0.3) * 0.04
end

function Egg.buffStats(monster, min)
    monster.data.spawnInterval = monster.data.spawnInterval - clamp(min/10)
end

function Egg.onKilled(monster)
    missionData.eggCount = missionData.eggCount - 1
    
end

function Egg.onTick(monster)
    data = monster.data
    
    data.spawnTime = data.spawnTime - dt
    
    if data.spawnTime < 0.0 then
        data.spawnTime = data.spawnTime + data.spawnInterval
        
        if canSpawnIgnoredMonster() then
            local m = addRandomMonster(data.monsterType, true, true, 1.0)
            m:setScale(m.scale * 0.8)
            m.position = monster.position
            ignoreMonsterForCount(m)
        end
    end
end


function Egg.getRandomSpawnPos()
    return getRandomPosition( {canBeEdge=false, notNearPlayer=true, notNearMonsters=true, playerRange=800.0})
end