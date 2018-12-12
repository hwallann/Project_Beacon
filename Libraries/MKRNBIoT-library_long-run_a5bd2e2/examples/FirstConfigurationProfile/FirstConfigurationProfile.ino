/*

 NB Operator Profile Selection

 This sketch is needed for first configuration of NB-IoT/CatM network.
 This sketch supports SIMs from following operators:

 * ATT
 * CT
 * Telstra
 * TMO
 * Verizon

 Circuit:
 * MKR NB 1500 board
 * Antenna
 * SIM card

 Created 2 Aug 2018
 by Giampaolo Mancini
*/

#include <MKRNBIoT.h>

NBConfig config(true);

void setup()
{
    Serial.begin(9600);

    while(!Serial){
    }

    Serial.println("NB-IoT / Cat M1 Operator Profile Selector");

    config.begin();

    config.listSupportedProfiles();
    /*
    config.listEnabledBands(CAT_NB1);
    config.listEnabledBands(CAT_M1);

    Serial.println("Enabling all NB1 bands.");
    for (int i = 0; i < CAT_NB1_NUM_BANDS; i++) {
        config.enableBand(CAT_NB1, catNB1Bands[i]);
    }
    config.apply();
    config.listEnabledBands(CAT_NB1);

    Serial.println("Enabling NB1 band 20 only.");
    config.selectBand(CAT_NB1, 20);
    config.apply();
    config.listEnabledBands(CAT_NB1);

    Serial.println(config.checkBand(CAT_NB1, 20));
    Serial.println(config.checkBand(CAT_NB1, 21));
    */
    Serial.println(config.getRAT());
    config.tryUnjamming("22210");

}

void loop()
{
    while(true);
}
