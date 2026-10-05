


Software Release Note


Handler Type：HT-9xxx Series
Standard RS232 Command










Index
Change List	3
1	COM Port Setting	4
2	Wiring	4
3	Communication Protocol Format	4
4	Command List	5
5	Command and Response Define	6
5.1	Number of Sites (CF)	6
5.2	Start of Test (CE)	6
5.2.1	Handler no needs to test.	6
5.2.2	Handler is ready for testing.	7
5.3	Bin Data (BA)	7
5.4	Handler Status (CZ status?)	8
5.5	Sorting Count (CZ testerbin?)	9
5.6	Index Heater Temperature (CZ all masstemp?)	10
5.7	Site Mapping (CZ sitemap?)	11
5.8	Alarm Code (CZ jam?)	13
5.9	Soak Time (CZ soaktime?)	13
5.10	Site Map (CN)	14
5.11	Multi double contact number (CZ doublecontact?)	16
5.12	2DID data of each IC (BARCODE?)	17
6	Exception Error	18
6.1	Test time out and skip.	18
6.2	Closed site has bin	19

Change List
Revision
Date
Engineer

V7.00.550.0
2018/03/14
Steven

V7.00.715.0
2018/03/15
Steven

V12.01.622.0
2019/06/27
Steven

V12.04.657.0
2020/07/09
Steven

V12.06.700.0
2021/09/01
Steven

V12.11.843.0
2024/09/18
Steven


































COM Port Setting
      	
Baud Rate:
9600

Bit Length:
_7

Stop Bit:
_1

Parity:
Even

		* All parameter should be same with the COM port setting of tester.

Wiring
      	
Handler COM

Tester COM

RD
Pin 2

RD
Pin 2

TD
Pin 3

TD
Pin 3

SG
Pin 5

SG
Pin 5

RTS
Pin 7

RTS
Pin 7

CTS
Pin 8

CTS
Pin 8


Communication Protocol Format

STX
Command
ETX


Command List
    
Command
Content
Description

BA
Bin data
Receipt of the test category data

BARCODE?
2DID Data
Reply the 2DID of each IC.

CA
Inhibits
Returns NULL Strings (“no data”). 
* A NULL string will also be returned when there is an undefined command received from the tester.

CB
Index heater temperature
Reply the temperature of index contact to socket.

CD
Machine ID
Reply machine ID.

CE
Start of test
Returns a site number to test. Return the “no data” message (NULL strings) when there are no sites to test.

CF
Number of sites
In response to the request, return the maximum test site setting (1 to 8).

CH
Sleeves full
In response to the request, return the “no data” message (NULL strings).

CI
Input empty
In response to the request, return the “no data” message (NULL strings).

CK
Handler status
This command returns 16 bit integer word, where each bit in the number represents some current internal status information

CN
Site enabled
Get enable/disable site

CZ all masstemp?
Index heater temperature
Reply the temperature of index contact to socket.

CZ doublecontact?
Multi Double contact 
Reply Multi double contact number

CZ id?
Handler model
Reply handler model.

CZ jam?
Alarm code
Reply current alarm code. If there has no alarm, it will reply 0.

CZ jamnumber?



CZ sitemap?
Site mapping
Reply current site mapping.

CZ soaktime?
Soak time
Reply current setting of soak time.

CZ status?
Handler status
This command returns 16 bit integer word, where each bit in the number represents some current internal status information

CZ testerbin?
Sorting count
Reply the count for each bin.

CZ which?
Machine ID
Reply machine ID.

GET2DID?
2DID Data
Reply the 2DID of each IC.


Command and Response Define
Number of Sites (CF)
Tester uses this command to ask Handler about the number of sites to test.


Start of Test (CE)
Tester requests handler to reply the sites number for testing.
Handler no needs to test.





Handler is ready for testing.
Example: Site 1, 2, 3 and 4 is ready for testing.


Bin Data (BA)
Format: [STX]BA<Test Site#1>,<Device ID#1>,<Bin#1>; <Test Site#2>,<Device ID#2>,<Bin#2>[ETX]
Example: Request to test site 1,2,3,4 and receive bin from tester.


Handler Status (CZ status?)
Format: [STX]integer number[ETX]
The table below shows the meaning of each bit.
Bit
Status
Response=1

0
Handler Reboot
Always 0

1
Handler Output Full
When alarm code as below:
MES1021 : No tray on empty tray.
MES1421 : No tray on color tray.
MES1120 : Auto 1 tray unloader is filled with trays.
MES1220 : Auto 2 tray unloader is filled with trays.
MES1320 : Auto 3 tray unloader is filled with trays.
MES1720 : Fix tray 1 is filled with devices.
MES1820 : Fix tray 2 is filled with devices.
MES1920 : Fix tray 2 is filled with devices.

2
Handler Input Empty
When there has no tray on loader.

3
Contact Cleaning
When handler is running auto clean procedure.

4
Handler Diagnostics
When someone doing the action as below:
    Handler is finding home.
    Doing contact test or auto height.
In teaching form.
    In motor test form.
    Handler is pause and in any form that can set parameters for handler.

5
Index Action Without Testing
When handler is running and not in test procedure. Such as:
    Handler is finding home.
    Index Check.
    Tray Feed.
    Waiting guard band or soak time.

6
Reversed
Always 0

7
Guard Band
Waiting handler heating up to the set value of temperature.
It will always 0 when running on ambient mode.

8
Handler Jam
When handler is alarm and such alarm code set bit 8 to 1.


9
Handler Stop
When the handler is stopped.

10
Handler Soak
When handler is waiting for soak time.
It will always 0 when running on ambient mode.

11
Handler Door Open
When a safe door is opened on the handler.

12
Handler Empty
There are no more devices or trays inside handler.

13
Handler OK
Always 0

14
Unloading Tray
When handler is running tray feed procedure.

15
Loading Trays
When handler is loading tray to loader.

16
Reversed
Always 0




Example: “514” means output full(bit-2) and handler stopped (bit-9).


Sorting Count (CZ testerbin?)
Format: [STX]Bin#1-Count#1,Bin#2-Count#2,Bin#3-Count#3,U-0[ETX]
The reply data provide the count for each bin. And it will not appear while the bin# got 0 devices.
Example: Bin1 got 10 devices and bin2 got 12 devices.




Index Heater Temperature (CZ all masstemp?)
Format: [STX]Site1Temp Site2Temp[ETX]
The reply data provide the temperature for each site. And it will NULL when site is closed. The result for different test mode is shown as the table below.


Test Mode
Command Format
Site Map

Single Site
[STX]Site1Temp[ETX]

1



Dual Site 1x2
[STX]Site1Temp Site2Temp[ETX]

1
2



Tri Site 1x3
[STX]Site1Temp Site2Temp Site3Temp[ETX]

1
2
3



Quad Site 1x4
[STX]Site1Temp Site2Temp Site3Temp Site4Temp[ETX]

1
2
3
4



Dual Site 2x1
[STX]Site1Temp Site2Temp[ETX]

1

2



Quad Site 2x2
[STX]Site1Temp Site2Temp Site3Temp Site4Temp[ETX]

1
2

3
4



6 Site 2x3
[STX]Site1Temp Site2Temp Site3Temp Site4Temp Site5Temp Site6Temp[ETX]

1
2
3

4
5
6



Octal Site 2x4
[STX]Site1Temp Site2Temp Site3Temp Site4Temp Site5Temp Site6Temp Site7Temp Site8Temp[ETX]

1
2
3
4

5
6
7
8



12 Site 2x6
[STX]Site1Temp Site2Temp Site3Temp Site4Temp Site5Temp Site6Temp Site7Temp Site8Temp Site9Temp Site10Temp Site11Temp Site12Temp[ETX]

1
2
3
4
5
6

7
8
9
10
11
12



16 Site 2x8
[STX]Site1Temp Site2Temp Site3Temp Site4Temp Site5Temp Site6Temp Site7Temp Site8Temp Site9Temp Site10Temp Site11Temp Site12Temp Site13Temp Site14Temp Site15Temp Site16Temp[ETX]


1
2
3
4
5
6
7
8

9
10
11
12
13
14
15
16




Site Mapping (CZ sitemap?)
Format: [STX]Site1,Site2[ETX]
The reply data provide the site map for each site. And it will 0 or -1 when site is closed. The result for different test mode is shown as the table below.



Test Mode
Command Format
Site Map

Single Site
[STX]Site1[ETX]

1



Dual Site 1x2
[STX]Site1,Site2[ETX]

1
2



Tri Site 1x3
[STX]Site1,Site2,Site3[ETX]

1
2
3



Quad Site 1x4
[STX]Site1,Site2,Site3,Site4[ETX]

1
2
3
4



Dual Site 2x1
[STX]Site1,Site2[ETX]

1

2



Quad Site 2x2
[STX]Site1,Site2,Site3,Site4[ETX]

1
3

2
4



6 Site 2x3
[STX]Site1,Site2,Site3,Site4,Site5,Site6 [ETX]

1
3
5

2
4
6



Octal Site 2x4
[STX]Site1,Site2,Site3,Site4,Site5,Site6,Site7,Site8[ETX]

1
3
5
7

2
4
6
8



12 Site 2x6
[STX]Site1,Site2,Site3,Site4,Site5,Site6,Site7,Site8,Site9,Site10,Site11,Site12[ETX]

1
3
5
7
9
11

2
4
6
8
10
12



16 Site 2x8
[STX]Site1,Site2,Site3,Site4,Site5,Site6,Site7,Site8,Site9,Site10,Site11,Site12,Site13,Site14,Site15,Site16 [ETX]

1
3
5
7
9
11
13
15

2
4
6
8
10
12
14
16






Alarm Code (CZ jam?)
Format: [STX]JamCode[ETX]
The reply data provide the jam code when jam happened. And it will reply 0 when no jam is happened.


Soak Time (CZ soaktime?)
Format: [STX]SoakTime[ETX]
The reply data provide the setting of soak time. And it will reply NONE when temperature mode is ambient.



Site Map (CN)
Format: “[xx,yy,z][x2,yy2,z2]…” 
Where: 
xx = site number 
yy = channel number; "--" if (no channel number set) 
zz = 00 (disabled) ; 01 (enabled) 


Handler Site Number: 
a.1x4
1
2
3
4




b. 2x4 

a
b
c
d

A
1
3
5
7

B
2
4
6
8


c. 2x8

a
b
c
d
e
f
g
h

A
1
3
5
7
9
11
13
15

B
2
4
6
8
10
12
14
16

Example:
a.

[STX][01,01,01][02,03,00][03,02,01][04,--,01] [ETX]
b.

[STX][01,01,01][02,02,01][03,04,00][04,03,01][05,05,01][06,06,01][07,08,01][08,07,01][ETX]
	
Multi double contact number (CZ doublecontact?)
Format: [STX]number[ETX]
The reply data provide the multi double contact number. 
It will be 0 when multi double contact function is closed. 
Reply 1 when multi double contact is 2, 2 when multi double contact is 3, 3 when multi double contact is 4.
			

2DID data of each IC (BARCODE?)
Format: [STX]BARCODE:Site32_2D,Site31_2D,Site30_2D,…,Site3_2D,Site2_2D,Site1_2D[ETX]
The reply data provide the 2DID of each IC. 
It will be 0 when site or function is closed. 
Reply ERROR for the 2DID read error.

Exception Error
Test time out and skip.
RS232 Program will make a ‘Has CE’ flag become TRUE when handler contact to socket and tester sent an CE signal. 
It means start of test (SOT).
Time goes by, if handler trigger ‘test time out’ alarm and user pressed ‘skip’ button. Handler will set all IC inside socket to error bin and send a ‘Halt test and Skip’ signal to RS232 program. The ‘Has CE’ flag will change to FALSE. Handler might change arm contact to socket.
After test finish, tester might send BA data (EOT) to RS232 program. If the ‘Has CE’ flag is FALSE, RS232 program will send ‘BA without CE Error’ to handler side. Handler will have an alarm and set all IC inside socket to error bin.


Closed site has bin
	For example, if handler only contact 1 IC inside socket, but tester reply 2 bins to RS232 program. RS232 program will trigger an error ‘Closed Site have Bin Error’. Handler will alarm and set all IC inside socket to error bin.





