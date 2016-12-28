
function LaserPistol.init(gun)
	gun.spreadAngle = 0.0
	gun.crosshairDistance = 350.0
	ShootTimer.initGun(gun, 0.40)
	SpreadHelper.initGun(gun)
end


function LaserPistol.onTick(gun)
	SpreadHelper.onTick(gun)
	if gun.isTriggered then
		if ShootTimer.checkGun(gun) then
			SpreadHelper.onShoot(gun)
			gun:addBullet()
		end
	end
end
