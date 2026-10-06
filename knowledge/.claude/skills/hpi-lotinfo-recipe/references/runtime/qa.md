# 目前main的QA計數與還原

唯讀基準main `f57d93f15`，未執行QA、CleanOut或動態測試。

HT9011UC_Cpp_V3.33.906.0/ainarm2.cpp::Check_QA_ModeCount先查IniConfig.bQAMode與LastSet.iRunStartMode==rsmQAMode。iQAModeLoaderCT>=Prod.iQAModeCount且兩個cleanout flags未設時進OneCycle；後續>Count且Quick=true／Finish=false時仍呼叫ModifyTester(OFF_LINE)，設Finish／清Quick，並按iCleanOut及CC_KYEC_LEE判InitCleanOutFunction。接近閾值的區域用InArmSuck.iMaxRow*iMaxCol*2與20較大者，不能把所有機型固定成Count-20。

RunStartMode.cpp::SetRunStartMode在bQAModeFinishCleanOut、原模式rsmQAMode、目的Mode不是QA／Null且iCleanOut==0時，才呼叫ModifyTester(IniConfig.iBackUpTesterMode)，還原InArm／AutoFeed並清兩個flags。這與V904.5沿革一致，保留OFF_LINE在停測階段的角色，不能把V904.4提前還原套回現在。

本次核對上述兩個方法，不等於重驗所有RunType、Bin結果或ART入口。iQAModeBin的0-based index、BinSelect[FT／OffT／ART]及UIcaption各自看 [QA原文](../qa/index.md)與[Bin Display](../../../hpi-bin-display/SKILL.md)。
