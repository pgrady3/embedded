--[[

  ========================
  ===  Altitude  Plot  ===
  ========================

  ver 1.1.1 10/16/2018 (update variable "ver" to match)
  J.W.G.  (filago on RCGroups)


  > LUA Script for OpenTX 2.1 & 2.2 w/Taranis to plot altitude vs time.
  > Y axis automatically re-scales to fit the data.
  > Altitudes are in meters.

  > Save this in SCRIPTS/TELEMETRY as AltPlt.lua

--]]

--  ===========================
--	  declare & set variables
--  ===========================

-- variables for user to adjust
local ver = "v1.1.1"  -- code revision
local yMaxInit = 20  -- initial max altitude on graph (meters)
local xMax = 60  -- duration of time shown on graph (seconds)
local xScale = 30  -- X-axis marker interval (seconds)

-- get field index numbers for Tx parameters
local alt_id = getFieldInfo("Alt").id  -- telemetry altitude (m)
local thr_id = getFieldInfo("thr").id  -- throttle stick value
local rud_id = getFieldInfo("rud").id  -- rudder stick value
local ail_id = getFieldInfo("ail").id  -- aileron stick value
-- local  SE_id = getFieldInfo("se").id  -- switch E (for demo mode)

local vario_id = getFieldInfo("VSpd").id  -- telemetry altitude (m)

-- other variables
local alt = {}  -- array of altitudes to graph (m)
local cursorAilAlt  -- for calculating glide Vspd in pause mode
local cursorAilIndex  -- for calculating glide Vspd in pause mode
local cursorAilTime  -- for calculating glide Vspd in pause mode
local cursorRudAlt  -- for calculating glide Vspd in pause mode
local cursorRudIndex  -- for calculating glide Vspd in pause mode
local cursorRudTime  -- for calculating glide Vspd in pause mode
local demo = false  -- flag for demo mode (if true, then get Altitude from Thr stick)
local gLeft = 15  -- X location of left graph border
local gWidth = 85  -- width of graph between borders, and alt array size
local gRight = gLeft + gWidth + 1  -- X location of right graph border
local index = 0  -- index position of array (graph column #)
local i  -- temporary use to control "for" loops
local maxAlt  -- maximum altitude shown on graph
local nextAltTime = 0  -- time to read next altitude
local nextVspdTime = 0  -- time to calc next live Vspeed
local nowTime  -- current time (sec since radio started)
local recentAlt = {} -- array of recent altitudes for live Vspd calculation
local RIGHT  -- see init(), OK to remove when OTX2.1 compatibility is no longer needed
local state = 1  -- 0 = pause state, 1 = run state
local vSpd = 0  -- vertical speed over last 10 seconds (m/s)
local varioVal = 0
local y  -- temp use for calculating Y coordinate (m)
local yMax = yMaxInit  -- Y-axis max value (m)
local yScale  -- Y-axis marker interval (m)


--  ======================
--	  initialize program
--  ======================

local function init()

	-- only needed for OTX 2.1 compatibility, so "+RIGHT" can be added to
	-- drawNumber statements for OTX 2.2 (because RIGHT flag doesn't exist in 2.1)
	local ver, radio = getVersion()  -- radio is "taranis-simu" if from Companion
	if string.find(ver, "^2.1") then	 -- if OTX version starts with 2.1
		RIGHT = 0
	else
		RIGHT = 8
	end

	-- set altitude array values to 0
	for i = 1, gWidth do
		alt[i] = 0
	end
	
	-- set recent altitude array values to 0
	for i = 0, 10 do
		recentAlt[i] = 0
	end

	index = 0
	maxAlt = 0

end  -- init()


--  ===================
--	  draw the screen
--  ===================

local function screenDraw()

-- Note: Taranis LCD is 212 pixels wide (0 to 211) and 64 pixels high (0 to 63.)
-- Position (0,0) is at top left, so Y of top line is 0, bottom line is 63.

	lcd.clear()  -- clear the display

	-- set Y max; 63/62 will touch top of graph, 63/61 will leave 1 pixel above top
	yMax = math.max(yMaxInit,(63/61 * maxAlt)) 

	--- set interval of Y scale (horizontal) lines
	-- define array of available intervals
	a = {200, 100, 50, 25, 20, 10, 5}
	
	-- find the smallest interval that will draw no more than 5 lines.
	for i,v in ipairs(a) do
		if (yMax-2)/v < 6 then
			yScale = v
		end
	end

	-- draw Y scale (horizontal) numbers and lines
	for i = yScale, yMax, yScale do
		
		-- calculate y coordinate
		y = 63 * (1 - i/yMax)
		
		-- draw the number if it will fit on the screen
		if y > 5 then
			lcd.drawNumber(1, y-3, i, SMLSIZE+RIGHT)
		end
		
		-- draw the line if it is far enough below top of graph
	--	if y > 2 then
	--		lcd.drawLine(gLeft+1, y, gRight, y, DOTTED, 0)
	--	end
	end

	-- draw the X scale (vertical) lines
	--for i = xScale, xMax, xScale do
	--	lcd.drawLine (gLeft+i*gWidth/xMax, 0, gLeft+i*gWidth/xMax, 63, DOTTED, 0)
	--end

	-- draw the altitude array

	oldy = -10000
	for i = 1, gWidth do
		
		-- calculate Y coordinate for graph point
		y = (63 * (1 - alt[i]/yMax))
		
		-- draw grey line down from altitude
		lcd.drawLine(gLeft+i, y+1, gLeft+i, 63, SOLID, 0)
		
		-- draw 3 pixel point for altitude
		--if oldy > -10000 then
		--	lcd.drawLine(gLeft+i-2, oldy, gLeft+i, y, SOLID, 0)
		--end

		oldy = y
	end
	
	-- draw the text
	-- I tried using string.format here but the control over centering
	-- isn't as good, and the decimal point is 4 dots instead of 1.
	--lcd.drawText (174, 2, "AltPlt")
	--lcd.drawText (176, 12, ver, SMLSIZE)
	--lcd.drawNumber (gRight + 10, 26, maxAlt*10, PREC1+RIGHT)
	--lcd.drawText (gRight + 11, 26, "m")
	--lcd.drawText (182, 35, "max")
	--lcd.drawNumber (202, 51, nowAlt*10, MIDSIZE+PREC1+RIGHT)
	--lcd.drawText (203, 55, "m")
	
	-- if in demo mode
	if demo then
		lcd.drawText (gRight-21, 11, "DEMO", SMLSIZE+BLINK)
	end
	
	-- if graph is paused
	if state == 0 then
		lcd.drawText (gRight-25, 2, "PAUSE", SMLSIZE+INVERS+BLINK)
	end

	-- draw the solid lines
	lcd.drawLine (gLeft, 0, gLeft, 63, SOLID, 0)
	lcd.drawLine (gRight, 0, gRight, 63, SOLID, 0)
	--lcd.drawLine (gRight+1, 21, 211, 21, SOLID, 0)
	--lcd.drawLine (gRight+1, 47, 211, 47, SOLID, 0)
	
	-- draw the 2 cursors when the graph is paused, but only when they are separated
	if (state == 0) and (math.abs(cursorRudIndex-cursorAilIndex) > 2) then
		lcd.drawLine(gLeft+cursorRudIndex, 0, gLeft+cursorRudIndex, 63, SOLID, 0)
		lcd.drawLine(gLeft+cursorAilIndex, 0, gLeft+cursorAilIndex, 63, SOLID, 0)
	end
	
	lcd.drawText (102, 0, "Alt m")
	lcd.drawNumber (102, 8, nowAlt*10, MIDSIZE+PREC1+RIGHT)
	--lcd.drawText (31, 55, "m/s")
	lcd.drawText (102, 21, "10s avg")
	lcd.drawNumber (102, 29, vSpd*10, MIDSIZE+PREC1+RIGHT)
	lcd.drawText (102, 44, "Vario")
	lcd.drawNumber (102, 52, varioVal*10, MIDSIZE+PREC1+RIGHT)
	--lcd.drawText (41, 55, "m")


end  -- screenDraw()


--  ===============
--	  user inputs
--  ===============

-- get and act on user inputs (set mode, state, and chage graph duration)
local function userInputs(event)

--[[  comment out this section for RCGroups so switch E is not used.
	-- set demo mode if switchE is up: value >0=down, 0=middle, <0=up
	if getValue(SE_id) < 0 then
		demo = true
	else
		demo = false
	end
--]]

	-- set state to "pause" with Exit button
	if event == 97 then
		state = 0
	end
	
	-- set state to "run" with Enter button
	if event == 98 then
		state = 1
	end
		
	-- decrease graph time by one minute with Minus button; force to >= 1 minute
	if (event == EVT_MINUS_FIRST)then
		xMax = math.max(xMax - 60,60)
		init()
	end
	
	-- increase graph time by one minute with Plus button
	if (event == EVT_PLUS_FIRST) then
		xMax = xMax + 60
		init()
	end

end  -- setState()


--  ==================
--	  "run" function
--  ==================

-- this function runs until it is stopped
local function run(event)

	-- get and act on user inputs
	userInputs(event)

	-- get current altitude; from Thr stick if demo, else from altimeter
	if demo then
		nowAlt = 200 * (getValue(thr_id)+1024) / 2048  -- 0 to 200 m
	else
		nowAlt = getValue(alt_id)
		varioVal = getValue(vario_id)
	end
	
	-- get altitude and time at cursors if paused
	if state == 0 then
		-- this formula returns integer values from 1 to gWidth, centered between them.
		cursorRudIndex = math.floor(((gWidth-1) * (getValue(rud_id)+1024) / 2048) + 1.5)
		cursorAilIndex = math.floor(((gWidth-1) * (getValue(ail_id)+1024) / 2048) + 1.5)
		cursorRudAlt = alt[cursorRudIndex]
		cursorAilAlt = alt[cursorAilIndex]
		cursorRudTime = cursorRudIndex * (xMax/gWidth)
		cursorAilTime = cursorAilIndex * (xMax/gWidth)
		
		-- calculate delta vSpd between cursors, and avoid divide by zero
		if cursorRudTime == cursorAilTime then
			vSpd = 0
		else
			vSpd = (cursorAilAlt - cursorRudAlt) / (cursorAilTime - cursorRudTime)
		end
	end
	
	
	-- if running
	if state == 1 then
		
		-- get current time (seconds since radio started)
		nowTime = getTime()/100
		
		-- if time to plot next altitude
		if nowTime > nextAltTime then
			
			-- add altitude to next array point
			alt[index+1] = nowAlt
			
			-- reset max altitude for re-calculation
			maxAlt = 0
				
			-- iterate through the graph array
			for i = 1, gWidth do
			
				-- if graph was already full, shift the altitudes one column to the left
				if (index == gWidth) then
					alt[i] = alt[i+1]
				end
				
				-- recalc max alt
				maxAlt = math.max(maxAlt, alt[i])
			end		
			
			-- increment the index if the graph isn't full
			index = math.min(index+1, gWidth)
						
			-- calculate when to plot next altitude
			nextAltTime = nowTime + (xMax/gWidth)
		end
		
		--- calculate Vspd every second, comparing altitude now vs 10 seconds ago
		-- if 1 second has elapsed since previous recent alt was captured
		if nowTime >= nextVspdTime then
			
			-- update the recent altitudes array
			for i = 10, 1, -1 do
				recentAlt[i] = recentAlt[i-1]
			end
			
			-- add the current altitude
			recentAlt[0] = nowAlt
			
			-- calc vSpd (m/s) vs 10 sec ago if it exists
			if recentAlt[10] ~= 0 then
				vSpd = (nowAlt - recentAlt[10]) / 10
			end
			
			-- set the time for next calculation
			nextVspdTime = nowTime + 1
		end
	end

	screenDraw()  -- draw everything to the Taranis screen

end  -- run(event)

return { init=init, run=run }


--[[  CODE NOTES:
			
			
--]]
