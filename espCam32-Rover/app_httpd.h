#pragma once
// Modo del robot
enum RobotMode {
  MODE_REMOTE,
  MODE_AUTO
};

// Estados del movimiento
enum state { fwd, rev, stp, tnl, tnr }; // tnl = turn left, tnr = turn right

// Variables globales (solo declaración)
extern RobotMode currentMode;
extern int speed;
extern int noStop;
extern state actstate;

// Función para iniciar el servidor
void startWebServer();
