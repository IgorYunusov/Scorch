#include "pch.h"
#include "wow/common/CGVector3.h"

/* Jadd of ownedcore */
#define M_PI 3.14159265358979323846
#define M_PI_F float(M_PI)
static float_t NormalizeAngle(float fAngle)
{
    if (fAngle < 0.0f)
        fAngle += M_PI_F * 2.0f;
    else if (fAngle > M_PI_F * 2.0f)
        fAngle -= M_PI_F * 2.0f;
    return fAngle;
}

float_t CGVector3::distanceTo(CGVector3 *other) 
{
    float_t dx = other->x - this->x;
    float_t dy = other->y - this->y;
    float_t dz = other->z - this->z;
    return sqrt((dx * dx) + (dy * dy) + (dz * dz));
}

float_t CGVector3::angleBetween(CGVector3* other)
{
    float_t dX = other->x - this->x;
    float_t dY = other->y - this->y;
    float_t angleTo = NormalizeAngle(atan2(dY, dX));
    return angleTo;
}