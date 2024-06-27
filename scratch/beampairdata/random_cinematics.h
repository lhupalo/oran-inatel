#include "ns3/trace-helper.h"
#include "ns3/core-module.h"
#include <random>

using namespace std;
using namespace ns3;

std::random_device rd{};
std::mt19937 gen1{rd()};

Vector random_position(double x0, double y0, double zue, double r, double ri, int sector)
{
    bool normal = false;

    std::uniform_real_distribution<double> dist1(0.0, 1.0);
    double anglemin;
    double anglemax;
    if (sector == 1)
    {
        anglemin = 0;
        anglemax = M_PI / 6;
    }
    else if (sector == 2)
    {
        anglemin = M_PI / 6;
        anglemax = M_PI / 3;
    }
    else if (sector == 3)
    {
        anglemin = M_PI / 3;
        anglemax = M_PI / 2;
    }
    else if (sector == 4)
    {
        anglemin = M_PI / 2;
        anglemax = 2 * M_PI / 3;
    }
    else if (sector == 5)
    {
        anglemin = 2 * M_PI / 3;
        anglemax = 5 * M_PI / 6;
    }
    else if (sector == 6)
    {
        anglemin = 5 * M_PI / 6;
        anglemax = M_PI;
    }
    else if (sector == 7)
    {
        anglemin = M_PI;
        anglemax = 7 * M_PI / 6;
    }
    else if (sector == 8)
    {
        anglemin = 7 * M_PI / 6;
        anglemax = 4 * M_PI / 3;
    }
    else if (sector == 9)
    {
        anglemin = 4 * M_PI / 3;
        anglemax = 3 * M_PI / 2;
    }
    else if (sector == 10)
    {
        anglemin = 3 * M_PI / 2;
        anglemax = 5 * M_PI / 3;
    }
    else if (sector == 11)
    {
        anglemin = 5 * M_PI / 3;
        anglemax = 11 * M_PI / 6;
    }
    else if (sector == 12)
    {
        anglemin = 11 * M_PI / 6;
        anglemax = 2 * M_PI;
    }
    else
    {
        std::uniform_real_distribution<double> dist2(0, 1);
        normal = true;
    }

    std::uniform_real_distribution<double> dist2(anglemin, anglemax);
    double random_r;
    double random_theta;
    bool stop = true;
    while (stop)
    {
        random_r = r * sqrt(dist1(gen1));
        if (normal == false)
        {
            random_theta = dist2(gen1);
        }
        else
        {
            random_theta = 2 * M_PI * dist2(gen1);
        }

        if (random_r >= ri)
        {
            stop = false;
        }
        else
        {
            stop = true;
        }
    }
    return Vector(x0 + random_r * cos(random_theta), y0 + random_r * sin(random_theta), zue);
}

Vector random_velocity()
{
    double vx = sqrt(2) / 2;
    double vy = sqrt(2) / 2;
    double vz = 0;

    std::uniform_real_distribution<double> dist1(0.0, 1.0);

    if (dist1(gen1) < 0.5)
    {
        vx = -1 * vx;
    }
    if (dist1(gen1) > 0.5)
    {
        vy = -1 * vy;
    }

    return Vector(vx, vy, vz);
}