TTL Communication with IPC by RS-232   VER.07082201

  HARDWARE

Communication

Interface

Baud Rate

Parity

Data bits

Stop bits

RS-232

115200

N

8

1

TTL Interface

Channel

4 Site(10 bit) / 8 Site(5 bit)

Active logic level

Programmable(start,data,dut,eot)

Isolated type

optocoupler

  SETTING  FRAME  FORMAT

SET or ASK:

SOF

W/R

CMD

DATA

CRC(crc16 mod bus)

EOF

ACK:

SOF

CMD

DATA

CRC

EOF

Field

SOF

W/R

CMD

DATA

CRC

EOF

Description

Length

起始段

W:WRITE

R:READ

選擇功能

參數值

Cyclic redundancy check

結束段

1 bytes

1 bytes

4 bytes

8 bytes

2 bytes

1 bytes

Data

type

ASCII

ASCII

ASCII

ASCII

CRC16

ASCII

Data

@

W(0x57)

R(0x52)

-

-

crc16 mod bus

#

1.TTL 板開機時主動傳送:@Runing’CRC’#

**未接收到 INIT 訊息時，則每秒傳送一次

2.SOF 錯誤時回傳:@ErrSOF’CRC’#

3.CMD 錯誤時回傳:@ErrCMD’CRC’#

4.CRC 錯誤時回傳:@ErrCRC’CRC’#

5.EOF 錯誤時回傳:@ErrEOF’CRC’#

6.流程未完卻收到 SOT 發送要求:@ErrSOT’CRC’#  7.DATA 錯誤時回傳: @ErrDAT’CRC’#

8.未使用 SITE 收到 BIN: @ErrSIT’CRC’#       9. BitBit 模式單 SITE 收到雙 BIN: @ErrBIN’CRC’#

**’CRC’為  crc16 mod bus  計算結果，由 SOF 開始計算到 DATA 結束

開機初始化指令，TTL 板每次上電都需進行初始化指令設定，未經該指令設定或設定內容

錯誤皆無法正常執行 TTL 功能。

SOF

R/W

: @

: W

Command

: INIT

DATA[0]

: TS+5V

(On:0x31 Off :0x30)

DATA[1]

: MODE

(5 BitBit:0x30、10 BitBit:0x31、5 BitBinary:0x32)

DATA [2-9]  : SOT Active Logic (分別對應 site1-8，Low :0x30、High:0x31)

DATA [10-17]: DATA Active Logic (分別對應 site1-8，Low :0x30、High:0x31)

DATA [18-25]: EOT Active Logic (分別對應 site1-8，Low :0x30、High:0x31)

DATA [26-33]: DUT Active Logic (分別對應 site1-8，Low :0x30、High:0x31)

DATA [34-37]: SOT 發送寬度

(unit:ms，上限 1,000ms，example:設定 528ms，DATA 依序為:0x30 0x35 0x32 0x38)

DATA [38-41]: DUT 發送寬度

(unit:ms，上限 1,000ms，example:設定 528ms，DATA 依序為:0x30 0x35 0x32 0x38)

DATA [42-47]: BIN 別等待 time out 上限設定(由第一個收到 BIN 的 SITE 開始計算時間)

(unit:ms，上限 500,000ms，example:設定 0s，無限等待 BIN 別，

DATA 依序為:0x30 0x30 0x30 0x30 0x30 0x30)

CRC [48-49]: CRC16 (MOD BUS，由 SOF 計算到 DATA 結束)

EOF

: #

確認正確後 echo 接收內容以及韌體版本號(@VERS 年月日次序’CRC’#)

  Other Command List:

Command

Description

Length

(byte)

Data

Note

5 Bit Bit Mode :0x30

MODE

Receiver Mode

8

10 Bit Bit Mode :0x31

Byte2~8:

0x00

5 Bit Binary Mode :0x32

Dtat[0-7]:site1-8

SOTL

Start active Logic

8

High:0x31

Low :0x30

Dtat[0-7]:site1-8

DATL

Data active Logic

8

High:0x31

Low :0x30

Dtat[0-7]:site1-8

EOTL

EOT active Logic

8

High:0x31

Low :0x30

DUTL

DUT active Logic

8

Dtat[0-7]:site1-8

SOTT

Start continued Time

DUTT

DUT continued Time

High:0x31

Low :0x30

Ex:330ms

0x33 0x33 0x30

Ex:330ms

0x33 0x33 0x30

Dtat[0-7]:site1-8

8

8

SOTS

Send Start single to tester

8

0x30:wait

DUTS

Send DUT single to tester

8

0x30: wait

0x31:send

Dtat[0-7]:site1-8

RBIN

Return BIN

8

0x02: bin2

0x31:send

0x00:null

0x01: bin1

PWON

Power ON( +TS5V)

OTME

Get bin Over Time

(該值設定為 0 則無限等待)

CSOT

Clear SOT

….

0x0A:bin10

On:0x31

Off :0x30

Ex:5000ms

0x35 0x30 0x30 0x30

byte1~8:

invalid

8

8

8

VERS

FW VERSION

8

07022701(AscII)

Unit : ms

靠右對齊

上限 1000ms

Unit : ms

靠右對齊

上限 1000ms

DATA 全為 0 時

等同清除指令

8 個 byte 分別

代 表 site1~8

的 分 BIN 結

果，收完則回傳

Byte2~8:

0x00

Unit : ms

靠右對齊

上限 500s

無 bin 別 回

傳 ， 但 不 等 待

時，需使用該指

令 清 除 後 才 可

重新發送 SOT

韌體版本別

07:107 年

02:2 月

27:27 號

01:01 版

FCRC

Fail crc

回傳 RSEN:bin……

READ ONLY

8

0x00:null

0x01: bin1

Bin 別 crc 錯誤

時，要求 TTL 板

ENBU

Enable Button

FIRM

Firm Ware upgrade

0x02: bin2

重送 BIN 別

….

0x0A:bin10

0x00:disable

0x01:enable

Byte1~8:

0x00

8

8

Byte2~8:

0x00

通訊格式中的

W/R 段需使用 R

範例舉例:

起測 (已設定 mode:5bitbit,並測試 1-4site):

PC→TTL board : @WSOTS11110000’CRC’#

TTL board→PC: @WSOTS11110000’CRC’#

狀況 A.

1-4 site 皆接收完畢，分別為 bin 1 5 2 3 則回傳:

TTL board→PC: @RBIN”0x01 0x05 0x02 0x03 0x00 0x00 0x00 0x00”CRC#

狀況 B.

僅 site 3 無 bin 別，且達到 TTL Board time out 設定:

TTL board→PC: @RBIN”0x01 0x05 0x00 0x03 0x00 0x00 0x00 0x00”CRC#

狀況 C.

PC 端一直無法等到 BIN 別回傳，可查詢當下已完成 bin 別:

PC→TTL board : @RRBIN00000000’CRC’#

TTL board→PC: @RBIN”0x01 0x05 0x00 0x03 0x00 0x00 0x00 0x00”CRC#

狀況 D.

PC 端一直無法等到 BIN 別回傳，想重發 SOT，需用 CSOT 指令強制結束前一流程:

PC→TTL board : @WCSOT00000000’CRC’#

TTL board→PC: @WCSOT00000000’CRC’#

PC→TTL board : @WSOTS11110000’CRC’#

TTL board→PC: @WSOTS11110000’CRC’#

同狀況 D.

@WSOTS00000000’CRC’# 效果等同 CSOT 指令

PC→TTL board : @WSOTS00000000’CRC’#

TTL board→PC: @WSOTS00000000’CRC’#

PC→TTL board : @WSOTS11110000’CRC’#

TTL board→PC: @WSOTS11110000’CRC’#

狀況 E.Handler 收 bin 驗算 CRC 發現錯誤時發送 FCRC，TTL 板會再一次發送 BIN 別

PC→TTL board : @WFCRC00000000’CRC’#

TTL board→PC: @RSEN11110000’CRC’#

通訊式 TTL 板說明:

TTL 板啟動用 Jump

(一般使用都需短路)

工程用接頭

RS-232
Baud rate: 115200

24Vdc

Sot 手動按鈕

需指令解鎖後使用

SITE 7-8 (5 bits)

SITE 4 (10bits)

強制 5Vdc 供電

測試機 JUMP

SITE 5-8 (5 bits)

SITE 3-4 (10bits)

SITE 3-4 (5 bits)

SITE 2 (10bits)

SITE 1-4 (5 bits)

SITE 1-2 (10bits)

通訊式 TTL 板燈號說明:

Brink:  運行狀態燈(閃爍)

RX:通訊接收燈號

5V、3.3V:

GETSTART:等待 bin 別時恆亮

TX:通訊發送燈號

內部電源燈號

EOT 訊號燈

BIN 訊號燈

作動模式燈號:

Mode1: 5 bit bit

Mode2: 5 bit binary

Mode3: 10 bit bit

Mode4:  保留

DUT 訊號燈

SOT 訊號燈

Bootloader 韌體更新教學:

先開啟超級終端機或 Tera Term 等 console 程式(需具備 Y modem 傳輸協定)，Baud rate 設定

115200。

1.  如何使 TTL 版進入韌體更新模式:

  方法 1.按住 SW8，並重開 TTL 版電源。

  方法 2.正常無生產情況下，通訊協定傳送@RFIRM00000000’CRC’#

2.  終端機顯示畫面: Download image to the internal Flash 字樣
3.  使用 Y MODEM 傳輸，並選擇 new firmware .bin 檔案。

4.  傳輸完成顯示 Start program execution......字樣，並自動執行 TTL 板功能。

5.  完成後請在測試機軟體 RS-232 頁面確認韌體版本號是否已變更。

6.  以上流程有任何異常請重開 TTL 板後重新執行上述動作。

請注意:

1.  誤入韌體更新流程時，只需重開 TTL 電源即可恢復正常使用。

2.  韌體更新過程中請勿拔除 RS-232 及關閉電源。

History:

VERSION.07022701:

1.  增加韌體版本詢問指令，並會在INIT後主動告知。

2.  增加bitbit模式下接收訊號異常通知。

3.  增加未開site接收到BIN的異常訊息通知。

4.  修改over time 定義:某site收到BIN後開始計時，作為各SITE收BIN超時監控，時間長度設定

為0則不使用。

VERSION.07041301:

1.  增加FCRC指令，Handler收bin驗算CRC發現錯誤時發送FCRC，TTL板會再一次發送BIN

別。

VERSION.07082201:

1.  增加ENBU指令，需使用該指令解鎖手動sot按鈕功能，否則該按鈕無作用。

2.  增加RFIRM指令，進入韌體燒錄流程。

3.  增加button 8特殊功能，TTL板開電時長按進入韌體燒錄流程。

4.  增加TTL板外觀與燈號說明。

5.  增加TTL板Bootloader更新韌體教學。

