#include <Arduino.h>
#include "MobileCountryCodes.h"

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while (!Serial) {}

  listKnownOperators();
}

void loop() {

  String cliAns;
  Serial.println();
  Serial.println("Please, choose your Country:");
  Serial.println();
  for (int i = 0; i < MCCs_NUM; i++) {
    auto &Country = MCCs[i];
    String line;
    if (i < 10) line += " ";
    line += i;
    line += ") ";
    line += Country.Nation;
    line += " - ";
    line += Country.ISO;
    Serial.println(line);
  }
  Serial.println();
  Serial.print("> ");

  Serial.setTimeout(-1);
  while (Serial.available() == 0) { }
  cliAns = Serial.readStringUntil('\r');

  cliAns.trim();
  int nat = cliAns.toInt();
  Serial.println(nat);

  if (nat < 0 || nat > MCCs_NUM) {
    Serial.println();
    Serial.println("Invalid Selection.");
    return;
  }

  auto &Country = MCCs[nat];

  String line = "Selected Nation: ";
  line += Country.Nation;
  line += " [";
  line += Country.ISO;
  line += "]";
  line += " - Operators: ";
  line += Country.OperatorsNum;
  Serial.println();
  Serial.println(line);
  Serial.println("\nPlease, select operator: ");
  Serial.println();
  for (int j = 0; j < Country.OperatorsNum; j++) {
    auto &Op = Country.Operators[j];

    String PLMN = String(Country.MCC) + String(Op.MNC);
    String oper;
    if (j < 10) oper += " ";
    oper += j;
    oper += ") ";
    oper += Op.Brand;
    oper += " [" + PLMN + "]";
    Serial.println(oper);
  }
  Serial.println();

  Serial.print("> ");
  Serial.setTimeout(-1);
  while (Serial.available() == 0) { }
  cliAns = Serial.readStringUntil('\r');

  cliAns.trim();
  int op = cliAns.toInt();
  Serial.println(op);

  if (op < 0 || op >= Country.OperatorsNum) {
    Serial.println();
    Serial.println("Invalid Selection.");
    return;
  }

  Serial.println(Country.Operators[op].Brand);
}

void listKnownOperators() {
  Serial.println("List on known NB-Iot / LTE-M Operators.");
  Serial.println();
  for (int i = 0; i < MCCs_NUM; i++) {
    String cnLine = "Country: ";
    cnLine += MCCs[i].Nation;
    cnLine += " (MCC:";
    cnLine += MCCs[i].MCC;
    cnLine += ")";
    Serial.println(cnLine);

    for (int j = 0; j < MCCs[i].OperatorsNum; j++) {
      String opLine = "  ";
      opLine += j;
      opLine += ") ";
      opLine += MCCs[i].Operators[j].Brand;
      opLine += " - MNC: ";
      opLine += MCCs[i].Operators[j].MNC;
      Serial.println(opLine);
    }
    Serial.println();
  }
}
