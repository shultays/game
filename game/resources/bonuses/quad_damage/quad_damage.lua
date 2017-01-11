QuadDamage.buffId = getGlobalUniqueId()

function QuadDamage.spawn(pos)
	local data = QuadDamage.data

	player.damageMultiplier:addBuffWithId(QuadDamage.buffId, 4.0)
	if data.boost == nil then
		data.boost = addGameObject("QuadDamage")
		QuadDamage.shaderStartTime = time
		data.postProcess = addPostProcess("resources/post_process/red_shift.ps")
		data.postProcess:setShaderWeight(0.0)
	else
		data.boost.data.time = time
	end
	playSound({path = "resources/sounds/metal_riff.ogg", volume = 0.5})
end

function QuadDamage.init(gameObject)
	gameObject.data.time = time
end


function QuadDamage.onTick(gameObject)
	local t = time - QuadDamage.shaderStartTime
	
	local duration = 8.0 * player.data.bonusDurationMultiplier
	
	local a = 1.0
	if t < 0.3 then
		a = t / 0.3
	end
	t = time - gameObject.data.time
	if t > duration - 1.0 then
		a = (duration- t)
	end
	QuadDamage.data.postProcess:setShaderWeight(a)

	if t > duration then
		QuadDamage.data.boost = nil
		removePostProcess(QuadDamage.data.postProcess)
		QuadDamage.data.postProcess = nil
		player.damageMultiplier:removeBuff(QuadDamage.buffId)
		gameObject.toBeRemoved = true
	end
end
