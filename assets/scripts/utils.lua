RAD = math.pi / 180
DEG = 180 / math.pi
local flags = {}
rawset(flags, "SHOW_ORIGIN", 1 << 1)
rawset(flags, "SHOW_BOX"   , 1 << 2)
rawset(flags, "SHOW_BOUND" , 1 << 3)
rawset(flags, "SHOW_PIVOT", 1 << 4)
rawset(flags, "SHOW_TRANSFORM", 1 << 5)
rawset(flags, "SHOW_COMPONENTS", 1 << 6)
rawset(flags, "SHOW_ALL", flags.SHOW_ORIGIN | flags.SHOW_BOX | flags.SHOW_BOUND | flags.SHOW_PIVOT | flags.SHOW_TRANSFORM )



function printTable(t, indent)
    indent = indent or 0
    for k, v in pairs(t) do
        if type(v) == "table" then
            print(string.rep("  ", indent) .. tostring(k) .. " = {")
            printTable(v, indent + 1)
            print(string.rep("  ", indent) .. "}")
        else
            print(string.rep("  ", indent) .. tostring(k) .. " = " .. tostring(v))
        end
    end
end

function max(a, b)
    return (a > b) and a or b
end

function min(a, b)
    return (a < b) and a or b
end

function lengthdir_x(length, direction)
    return length * math.cos(direction * RAD)
end

function lengthdir_y(length, direction)
    return length * math.sin(direction * RAD)
end

function point_distance(x1, y1, x2, y2)
    return math.sqrt((x1 - x2)^2 + (y1 - y2)^2)
end

function point_direction(x1, y1, x2, y2)
    return math.atan2(y2 - y1, x2 - x1) * DEG
end

function degtorad(degrees)
    return degrees * RAD
end

function radtodeg(radians)
    return radians * DEG
end

return {
    flags = flags,
    radtodeg = radtodeg,
    degtorad = degtorad,
    point_direction = point_direction,
    point_distance = point_distance,
    lengthdir_x = lengthdir_x,
    lengthdir_y = lengthdir_y,
    min = min,
    max = max,
    printTable = printTable

  }