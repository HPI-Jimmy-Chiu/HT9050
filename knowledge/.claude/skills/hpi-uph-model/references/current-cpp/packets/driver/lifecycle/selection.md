# 注入、NI選擇與Start返回

以 `GpibEngine::InjectDriver`／`Start`／`Create`／ctor、g_live、g_injected、ownedDriver_定位，完整原文見 [manifest](source-manifest.json)。

| 路徑 | 選讀來源行為 |
| --- | --- |
| 初值 | g_live=0、g_injected=0；ctor的mailbox_／ownedDriver_為0，started_／closed_／up_為false |
| InjectDriver(driver) | 只g_injected.store(driver)；body未呼叫SetGpibDriver或建立Sim |
| Create | 返回new GpibEngine；所有factory登錄端仍待查 |
| Start入口 | mailbox為空或started_為true直接false；g_live由0改1的CAS失敗也false |
| 入口通過 | 保存mailbox、started_=true、closed_=false、depth_=0；讀g_injected |
| 注入指標非0 | d沿用該指標，選讀分支沒有將它存入ownedDriver_ |
| 未注入 | new NiGpibDriver；Loaded為true時d／ownedDriver_指向ni，否則delete ni，d仍0 |
| driver連接 | SetGpibDriver(d)，再ResetBridgeGlobals與後續表單初始化；沒有在此選擇Sim的建置分支 |

Start的try只包VclCreateForm／FormCreate／FormShow等表單段；catch呼叫Teardown再false。driver建立、SetGpibDriver及ResetBridgeGlobals等在這個try之前，不能把這個catch當作全Start的例外保證。表單／Reset內部未在本單元查完。

成功走到結尾時timer slot歸初值，up_設為`!SerialPoll->closeRequested`，然後返回true；所以Start返回true與IsUp()為true仍是兩個判讀。header IsUp只up_.load()。

header註解要求注入driver在engine之前設定且存活較久；這是來源契約，全部注入／建置／機型caller尚待核對。回 [索引](index.md)、[解除](cleanup.md)、[界線](limits.md)。
