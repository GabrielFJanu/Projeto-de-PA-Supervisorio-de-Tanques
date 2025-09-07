#ifndef _TANQUES_PARAM_
#define _TANQUES_PARAM_

/// Caracteristicas dos tanques (em metros)
/// Altura dos tanques
#define TankHeight 0.28
/// Profundidade dos tanques
#define TankDepth 0.26
/// Largura do tanque 1
#define Tank1Width 0.22
/// Largura do tanque 2
#define Tank2Width 0.14

/// Caracteristicas dos orificios
/// Altura do orificio entre tanques 1 e 2 (em m)
#define Hole12Height 0.065
/// Altura do orificio de transbordamento (em m)
#define OverflowHeight 0.25

/// Caracteristicas dos sensores
/// Medicao maxima do sensor de nivel do tanque.
#define MaxTankLevelMeasurement 0.28
/// Medicao maxima do sensor de vazao da bomba - m3/s.
#define MaxPumpFlowMeasurement 6.5E-5

#endif // _TANQUES_PARAM_
