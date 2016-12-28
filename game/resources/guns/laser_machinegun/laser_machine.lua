
function LaserMachineGun.init(gun)
	gun.crosshairDistance = 350.0
	
	ShootTimer.initGun(gun, 0.17)
	SpreadHelper.initGun(gun)
	gun.data.minSpread = 0.05
	gun.data.maxSpread = 0.15
	gun.data.spreadIncreasePerShoot = 0.01
end


function LaserMachineGun.onTick(gun)
	SpreadHelper.onTick(gun)
	if gun.isTriggered then
		if ShootTimer.checkGun(gun) then
			SpreadHelper.onShoot(gun)
			local bullet = gun:addBullet()
			
			bullet.onDamageArgs.slowdownOnly = true
		end
	end
end
