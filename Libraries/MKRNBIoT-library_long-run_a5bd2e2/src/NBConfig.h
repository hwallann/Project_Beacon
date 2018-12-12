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

#ifndef _NB_CONFIG_H_INCLUDED
#define _NB_CONFIG_H_INCLUDED

#include "Modem.h"
#include "NB.h"

/*
  Bands 1, 2, 3, 4, 5, 8, 12, 13, 17, 18, 19, 20, 25, 26, 28 (and band 39 in M1-only)

  See Paragraph 1.1
  https://www.u-blox.com/sites/default/files/SARA-R4_SysIntegrManual_%28UBX-16029218%29.pdf

*/

static const int CAT_NB1_NUM_BANDS = 15;
static const int CAT_M1_NUM_BANDS = 16;
const int catNB1Bands[CAT_NB1_NUM_BANDS] = { 1, 2, 3, 4, 5, 8, 12, 13, 17, 18, 19, 20, 25, 26, 28 };
const int catM1Bands[CAT_M1_NUM_BANDS] = { 1, 2, 3, 4, 5, 8, 12, 13, 17, 18, 19, 20, 25, 26, 28, 39 };

enum NB_RATS_t { CAT_M1 = 7, CAT_NB1 = 8, CAT_BOTH = 15, CAT_NONE = 0 };

class NBConfig
{
public:
    NBConfig(bool trace = false);

    NB_NetworkStatus_t begin();

    void listSupportedProfiles();
    int getCurrentProfile(String *profile);
    void setProfile(int profile);

    bool enableBand(NB_RATS_t rat, int band) { return setBand(rat, band, true); };
    bool disableBand(NB_RATS_t rat, int band) { return setBand(rat, band, false); };
    bool selectBand(NB_RATS_t rat, int band) { return setBand(rat, band, false, true); };
    bool checkBand(NB_RATS_t rat, int band);
    void listEnabledBands(NB_RATS_t rat);

    bool enableRAT(NB_RATS_t rat);
    bool disableRAT(NB_RATS_t rat);
    NB_RATS_t getRAT();

    bool tryUnjamming(char *plmn);

    bool apply();


private:

    uint64_t getBandMask(NB_RATS_t rat);
    bool setBand(NB_RATS_t rat, int band, bool set, bool one = false);
    void printBands(const bool *enabled, const int* bands, const int num, bool status);
};

String uint64ToString(uint64_t bn);

#endif
