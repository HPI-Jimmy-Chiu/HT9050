//------------------------------------------------------------------------------
//  BootSummary.cpp  --  AI(W906-BOOTSUM) 20260925
//
//  wb_serve 開機時（InitialHandler 之後）印一行 [BOOT] 摘要，純觀測、不改任何行為。
//  為什麼：20260925 EastSun 在 HT9050 機台上「IO 畫面完全不更新」查了一上午，根因是 Gerneral.ini 的 IO_CARD_TYPE=0
//  ⇒ cinitial.cpp 讀 IO_Table／Mot_Table 的 6 個 gate 全不成立，259 個啟用列綁 0 列 —— 而 console 上沒有任何一行講這件事。
//  這一行把「機種、卡別、兩張表讀到幾列、實際綁上幾個點、1203 軸幾個、模擬或真機」放在同一行；IO_CARD_TYPE 不在 {2,3,4}
//  或 GPIB 型號沒讀到（bHandlerModel=false）時再印一行 WARNING（golden 沒有這兩行）。
//------------------------------------------------------------------------------
#include "vclcompat/vcl_compat.h"   // AnsiString
#include "cprod.h"                  // 也帶進 MachineType.h（SOFT_SIMULTE、MAX_SENSOR_ITEM）
#include "cmydef.h"                 // IO_CARD_TYPE、INDEX_MOTION_CARD、bHandlerModel、MachineTypeChoice
#include "database.h"               // HSys（IOTable／MotTable）、W906_GpibModel
#include "myswitch.h"               // SW[MAX_SWITCH_ITEM]
#include "mysensor.h"               // Sen[MAX_SENSOR_ITEM]

#include <cstdio>
#include <string>

void W906_BootSummary(bool bHavePci1203)
{
    int swNamed = 0, swEnabled = 0, senNamed = 0, senEnabled = 0;
    for (int i = 0; i < MAX_SWITCH_ITEM; i++) {
        if (SW[i].Name != AnsiString("")) swNamed++;
        if (SW[i].Enable) swEnabled++;
    }
    for (int i = 0; i < MAX_SENSOR_ITEM; i++) {
        if (Sen[i].Name != AnsiString("")) senNamed++;
        if (Sen[i].Enable) senEnabled++;
    }
    int ax1203 = 0, ax1203Enabled = 0;
    for (size_t i = 0; i < HSys.MotTable.size(); i++) {
        const TMOTDATA* r = HSys.MotTable[i];
        if (r == 0) continue;
        if (std::string(r->CardModel.c_str()) == "PCI1203") {   // 與 WebMotorAccess.h MotorAccessAxis::Is1203() 同一判準
            ax1203++;
            if (r->iEnable) ax1203Enabled++;
        }
    }
#ifdef SOFT_SIMULTE
    const char* sim = "ON (simulation build)";
#else
    const char* sim = "OFF (machine build)";
#endif
    std::printf("[BOOT] Model=\"%s\" bHandlerModel=%d MachineTypeChoice=%d IO_CARD_TYPE=%d INDEX_MOTION_CARD=%d"
                " IO_Table rows=%d Mot_Table rows=%d SW named/enabled=%d/%d Sen named/enabled=%d/%d"
                " PCI1203 axes=%d (Enable=1: %d) SOFT_SIMULTE=%s HAVE_PCI1203=%s\n",
                W906_GpibModel.c_str(), bHandlerModel ? 1 : 0, (int)MachineTypeChoice, IO_CARD_TYPE, INDEX_MOTION_CARD,
                (int)HSys.IOTable.size(), (int)HSys.MotTable.size(), swNamed, swEnabled, senNamed, senEnabled,
                ax1203, ax1203Enabled, sim, bHavePci1203 ? "yes" : "no");
    const bool cardOk = (IO_CARD_TYPE == 2 || IO_CARD_TYPE == 3 || IO_CARD_TYPE == 4);
    if (!cardOk)
        std::printf("[BOOT] WARNING: IO_CARD_TYPE=%d -- cinitial binds IO_Table.csv / Mot_Table.csv rows only for 2 (NewIO_MN200),"
                    " 3 (PCI_P64C64) or 4 (PCI1203_IO); with this value every enabled row stays unbound"
                    " (check system\\Gerneral.ini [System] IO_CARD_TYPE)\n", IO_CARD_TYPE);
    if (!bHandlerModel)
        std::printf("[BOOT] WARNING: bHandlerModel=false -- the GPIB model (D:\\GPIB9045\\system\\general.ini [Version] Model)"
                    " was not read; MachineTypeChoice may not match this machine\n");
}
