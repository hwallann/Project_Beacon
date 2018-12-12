/*
  This file is part of the MKR NB IoT library.
  Copyright (c) 2018 Arduino SA. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "NBConfig.h"

NBConfig::NBConfig(bool trace)
{
    if (trace) {
        MODEM.debug();
    }
}

NB_NetworkStatus_t NBConfig::begin()
{
    MODEM.begin();

    return IDLE;
}

void NBConfig::listSupportedProfiles()
{
    String response;
    MODEM.send("AT+UMNOPROF=?");
    MODEM.waitForResponse(1000, &response);

    response.remove(0, 13);

    Serial.println(response);
}

int NBConfig::getCurrentProfile(String* profile)
{
    String response;
    MODEM.send("AT+UMNOPROF?");
    MODEM.waitForResponse(1000, &response);

    return response.toInt();
}

void NBConfig::setProfile(int profile)
{
    String response;
    MODEM.sendf("AT+UMNOPROF=%d", profile);
    MODEM.waitForResponse(1000, &response);

    MODEM.reset();
}

bool NBConfig::setBand(NB_RATS_t rat, int band, bool set, bool one)
{
    int numBands, _rat;
    const int* bands;
    String tech;

    switch (rat) {
    case CAT_M1:
        numBands = CAT_M1_NUM_BANDS;
        bands = catM1Bands;
        tech = "M1";
        _rat = 0;
        break;
    case CAT_NB1:
        numBands = CAT_NB1_NUM_BANDS;
        bands = catNB1Bands;
        tech = "NB1";
        _rat = 1;
        break;
    }

    uint64_t bandMask = getBandMask(rat);

    bool validBand = false;
    for (int i = 0; i < numBands; i++)
        validBand |= band == bands[i];

    if (validBand) {
        int bitPos = band - 1;
        uint64_t mask = 1ULL << bitPos;

        if (one)
            bandMask = mask;
        else
            // if (set) bandMask |= mask; else bandMask &= ~mask;
            bandMask = (bandMask & ~(1ULL << bitPos)) | ((uint64_t)set << bitPos);

        String bands = uint64ToString(bandMask);
        MODEM.sendf("AT+UBANDMASK=%d,%s", _rat, bands.c_str());
        MODEM.waitForResponse(1000);
    }

    return validBand;
}

bool NBConfig::checkBand(NB_RATS_t rat, int band)
{
    uint64_t bandMask = getBandMask(rat);
    uint64_t mask = 1ULL << (band - 1);
    return ((bandMask & mask) == mask);
}

void NBConfig::listEnabledBands(NB_RATS_t rat)
{
    int numBands;
    const int* bands;
    String tech;

    switch (rat) {
    case CAT_M1:
        numBands = CAT_M1_NUM_BANDS;
        bands = catM1Bands;
        tech = "M1";
        break;
    case CAT_NB1:
        numBands = CAT_NB1_NUM_BANDS;
        bands = catNB1Bands;
        tech = "NB1";
        break;
    default:
        return;
    }

    bool enabledBands[numBands];
    uint64_t bandMask = getBandMask(rat);

    for (int i = 0; i < numBands; i++)
        enabledBands[i] = bandMask & 1ULL << (bands[i] - 1);

    Serial.print(tech + " - Enabled Bands: ");
    printBands(enabledBands, bands, numBands, true);
    Serial.print(tech + " - Disabled Bands: ");
    printBands(enabledBands, bands, numBands, false);
}

uint64_t NBConfig::getBandMask(NB_RATS_t rat)
{
    String response;
    int numCommas = 3, commaIndex = 0, lastCommaIndex = 0;
    int commas[] = { 0, 0, 0 };

    MODEM.send("AT+UBANDMASK?");
    MODEM.waitForResponse(1000, &response);

    String parsing = response;

    for (int i = 0; i < numCommas; i++) {
        commaIndex = parsing.indexOf(",");
        commas[i] = commaIndex + lastCommaIndex + 1;
        lastCommaIndex = commas[i];
        parsing = parsing.substring(commaIndex + 1);
    }

    String bandParse;
    uint64_t bands;

    switch (rat) {
    case CAT_M1:
        bandParse = response.substring(commas[0], commas[1]);
        bands = strtoull(bandParse.c_str(), NULL, 10);
        break;
    case CAT_NB1:
        bandParse = response.substring(commas[2]);
        bands = strtoull(bandParse.c_str(), NULL, 10);
        break;
    default:
        bands = 0;
        break;
    }
    return bands;
}

bool NBConfig::enableRAT(NB_RATS_t rats)
{
    String response;

    NB_RATS_t enabledRats = getRAT();
    String ratString;

    if (enabledRats == rats || rats == CAT_NONE || enabledRats == CAT_NONE) {
        return false;
    } else {
        switch (rats) {
        case CAT_BOTH:
            ratString = "7,8";
            break;
        case CAT_M1:
            ratString = "7";
            break;
        case CAT_NB1:
            ratString = "8";
            break;
        }
        MODEM.sendf("AT+URAT=%s", ratString.c_str());
        MODEM.waitForResponse(1000, &response);
        return true;
    }
}

bool NBConfig::disableRAT(NB_RATS_t rats)
{
    String response;

    NB_RATS_t enabledRats = getRAT();
    String ratString;

    if (enabledRats != CAT_BOTH) {
        return false;
    } else {
        switch (rats) {
        case CAT_M1:
            ratString = "8";
            break;
        case CAT_NB1:
            ratString = "7";
            break;
        }
        MODEM.sendf("AT+URAT=%s", ratString.c_str());
        MODEM.waitForResponse(1000, &response);
        return true;
    }
}

NB_RATS_t NBConfig::getRAT()
{

    String response;
    MODEM.sendf("AT+URAT?");
    MODEM.waitForResponse(1000, &response);

    int rat = 0;

    int colonIndex = response.indexOf(":") + 2;

    if (colonIndex > 0) {
        String parsing = response.substring(colonIndex);
        int commaIndex = parsing.indexOf(",");

        if (commaIndex > 0)
            rat = 7 + 8;
        else
            rat = parsing.substring(0, commaIndex).toInt();
    }

    switch (rat) {
    case 15:
        return CAT_BOTH;
    case 7:
        return CAT_M1;
    case 8:
        return CAT_NB1;
    default:
        return CAT_NONE;
    }
}

void NBConfig::printBands(const bool* enabled, const int* bands, const int num, bool status)
{
    bool notFirst = false;
    for (int i = 0; i < num; i++) {
        if (enabled[i] == status) {
            if (notFirst)
                Serial.print(", ");
            Serial.print(bands[i]);
            notFirst = true;
        }
    }
    Serial.println();
}

bool NBConfig::apply()
{
    MODEM.send("AT+CFUN=15");
    MODEM.waitForResponse(5000);
    delay(5000);

    do {
        delay(1000);
        MODEM.noop();
    } while (MODEM.waitForResponse(1000) != 1);

    return true;
}

String uint64ToString(uint64_t bn)
{
    String bigNum = "";
    const uint32_t limit = 1000000LU;
    char buffer[100];
    if (bn > limit) {
        sprintf(buffer, "%lu", bn / limit);
        bigNum = buffer;
        sprintf(buffer, "%06lu", bn % limit);
    } else {
        sprintf(buffer, "%lu", bn % limit);
    }
    bigNum += buffer;
    return (bigNum);
}

bool NBConfig::tryUnjamming(char* plmn)
{
    String response;
    // MODEM.setResponseDataStorage(&response);

    MODEM.send("AT+CEREG=2");
    MODEM.waitForResponse(10000, &response);

    // We wait for module to autoregister.
    // If it doesn't we will force a manual registration.
    MODEM.send("AT+CFUN=0");
    MODEM.waitForResponse(10000, &response);
    MODEM.send("AT+CFUN=1");
    MODEM.waitForResponse(1000, &response);

    unsigned int timeout = 10000;
    unsigned int now = millis();
    bool copsStatus = false;

    do {
        MODEM.send("AT+COPS?");
        MODEM.waitForResponse(1000, &response);
        response.trim();
        copsStatus = response.length() == 8;
        delay(1000);
    } while (copsStatus && millis() - now < timeout);

    MODEM.send("AT+CEREG?");
    MODEM.waitForResponse(2000, &response);
    response.trim();
    bool ceregStatus = response.length() == 11;

    if (strlen(plmn) > 0) {
        if (ceregStatus && copsStatus) {
            Serial.println("Modem seems jammed. Trying to unjam...");

            MODEM.send("AT+URAT=7,8");
            MODEM.waitForResponse(1000);
            MODEM.send("AT+CFUN=15");
            MODEM.waitForResponse(5000);
            delay(5000);

            // MODEM.send("AT+UBANDMASK=0,275063445663");
            // MODEM.waitForResponse(1000);
            // MODEM.send("AT+CFUN=15");
            // MODEM.waitForResponse(5000);

            MODEM.send("AT+UBANDMASK=1,185538719");
            MODEM.waitForResponse(1000);
            MODEM.send("AT+CFUN=15");
            MODEM.waitForResponse(5000);
            delay(5000);

            MODEM.send("AT+CGDCONT=1,\"IP\",\"\"");
            MODEM.waitForResponse(1000);

            MODEM.sendf("AT+COPS=1,2,\"%s\"", plmn);
            // Fail fast.
            // Timeout should be 120s but if we wait for modem response
            // R4 will reset, get stuck and wouldn't register anymore.
            int status = MODEM.waitForResponse(1000, &response);
            if (status != 1) {
                // Ugly hack to unstuck the R4.
                // In case of problematic registrations R4 become unresponsive.
                // We keep querying in order to don't let it go idle/asleep or whatever.
                MODEM.noop();
                MODEM.noop();
                MODEM.noop();
            }
        }
    }

    int status;
    Serial.print("Checking for status: ");
    do {
        MODEM.send("AT+CEREG?");
        MODEM.waitForResponse(1000, &response);

        status = response.charAt(10) - '0';
        Serial.print(status);
        Serial.print(" ");
        delay(1000);
    } while (status != 5 && status != 1 && status != 8);

    Serial.println(" done.");
    return true;
}
