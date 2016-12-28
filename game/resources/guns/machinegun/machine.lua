
function MachineGun.init(gun)
	gun.spreadAngle = 0.10
	gun.crosshairDistance = 550.0
	ShootTimer.initGun(gun, 0.15)
end


function MachineGun.onTick(gun)
	if gun.isTriggered then
		if ShootTimer.checkGun(gun) then
			local bullet = gun:addBullet()
			bullet.onDamageArgs.customSlowdownAmount = 4.0
			bullet.onDamageArgs.slowdownOnly = true
			
			local particle = bullet:addTrailParticle("BulletTrailParticle", Vec2.new(0.0, 14.0), 15.0, {})
			particle.args.initialScale = 2.0
			particle.args.fadeOutSpeed = 1.2
		end
	end
end
