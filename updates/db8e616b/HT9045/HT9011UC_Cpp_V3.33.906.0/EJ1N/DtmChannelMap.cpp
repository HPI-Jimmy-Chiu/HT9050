// =============================================================================
//  DtmChannelMap.cpp  --  AI(W906-I03b) 20261005 (Ifor01).  See DtmChannelMap.h (new design, no golden source).
// =============================================================================
#include "EJ1N/DtmChannelMap.h"
#include "MachineType.h"          // eTempControll (identical to V912 MachineType.h:638-655)
#include <cctype>
#include <string>

namespace {

// HT9050 -- temp-dtm-map.md s2 / HT9050-TempMap.json.  Station 2 CH4-8 are empty (not listed).
const TDtmMapRow kHt9050[]=
{
    {1, 1, tcHotPlate1, edmsPT100, true,  "Hotplate 1"},
    {1, 2, tcHotPlate2, edmsPT100, true,  "Hotplate 2"},
    {1, 3, tcShuttle1,  edmsPT100, true,  "In Shuttle 1"},
    {1, 4, tcShuttle2,  edmsPT100, true,  "In Shuttle 2"},
    {1, 5, tcDUT1,      edmsPT100, true,  "DUT 1"},
    {1, 6, tcDUT2,      edmsPT100, true,  "DUT 2"},
    {1, 7, tcDUT3,      edmsPT100, true,  "DUT 3"},
    {1, 8, tcDUT4,      edmsPT100, true,  "DUT 4"},
    {2, 1, tcChamber,   edmsPT100, true,  "Chamber"},
    {2, 2, tcHeatGun1,  edmsKType, true,  "Hot Air 1"},
    {2, 3, tcHeatGun2,  edmsKType, true,  "Hot Air 2"},
    // SLK-1..8: row-major as temp-dtm-map.md s3 derives it (Command.cpp:568 tcAa1+(i*4+j)); NOT ARMED until the wiring
    // labels confirm it (the other candidate is Aa1,Ba1,Ab1,Bb1,Ac1,Bc1,Ad1,Bd1).
    {3, 1, tcAa1,       edmsPT100, false, "SLK-1"},
    {3, 2, tcAb1,       edmsPT100, false, "SLK-2"},
    {3, 3, tcAc1,       edmsPT100, false, "SLK-3"},
    {3, 4, tcAd1,       edmsPT100, false, "SLK-4"},
    {3, 5, tcBa1,       edmsPT100, false, "SLK-5"},
    {3, 6, tcBb1,       edmsPT100, false, "SLK-6"},
    {3, 7, tcBc1,       edmsPT100, false, "SLK-7"},
    {3, 8, tcBd1,       edmsPT100, false, "SLK-8"},
};

const int kChPerStation=8;      // uDTME08Control GetChannelNumberPerStation()

struct TTable { const char* szName; const TDtmMapRow* pRows; int iRows; };
const TTable kTables[]=
{
    {"HT9050", kHt9050, (int)(sizeof(kHt9050)/sizeof(kHt9050[0]))},
};

const TTable* g_pActive=0;
int g_iChannels=0;
int g_iChOfAddr[tcTotalCount];
int g_iRowOfCh[64];             // GetMaxStationNumber() 4 x 8 = 32 is the most a uDTME08Control addresses

void Clear()
{
    g_pActive=0;
    g_iChannels=0;
    for(int i=0; i<tcTotalCount; i++) g_iChOfAddr[i]=-1;
    for(int i=0; i<64; i++) g_iRowOfCh[i]=-1;
}

std::string Norm(const char* s)
{
    std::string r;
    for(const char* p=s; p && *p; ++p)
        if(!std::isspace((unsigned char)*p)) r+=(char)std::toupper((unsigned char)*p);
    return r;
}

const TDtmMapRow* RowOfCh(int iChannel)
{
    if(!g_pActive || iChannel<0 || iChannel>=g_iChannels || iChannel>=64) return 0;
    const int r=g_iRowOfCh[iChannel];
    return (r<0) ? 0 : &g_pActive->pRows[r];
}

} // namespace

bool DtmMap_Select(const char* szName)
{
    Clear();
    const std::string n=Norm(szName);
    if(n.empty()) return false;
    for(const TTable& t : kTables)
    {
        if(n!=t.szName) continue;
        int iStations=0;
        for(int r=0; r<t.iRows; r++)
            if(t.pRows[r].iStation>iStations) iStations=t.pRows[r].iStation;
        g_pActive=&t;
        g_iChannels=iStations*kChPerStation;
        for(int r=0; r<t.iRows; r++)
        {
            const TDtmMapRow& w=t.pRows[r];
            const int ch=(w.iStation-1)*kChPerStation+(w.iCh-1);
            if(ch>=0 && ch<64) g_iRowOfCh[ch]=r;
            if(w.iAddr>=0 && w.iAddr<tcTotalCount) g_iChOfAddr[w.iAddr]=ch;
        }
        return true;
    }
    return false;
}

bool        DtmMap_Active()        { return g_pActive!=0; }
const char* DtmMap_Name()          { return g_pActive ? g_pActive->szName : ""; }
int         DtmMap_ChannelCount()  { return g_iChannels; }

int DtmMap_ChannelOf(int iAddr)
{
    if(!g_pActive || iAddr<0 || iAddr>=tcTotalCount) return -1;
    return g_iChOfAddr[iAddr];
}

int DtmMap_AddrOf(int iChannel)
{
    const TDtmMapRow* w=RowOfCh(iChannel);
    return w ? w->iAddr : -1;
}

int DtmMap_SensorOf(int iChannel)
{
    const TDtmMapRow* w=RowOfCh(iChannel);
    return w ? w->iSensor : (int)edmsPT100;
}

bool DtmMap_ArmedCh(int iChannel)
{
    const TDtmMapRow* w=RowOfCh(iChannel);
    return w ? w->bArmed : false;
}

int DtmMap_RowCount()              { return g_pActive ? g_pActive->iRows : 0; }

const TDtmMapRow* DtmMap_Row(int i)
{
    if(!g_pActive || i<0 || i>=g_pActive->iRows) return 0;
    return &g_pActive->pRows[i];
}

// No static initialiser on purpose: g_pActive is zero-initialised (= inactive) before any code runs, and every
// accessor checks it first, so nothing depends on the order of static construction across TUs.
