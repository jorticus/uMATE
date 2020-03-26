
DissectorTable.new("matenet")
mate_proto = Proto("matenet", "Outback MATE serial protocol")

local commands = {
    [0] = "Dec/Dis",
    [1] = "Inc/En",
    [2] = "Read",
    [3] = "Write",
    [4] = "Status",
    [22] = "Get Logpage"
}

local query_registers = {
    -- MX/FX (Not DC)
    -- [0x0000] = "Device ID",
    -- [0x0001] = "FW Revision",

    -- FX
    -- [0x0039] = "Errors",
    -- [0x0059] = "Warnings",
    -- [0x003D] = "Inverter Control",
    -- [0x003A] = "AC In Control",
    -- [0x003C] = "Charge Control",
    -- [0x005A] = "AUX Mode",
    -- [0x0038] = "Equalize Control",
    -- [0x0084] = "Disconn Status",
    -- [0x008F] = "Sell Status",
    -- [0x0032] = "Battery Temperature",
    -- [0x0033] = "Air Temperature",
    -- [0x0034] = "MOSFET Temperature",
    -- [0x0035] = "Capacitor Temperature",
    -- [0x002D] = "Output Voltage",
    -- [0x002C] = "Input Voltage",
    -- [0x006D] = "Inverter Current",
    -- [0x006A] = "Charger Current",
    -- [0x006C] = "Input Current",
    -- [0x006B] = "Sell Current",
    -- [0x0019] = "Battery Actual",
    -- [0x0016] = "Battery Temperature Compensated",
    -- [0x000B] = "Absorb Setpoint",
    -- [0x0070] = "Absorb Time Remaining",
    -- [0x000A] = "Float Setpoint",
    -- [0x006E] = "Float Time Remaining",
    -- [0x000D] = "Refloat Setpoint",
    -- [0x000C] = "Equalize Setpoint",
    -- [0x0071] = "Equalize Time Remaining",

    -- MX
    -- [0x0008] = "Battery Voltage",
    -- [0x000F] = "Max Battery",
    -- [0x0010] = "V OC",
    -- [0x0012] = "Max V OC",
    -- [0x0013] = "Total kWh DC",
    -- [0x0014] = "Total kAh",
    -- [0x0015] = "Max Wattage",
    -- [0x016A] = "Charger Watts",
    -- [0x01EA] = "Charger kWh",
    -- [0x01C7] = "Charger Amps DC",
    -- [0x01C6] = "Panel Voltage",
    -- [0x01C8] = "Status",
    -- [0x01C9] = "Aux Relay Mode",
    -- [0x0170] = "Setpoint Absorb",
    -- [0x0172] = "Setpont Float",

    -- DC
    --[0x0064] = "",
    --[0x4004] = "",
    --[0x0044] = "",
    --[0x002a]
    --[0x003a]
    --[0x0024]
    --[0x0046]
    --[0x0040]
    --[0x0026]
    --[0x0048]
    --[0x002e]
    --[0x0064]
    --[0x4005]
}

local pf = {
    --bus = ProtoField.uint8("matenet.bus", "Bus", base.HEX),
    port = ProtoField.uint8("matenet.port", "Port", base.DEC),
    cmd  = ProtoField.uint8("matenet.cmd", "Command", base.HEX, commands),
    data = ProtoField.bytes("matenet.data", "Data", base.NONE),
    addr = ProtoField.uint16("matenet.addr", "Address", base.HEX),
    query_addr = ProtoField.uint16("matenet.queryaddr", "Address", base.HEX, query_registers),
    value = ProtoField.uint16("matenet.value", "Value", base.HEX),
    check = ProtoField.uint16("matenet.checksum", "Checksum", base.HEX)
}

mate_proto.fields = pf

--local ef_too_short = ProtoExpert.new("mate.too_short.expert", "MATE packet too short",
--                                    expert.group.MALFORMED, expert.severity.ERROR)

function dissect_frame(bus, buffer, pinfo, tree, combine)
    -- MATE TX (Command)
    if bus == 0xA then
		if not combine then
			pinfo.cols.src = "MATE"
			pinfo.cols.dst = "Device"
		end
		
		local subtree = tree:add(mate_proto, buffer(), "Command")
		
		if buffer:len() <= 7 then
			return
		end

        port = buffer(0, 1)
        cmd  = buffer(1, 1)
        addr = buffer(2, 2)
        value = buffer(4, 2)
        check = buffer(6, 2)
        --data = buffer(4, buffer:len()-4)
        subtree:add(pf.port, port)
        subtree:add(pf.cmd,  cmd)
        --subtree:add(pf.data, data)

        --pinfo.cols.info:set("Command")
        info = commands[cmd:uint()]
        if info then
            pinfo.cols.info:prepend(info .. " ")
        end

        if cmd:uint() <= 4 then
            subtree:add(pf.query_addr, addr)
            info = query_registers[addr:uint()]
            if info then
                pinfo.cols.info:append(" ["..info.."]")
            else
                pinfo.cols.info:append(" ["..addr.."]")
            end
        else
            subtree:add(pf.addr, addr)
        end

        subtree:add(pf.value, value)
        subtree:add(pf.check, check)

        pinfo.cols.dst = "Port " .. port
		
		return -1
		
    -- MATE RX (Response)
    elseif bus == 0xB then
		if not combine then
			pinfo.cols.src = "Device"
			pinfo.cols.dst = "MATE"
		end
		
		local subtree = tree:add(mate_proto, buffer(), "Response")

		if buffer:len() <= 3 then
			return
		end
		
		cmd = buffer(0, 1)
		subtree:add(pf.cmd, cmd)
	
		data = buffer(1, buffer:len()-3)
		check = buffer(buffer:len()-2, 2)
		subtree:add(pf.data, data)
		subtree:add(pf.check, check)
	
		if not combine then
			pinfo.cols.info:set("Response")
			info = commands[cmd:uint()]
			if info then
				pinfo.cols.info:prepend(info .. " ")
			end
		else
			-- append the response value
			if cmd:uint() <= 3 then
				pinfo.cols.info:append(" : " .. data)
			end
		end
    end
end

function mate_proto.dissector(buffer, pinfo, tree)
    len = buffer:len()
    if len == 0 then return end

    pinfo.cols.protocol = mate_proto.name

    --local subtree = tree:add(mate_proto, buffer(), "MATE Data")

    -- if len < 5 then
    --     subtree.add_proto_expert_info(ef_too_short)
    --     return
    -- end

    bus = buffer(0, 1):uint()
    --subtree:add(pf.bus, bus)
    buffer = buffer(1, buffer:len()-1)

    -- local data = {}
    -- for i=0,buffer:len() do
    --     data[i] = i
    -- end
	
	-- Combined RX/TX
	if bus == 0x0F then
		len_a = buffer(0, 1):uint()
		len_b = buffer(1, 1):uint()
		
		buf_a = buffer(2, len_a)
		buf_b = buffer(2+len_a, len_b)
		
		r_a = dissect_frame(0xA, buf_a, pinfo, tree, true)
		r_b = dissect_frame(0xB, buf_b, pinfo, tree, true)
		--return r_a + r_b
		
		pinfo.cols.src = "MATE"
		--pinfo.cols.dst = "Device"
		
	else
		return dissect_frame(bus, buffer, pinfo, tree, false)
	end


end

DissectorTable.get("matenet"):add(147, mate_proto) -- DLT_USER0