#pragma once

#include "main.h"

#include "wrappers/Task.hpp"
#include "util/gpio.hpp"

#include "AbstractTube.hpp"
#include "Nixie.hpp"
#include "VFD.hpp"

class TubeControl : public util::wrappers::TaskWithMemberFunctionBase
{
public:
    TubeControl() : TaskWithMemberFunctionBase("tubeControlTask", 256, osPriorityRealtime5){};

protected:
    void taskMain(void *) override;

private:
    // Nixie nixieTubes{};
    VFD vfdTubes{};
    AbstractTube *tubes = &vfdTubes; //&nixieTubes;
};