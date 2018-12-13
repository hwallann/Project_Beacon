var variables = payload.toString('ascii').split(",");
var deviceIdInt = parseFloat(variables[0]);
var deviceTimeULong = parseFloat(variables[1]);
var currentRoundULong = parseFloat(variables[2]);
var yawFloat = parseFloat(variables[3]);
var pitchFloat = parseFloat(variables[4]);
var rollFloat = parseFloat(variables[5]);
var accelXFloat = parseFloat(variables[6]);
var accelYFloat = parseFloat(variables[7]);
var accelZFloat = parseFloat(variables[8]);
var pressureHPaFloat = parseFloat(variables[9]);
var pressurePSIFloat = parseFloat(variables[10]);
var pressureMH2OFloat = parseFloat(variables[11]);

return {
   payload: payload.toString('utf-8'),
   deviceId: deviceIdInt,
   deviceTime: deviceTimeULong,
   currentRound: currentRoundULong,
   yaw: yawFloat,
   pitch: pitchFloat,
   roll: rollFloat,
   accelX: accelXFloat,
   accelY: accelYFloat,
   accelZ: accelZFloat,
   hPa: pressureHPaFloat,
   PSI: pressurePSIFloat,
   mH2O: pressureMH2OFloat,
   timestamp: + new Date()
};