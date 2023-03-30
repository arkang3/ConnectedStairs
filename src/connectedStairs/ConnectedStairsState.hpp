#pragma once

enum ConnectedStairsState : unsigned char {
    NONE,
    DOWN2UP_START,
    UP2DOWN_START,
    DOWN2UP_RUNNING,
    UP2DOWN_RUNNING,
    DOWN2UP_FINISHED,
    UP2DOWN_FINISHED,
    FINISHED,
    ON,
    OFF
};