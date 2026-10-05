// =============================================================================
//  DtmChannelMap.h  --  AI(W906-I03b) 20261005 (Ifor01)
//
//  A machine whose temperature zones ALL sit on Delta DTM controllers (DTME08 / DTMN08, Modbus/TCP), not only the
//  Index heaters.  golden 906 / V912 / Frank 910 have no such machine: their DTM path is Index-only (`iTempCode[]`,
//  cmydef.cpp:111-117, DTM channel i <-> iTempCode[i]) and every other zone goes to the serial controller of
//  HEATER_CTRL_TYPE.  NEW DESIGN -- docs/I03B_HT9050_DTM_CHANNEL_MAP.md (MR !201), option A; Ifor 1005 decided
//  D1-D4 as recommended (FROM_IFOR s3 1005 13:4x).
//
//  SWITCH: Gerneral.ini [TempCtrl] DTM_CHANNEL_MAP=HT9050 (read only, database.cpp next to HEATER_CTRL_TYPE).
//  Missing / empty / unknown name => inactive => every caller behaves exactly as golden (DtmMap_ChannelOf() == -1).
//
//  WHAT USES IT
//   * bthermo.cpp DoThermoReal case 100: a channel the map serves gets its SV computed by golden's serial path
//     (staged Hotplate / Shuttle heating, DUT fixed temperature, ...) and only the transport changes: the SV goes to
//     the DTM panel, the PV comes back from it (converted as golden case 2500, Delta DTK4848).  DoSetSVOfDTME08
//     (the Index-only golden DTM writer) stands down while the map is active.
//   * forms/fDTME08.cpp: the form is built, the channel count is DtmMap_ChannelCount(), the sensor type is per
//     channel (HT9050 station 2 mixes PT100 and K-type), every panel boots with SV 0.
//
//  HT9050 TABLE (HP-9050開發機資料-20260717.xlsx sheet 04; .claude/skills/ht9050-hw/references/temp-dtm-map.md s2,
//  machine readable .claude/skills/ht9050-hw/data/HT9050-TempMap.json -- tests/test_dtm_channel_map.cpp compares the
//  two row by row).  Station 3 (SLK-1..8) is NOT ARMED: the wiring order is not confirmed on the machine (row-major
//  Aa1,Ab1,... vs interleaved Aa1,Ba1,...), so its SV is always written 0 (heater off) and only its PV is read.
//  Arming it is a one-row change here after EastSun reads the wiring labels (W-06 / E-05).
// =============================================================================
#pragma once

enum eDtmMapSensor { edmsPT100=0, edmsKType=1 };

struct TDtmMapRow
{
    int         iStation;       // 1-based, as the station knob / the hardware workbook
    int         iCh;            // 1-based within the station (1..8)
    int         iAddr;          // eTempControll
    int         iSensor;        // eDtmMapSensor
    bool        bArmed;         // false => SV forced 0, PV read only
    const char* szZone;         // the workbook's zone name
};

// Select the table by name ("HT9050", case and surrounding blanks ignored).  "" or an unknown name => inactive.
// Returns true when a table is active afterwards.
bool        DtmMap_Select(const char* szName);
bool        DtmMap_Active();
const char* DtmMap_Name();                      // "" when inactive

// Global DTM channel index (0-based) = (station-1)*8 + (ch-1) -- fDTME08 GetStationAndNumber (golden :266-270).
int         DtmMap_ChannelCount();              // stations x 8 (HT9050: 24); 0 when inactive
int         DtmMap_ChannelOf(int iAddr);        // the channel serving eTempControll iAddr; -1 when none / inactive
int         DtmMap_AddrOf(int iChannel);        // eTempControll on that channel; -1 for an empty channel / inactive
int         DtmMap_SensorOf(int iChannel);      // eDtmMapSensor; edmsPT100 for an empty channel
bool        DtmMap_ArmedCh(int iChannel);       // false for an empty / unconfirmed channel

// The active table's rows (tests, inspection).  0 / NULL when inactive.
int               DtmMap_RowCount();
const TDtmMapRow* DtmMap_Row(int i);
