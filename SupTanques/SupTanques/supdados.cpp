#include <iostream>
#include <iomanip>
#include <string>
#include "tanques-param.h"
#include "supdados.h"

/// Funcoes auxiliares para impressao
inline double H_cm(uint16_t H)
{
  return (100.0*MaxTankLevelMeasurement*H)/UINT16_MAX;
}
inline std::string V_string(uint16_t V)
{
  return (V!=0 ? "OP" : "CL");
}

/// Impressao em console do estado da planta
void SupState::print() const
{
  std::cout << "+------------------+--------------+-----------------+\n";
  std::cout << "|    LEVEL (cm)    |    VALVES    | PUMP (%, l/min) |\n";
  std::cout << "+------------------+--------------+-----------------+\n";
  std::cout << std::fixed;
  std::cout.precision(1);
  std::cout << "| H1 " << std::setw(4) << H_cm(H1)
            << "  H2 " << std::setw(4) << H_cm(H2) << ' ';
  std::cout << "| V1 " << V_string(V1)
            << "  V2 " << V_string(V2) << ' ';
  std::cout.precision(0);
  std::cout << "| In " << std::setw(3)
            << (100.0*PumpInput)/UINT16_MAX << '%';
  std::cout.precision(1);
  std::cout << "  Fl " << std::setw(3)
            << (60000.0*MaxPumpFlowMeasurement*PumpFlow)/UINT16_MAX << " |\n";
  if (ovfl!=0) std::cout << "+-----OVERFLOW-----";
  else std::cout         << "+------------------";
  std::cout << "+--------------+-----------------+\n";
  std::cout << std::setw(0) << std::defaultfloat;
}

