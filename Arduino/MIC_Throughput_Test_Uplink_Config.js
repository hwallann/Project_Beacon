var variables = payload.toString('ascii').split(",");
var deviceIdInt = parseFloat(variables[0]);
var deviceTimeULong = parseFloat(variables[1]);
var currentRoundULong = parseFloat(variables[2]);
var testSpecificationFloat = parseFloat(variables[3]);

return {
   payload: payload.toString('utf-8'),
   deviceId: deviceIdInt,
   deviceTime: deviceTimeULong,
   currentRound: currentRoundULong,
   testSpecification: testSpecificationFloat,

   timestamp: + new Date()
};