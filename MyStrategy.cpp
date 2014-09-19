#include "MyStrategy.h"
#include "geometry.h"

#include <cmath>
#include <cstdlib>
#include <iostream>


using namespace model;
using namespace std;

bool IsReachable(uint16_t ax, uint16_t ay, uint16_t bx, uint16_t by, double aAngle) {
    double angleTo = GetAngle(ax, ay, bx, by);
    double minAngle = GetAngle(aAngle, angleTo);
    return Distance(ax, ay, bx, by) <= 120.0 && (fabs(minAngle) < PI / 12);
}

MyStrategy::MyStrategy() {
}

void MyStrategy::move(const Hockeyist& self, const model::World& world, const model::Game& game, model::Move& move) {
    Self = &self;
    World = &world;
    Game = &game;
    Move = &move;
    UpdateState();

    if (Self->getSwingTicks() && !HavePuck) {
        Move->setAction(CANCEL_STRIKE);
        return;
    }

    if (world.getPuck().getOwnerPlayerId() != self.getPlayerId()) {
        if (Distance(Self->getX(), Self->getY(), Puck->getX(), Puck->getY()) <
            Distance(Partner->getX(), Partner->getY(), Puck->getX(), Puck->getY()))
        {
            MoveToPuck();
        } else {
            MoveToDefence();
        }
    } else {
        if (HavePuck) {
            MakeStrike();
        } else {
            MoveToDefence();
        }
    }
}

void MyStrategy::UpdateState() {
    EnemyGoalie = nullptr;
    SelfGoalie = nullptr;
    Partner = nullptr;
    PuckHockeist = nullptr;
    Puck = &World->getPuck();
    for (const Hockeyist& h: World->getHockeyists()) {
        if (h.getType() == GOALIE) {
            if (h.isTeammate()) {
                SelfGoalie = &h;
            } else {
                EnemyGoalie = &h;
            }
        } else if (h.isTeammate() && h.getId() != Self->getId()) {
            Partner = &h;
        }
        if (h.getId() == Puck->getOwnerHockeyistId()) {
            PuckHockeist = &h;
        }
    }

    HavePuck = Puck->getOwnerHockeyistId() == Self->getId();

    if (EnemyGateX == uint16_t(-1)) {
        FieldWidth = World->getWidth();
        FieldHeight = World->getHeight();
        FieldCenterX = Game->getRinkLeft() + 0.5 * (Game->getRinkRight() - Game->getRinkLeft());
        FieldCenterY = Game->getRinkTop() + 0.5 * (Game->getRinkBottom() - Game->getRinkTop());
        GateX1 = 0;
        GateX2 = World->getWidth();

        EnemyGateX = EnemyGoalie->getX() > FieldWidth / 2 ? GateX2 : GateX1;
        SelfGateX = SelfGoalie->getX() > FieldWidth / 2 ? GateX2 : GateX1;
    }
}

void MyStrategy::MoveToPuck() {
    double puckAngle = GetAngle(Self->getX(), Self->getY(), Puck->getX(), Puck->getY());
    double minAngle = GetAngle(Self->getAngle(), puckAngle);
    if (fabs(minAngle < 0.4)) {
        Move->setSpeedUp(1.0);
    }
    Move->setTurn(minAngle);
    if (World->getPuck().getOwnerPlayerId() == Self->getPlayerId()) {
        return;
    }

    bool reachableForStrike = IsReachable(Self->getX(), Self->getY(), Puck->getX(), Puck->getY(), Self->getAngle());

    double puckSpeed = sqrt(Puck->getSpeedX() * Puck->getSpeedX() + Puck->getSpeedY() * Puck->getSpeedY());
    double puckSpeedAngle;
    bool puckMovingToOurGatesDirection = false;
    if (SelfGateX > FieldCenterX) {
        puckMovingToOurGatesDirection = Puck->getSpeedX() > 0;
        puckSpeedAngle = atan2(-Puck->getSpeedY(), Puck->getSpeedX());
    } else {
        puckSpeedAngle = atan2(-Puck->getSpeedY(), -Puck->getSpeedX());
        puckMovingToOurGatesDirection = Puck->getSpeedX() < 0;
    }

    puckSpeedAngle *= 57.0;


    if (Puck->getOwnerHockeyistId() == -1) {
        // Если угроза воротам - выбиваем нафиг шайбу
        if (puckMovingToOurGatesDirection && puckSpeed > 16.0 &&
            fabs(puckSpeedAngle) > 22 && reachableForStrike)
        {
            Move->setAction(STRIKE);
        } else { // иначе просто пытаемся отобрать
            Move->setAction(TAKE_PUCK);
        }
    } else if (reachableForStrike) {
        Move->setAction(STRIKE);
    } else {
        Move->setAction(TAKE_PUCK);
    }
}

void MyStrategy::MoveToCenter() {
    if (Distance(Self->getX(), Self->getY(), FieldCenterX, FieldCenterY) > 250) {
        double centerAngle = GetAngle(Self->getX(), Self->getY(), FieldCenterX, FieldCenterY);
        double minAngle = GetAngle(Self->getAngle(), centerAngle);
        Move->setTurn(minAngle);
        if (fabs(minAngle) < 0.1) {
            Move->setSpeedUp(1.0);
        }
    } else {
        StopMoving();
    }
}

template<typename T>
bool IsBetween(T c, T a, T b) {
    return (c > a && c < b) || (c > b && c < a);
}

void MyStrategy::MoveToDefence() {    
    double defencePosX = SelfGateX < FieldWidth / 2 ? SelfGateX + 90 : SelfGateX - 90;
    double defencePosY = FieldCenterY;
    if (SelfGoalie) {
        if (SelfGoalie->getY() > FieldCenterY) {
            defencePosY = FieldCenterY - 100 + 0.5 * (SelfGoalie->getY() - (FieldCenterY - 100));
        } else {
            defencePosY = FieldCenterY + 100 - 0.5 * (FieldCenterY + 100 - SelfGoalie->getY());
        }
    }

    double defenceDistance = Distance(Self->getX(), Self->getY(), defencePosX, defencePosY);
    double defenceDistanceX = fabs(Self->getX() - defencePosX);

    double puckAngle = GetAngle(Self->getX(), Self->getY(), Puck->getX(), Puck->getY());

    double puckDistance = Distance(SelfGateX, FieldCenterY, Puck->getX(), Puck->getY());


    if (Puck->getOwnerPlayerId() != Self->getPlayerId() &&
            (puckDistance < 500))
    {
        MoveToPuck();
    } else {
        if (defenceDistance > 70) {
            double defenceAngle = GetAngle(Self->getX(), Self->getY(), defencePosX, defencePosY);
            double minAngle = GetAngle(Self->getAngle(), defenceAngle);
            double minReverseAngle = GetAngle(Self->getAngle() + PI, defenceAngle);

            if (defenceDistanceX < 30 && SelfGoalie &&
                (IsBetween(SelfGoalie->getY(), defencePosY, Self->getY()) ||
                 (Self->getY() > FieldCenterY + 90) || (Self->getY() < FieldCenterY - 90)))
            {
                double centerAngle = GetAngle(Self->getX(), Self->getY(), FieldCenterX, FieldCenterY);
                double minCenterAngle = GetAngle(Self->getAngle(), centerAngle);
                Move->setTurn(minCenterAngle);
                Move->setSpeedUp(1.0);
            } else if (defenceDistance > 220) {
                Move->setTurn(minAngle);
                if (fabs(minAngle) < 0.1) {
                    Move->setSpeedUp(1.0);
                }
            } else {
                Move->setTurn(minReverseAngle);
                if (fabs(minReverseAngle) < 0.1) {
                    Move->setSpeedUp(-1.0);
                }
            }
        } else {
            StopMoving();
            double minAngle = GetAngle(Self->getAngle(), puckAngle);
            Move->setTurn(minAngle);
        }
    }
}

void MyStrategy::MakeStrike() {
    StopMoving();

    uint16_t gateY = FieldCenterY;

    if (EnemyGoalie) {
        if (EnemyGoalie->getY() > FieldCenterY) {
            gateY = FieldCenterY - 0.8 * (EnemyGoalie->getY() - (FieldCenterY - 100));
        } else {
            gateY = FieldCenterY + 0.8 * (FieldCenterY + 100 - EnemyGoalie->getY());
        }
    } else {
        gateY = FieldCenterY;
    }

    double gateDistance = fabs(EnemyGateX - Self->getX());
    double speed = sqrt(Self->getSpeedX() * Self->getSpeedX() + Self->getSpeedY() * Self->getSpeedY());

    if (gateDistance > 550) {
        double gateX = EnemyGateX;

        // Если далеко от ворот - едем ближе к краю (с края проще забить)
        if (gateDistance > 600) {
            gateY = Self->getY() > FieldCenterY ? FieldCenterY + FieldWidth / 2 : FieldCenterY - FieldWidth / 2;
            gateX = EnemyGateX > FieldCenterX ? EnemyGateX - 400 : EnemyGateX + 400;
        }

        // Если подъезжаем - нацеливаемся на центр ворот
        gateY = Self->getY() > FieldCenterY ? FieldCenterY + 110 : FieldCenterY - 110;

        double gateAngle = GetAngle(Self->getX(), Self->getY(), gateX, gateY);
        double minAngle = GetAngle(Self->getAngle(), gateAngle);

        if (fabs(minAngle) < 0.9) {
            Move->setSpeedUp(1.0);
        }
        Move->setTurn(minAngle);

    } else {

        // по прежнему едем к центру
        double gateAngle = GetAngle(Self->getX(), Self->getY(), EnemyGateX, gateY);
        double minAngle = GetAngle(Self->getAngle(), gateAngle);

        Move->setSpeedUp(1.0);
        Move->setTurn(minAngle);

        // за 350 px замахиваемся
        if (gateDistance < 395) {
            if (!Self->getSwingTicks()) {
                Move->setAction(SWING);
                return;
            }
        }

        // за 300 - бьём
        if (gateDistance < 340) {
            Move->setAction(STRIKE);
        }
    }

}

void MyStrategy::StopMoving() {
    double currentSpeed = sqrt(Self->getSpeedX() * Self->getSpeedX() + Self->getSpeedY() * Self->getSpeedY());
    double currentAngle = atan2(Self->getSpeedY(), Self->getSpeedX());
    double currentMinAngle = GetAngle(Self->getAngle(), currentAngle);

    if (currentSpeed > 0.1) {
        if (currentMinAngle < 0.4) {
            Move->setSpeedUp(-currentSpeed);
        } else if (currentMinAngle > 3.0) {
            Move->setSpeedUp(currentSpeed);
        }
    }
}
