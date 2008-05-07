#ifndef RUNGEKUTTA4_H
#define RUNGEKUTTA4_H

#include <vector>

std::vector<double> rungeKutta4(
    double xi,
    std::vector<double> yi,
    double dx,
    std::vector<double> (*derivatives)(double x, std::vector<double> y) 
);

#endif
