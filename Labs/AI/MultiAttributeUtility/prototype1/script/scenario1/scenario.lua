
Scenario = class(ScenarioBase, function(a)
	ScenarioBase.init(a) 
end)

function Scenario:load(engine)
	
	local entities = engine.UI.enabled
	
	local entity = Entity('avatar', -8.0, -3.0, 1.0)
	local avatar = entity
	entity.color = love.graphics.newColor(0, 0, 255)
	entity.weapon = Shotgun()
	entity.health = 0.5
	entities[table.getn(entities)+1] = entity
	
	entity = Entity('mate', -12.0, -10.0, 1.0)
	local mate = entity
	entity.color = love.graphics.newColor(0, 100, 255)
	entity.weapon = Shotgun()
	entity.weapon.rounds = entity.weapon.maxRounds / 4
	entity.health = 0.7
	entities[table.getn(entities)+1] = entity
	
	entity = Entity('medkit', -1.0, 1.0, 0.5)
	local medkit = entity
	entity.color = love.graphics.newColor(0, 255, 0)
	entities[table.getn(entities)+1] = entity
	
	entity = Entity('ammo', -17.0, 0.0, 0.5)
	local ammo = entity
	entity.color = love.graphics.newColor(0, 255, 100)
	entities[table.getn(entities)+1] = entity
	
	entity = Entity('zombie1', 10.0, 7.0, 1.0)
	local zombie1 = entity
	entity.color = love.graphics.newColor(255, 100, 0)
	entity.health = 1.0
	entities[table.getn(entities)+1] = entity
	
	entity = Entity('zombie2', 11.0, 9.0, 1.0)
	local zombie2 = entity
	entity.color = love.graphics.newColor(255, 100, 0)
	entity.health = 0.9
	entities[table.getn(entities)+1] = entity
	
	entity = Entity('zombie3', -14.0, -12.0, 1.0)
	local zombie3 = entity
	entity.color = love.graphics.newColor(255, 100, 0)
	entity.health = 1.0
	entities[table.getn(entities)+1] = entity
	
	entity = Entity('smoker', -8.0, 6.0, 1.0)
	local smoker = entity
	entity.color = love.graphics.newColor(255, 0, 0)
	entity.health = 1.5
	--entity.weapon = Shotgun()
	entities[table.getn(entities)+1] = entity
	
	entity = BoxEntity('cover', -10.0, 2.0, 7.0, 1.0)
	local cover1 = entity
	entity.color = love.graphics.newColor(0, 0, 0)
	entities[table.getn(entities)+1] = entity
	
	entity = BoxEntity('cover', 5.0, -2.0, 1.0, 6.0)
	local cover2 = entity
	entity.color = love.graphics.newColor(0, 0, 0)
	entities[table.getn(entities)+1] = entity
	
	avatar.drives = {}
	--avatar.drives[table.getn(avatar.drives)+1] = ResponseCurveUtility(EntityDistanceUtility(entities[1], entities[2]), ExpResponseCurve(0.5))
	--avatar.drives[table.getn(avatar.drives)+1] = EntityDistanceUtility(entities[1], entities[3])
	--avatar.drives[table.getn(avatar.drives)+1] = EntityDistanceUtility(entities[1], entities[4])
	--avatar.drives[table.getn(avatar.drives)+1] = EntityDistanceUtility(entities[1], entities[5])
	--avatar.drives[table.getn(avatar.drives)+1] = EntityDistanceUtility(entities[1], entities[6])
	avatar.drives[table.getn(avatar.drives)+1] = Drive('Heal', WeightUtility(ResponseCurveUtility(EntityFuncUtility(avatar,'getHealth'), InvExpResponseCurve(12.0)), 1.0))
	avatar.drives[table.getn(avatar.drives)+1] = Drive('Reload', WeightUtility(ResponseCurveUtility(EntityFuncUtility(avatar,'getWeaponRoundRatio'), InvExpResponseCurve(16.0)), 1.0))
	avatar.drives[table.getn(avatar.drives)+1] = Drive('Protect-self-zombie1', WeightUtility(ResponseCurveUtility(EntityDistanceUtility(avatar, zombie1), InvExpResponseCurve(1.25)), 1.0))
	avatar.drives[table.getn(avatar.drives)+1] = Drive('Protect-mate-zombie1', WeightUtility(ResponseCurveUtility(EntityDistanceUtility(mate, zombie1), InvExpResponseCurve(1.25)), 0.85))
	avatar.drives[table.getn(avatar.drives)+1] = Drive('Protect-self-zombie2', WeightUtility(ResponseCurveUtility(EntityDistanceUtility(avatar, zombie2), InvExpResponseCurve(1.25)), 1.0))
	avatar.drives[table.getn(avatar.drives)+1] = Drive('Protect-mate-zombie2', WeightUtility(ResponseCurveUtility(EntityDistanceUtility(mate, zombie2), InvExpResponseCurve(1.25)), 0.85))
	avatar.drives[table.getn(avatar.drives)+1] = Drive('Protect-self-zombie3', WeightUtility(ResponseCurveUtility(EntityDistanceUtility(avatar, zombie3), InvExpResponseCurve(1.25)), 1.0))
	avatar.drives[table.getn(avatar.drives)+1] = Drive('Protect-mate-zombie3', WeightUtility(ResponseCurveUtility(EntityDistanceUtility(mate, zombie3), InvExpResponseCurve(1.25)), 0.85))
	avatar.drives[table.getn(avatar.drives)+1] = Drive('Protect-self-smoker', WeightUtility(ResponseCurveUtility(EntityDistanceUtility(avatar, smoker), InvExpResponseCurve(1.175)), 1.0))
	avatar.drives[table.getn(avatar.drives)+1] = Drive('Protect-mate-smoker', WeightUtility(ResponseCurveUtility(EntityDistanceUtility(mate, smoker), InvExpResponseCurve(1.175)), 0.85))
	
	local drivesWidget = nil
	
	drivesWidget = EntityDriveWidget(avatar)
	drivesWidget.pos = engine.camera:invTransform(Vector2D(0.0, (engine.screenSize[2]) - (table.getn(avatar.drives) * 12)))
	entities[table.getn(entities)+1] = drivesWidget
	
end