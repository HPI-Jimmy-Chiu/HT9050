# dialog host完整原文（1／3）

[上層](../index.md)／[manifest](../source-manifest.json)。固定pin `06c430c157a3e39c14d33a5b29219925b2879854`，來源 `web/page/ht9045_dialog_host.js`。
整檔原文380行含所有metadata、註解與IIFE／anonymous callbacks；本頁payload 20行，context credit0。



```javascript
<!-- preserved-content:start -->
    } else if (m.type === 'HT_DIALOG_AUTH_CANCEL' && m.kind === 'auth') {
      sendAuthCancel();
    }
  });
  global.addEventListener('ht-dialog-auth', function (ev) {
    var d = ev && ev.detail;
    if (d && d.state === 'prompt') authPrompt = { authId: d.authId, action: lastAlarmAction, submitted: false };
  });

  global.HTDialogHost = {
    verifyAuth: verifyAuth,   // AI(W906-D026) 20261001 St01：上面那段
    submitResponse: submitResponse,
    /* 給測試與除錯用：看目前掛著哪一個 query、手動注入一個 */
    pending: function () { return pendingQuery; },
    _inject: function (q) { pendingQuery = q; if (q) delete answered[q.qid]; }
  };

  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', watch);
  else watch();
})(window);

<!-- preserved-content:end -->
```
