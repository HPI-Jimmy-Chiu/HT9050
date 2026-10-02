# 掃描各表單 cpp 是否存取工作檔資料夾（DataPath / IniData\Data / GetLastOpenFN / *.Data）
import os, re
BASE = r"D:\HT9045\HT9011UC_Code_V3.33.910.0_20260716_NN Mode 2D + AutoClean V2"
PAGES = {  # html base → cpp 檔
 'cOffSet':'cOffSet.cpp','cSpeed':'cSpeed.cpp','IoSetView':'iosetview.cpp','cConfiguration':'cConfiguration.cpp',
 'cCounterSel':'cCounterSel.cpp','cCounterClear':'cCounterClear.cpp','cBuilder':'cBuilder.cpp','DIOInterFaceCFG':'DIOInterFaceCFG.cpp',
 'LtcSensor':'LtcSensor.cpp','cTowerLight':'cTowerLight.cpp','OmronEJ1N':r'EJ1N\OmronEJ1N.cpp','QAMode':'QAMode.cpp',
 'BarCode':r'BarCode\BarCode.cpp','MyCCLinkSensor':r'CCLink\MyCCLinkSensor.cpp','uCleaning':r'AutoClean\uCleaning.cpp',
 'cContact':'cContact.cpp','cTesterIF':'cTesterIF.cpp','GroundMan':r'GroundMan\GroundMan.cpp','cLd_ULd':'cLd_ULd.cpp',
 'cSecurity':'cSecurity.cpp','cTrayForm':'cTrayForm.cpp','SCK_ART':r'Automation\SCK_ART.cpp','uYieldMonitoring':'uYieldMonitoring.cpp',
 'cHotPlate':'cHotPlate.cpp','cObserver':'cObserver.cpp','cSetUp':'cSetUp.cpp','SmartDiagnostic':'SmartDiagnostic.cpp',
 'cStartCondition':'cStartCondition.cpp','uTemp_Set':'uTemp_Set.cpp','cBinSel':'cBinSel.cpp','uteach':'uteach.cpp',
 'uMotorTest':'uMotorTest.cpp','uhome':'uhome.cpp','cTrayAssignment':'cTrayAssignment.cpp','HandlerSys':'HandlerSys.cpp',
 'cSortCT':'cSortCT.cpp','cContactCT':'cContactCT.cpp','uLotInfo':'uLotInfo.cpp','cTemperFrom':'cTemperFrom.cpp',
 'cTestCategory':'cTestCategory.cpp','cShowBinSelect':'cShowBinSelect.cpp','uShowMessage':'uShowMessage.cpp',
}
PAT = re.compile(r'DataPath|IniData\\\\Data|GetLastOpenFN|\.Data"|SetupFile', re.I)
for k, cpp in PAGES.items():
    p = os.path.join(BASE, cpp)
    if not os.path.exists(p):
        print(f'{k:18} MISSING {cpp}'); continue
    s = open(p, encoding='cp950', errors='replace').read()
    hits = PAT.findall(s)
    files = sorted(set(re.findall(r'"([\w\-]+\.Data)"', s)))
    print(f'{k:18} hits={len(hits):3} data={files[:6]}')
