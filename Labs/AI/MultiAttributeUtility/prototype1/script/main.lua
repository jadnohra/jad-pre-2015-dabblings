love.filesystem.require('require.lua')
love.filesystem.require('scenario1/require.lua')

engine = Engine()

function load()
	
	engine:load()
	
	local scenario = Scenario()
	scenario:load(engine)
		
end

function update(dt)
	
	engine:update(dt)
	
end

function draw()

	engine:draw()
	
end



