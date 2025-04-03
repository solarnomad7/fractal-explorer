#pragma once

#include "Set.h"

class JuliaSet : public Set {
    public:
        JuliaSet(int maxIterations_, int imageWidth_, int imageHeight_, lua_State* lua_);
        void generateSet();

        double cx = -0.8;
        double cy = 0.2;
    
    private:
        int calculatePoint(double x0, double y0);
};