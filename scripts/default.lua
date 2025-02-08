local MAX <const> = 255

-- Iterates over the palette (i = 0->maxIterations+1)
-- Returns an RGB value and inserts it into the palette at index i
function setPaletteColor(i, maxIterations)
    r, g, b = 0, 0, 0

    -- Check if pixel is outside the set
    if i < maxIterations then
        r = 0
        g = MAX * (i / maxIterations)
        b = 0
    end

    return r, g, b
end
