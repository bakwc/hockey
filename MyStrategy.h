#pragma once

#include <ctype.h>

#include "Strategy.h"

class MyStrategy : public Strategy {
public:
    MyStrategy();

    void move(const model::Hockeyist& self, const model::World& world, const model::Game& game, model::Move& move);
private:
    void UpdateState();

    void MoveToPuck();
    void MoveToCenter();
    void MoveToDefence();
    void MakeStrike();
    void StopMoving();
private:
    const model::Hockeyist* Self;
    const model::World* World;
    const model::Game* Game;
    model::Move* Move;
    const model::Hockeyist* EnemyGoalie;
    const model::Hockeyist* SelfGoalie;
    const model::Hockeyist* Partner;
    const model::Puck* Puck;
    bool HavePuck = false;
    uint16_t EnemyGateX = -1;
    uint16_t SelfGateX = -1;
    uint16_t FieldWidth = -1;
    uint16_t FieldHeight = -1;
    uint16_t FieldCenterX = -1;
    uint16_t FieldCenterY = -1;
    uint16_t GateX1 = -1;
    uint16_t GateX2 = -1;
};
