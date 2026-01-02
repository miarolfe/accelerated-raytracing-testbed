// Copyright Mia Rolfe. All rights reserved.
#include <stdio.h>
#include "../lib/AxisAlignedBoundingBox.h"
#include "../lib/Logger.h"

int main()
{
    ART::Logger::Get().LogInfo("Booting up");
    ART::AABB aabb(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    bool running = true;
    while (running)
    {
        printf("Hello, World!\n");
    }
}
