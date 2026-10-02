# SECS-II 資料字典 (Data Items Reference)

> 來源：hume.com/secs/items.html (SEMI E5)
> 說明：所有 SECS-II 資料項目的格式定義與使用位置

共 **377** 個資料項目

## 資料項目清單

| 名稱 | 格式 | 說明 | 使用於 |
|------|------|------|--------|
| **ABS** | `B:n` | Used by: S2F25 S2F26 any binary string | S2F25 S2F26 any binary string |
| **ACCESSMODE** | `U1:1` | Used by: S3F21 S3F27 load port access mode | S3F21 S3F27 load port access mode |
| **ACDS** | `U2:1` |  | S7F22 after command codes |
| **ACKA** | `TF:1` | Used by: S5F14 S5F15 S5F18 S16F4 S16F6 S16F7 S16F12 S16F16 S | S5F14 S5F15 S5F18 S16F4 S16F6 S16F7 S16F12 S16F16 S16F18 S16F24 S16F26 S16F28 S1 |
| **ACKC10** | `B:1` | Used by: S10F2 S10F4 S10F6 S10F10 acknowledge code | S10F2 S10F4 S10F6 S10F10 acknowledge code |
| **ACKC13** | `B:1` | Used by: S13F2 S13F4 S13F6 S13F8 acknowledge code, 0 ok | S13F2 S13F4 S13F6 S13F8 acknowledge code, 0 ok |
| **ACKC15** | `B:1` | Used by: S15F50 S15F52 acknowledge code, 0 ok | S15F50 S15F52 acknowledge code, 0 ok |
| **ACKC3** | `B:1` | Used by: S3F6 S3F8 S3F10 acknowledge code, 0 ok | S3F6 S3F8 S3F10 acknowledge code, 0 ok |
| **ACKC5** | `B:1` | Used by: S5F2 S5F4 acknowledge code, 0 ok | S5F2 S5F4 acknowledge code, 0 ok |
| **ACKC6** | `B:1` | Used by: S6F2 S6F4 S6F10 S6F12 S6F14 S6F26 acknowledge code, | S6F2 S6F4 S6F10 S6F12 S6F14 S6F26 acknowledge code, 0 ok |
| **ACKC7** | `B:1` | Used by: S7F4 S7F12 S7F14 S7F16 S7F18 S7F24 S7F32 S7F38 S7F4 | S7F4 S7F12 S7F14 S7F16 S7F18 S7F24 S7F32 S7F38 S7F40 S7F42 S7F44 S7 acknowledge  |
| **ACKC7A** | `U4:1` |  | S7F27 process program check code |
| **AGENT** | `A:n` | Used by: S15F11 S15F12 S15F21 S15F22 S15F25 no description,  | S15F11 S15F12 S15F21 S15F22 S15F25 no description, no max length |
| **ALCD** | `B:1` | Used by: S5F1 S5F6 S5F8 alarm code byte, >= 128 alarm is set | S5F1 S5F6 S5F8 alarm code byte, >= 128 alarm is set, bit field use is deprecated |
| **ALED** | `B:1` | Used by: S5F3 enable/disable alarm, 128 means enable, 0 disa | S5F3 enable/disable alarm, 128 means enable, 0 disable |
| **ALID** | `U4:1` |  | S5F1 S5F3 S5F6 S5F8 Alarm type ID |
| **ALIDVECTOR** | `U4:n` |  | S5F5 alarm ID vector |
| **ALTX** | `A:120` | Used by: S5F1 S5F6 S5F8 alarm text, the length limit was rec | S5F1 S5F6 S5F8 alarm text, the length limit was recently raised from 40 |
| **ASSGNID** | `U1:1` | Used by: S20F12 S20F13 S20F15 S20F17 S20F18 S20F20 S20F21 S2 | S20F12 S20F13 S20F15 S20F17 S20F18 S20F20 S20F21 S20F23 S20F27 S20F30 S20F32 Ass |
| **ATTRDATA** | `A:n` |  | S1F20 S3F35 S3F35 S13F13 S13F16 S14F1 S14F2 S14F3 S14F4 S14F9 S14F10 S14F11 S14F |
| **ATTRID** | `A:40` |  | S1F19 S3F35 S3F35 S13F13 S13F16 S14F1 S14F1 S14F2 S14F3 S14F4 S14F8 S14F9 S14F10 |
| **ATTRRELN** | `U1:1` | Used by: S14F1 relationship of a value to an attribute value | S14F1 relationship of a value to an attribute value of an object |
| **AUTOCLEAR_DISABLE** | `U1:1` | Used by: S20F1 Disable automatic clear of recipes on SRO tra | S20F1 Disable automatic clear of recipes on SRO transition to Local (E171) |
| **AUTOCLOSE** | `U2:1` | Used by: S20F1 Interaction timeout for closing operator sess | S20F1 Interaction timeout for closing operator session, 0 is no limit |
| **AUTOPOST_DISABLE** | `U1:1` | Used by: S20F1 Disable automatic posting of recipes to RMS p | S20F1 Disable automatic posting of recipes to RMS preceeding SRO move to Local s |
| **BCDS** | `U2:n` |  | S7F22 before command code vector |
| **BCEQU** | `U1:n` |  | S12F3 S12F4 array of bin code equivalents |
| **BINLT** | `U1:n` |  | S12F7 S12F9 S12F11 S12F14 S12F16 S12F18 array of bin values, text or U1 array |
| **BLKDEF** | `I1:1` | Used by: S7F22 command definition block relationship, standa | S7F22 command definition block relationship, standard incorrectly says type U1 i |
| **BPD** | `B:n` | Used by: S8F2 boot program data, the fantasy of using SECS f | S8F2 boot program data, the fantasy of using SECS for boot programs has been rev |
| **BYTMAX** | `U4:1` |  | S7F22 process program maximum byte length, 0 means no limit |
| **CAACK** | `U1:1` | Used by: S3F18 S3F20 S3F22 S3F24 S3F26 S3F28 S3F30 S3F32 S3F | S3F18 S3F20 S3F22 S3F24 S3F26 S3F28 S3F30 S3F32 S3F34 carrier action acknowledge |
| **CARRIERACTION** | `A:n` | Used by: S3F17 carrier action request | S3F17 carrier action request |
| **CARRIERID** | `A:n` | Used by: S3F17 S16F11 S16F15 carrier ID | S3F17 S16F11 S16F15 carrier ID |
| **CARRIERSPEC** | `A:n` | Used by: S3F29 S3F31 carrier object specifier (OBJSPEC) | S3F29 S3F31 carrier object specifier (OBJSPEC) |
| **CATTRDATA** | `A:n` |  | S3F17 carrier attribute value (any data type) |
| **CATTRID** | `A:n` |  | S3F17 carrier attribute identifier, E87 requires text per E39.1, Sec 6 |
| **CCEACK** | `U1:1` | Used by: S20F8 event completion code | S20F8 event completion code |
| **CCODE** | `A:n` |  | S7F22 S7F23 S7F26 S7F31 process operation command code |
| **CEED** | `TF:1` | Used by: S2F37 S17F5 collection event or trace enablement, t | S2F37 S17F5 collection event or trace enablement, true is enabled |
| **CEID** | `U4:1` |  | S1F23 S1F24 S2F35 S2F37 S2F55 S2F56 S2F58 S6F3 S6F8 S6F9 S6F11 S6F13 S6F15 S6F16 |
| **CEIDSTART** | `U4:1` |  | S17F5 the CEID of a start event |
| **CEIDSTOP** | `U4:1` |  | S17F5 the CEID of a stop event |
| **CENAME** | `A:n` | Used by: S1F24 S2F56 a descriptive name for a Data Collectio | S1F24 S2F56 a descriptive name for a Data Collection Event |
| **CEPACK** | `B:1` | Used by: S2F50 command enhanced parameter acknowledge, may b | S2F50 command enhanced parameter acknowledge, may be a list or nested list struc |
| **CEPVAL** | `A:n` |  | S2F49 an enhanced parameter value, may be a scalar of any type, a list of values |
| **CHKINFO** | `A:n` |  | S20F31 User defined value, any type |
| **CKPNT** | `U4:1` | Used by: S13F3 S13F6 data set checkpoint defined by sender | S13F3 S13F6 data set checkpoint defined by sender |
| **CMDA** | `B:1` | Used by: S2F22 S2F28 command acknowledge code | S2F22 S2F28 command acknowledge code |
| **CMDMAX** | `U1:1` |  | S7F22 maximum number of commands allowed, 0=unlimited |
| **CNAME** | `A:16` | Used by: S7F22 text name for a CCODE | S7F22 text name for a CCODE |
| **COACK** | `U1:1` | Used by: S20F10 service completion code | S20F10 service completion code |
| **COLCT** | `U4:1` |  | S12F1 S12F4 column count in die increments |
| **COLHDR** | `A:20` | Used by: S13F13 S13F15 S13F16 table column name | S13F13 S13F15 S13F16 table column name |
| **COMMACK** | `B:1` | Used by: S1F14 establish communications acknowledgement code | S1F14 establish communications acknowledgement code |
| **COMPARISONOPERATOR** | `U1:1` | Used by: S19F1 choice of comparison operators. Interpreted a | S19F1 choice of comparison operators. Interpreted as "target <op> <const>" where |
| **CONDITION** | `A:n` | Used by: S18F16 sub-system condition info tag | S18F16 sub-system condition info tag |
| **COPYID** | `U1:1` | Used by: S20F12 S20F13 S20F15 S20F17 S20F18 S20F20 S20F21 S2 | S20F12 S20F13 S20F15 S20F17 S20F18 S20F20 S20F21 S20F23 S20F27 S20F30 S20F32 Rec |
| **CPACK** | `B:1` | Used by: S2F42 remote command parameter acknowledge, only re | S2F42 remote command parameter acknowledge, only received if error |
| **CPNAME** | `A:n` |  | S2F41 S2F42 S2F49 S2F50 S4F21 S4F29 S16F5 S16F27 command parameter name |
| **CPVAL** | `A:n` |  | S2F41 S4F21 S4F29 S16F5 S16F27 S18F13 command parameter value, any scalar type |
| **CSAACK** | `U1:1` | Used by: S2F8 equipment acknowledge code | S2F8 equipment acknowledge code |
| **CTLJOBCMD** | `U1:1` | Used by: S16F27 control job command | S16F27 control job command |
| **CTLJOBID** | `A:n` | Used by: S16F27 control job ID, an OBJID | S16F27 control job ID, an OBJID |
| **DATA** | `A:n` |  | S3F30 S3F31 S18F6 S18F7 unformatted data |
| **DATAACK** | `B:1` | Used by: S14F22 Acknowledgement code | S14F22 Acknowledgement code |
| **DATAID** | `U4:1` |  | S2F33 S2F35 S2F39 S2F45 S2F49 S3F15 S3F17 S4F19 S4F25 S6F3 S6F5 S6F7 S6F8 S6F9 < |
| **DATALENGTH** | `U4:1` |  | S2F39 S3F15 S3F29 S3F31 S4F25 S6F5 S13F11 S14F23 S16F1 S18F5 S18F7 S19F19 total  |
| **DATASEG** | `A:n` |  | S3F29 S3F31 S18F5 S18F7 identifies data requested, E87 requires text |
| **DATASRC** | `A:n` | Used by: S17F1 identifies a data source, use length 0 to mea | S17F1 identifies a data source, use length 0 to mean the default |
| **DATLC** | `U1:1` | Used by: S12F19 location of invalid data, offset in bytes in | S12F19 location of invalid data, offset in bytes in the SECS-II message body |
| **DELRSPSTAT** | `U1:1` | Used by: S19F4 Response code for the PDE deletion request, n | S19F4 Response code for the PDE deletion request, non-zero means not deleted |
| **DIRRSPSTAT** | `U1:1` | Used by: S19F2 get dir status response | S19F2 get dir status response |
| **DRACK** | `B:1` | Used by: S2F34 define report acknowledge | S2F34 define report acknowledge |
| **DRRACK** | `U1:1` | Used by: S20F14 service completion code | S20F14 service completion code |
| **DSID** | `A:n` |  | S6F3 S6F8 S6F9 data set ID, akin to a report type |
| **DSNAME** | `A:50` |  | S7F37 S7F39 S7F41 S7F43 S13F1 S13F2 S13F3 S13F4 S15F49 S15F51 the name of a data |
| **DSPER** | `A:6` |  | S2F23 data sample period, hhmmss is always supported, A:8 hhmmsscc may be suppor |
| **DUTMS** | `A:n` | Used by: S12F1 S12F4 die units of measure (per E5 Section 12 | S12F1 S12F4 die units of measure (per E5 Section 12) |
| **DVNAME** | `U4:1` |  | S6F3 S6F8 data value name, generically a VID, therefore GEM requires Un type |
| **DVVAL** | `A:n` |  | S6F3 S6F8 S6F9 data value, any format including list |
| **DVVALNAME** | `A:n` | Used by: S1F22 a descriptive name for a Data Value variable  | S1F22 a descriptive name for a Data Value variable (DVVAL) |
| **EAC** | `B:1` | Used by: S2F16 equipment acknowledge code, 0 ok | S2F16 equipment acknowledge code, 0 ok |
| **ECDEF** | `A:n` |  | S2F30 equipment constant default value |
| **ECID** | `U4:1` |  | S2F13 S2F15 S2F29 S2F30 equipment constant ID, GEM requires U4 |
| **ECMAX** | `A:n` |  | S2F30 equipment constant maximum value, any scalar type |
| **ECMIN** | `A:n` |  | S2F30 equipment constant minimum value, any scalar type |
| **ECNAME** | `A:n` | Used by: S2F30 equipment constant name | S2F30 equipment constant name |
| **ECV** | `A:n` |  | S2F14 S2F15 equipment constant value, any scalar type (constant is a misnomer) |
| **EDID** | `A:80` |  | S9F13 expected data identification, PPID or SPID or PTN |
| **EMID** | `A:16` |  | S3F9 equivalent material ID |
| **EPD** | `B:n` | Used by: S8F4 executive program data, the fantasy of using S | S8F4 executive program data, the fantasy of using SECS for this has been revived |
| **EQID** | `A:256` | Used by: S20F12 S20F13 S20F15 S20F17 S20F18 S20F20 S20F21 S2 | S20F12 S20F13 S20F15 S20F17 S20F18 S20F20 S20F21 S20F23 S20F27 S20F30 S20F32 rec |
| **EQNAME** | `A:80` | Used by: S4F27 factory assigned equipment identifier | S4F27 factory assigned equipment identifier |
| **EQUSERID** | `A:64` | Used by: S20F5 Equipment userID for recipe use authenticatio | S20F5 Equipment userID for recipe use authentication |
| **ERACK** | `B:1` | Used by: S2F38 enable/disable event report acknowledge | S2F38 enable/disable event report acknowledge |
| **ERRCODE** | `U4:1` |  | S1F20 S3F18 S3F20 S3F22 S3F24 S3F26 S3F28 S3F30 S3F32 S3F34 S3F36 S4F20 S4F22 S4 |
| **ERRTEXT** | `A:80` | Used by: S1F20 S3F18 S3F20 S3F22 S3F24 S3F26 S3F28 S3F30 S3F | S1F20 S3F18 S3F20 S3F22 S3F24 S3F26 S3F28 S3F30 S3F32 S3F34 S3F36 S4F20 S4F22 S4 |
| **ERRW7** | `A:n` |  | S7F27 process program error description |
| **EVNTSRC** | `A:n` | Used by: S17F5 S17F9 S17F10 S17F11 S17F12 identifies an even | S17F5 S17F9 S17F10 S17F11 S17F12 identifies an event source, use length 0 to spe |
| **EVNTSRC2** | `A:n` | Used by: S17F5 a second event source EVNTSRC | S17F5 a second event source EVNTSRC |
| **EXID** | `A:20` | Used by: S5F9 S5F11 S5F13 S5F14 S5F15 S5F17 S5F18 exception  | S5F9 S5F11 S5F13 S5F14 S5F15 S5F17 S5F18 exception identifier |
| **EXMESSAGE** | `A:n` | Used by: S5F9 S5F11 exception description | S5F9 S5F11 exception description |
| **EXRECVRA** | `A:40` | Used by: S5F9 S5F13 exception recovery action description | S5F9 S5F13 exception recovery action description |
| **EXTYPE** | `A:5` | Used by: S5F9 S5F11 exception type, "ALARM" or "ERROR" | S5F9 S5F11 exception type, "ALARM" or "ERROR" |
| **FCNID** | `U1:1` | Used by: S2F43 S2F44 S2F60 message type function value | S2F43 S2F44 S2F60 message type function value |
| **FFROT** | `U2:1` | Used by: S12F1 S12F3 film frame location in degrees clockwis | S12F1 S12F3 film frame location in degrees clockwise from bottom |
| **FILDAT** | `B` |  | S13F6 Data Set Data, binary or ascii. Max length is the RECLEN from open. |
| **FNLOC** | `U2:1` | Used by: S12F1 S12F3 S12F4 flat/notch location in degrees cl | S12F1 S12F3 S12F4 flat/notch location in degrees clockwise from bottom |
| **FRMLEN** | `U4:1` |  | S7F34 formatted process program length if available, else 0 |
| **GETRSPSTAT** | `U1:1` | Used by: S19F6 S19F8 Response code for PDE queries, non-zero | S19F6 S19F8 Response code for PDE queries, non-zero indicates failure |
| **GOILACK** | `U1:1` | Used by: S20F4 completion code | S20F4 completion code |
| **GRANT** | `B:1` | Used by: S2F2 S2F40 S3F16 S4F26 S13F12 S14F24 S16F2 S19F20 m | S2F2 S2F40 S3F16 S4F26 S13F12 S14F24 S16F2 S19F20 multiblock grant code |
| **GRANT6** | `B:1` | Used by: S6F6 multblock permission grant | S6F6 multblock permission grant |
| **GRNT1** | `B:1` | Used by: S12F6 grant code | S12F6 grant code |
| **GRXLACK** | `U1:1` | Used by: S20F12 service completion code | S20F12 service completion code |
| **HANDLE** | `U4` |  | S13F3 S13F4 S13F5 S13F6 S13F7 S13F8 logical unit or handle for a data set |
| **HCACK** | `B:1` | Used by: S2F42 S2F50 remote command acknowledge | S2F42 S2F50 remote command acknowledge |
| **HOACK** | `TF:1` | Used by: S4F31 S4F33 handoff success flag | S4F31 S4F33 handoff success flag |
| **HOCANCELACK** | `U1:1` | Used by: S4F37 hand off cancel ack | S4F37 hand off cancel ack |
| **HOCMDNAME** | `A:n` |  | S4F29 handoff command identifier |
| **HOHALTACK** | `U1:1` | Used by: S4F41 hand off halt ack | S4F41 hand off halt ack |
| **IACDS** | `U2:n` |  | S7F22 vector of immediately after command codes |
| **IBCDS** | `U2:n` |  | S7F22 vector of immediately before command codes |
| **IDTYP** | `B:1` | Used by: S12F1 S12F3 S12F4 S12F5 S12F7 S12F9 S12F11 S12F13 S | S12F1 S12F3 S12F4 S12F5 S12F7 S12F9 S12F11 S12F13 S12F14 S12F15 S12F16 S12F17 S1 |
| **INPTN** | `B:1` | Used by: S3F35 input material port number | S3F35 input material port number |
| **ITEMACK** | `B:1` | Used by: S21F2 S21F4 S21F6 S21F8 S21F10 S21F12 S21F12 S21F14 | S21F2 S21F4 S21F6 S21F8 S21F10 S21F12 S21F12 S21F14 S21F16 S21F18 S21F20 item re |
| **ITEMERROR** | `A:1024` | Used by: S21F2 S21F4 S21F6 S21F8 S21F10 S21F12 S21F14 S21F16 | S21F2 S21F4 S21F6 S21F8 S21F10 S21F12 S21F14 S21F16 S21F18 S21F20 error descript |
| **ITEMID** | `A:256` | Used by: S21F1 S21F3 S21F5 S21F6 S21F8 S21F11 S21F12 S21F13  | S21F1 S21F3 S21F5 S21F6 S21F8 S21F11 S21F12 S21F13 S21F15 S21F16 S21F17 item ide |
| **ITEMINDEX** | `U4:1` | Used by: S21F17 1-based index of a component part, 0 means d | S21F17 1-based index of a component part, 0 means done, 0xFFFFFFFF means abort |
| **ITEMLENGTH** | `U4:1` |  | S21F1 S21F3 S21F6 S21F8 S21F13 S21F16 S21F17 sum of item part lengths in bytes,  |
| **ITEMPART** | `A:n` |  | S21F3 S21F6 S21F17 component part of an item, may be data type A:n or B:n |
| **ITEMPARTCOUNT** | `U4:1` | Used by: S21F13 S21F16 S21F17 total number of item parts as  | S21F13 S21F16 S21F17 total number of item parts as split for transfer |
| **ITEMPARTLENGTH** | `U4:1` | Used by: S21F17 length of a specific item part presumably in | S21F17 length of a specific item part presumably in bytes |
| **ITEMTYPE** | `A:n` | Used by: S21F1 S21F3 S21F5 S21F6 S21F7 S21F8 S21F10 S21F11 S | S21F1 S21F3 S21F5 S21F6 S21F7 S21F8 S21F10 S21F11 S21F12 S21F13 S21F15 S21F16 S2 |
| **ITEMTYPESUPPORT** | `U4:1` | Used by: S21F20 bitfield to specify which S21Fx messages acc | S21F20 bitfield to specify which S21Fx messages accepted |
| **ITEMVERSION** | `A:n` | Used by: S21F1 S21F3 S21F6 S21F8 S21F13 S21F16 S21F17 versio | S21F1 S21F3 S21F6 S21F8 S21F13 S21F16 S21F17 version value, empty for unknown, d |
| **JOBACTION** | `A:n` | Used by: S3F35 reticle transfer command | S3F35 reticle transfer command |
| **LENGTH** | `U4:1` | Used by: S2F1 S7F1 S7F29 program length in bytes | S2F1 S7F1 S7F29 program length in bytes |
| **LIMITACK** | `B:1` | Used by: S2F46 variable limit value error code | S2F46 variable limit value error code |
| **LIMITID** | `B:1` | Used by: S2F45 S2F46 S2F48 identifies a specific limit | S2F45 S2F46 S2F48 identifies a specific limit |
| **LIMITMAX** | `F4:1` |  | S2F48 The maximum value allowed for the upper dead band limit |
| **LIMITMIN** | `F4:1` |  | S2F48 The minimum value allowed for the lower dead band limit |
| **LINKID** | `U4:1` | Used by: S6F25 S14F20 S14F21 S15F22 S15F30 correlates the RM | S6F25 S14F20 S14F21 S15F22 S15F30 correlates the RMOPID value in a request to a  |
| **LOC** | `B:1` | Used by: S2F27 S3F2 material location code | S2F27 S3F2 material location code |
| **LOCID** | `A:n` |  | S3F29 S3F31 logical ID of carrier location, E87 requires text |
| **LOWERDB** | `F4:1` |  | S2F45 S2F48 the lower bound of a deadband limit |
| **LRACK** | `B:1` | Used by: S2F36 link report acknowledge | S2F36 link report acknowledge |
| **LVACK** | `B:1` | Used by: S2F46 variable limit error code | S2F46 variable limit error code |
| **MAPER** | `B:1` | Used by: S12F19 map error | S12F19 map error |
| **MAPFT** | `B:1` | Used by: S12F3 S12F5 map data format type | S12F3 S12F5 map data format type |
| **MAXNUMBER** | `U2:1` | Used by: S20F25 subspace maximum | S20F25 subspace maximum |
| **MAXTIME** | `U2:1` | Used by: S20F25 maximum minutes for a PEM recipe to be prese | S20F25 maximum minutes for a PEM recipe to be preserved in PRC post use, 0 means |
| **MCINDEX** | `U4:1` |  | S4F29 S4F31 correlation value for handoff command |
| **MDACK** | `B:1` | Used by: S12F8 S12F10 S12F12 map data ack | S12F8 S12F10 S12F12 map data ack |
| **MDLN** | `A:20` | Used by: S1F2 S1F13 S1F14 S7F22 S7F23 S7F26 S7F31 equipment  | S1F2 S1F13 S1F14 S7F22 S7F23 S7F26 S7F31 equipment model type |
| **MEXP** | `A:6` | Used by: S9F13 message expected in form of SxxFyy | S9F13 message expected in form of SxxFyy |
| **MF** | `B:1` |  | S3F2 S3F4 S3F5 S3F7 S16F3 S16F11 S16F15 material format code, ASCII indicates ge |
| **MHEAD** | `B:10` | Used by: S9F1 S9F3 S9F5 S9F7 S9F11 message header of receive | S9F1 S9F3 S9F5 S9F7 S9F11 message header of received block |
| **MID** | `A:16` |  | S2F27 S3F2 S3F4 S3F7 S3F9 S3F12 S3F13 S4F1 S4F3 S4F5 S4F7 S4F9 S4F11 S4F13 S4F |
| **MIDAC** | `B:1` | Used by: S3F14 material ID ack | S3F14 material ID ack |
| **MIDRA** | `B:1` | Used by: S3F12 material ID Ack code | S3F12 material ID Ack code |
| **MLCL** | `U4:1` |  | S12F4 S12F5 message length in bytes |
| **MMODE** | `B:1` | Used by: S7F15 matrix mode selection | S7F15 matrix mode selection |
| **NACDS** | `U2:n` |  | S7F22 vector of not after command codes |
| **NBCDS** | `U2:n` |  | S7F22 vector of not before command codes |
| **NULBC** | `A:n` |  | S12F1 S12F3 S12F4 null bin code value |
| **OBJACK** | `U1:1` | Used by: S14F2 S14F4 S14F6 S14F8 S14F10 S14F12 S14F14 S14F16 | S14F2 S14F4 S14F6 S14F8 S14F10 S14F12 S14F14 S14F16 S14F18 S14F26 S14F28 acknowl |
| **OBJCMD** | `U1:1` | Used by: S14F15 S14F17 Specifies an action to be performed b | S14F15 S14F17 Specifies an action to be performed by an object |
| **OBJID** | `A:80` |  | S1F19 S14F1 S14F2 S14F3 S14F4 S20F1 S20F3 S20F5 S20F7 S20F9 S20F11 S20F13 S20F15 |
| **OBJSPEC** | `A:n` |  | S2F49 S13F11 S13F13 S13F15 S14F1 S14F3 S14F5 S14F7 S14F9 S14F10 S14F11 S14F13 S1 |
| **OBJTOKEN** | `U4:1` | Used by: S14F14 S14F15 S15F37 S15F39 S15F41 token used for a | S14F14 S14F15 S15F37 S15F39 S15F41 token used for authorization |
| **OBJTYPE** | `A:40` |  | S1F19 S14F1 S14F3 S14F6 S14F7 S14F8 S14F9 S14F25 S14F26 S14F27 S20F1 S20F3 S20F5 |
| **OCEACK** | `U1:1` | Used by: S20F6 event completion code | S20F6 event completion code |
| **OFLACK** | `B:1` | Used by: S1F16 offline acknowledge, 0 ok | S1F16 offline acknowledge, 0 ok |
| **ONLACK** | `B:1` | Used by: S1F18 online acknowledge, 0 ok | S1F18 online acknowledge, 0 ok |
| **OPEID** | `A:16` | Used by: S20F4 S20F5 S20F6 S20F7 S20F8 S20F9 S20F11 S20F12 S | S20F4 S20F5 S20F6 S20F7 S20F8 S20F9 S20F11 S20F12 S20F13 S20F13 S20F15 S20F15 S2 |
| **OPETYPE** | `U1:1` | Used by: S20F3 S20F5 S20F7 S20F9 S20F11 S20F13 S20F15 S20F17 | S20F3 S20F5 S20F7 S20F9 S20F11 S20F13 S20F15 S20F17 S20F19 S20F21 S20F23 S20F27  |
| **OPID** | `U4:1` |  | S6F25 S14F19 S14F21 S15F21 S15F29 S15F30 S15F37 S15F41 S15F44 S15F46 operation i |
| **ORLOC** | `B:1` | Used by: S12F1 S12F3 S12F4 origin location | S12F1 S12F3 S12F4 origin location |
| **OUTPTN** | `B:1` |  | S3F35 output port (PTN) |
| **PARAMNAME** | `A:n` | Used by: S3F23 S3F25 argument name | S3F23 S3F25 argument name |
| **PARAMVAL** | `U1:1` |  | S3F23 S3F25 argument value, only defined use is ServiceStatus, 0 = OUT OF SERVIC |
| **PDEATTRIBUTE** | `U1:1` | Used by: S19F1 S19F2 a reportable PDE attribute type, not ne | S19F1 S19F2 a reportable PDE attribute type, not necessarily useable in a filter |
| **PDEATTRIBUTENAME** | `U1:1` | Used by: S19F1 identifies a PDE attribute type | S19F1 identifies a PDE attribute type |
| **PDEATTRIBUTEVALUE** | `A` |  | S19F1 S19F2 contains the value of a PDE Attribute, may be type L, A, TF, U1 |
| **PDEREF** | `A:36` | Used by: S19F15 S19F16 S19F17 The UID of a PDE or of a PDE g | S19F15 S19F16 S19F17 The UID of a PDE or of a PDE group formatted as a 36 charac |
| **PECEACK** | `U1:1` | Used by: S20F32 event completion code | S20F32 event completion code |
| **PECRSLT** | `U1:1` | Used by: S20F32 RMS result | S20F32 RMS result |
| **PFCD** | `B:1` | Used by: S6F9 predefined form selector | S6F9 predefined form selector |
| **PGRPACTION** | `A:n` | Used by: S3F23 port group command, an alias for PORTACTION? | S3F23 port group command, an alias for PORTACTION? |
| **PODID** | `A:n` | Used by: S3F35 OBJSPEC for a Pod instance | S3F35 OBJSPEC for a Pod instance |
| **PORTACTION** | `A:n` | Used by: S3F25 ChangeServiceStatus, CancelReservationAtPort  | S3F25 ChangeServiceStatus, CancelReservationAtPort or ReserveAtPort |
| **PORTGRPNAME** | `A:n` | Used by: S3F21 S3F23 name of a group of ports | S3F21 S3F23 name of a group of ports |
| **PPARM** | `A:n` |  | S7F23 S7F26 S7F31 process parameter, any scalar or vector |
| **PPBODY** | `B:n` |  | S7F3 S7F6 S7F36 process program data, any non-list type |
| **PPGNT** | `B:1` | Used by: S7F2 S7F30 process program transfer grant status | S7F2 S7F30 process program transfer grant status |
| **PPID** | `A:80` |  | S2F27 S7F1 S7F3 S7F5 S7F6 S7F8 S7F10 S7F11 S7F13 S7F17 S7F20 S7F23 S7F25 S7F26 < |
| **PRAXI** | `B:1` | Used by: S12F1 S12F3 process access | S12F1 S12F3 process access |
| **PRCMDNAME** | `A:6` | Used by: S16F5 process job commands, START, STOP, PAUSE, RES | S16F5 process job commands, START, STOP, PAUSE, RESUME, ABORT, CANCEL |
| **PRCPREEXECHK** | `U1:1` | Used by: S20F25 Enable Pre-Execution checking | S20F25 Enable Pre-Execution checking |
| **PRDCT** | `U4:1` |  | S12F1 S12F4 process die count |
| **PREACK** | `U1:1` | Used by: S20F24 event completion code | S20F24 event completion code |
| **PREVENTID** | `U1:1` |  | S16F9 process job event ID |
| **PRJOBID** | `A:n` | Used by: S16F4 S16F5 S16F6 S16F7 S16F9 S16F11 S16F12 S16F15  | S16F4 S16F5 S16F6 S16F7 S16F9 S16F11 S16F12 S16F15 S16F16 S16F17 S16F18 S16F20 S |
| **PRJOBMILESTONE** | `U1:1` |  | S16F7 process job status |
| **PRJOBSPACE** | `U2:1` | Used by: S16F22 the number of process jobs that can be creat | S16F22 the number of process jobs that can be created |
| **PRMTRLORDER** | `U1:1` | Used by: S16F29 ordering method for pending process jobs | S16F29 ordering method for pending process jobs |
| **PRPAUSEEVENTID** | `U4:1` |  | S16F11 S16F15 an event identifier for which a process job should be paused |
| **PRPROCESSSTART** | `TF:1` | Used by: S16F3 S16F11 S16F15 S16F25 automatic start flag, fa | S16F3 S16F11 S16F15 S16F25 automatic start flag, false implies manual start |
| **PRRECIPEMETHOD** | `U1:1` | Used by: S16F3 S16F11 S16F15 recipe type | S16F3 S16F11 S16F15 recipe type |
| **PRSTATE** | `U1:1` | Used by: S16F20 process job state, E40 definition | S16F20 process job state, E40 definition |
| **PSRACK** | `U1:1` | Used by: S20F28 service completion code | S20F28 service completion code |
| **PSREACK** | `U1:1` | Used by: S20F34 event completion code | S20F34 event completion code |
| **PTN** | `U1:1` |  | S3F11 S3F12 S3F13 S3F17 S3F21 S3F25 S3F27 S3F28 S4F1 S4F3 S4F5 S4F7 S4F9 S4F11 < |
| **QPRKEACK** | `U1:1` | Used by: S20F30 event completion code | S20F30 event completion code |
| **QREACK** | `U1:1` | Used by: S20F22 event completion code | S20F22 event completion code |
| **QRXLEACK** | `U1:1` | Used by: S20F20 event completion code | S20F20 event completion code |
| **QUA** | `B:1` | Used by: S3F2 S3F4 S3F5 S3F7 quantity (format limits max to  | S3F2 S3F4 S3F5 S3F7 quantity (format limits max to 255!) |
| **RAC** | `U1:1` |  | S2F20 reset acknowledge |
| **RCMD** | `A:n` |  | S2F21 S2F41 S2F49 remote command, GEM requires a maximum length of 20 printable  |
| **RCPATTRDATA** | `A:n` |  | S6F25 S15F13 S15F15 S15F15 S15F18 S15F18 S15F27 S15F28 S15F30 S15F32 the value o |
| **RCPATTRID** | `A:n` | Used by: S6F25 S15F13 S15F15 S15F15 S15F18 S15F18 S15F27 S15 | S6F25 S15F13 S15F15 S15F15 S15F18 S15F18 S15F27 S15F28 S15F30 S15F32 the name of |
| **RCPBODY** | `B:n` |  | S15F13 S15F15 S15F18 S15F27 S15F32 Recipe body |
| **RCPBODYA** | `A:n` |  | S20F15 S20F18 S20F23 S20F32 user defined recipe body, list allowed |
| **RCPCLASS** | `A:n` | Used by: S15F11 Recipe class | S15F11 Recipe class |
| **RCPCMD** | `U1:1` | Used by: S15F21 S15F22 recipe action | S15F21 S15F22 recipe action |
| **RCPDEL** | `U1:1` | Used by: S15F35 recipe action | S15F35 recipe action |
| **RCPDESCLTH** | `U4:1` |  | S15F24 the byte length of a recipe |
| **RCPDESCNM** | `A:n` | Used by: S15F24 Identifies a descriptor type "ASDesc", "Body | S15F24 Identifies a descriptor type "ASDesc", "BodyDesc", "GenDesc" |
| **RCPDESCTIME** | `A:16` | Used by: S15F24 timestamp of a recipe section "YYYYMMDDhhmms | S15F24 timestamp of a recipe section "YYYYMMDDhhmmsscc" |
| **RCPID** | `A:n` | Used by: S15F21 S15F23 S15F28 S15F29 S15F30 S15F33 S15F35 S1 | S15F21 S15F23 S15F28 S15F29 S15F30 S15F33 S15F35 S15F37 S15F41 S15F44 S15F53 rec |
| **RCPNAME** | `A:n` | Used by: S15F11 recipe name | S15F11 recipe name |
| **RCPNEWID** | `A:n` | Used by: S15F19 S15F41 S15F44 S15F45 the new recipe identifi | S15F19 S15F41 S15F44 S15F45 the new recipe identifier |
| **RCPOWCODE** | `TF:1` | Used by: S15F27 S15F49 recipe overwrite code, true=overwrite | S15F27 S15F49 recipe overwrite code, true=overwrite ok, false=do not overwrite |
| **RCPPARNM** | `A:256` | Used by: S15F25 S15F33 S16F3 S16F11 S16F15 S16F23 the name o | S15F25 S15F33 S16F3 S16F11 S16F15 S16F23 the name of a recipe variable parameter |
| **RCPPARRULE** | `A:80` | Used by: S15F25 the restrictions applied to a recipe variabl | S15F25 the restrictions applied to a recipe variable parameter setting |
| **RCPPARVAL** | `A:80` |  | S15F25 S15F33 S16F3 S16F11 S16F15 S16F23 the value of a recipe variable paramete |
| **RCPRENAME** | `TF:1` | Used by: S15F19 whether a recipe is to be renamed (TRUE) or  | S15F19 whether a recipe is to be renamed (TRUE) or copied (FALSE) |
| **RCPSECCODE** | `B:1` | Used by: S15F15 S15F16 S15F17 indicates the sections of a re | S15F15 S15F16 S15F17 indicates the sections of a recipe |
| **RCPSECNM** | `A:n` | Used by: S15F15 S15F15 S15F18 S15F18 Recipe section name, "G | S15F15 S15F15 S15F18 S15F18 Recipe section name, "Generic", "Body", "ASDS" |
| **RCPSPEC** | `A:n` | Used by: S6F25 S15F1 S15F9 S15F13 S15F15 S15F17 S15F19 S15F2 | S6F25 S15F1 S15F9 S15F13 S15F15 S15F17 S15F19 S15F27 S15F31 S15F32 S15F45 S15F53 |
| **RCPSTAT** | `U1:1` | Used by: S15F10 Recipe status code | S15F10 Recipe status code |
| **RCPUPDT** | `TF:1` | Used by: S15F13 true for a recipe update, false for create | S15F13 true for a recipe update, false for create |
| **RCPVERS** | `A:n` | Used by: S15F10 S15F12 recipe version | S15F10 S15F12 recipe version |
| **READLN** | `U4:1` |  | S13F5 maximum number of bytes or characters to read |
| **REAPER** | `A:80` | Used by: S4F27 transfer hand-off reader PER configuration | S4F27 transfer hand-off reader PER configuration |
| **RECLEN** | `U4:1` |  | S13F4 maximum number of bytes or characters in a discrete record |
| **REFP** | `I4:2` |  | S12F1 S12F4 x y reference point |
| **REPGSZ** | `U4:1` |  | S2F23 S17F5 reporting group size, TOTSMP modulo REPGSZ should be 0 |
| **RESOLUTION** | `A:36` | Used by: S19F15 S19F16 S19F17 the UID of a PDE | S19F15 S19F16 S19F17 the UID of a PDE |
| **RESPDESTAT** | `U1:1` | Used by: S19F16 status codes for PDE resolution | S19F16 status codes for PDE resolution |
| **RESPEC** | `A:n` | Used by: S15F29 S15F33 S15F35 object specifier for the recip | S15F29 S15F33 S15F35 object specifier for the recipe executor |
| **RETAINRECIPE_DISABLE** | `U1:1` | Used by: S20F1 Disable automatic retention of recipes on dis | S20F1 Disable automatic retention of recipes on disconnect |
| **RETICLEID** | `A:n` | Used by: S3F35 OBJSPEC value for a reticle | S3F35 OBJSPEC value for a reticle |
| **RETICLEID2** | `A:n` | Used by: S3F35 OBJSPEC value for a second reticle | S3F35 OBJSPEC value for a second reticle |
| **RETPLACEINSTR** | `U1:1` | Used by: S3F35 pod slot reticle place instruction | S3F35 pod slot reticle place instruction |
| **RETREMOVEINSTR** | `U1:1` | Used by: S3F35 pod slot reticle remove instruction | S3F35 pod slot reticle remove instruction |
| **REVID** | `A:256` | Used by: S20F12 S20F13 S20F15 S20F17 S20F18 S20F20 S20F21 S2 | S20F12 S20F13 S20F15 S20F17 S20F18 S20F20 S20F21 S20F23 S20F27 S20F30 S20F32 rec |
| **RIC** | `U1:1` |  | S2F19 reset code, 1 means power up reset |
| **RMACK** | `U1:1` | Used by: S6F25 S15F4 S15F6 S15F8 S15F10 S15F12 S15F14 S15F16 | S6F25 S15F4 S15F6 S15F8 S15F10 S15F12 S15F14 S15F16 S15F18 S15F20 S15F22 S15F24  |
| **RMCHGSTAT** | `U4:1` |  | S6F25 object change type |
| **RMCHGTYPE** | `U4:1` |  | S15F37 S15F41 S15F44 S15F45 S15F46 type of change for a recipe 0 - no change 1 - |
| **RMDATASIZE** | `U4:1` |  | S15F1 the maximum total message body length of a SECS-II message |
| **RMGRNT** | `B:1` | Used by: S15F2 S15F37 S15F46 grant code, 0 ok | S15F2 S15F37 S15F46 grant code, 0 ok |
| **RMNEWNS** | `A:n` | Used by: S15F5 new name for a recipe namespace | S15F5 new name for a recipe namespace |
| **RMNSCMD** | `U1:1` | Used by: S15F3 S15F39 S15F41 recipe namespace command | S15F3 S15F39 S15F41 recipe namespace command |
| **RMNSSPEC** | `A:n` | Used by: S15F3 S15F5 S15F11 S15F21 S15F25 S15F47 object id o | S15F3 S15F5 S15F11 S15F21 S15F25 S15F47 object id of a recipe namespace |
| **RMRECSPEC** | `A:n` | Used by: S15F39 S15F41 S15F47 object id of a distributed rec | S15F39 S15F41 S15F47 object id of a distributed recipe namespace recorder |
| **RMREQUESTOR** | `TF:1` | Used by: S15F41 S15F44 True when the initiator of a change r | S15F41 S15F44 True when the initiator of a change request is an attached segment |
| **RMSEGSPEC** | `A:n` | Used by: S15F37 S15F39 S15F41 S15F44 S15F47 The object ID of | S15F37 S15F39 S15F41 S15F44 S15F47 The object ID of a distributed recipe namespa |
| **RMSPACE** | `U4:1` |  | S15F8 the amount of storage available in bytes for at least one recipe |
| **RMSPWD** | `A:64` | Used by: S20F5 password of SRO user | S20F5 password of SRO user |
| **RMSUSERID** | `A:64` | Used by: S20F5 SRO userID | S20F5 SRO userID |
| **ROWCT** | `U4:1` |  | S12F1 S12F4 row count in die increments |
| **RPMACK** | `U1:1` | Used by: S3F36 reticle pod management ack code | S3F36 reticle pod management ack code |
| **RPSEL** | `U1:1` | Used by: S12F1 S12F4 reference point select | S12F1 S12F4 reference point select |
| **RPTID** | `U4:1` |  | S2F33 S2F35 S2F52 S2F53 S2F54 S2F56 S6F11 S6F13 S6F16 S6F18 S6F19 S6F21 S6F27 S6 |
| **RPTOC** | `TF:1` | Used by: S17F5 send only changed data trace report flag | S17F5 send only changed data trace report flag |
| **RQCMD** | `TF:1` | Used by: S7F22 flag that command is required | S7F22 flag that command is required |
| **RRACK** | `B:1` | Used by: S4F18 request to receive acknowledge | S4F18 request to receive acknowledge |
| **RRACK_S20** | `U1:1` | Used by: S20F18 service completion code | S20F18 service completion code |
| **RSACK** | `B:1` | Used by: S4F2 ready to send acknowledge | S4F2 ready to send acknowledge |
| **RSDA** | `B:1` | Used by: S6F24 spool request reply | S6F24 spool request reply |
| **RSDC** | `U1:1` | Used by: S6F23 spool request code | S6F23 spool request code |
| **RSINF** | `I4:3` |  | S12F7 S12F14 starting location for row or column, x,y,direction triplet |
| **RSPACK** | `B:1` | Used by: S2F44 spooling response | S2F44 spooling response |
| **RTSRSPSTAT** | `U1:1` | Used by: S19F10 PDE transfer request reply code, non-zero me | S19F10 PDE transfer request reply code, non-zero means denied |
| **RTYPE** | `U1:1` |  | S13F4 type of data record |
| **RecID** | `A:n` | Used by: S20F12 S20F13 S20F15 S20F17 S20F18 S20F20 S20F21 S2 | S20F12 S20F13 S20F15 S20F17 S20F18 S20F20 S20F21 S20F23 S20F27 S20F30 S20F32 rec |
| **SDACK** | `B:1` | Used by: S12F2 setup data ack, 0 ok | S12F2 setup data ack, 0 ok |
| **SDBIN** | `B:1` | Used by: S12F17 send bin data flag, 0=send, else do not | S12F17 send bin data flag, 0=send, else do not |
| **SENDRSPSTAT** | `U1:1` | Used by: S19F13 Return codes for the Send PDE request, non-z | S19F13 Return codes for the Send PDE request, non-zero means failure |
| **SEQNUM** | `U4:1` |  | S7F27 process program command number |
| **SFCD** | `B:1` | Used by: S1F5 S1F7 status form code | S1F5 S1F7 status form code |
| **SHEAD** | `B:10` | Used by: S9F9 message header of sent block | S9F9 message header of sent block |
| **SLOTID** | `U1:1` | Used by: S16F11 S16F15 slot position within a carrier | S16F11 S16F15 slot position within a carrier |
| **SMPLN** | `U4:1` |  | S6F1 sample number |
| **SOFTREV** | `A:20` | Used by: S1F2 S1F13 S1F14 S7F22 S7F23 S7F26 S7F31 software r | S1F2 S1F13 S1F14 S7F22 S7F23 S7F26 S7F31 software revision |
| **SPAACK** | `U1:1` | Used by: S2F4 S20F26 service completion code | S2F4 S20F26 service completion code |
| **SPD** | `B:n` | Used by: S2F3 S2F6 service program data | S2F3 S2F6 service program data |
| **SPID** | `A:6` | Used by: S2F1 S2F5 S2F7 S2F9 S2F12 service program identifie | S2F1 S2F5 S2F7 S2F9 S2F12 service program identifier |
| **SPNAME** | `A:n` | Used by: S14F19 S14F20 S14F21 S14F28 service parameter name | S14F19 S14F20 S14F21 S14F28 service parameter name |
| **SPR** | `A:n` |  | S2F10 device dependent, any data type |
| **SPVAL** | `A:n` |  | S14F19 S14F20 S14F21 service parameter value, any format type |
| **SSAACK** | `U1:1` | Used by: S20F2 service completion code | S20F2 service completion code |
| **SSACK** | `A:2` | Used by: S18F2 S18F4 S18F6 S18F8 S18F10 S18F12 S18F14 S18F16 | S18F2 S18F4 S18F6 S18F8 S18F10 S18F12 S18F14 S18F16 two character codes for succ |
| **SSCMD** | `A:n` | Used by: S18F13 subsystem action command | S18F13 subsystem action command |
| **STATUS** | `A:n` | Used by: S18F2 S18F4 S18F6 S18F8 S18F10 S18F12 S18F14 S18F16 | S18F2 S18F4 S18F6 S18F8 S18F10 S18F12 S18F14 S18F16 subsystem status data |
| **STATUSTXT** | `A:80` | Used by: S19F2 S19F4 S19F6 S19F8 S19F10 S19F13 S19F16 S19F18 | S19F2 S19F4 S19F6 S19F8 S19F10 S19F13 S19F16 S19F18 status response description |
| **STIME** | `A:32` | Used by: S6F1 ECV TimeFormat controls format, 0=A:12 YYMMDDH | S6F1 ECV TimeFormat controls format, 0=A:12 YYMMDDHHMMSS, 1=A:16 YYYYMMDDHHMMSSc |
| **STRACK** | `B:1` | Used by: S2F44 spooling stream acknowledge | S2F44 spooling stream acknowledge |
| **STRID** | `U1:1` | Used by: S2F43 S2F44 S2F60 stream value | S2F43 S2F44 S2F60 stream value |
| **STRP** | `I2:2` |  | S12F9 S12F16 x y die coordinate starting position |
| **SV** | `A:n` |  | S1F4 S1F6 S6F1 status variable value |
| **SV0** | `A:0` |  | S1F8 Zero length value used to convey format type |
| **SVCACK** | `B:1` | Used by: S14F20 S14F20 S14F21 service acknowledge code | S14F20 S14F20 S14F21 service acknowledge code |
| **SVCNAME** | `A:n` | Used by: S14F19 S14F26 S14F27 S14F28 service name | S14F19 S14F26 S14F27 S14F28 service name |
| **SVID** | `U4:1` |  | S1F3 S1F11 S1F12 S2F23 status variable ID |
| **SVNAME** | `A:n` | Used by: S1F8 S1F12 status variable name | S1F8 S1F12 status variable name |
| **TARGETID** | `A:n` | Used by: S18F1 S18F2 S18F3 S18F4 S18F5 S18F6 S18F7 S18F8 S18 | S18F1 S18F2 S18F3 S18F4 S18F5 S18F6 S18F7 S18F8 S18F9 S18F10 S18F11 S18F12 S18F1 |
| **TARGETPDE** | `A:36` | Used by: S19F15 S19F17 the UID of the target PDE, a 36 chara | S19F15 S19F17 the UID of the target PDE, a 36 character string with runs of 8,4, |
| **TARGETSPEC** | `A:40` | Used by: S14F17 S15F43 Specifier of target object | S14F17 S15F43 Specifier of target object |
| **TBLACK** | `U1:1` | Used by: S13F14 S13F16 acknowledge code | S13F14 S13F16 acknowledge code |
| **TBLCMD** | `U1:1` | Used by: S13F13 S13F15 table command | S13F13 S13F15 table command |
| **TBLELT** | `A:n` |  | S13F13 S13F15 S13F16 table element any type, list types or array types are disco |
| **TBLID** | `A:80` |  | S13F13 S13F15 S13F16 table identifier, a kind of OBJSPEC |
| **TBLTYP** | `A:n` | Used by: S13F13 S13F15 S13F16 denotes the format and applica | S13F13 S13F15 S13F16 denotes the format and application of the table, conforms t |
| **TCID** | `A:36` | Used by: S19F6 S19F8 S19F9 S19F10 S19F11 The identity of a t | S19F6 S19F8 S19F9 S19F10 S19F11 The identity of a transfer container specified a |
| **TEXT** | `A:120` |  | S10F1 S10F3 S10F5 S10F9 line of text for display, no standard max size |
| **TIAACK** | `B:1` | Used by: S2F24 trace acknowledgement code | S2F24 trace acknowledgement code |
| **TIACK** | `B:1` | Used by: S2F32 time set acknowledge | S2F32 time set acknowledge |
| **TID** | `B:1` | Used by: S10F1 S10F3 S10F5 S10F7 terminal ID | S10F1 S10F3 S10F5 S10F7 terminal ID |
| **TIME** | `A:32` | Used by: S2F18 S2F31 ECV TimeFormat controls format, 0=A:12  | S2F18 S2F31 ECV TimeFormat controls format, 0=A:12 YYMMDDHHMMSS, 1=A:16 YYYYMMDD |
| **TIMESTAMP** | `A:32` | Used by: S5F9 S5F11 S5F15 S15F41 S15F44 S16F7 S16F9 S20F12 S | S5F9 S5F11 S5F15 S15F41 S15F44 S16F7 S16F9 S20F12 S20F13 S20F15 S20F17 S20F18 S2 |
| **TOTSMP** | `U4:1` |  | S2F23 S17F5 total samples to be made, should be an even multiple of REPGSZ |
| **TRACK** | `TF:1` | Used by: S4F20 S4F22 S4F23 transfer activity success flag | S4F20 S4F22 S4F23 transfer activity success flag |
| **TRANSFERSIZE** | `U8:1` | Used by: S19F9 SIze in bytes of the TransferContainer. An 8  | S19F9 SIze in bytes of the TransferContainer. An 8 byte value but HSMS uses 4 by |
| **TRATOMCID** | `U4:1` |  | S4F20 assigned identifier for atomic transfer |
| **TRAUTOD** | `TF:1` | Used by: S17F5 delete upon completion flag | S17F5 delete upon completion flag |
| **TRAUTOSTART** | `TF:1` | Used by: S4F19 if true material transfer is initiated by the | S4F19 if true material transfer is initiated by the primary when ready |
| **TRCMDNAME** | `A:n` | Used by: S4F21 text enum, CANCEL, PAUSE, RESUME, ABORT, STOP | S4F21 text enum, CANCEL, PAUSE, RESUME, ABORT, STOP, STARTHANDOFF |
| **TRDIR** | `U1:1` | Used by: S4F19 S4F27 transfer direction | S4F19 S4F27 transfer direction |
| **TRID** | `A:n` |  | S2F23 S2F62 S6F1 S6F27 S6F28 S6F30 S17F5 S17F6 S17F7 S17F8 S17F13 S17F14 trace r |
| **TRJOBID** | `B:1` | Used by: S4F20 S4F21 S4F23 assigned identifier for transfer  | S4F20 S4F21 S4F23 assigned identifier for transfer job |
| **TRJOBMS** | `U1:1` | Used by: S4F23 transfer job milestone | S4F23 transfer job milestone |
| **TRJOBNAME** | `A:80` | Used by: S4F19 S4F23 host assigned id for transfer job | S4F19 S4F23 host assigned id for transfer job |
| **TRLINK** | `U4:1` |  | S4F19 S4F27 S4F29 S4F31 S4F33 S4F35 S4F37 S4F39 S4F41 task identifier correlatio |
| **TRLOCATION** | `U4:1` |  | S4F19 S4F27 material transfer location |
| **TROBJNAME** | `A:n` |  | S4F19 S4F27 identifies material to be transferred |
| **TROBJTYPE** | `U4:1` |  | S4F19 S4F27 identifies type of object to be transferred |
| **TRPORT** | `U4:1` |  | S4F19 S4F27 port identifier |
| **TRPTNR** | `A:n` | Used by: S4F19 S4F27 EQNAME of transfer partner equipment | S4F19 S4F27 EQNAME of transfer partner equipment |
| **TRPTPORT** | `U4:1` |  | S4F19 S4F27 transfer partner port |
| **TRRCP** | `A:80` | Used by: S4F19 name of transfer recipe for this handoff | S4F19 name of transfer recipe for this handoff |
| **TRROLE** | `U1:1` | Used by: S4F19 S4F27 indicates equipment transfer role | S4F19 S4F27 indicates equipment transfer role |
| **TRTYPE** | `U1:1` | Used by: S4F19 S4F27 equipment is active or passive transfer | S4F19 S4F27 equipment is active or passive transfer participant |
| **TSIP** | `B:n` | Used by: S1F10 transfer status of input ports | S1F10 transfer status of input ports |
| **TSOP** | `B:n` | Used by: S1F10 transfer status of output ports | S1F10 transfer status of output ports |
| **TTC** | `U4:1` |  | S3F4 time to completion, standard does not specify units, in seconds?? |
| **TYPEID** | `U1:1` | Used by: S20F12 S20F13 S20F15 S20F17 S20F18 S20F20 S20F21 S2 | S20F12 S20F13 S20F15 S20F17 S20F18 S20F20 S20F21 S20F23 S20F27 S20F30 S20F32 rec |
| **UID** | `A:36` | Used by: S19F2 S19F3 S19F4 S19F5 S19F6 S19F7 S19F8 S19F13 S1 | S19F2 S19F3 S19F4 S19F5 S19F6 S19F7 S19F8 S19F13 S19F16 S19F18 See SEMI E139. A  |
| **UNFLEN** | `U4:1` |  | S7F34 unformatted process program length if available, else 0 |
| **UNITS** | `A:n` | Used by: S1F12 S1F22 S2F30 S2F48 units identifier (see E5 Se | S1F12 S1F22 S2F30 S2F48 units identifier (see E5 Section 9) |
| **UPPERDB** | `F4:1` |  | S2F45 S2F48 the upper bound of a deadband limit |
| **V** | `A:n` |  | S6F11 S6F13 S6F16 S6F18 S6F20 S6F22 S6F27 S6F30 S16F9 variable value, any type i |
| **VERID** | `A:n` | Used by: S20F12 S20F13 S20F15 S20F17 S20F18 S20F20 S20F21 S2 | S20F12 S20F13 S20F15 S20F17 S20F18 S20F20 S20F21 S20F23 S20F27 S20F30 S20F32 com |
| **VERIFYDEPTH** | `U1:1` | Used by: S19F17 whether to check only the target, or the tar | S19F17 whether to check only the target, or the target and all referenced PDEs |
| **VERIFYRSPSTAT** | `U1:1` | Used by: S19F13 S19F18 PDE verification result, 0 success, 1 | S19F13 S19F18 PDE verification result, 0 success, 10 none, other error |
| **VERIFYSUCCESS** | `TF:1` | Used by: S19F18 True if no errors were found | S19F18 True if no errors were found |
| **VERIFYTYPE** | `U1:1` | Used by: S19F17 chooses the type of verification | S19F17 chooses the type of verification |
| **VID** | `A:n` |  | S1F21 S1F22 S1F24 S2F33 S2F45 S2F46 S2F47 S2F48 S2F54 S6F13 S6F18 S6F22 S16F9 S1 |
| **VLAACK** | `B:1` | Used by: S2F46 variable limit attribute acknowledge | S2F46 variable limit attribute acknowledge |
| **WRACK** | `U1:1` | Used by: S20F16 service completion code | S20F16 service completion code |
| **XDIES** | `F4:1` |  | S12F1 S12F4 X-axis die size |
| **XYPOS** | `I2:2` |  | S12F11 S12F18 x y coordinate position |
| **YDIES** | `F4:1` |  | S12F1 S12F4 Y-axis die size |

## 詳細說明

### ABS

- **格式**：`B:n`
- **說明**：Used by: S2F25 S2F26 any binary string
- **範例**：`&quot;B:18 0x00 0x01 0x03 0x03 0x0a 0x0d 0x1b 0x5d 0x18 0x18 0x18 0x1a 0x04 0x13 0x7f 0x80 0xfe 0xff`
- **使用於**：S2F25 S2F26 any binary string

### ACCESSMODE

- **格式**：`U1:1`
- **說明**：Used by: S3F21 S3F27 load port access mode
- **範例**：`&quot;U1:1 0&quot; 0 - Manual 1 - Auto`
- **使用於**：S3F21 S3F27 load port access mode

### ACDS

- **格式**：`U2:1`
- **範例**：`&quot;U2:1 0&quot;`
- **使用於**：S7F22 after command codes

### ACKA

- **格式**：`TF:1`
- **說明**：Used by: S5F14 S5F15 S5F18 S16F4 S16F6 S16F7 S16F12 S16F16 S16F18 S16F24 S16F26 S16F28 S16F30 S17F4 <a hr
- **使用於**：S5F14 S5F15 S5F18 S16F4 S16F6 S16F7 S16F12 S16F16 S16F18 S16F24 S16F26 S16F28 S16F30 S17F4 <a hr

### ACKC10

- **格式**：`B:1`
- **說明**：Used by: S10F2 S10F4 S10F6 S10F10 acknowledge code
- **範例**：`&quot;B:1 0x00&quot; 0 - accepted for display 1 - message will not be displayed 2 - terminal not ava`
- **使用於**：S10F2 S10F4 S10F6 S10F10 acknowledge code

### ACKC13

- **格式**：`B:1`
- **說明**：Used by: S13F2 S13F4 S13F6 S13F8 acknowledge code, 0 ok
- **範例**：`&quot;B1:1 0x00&quot; 1 - retryable error 2 - unknown data set name 3 - illegal checkpoint value 4 -`
- **使用於**：S13F2 S13F4 S13F6 S13F8 acknowledge code, 0 ok

### ACKC15

- **格式**：`B:1`
- **說明**：Used by: S15F50 S15F52 acknowledge code, 0 ok
- **範例**：`&quot;B1:1 0x00&quot; 1 - initiated for asynchronous completion 2 - DSNAME not found 3 - permission `
- **使用於**：S15F50 S15F52 acknowledge code, 0 ok

### ACKC3

- **格式**：`B:1`
- **說明**：Used by: S3F6 S3F8 S3F10 acknowledge code, 0 ok
- **範例**：`&quot;B:1 0x00&quot;`
- **使用於**：S3F6 S3F8 S3F10 acknowledge code, 0 ok

### ACKC5

- **格式**：`B:1`
- **說明**：Used by: S5F2 S5F4 acknowledge code, 0 ok
- **範例**：`&quot;B:1 0x00&quot;`
- **使用於**：S5F2 S5F4 acknowledge code, 0 ok

### ACKC6

- **格式**：`B:1`
- **說明**：Used by: S6F2 S6F4 S6F10 S6F12 S6F14 S6F26 acknowledge code, 0 ok
- **範例**：`&quot;B:1 0x00&quot;`
- **使用於**：S6F2 S6F4 S6F10 S6F12 S6F14 S6F26 acknowledge code, 0 ok

### ACKC7

- **格式**：`B:1`
- **說明**：Used by: S7F4 S7F12 S7F14 S7F16 S7F18 S7F24 S7F32 S7F38 S7F40 S7F42 S7F44 S7 acknowledge code
- **範例**：`&quot;B:1 0x00&quot; 0 - Accepted 1 - Permission not granted 2 - length e`
- **使用於**：S7F4 S7F12 S7F14 S7F16 S7F18 S7F24 S7F32 S7F38 S7F40 S7F42 S7F44 S7 acknowledge code

### ACKC7A

- **格式**：`U4:1`
- **範例**：`&quot;U4:1 0&quot; 0 - ok 1 - inconsistent MDLN 2 - inconsistent SOFTREV 3 - invalid CCODE 4 - inval`
- **使用於**：S7F27 process program check code

### AGENT

- **格式**：`A:n`
- **說明**：Used by: S15F11 S15F12 S15F21 S15F22 S15F25 no description, no max length
- **範例**：`&quot;A:20 admin@model-shop.net&quot;`
- **使用於**：S15F11 S15F12 S15F21 S15F22 S15F25 no description, no max length

### ALCD

- **格式**：`B:1`
- **說明**：Used by: S5F1 S5F6 S5F8 alarm code byte, >= 128 alarm is set, bit field use is deprecated
- **範例**：`&quot;B:1 0x00&quot;`
- **使用於**：S5F1 S5F6 S5F8 alarm code byte, >= 128 alarm is set, bit field use is deprecated

### ALED

- **格式**：`B:1`
- **說明**：Used by: S5F3 enable/disable alarm, 128 means enable, 0 disable
- **範例**：`&quot;B:1 0x00&quot; 0 - disable alarm 128 - enable alarm`
- **使用於**：S5F3 enable/disable alarm, 128 means enable, 0 disable

### ALID

- **格式**：`U4:1`
- **範例**：`&quot;U4 1000&quot;`
- **使用於**：S5F1 S5F3 S5F6 S5F8 Alarm type ID

### ALIDVECTOR

- **格式**：`U4:n`
- **範例**：`&quot;U4:0&quot;`
- **使用於**：S5F5 alarm ID vector

### ALTX

- **格式**：`A:120`
- **說明**：Used by: S5F1 S5F6 S5F8 alarm text, the length limit was recently raised from 40
- **範例**：`&quot;A:31 {sensor timeout at load elevator}&quot;`
- **使用於**：S5F1 S5F6 S5F8 alarm text, the length limit was recently raised from 40

### ASSGNID

- **格式**：`U1:1`
- **說明**：Used by: S20F12 S20F13 S20F15 S20F17 S20F18 S20F20 S20F21 S20F23 S20F27 S20F30 S20F32 Assigner of the RecipeXID Base Part
- **範例**：`&quot;U1 0&quot; 0 - RMSASN 1 -`
- **使用於**：S20F12 S20F13 S20F15 S20F17 S20F18 S20F20 S20F21 S20F23 S20F27 S20F30 S20F32 Assigner of the RecipeXID Base Part

### ATTRDATA

- **格式**：`A:n`
- **使用於**：S1F20 S3F35 S3F35 S13F13 S13F16 S14F1 S14F2 S14F3 S14F4 S14F9 S14F10 S14F11 S14F12 S14F13 <a

### ATTRID

- **格式**：`A:40`
- **使用於**：S1F19 S3F35 S3F35 S13F13 S13F16 S14F1 S14F1 S14F2 S14F3 S14F4 S14F8 S14F9 S14F10 S14F11 <a href="m

### ATTRRELN

- **格式**：`U1:1`
- **說明**：Used by: S14F1 relationship of a value to an attribute value of an object
- **範例**：`&quot;U1:1 0&quot; 0 - equal 1 - not equal 2 - value 3 - value 4 - value > obj attribute value 5 - v`
- **使用於**：S14F1 relationship of a value to an attribute value of an object

### AUTOCLEAR_DISABLE

- **格式**：`U1:1`
- **說明**：Used by: S20F1 Disable automatic clear of recipes on SRO transition to Local (E171)
- **範例**：`&quot;U1:1 0&quot; 0 - autoclear is not disabled 1 - autoclear is disabled`
- **使用於**：S20F1 Disable automatic clear of recipes on SRO transition to Local (E171)

### AUTOCLOSE

- **格式**：`U2:1`
- **說明**：Used by: S20F1 Interaction timeout for closing operator session, 0 is no limit
- **範例**：`&quot;U1:1 0&quot;`
- **使用於**：S20F1 Interaction timeout for closing operator session, 0 is no limit

### AUTOPOST_DISABLE

- **格式**：`U1:1`
- **說明**：Used by: S20F1 Disable automatic posting of recipes to RMS preceeding SRO move to Local state (E171)
- **範例**：`&quot;U1:1 0&quot; 0 - autopost is not disabled 1 - autopost is disabled`
- **使用於**：S20F1 Disable automatic posting of recipes to RMS preceeding SRO move to Local state (E171)

### BCDS

- **格式**：`U2:n`
- **範例**：`&quot;U2:4 72 85 109 101&quot;`
- **使用於**：S7F22 before command code vector

### BCEQU

- **格式**：`U1:n`
- **範例**：`&quot;U1:3 1 2 3&quot;`
- **使用於**：S12F3 S12F4 array of bin code equivalents

### BINLT

- **格式**：`U1:n`
- **範例**：`&quot;U1:3 3 4 5&quot;`
- **使用於**：S12F7 S12F9 S12F11 S12F14 S12F16 S12F18 array of bin values, text or U1 array

### BLKDEF

- **格式**：`I1:1`
- **說明**：Used by: S7F22 command definition block relationship, standard incorrectly says type U1 is possible
- **範例**：`&quot;I1:1 0&quot; -1 - terminates a block 0 - within block body 1 - starts a block`
- **使用於**：S7F22 command definition block relationship, standard incorrectly says type U1 is possible

### BPD

- **格式**：`B:n`
- **說明**：Used by: S8F2 boot program data, the fantasy of using SECS for boot programs has been revived
- **範例**：`&quot;B:0&quot;`
- **使用於**：S8F2 boot program data, the fantasy of using SECS for boot programs has been revived

### BYTMAX

- **格式**：`U4:1`
- **範例**：`&quot;U4:1 0&quot;`
- **使用於**：S7F22 process program maximum byte length, 0 means no limit

### CAACK

- **格式**：`U1:1`
- **說明**：Used by: S3F18 S3F20 S3F22 S3F24 S3F26 S3F28 S3F30 S3F32 S3F34 carrier action acknowledge
- **範例**：`&quot;U1:1 0&quot; 0 - ok 1 - invalid command 2 - cannot perform now 3 - invalid data or argument 4 `
- **使用於**：S3F18 S3F20 S3F22 S3F24 S3F26 S3F28 S3F30 S3F32 S3F34 carrier action acknowledge

### CARRIERACTION

- **格式**：`A:n`
- **說明**：Used by: S3F17 carrier action request
- **範例**：`&quot;A:18 {ProceedWithCarrier}&quot;`
- **使用於**：S3F17 carrier action request

### CARRIERID

- **格式**：`A:n`
- **說明**：Used by: S3F17 S16F11 S16F15 carrier ID
- **範例**：`&quot;A:9 {CSX 52078}&quot;`
- **使用於**：S3F17 S16F11 S16F15 carrier ID

### CARRIERSPEC

- **格式**：`A:n`
- **說明**：Used by: S3F29 S3F31 carrier object specifier (OBJSPEC)
- **範例**：`&quot;A:17 {Carrier:CSX 52078}&quot;`
- **使用於**：S3F29 S3F31 carrier object specifier (OBJSPEC)

### CATTRDATA

- **格式**：`A:n`
- **範例**：`&quot;A:7 product&quot;`
- **使用於**：S3F17 carrier attribute value (any data type)

### CATTRID

- **格式**：`A:n`
- **範例**：`&quot;A:5 Usage&quot;`
- **使用於**：S3F17 carrier attribute identifier, E87 requires text per E39.1, Sec 6

### CCEACK

- **格式**：`U1:1`
- **說明**：Used by: S20F8 event completion code
- **範例**：`&quot;U1:1 0&quot; 0 - Success 1 - Error`
- **使用於**：S20F8 event completion code

### CCODE

- **格式**：`A:n`
- **範例**：`&quot;A:5 {50 54}&quot;`
- **使用於**：S7F22 S7F23 S7F26 S7F31 process operation command code

### CEED

- **格式**：`TF:1`
- **說明**：Used by: S2F37 S17F5 collection event or trace enablement, true is enabled
- **範例**：`&quot;TF:1 1&quot;`
- **使用於**：S2F37 S17F5 collection event or trace enablement, true is enabled

### CEID

- **格式**：`U4:1`
- **使用於**：S1F23 S1F24 S2F35 S2F37 S2F55 S2F56 S2F58 S6F3 S6F8 S6F9 S6F11 S6F13 S6F15 S6F16 <a href="msgs.html#S6F17" ta

### CEIDSTART

- **格式**：`U4:1`
- **範例**：`&quot;U4:1 4050&quot;`
- **使用於**：S17F5 the CEID of a start event

### CEIDSTOP

- **格式**：`U4:1`
- **範例**：`&quot;U4:1 4050&quot;`
- **使用於**：S17F5 the CEID of a stop event

### CENAME

- **格式**：`A:n`
- **說明**：Used by: S1F24 S2F56 a descriptive name for a Data Collection Event
- **範例**：`&quot;A ProcessStateUpdate&quot;`
- **使用於**：S1F24 S2F56 a descriptive name for a Data Collection Event

### CEPACK

- **格式**：`B:1`
- **說明**：Used by: S2F50 command enhanced parameter acknowledge, may be a list or nested list structure to mirror the input structure of a CEPVAL
- **範例**：`&quot;B:1 0x00&quot; 1 - CPNAME value not defined 2 - illegal value for CEPVAL 3 - illegal format fo`
- **使用於**：S2F50 command enhanced parameter acknowledge, may be a list or nested list structure to mirror the input structure of a CEPVAL

### CEPVAL

- **格式**：`A:n`
- **範例**：`&quot;A:0&quot;`
- **使用於**：S2F49 an enhanced parameter value, may be a scalar of any type, a list of values of the same type, or a list of possibly nested {L:2 CPNAME CEPVAL}

### CHKINFO

- **格式**：`A:n`
- **範例**：`&quot;A:44 {S20 has inverted booleans specified as enums}&quot;`
- **使用於**：S20F31 User defined value, any type

### CKPNT

- **格式**：`U4:1`
- **說明**：Used by: S13F3 S13F6 data set checkpoint defined by sender
- **範例**：`&quot;U4:1 28956&quot;`
- **使用於**：S13F3 S13F6 data set checkpoint defined by sender

### CMDA

- **格式**：`B:1`
- **說明**：Used by: S2F22 S2F28 command acknowledge code
- **範例**：`&quot;B:1 0x00&quot; 0 - ok 1 - command does not exist 2 - not now`
- **使用於**：S2F22 S2F28 command acknowledge code

### CMDMAX

- **格式**：`U1:1`
- **範例**：`&quot;U1:1 0&quot;`
- **使用於**：S7F22 maximum number of commands allowed, 0=unlimited

### CNAME

- **格式**：`A:16`
- **說明**：Used by: S7F22 text name for a CCODE
- **範例**：`&quot;A:13 {spin motor on}&quot;`
- **使用於**：S7F22 text name for a CCODE

### COACK

- **格式**：`U1:1`
- **說明**：Used by: S20F10 service completion code
- **範例**：`&quot;U1:1 0&quot; 0 - Success 1 - Error`
- **使用於**：S20F10 service completion code

### COLCT

- **格式**：`U4:1`
- **範例**：`&quot;U4:1 30&quot;`
- **使用於**：S12F1 S12F4 column count in die increments

### COLHDR

- **格式**：`A:20`
- **說明**：Used by: S13F13 S13F15 S13F16 table column name
- **範例**：`&quot;A:4 Code&quot;`
- **使用於**：S13F13 S13F15 S13F16 table column name

### COMMACK

- **格式**：`B:1`
- **說明**：Used by: S1F14 establish communications acknowledgement code
- **範例**：`&quot;B:1 0x00&quot; 0 - ok 1 - denied`
- **使用於**：S1F14 establish communications acknowledgement code

### COMPARISONOPERATOR

- **格式**：`U1:1`
- **說明**：Used by: S19F1 choice of comparison operators. Interpreted as "target <op> <const>" where <op> is this value and <const> is supplied in the expression
- **範例**：`&quot;U1:1 0&quot; 0 - equals 1 - not equal to 2 - less than 3 - less than or equal to 4 - greater t`
- **使用於**：S19F1 choice of comparison operators. Interpreted as "target <op> <const>" where <op> is this value and <const> is supplied in the expression

### CONDITION

- **格式**：`A:n`
- **說明**：Used by: S18F16 sub-system condition info tag
- **使用於**：S18F16 sub-system condition info tag

### COPYID

- **格式**：`U1:1`
- **說明**：Used by: S20F12 S20F13 S20F15 S20F17 S20F18 S20F20 S20F21 S20F23 S20F27 S20F30 S20F32 Recipe copy type
- **範例**：`&quot;U1:1 0&quot;`
- **使用於**：S20F12 S20F13 S20F15 S20F17 S20F18 S20F20 S20F21 S20F23 S20F27 S20F30 S20F32 Recipe copy type

### CPACK

- **格式**：`B:1`
- **說明**：Used by: S2F42 remote command parameter acknowledge, only received if error
- **範例**：`&quot;B:1 0x01&quot; 1 - unknown CPNAME 2 - illegal value for CPVAL 3 - illegal format for CPVAL`
- **使用於**：S2F42 remote command parameter acknowledge, only received if error

### CPNAME

- **格式**：`A:n`
- **範例**：`&quot;A:10 ppexecname&quot;`
- **使用於**：S2F41 S2F42 S2F49 S2F50 S4F21 S4F29 S16F5 S16F27 command parameter name

### CPVAL

- **格式**：`A:n`
- **範例**：`&quot;A:14 cmos168-zl0EC3&quot;`
- **使用於**：S2F41 S4F21 S4F29 S16F5 S16F27 S18F13 command parameter value, any scalar type

### CSAACK

- **格式**：`U1:1`
- **說明**：Used by: S2F8 equipment acknowledge code
- **範例**：`&quot;U1:1 0&quot; 0 - success 1 - busy 2 - invalid SPID 3 - invalid data`
- **使用於**：S2F8 equipment acknowledge code

### CTLJOBCMD

- **格式**：`U1:1`
- **說明**：Used by: S16F27 control job command
- **範例**：`&quot;U1:1 1&quot; 1 - start 2 - pause 3 - resume 4 - cancel 5 - deselect 6 - stop 7 - abort 8 - HOQ`
- **使用於**：S16F27 control job command

### CTLJOBID

- **格式**：`A:n`
- **說明**：Used by: S16F27 control job ID, an OBJID
- **範例**：`&quot;A:7 {job0001}&quot;`
- **使用於**：S16F27 control job ID, an OBJID

### DATA

- **格式**：`A:n`
- **範例**：`&quot;A:0&quot;`
- **使用於**：S3F30 S3F31 S18F6 S18F7 unformatted data

### DATAACK

- **格式**：`B:1`
- **說明**：Used by: S14F22 Acknowledgement code
- **範例**：`&quot;B:1 0&quot; 0 - OK 1 - unknown DATAID 2 - at least one parameter is invalid`
- **使用於**：S14F22 Acknowledgement code

### DATAID

- **格式**：`U4:1`
- **使用於**：S2F33 S2F35 S2F39 S2F45 S2F49 S3F15 S3F17 S4F19 S4F25 S6F3 S6F5 S6F7 S6F8 S6F9 <a href="msgs.html#S6F11" ta

### DATALENGTH

- **格式**：`U4:1`
- **範例**：`&quot;U4:1 649`
- **使用於**：S2F39 S3F15 S3F29 S3F31 S4F25 S6F5 S13F11 S14F23 S16F1 S18F5 S18F7 S19F19 total bytes of the message body

### DATASEG

- **格式**：`A:n`
- **範例**：`&quot;A:3 S01&quot;`
- **使用於**：S3F29 S3F31 S18F5 S18F7 identifies data requested, E87 requires text

### DATASRC

- **格式**：`A:n`
- **說明**：Used by: S17F1 identifies a data source, use length 0 to mean the default
- **範例**：`&quot;A:0&quot;`
- **使用於**：S17F1 identifies a data source, use length 0 to mean the default

### DATLC

- **格式**：`U1:1`
- **說明**：Used by: S12F19 location of invalid data, offset in bytes in the SECS-II message body
- **範例**：`&quot;U1:1 200&quot;`
- **使用於**：S12F19 location of invalid data, offset in bytes in the SECS-II message body

### DELRSPSTAT

- **格式**：`U1:1`
- **說明**：Used by: S19F4 Response code for the PDE deletion request, non-zero means not deleted
- **範例**：`&quot;U1:1 0&quot; 0 - OK 1 - not found 2 - locked 255 - other error`
- **使用於**：S19F4 Response code for the PDE deletion request, non-zero means not deleted

### DIRRSPSTAT

- **格式**：`U1:1`
- **說明**：Used by: S19F2 get dir status response
- **範例**：`&quot;U1:1 0&quot; 0 - OK 1 - BadFilter 2 - BadAttribute 255 - Other error`
- **使用於**：S19F2 get dir status response

### DRACK

- **格式**：`B:1`
- **說明**：Used by: S2F34 define report acknowledge
- **範例**：`&quot;B:1 0x00&quot; 0 - ok 1 - out of space 2 - invalid format 3 - 1 or more RPTID already defined `
- **使用於**：S2F34 define report acknowledge

### DRRACK

- **格式**：`U1:1`
- **說明**：Used by: S20F14 service completion code
- **範例**：`&quot;U1:1 0&quot; 0 - done 1 - error`
- **使用於**：S20F14 service completion code

### DSID

- **格式**：`A:n`
- **範例**：`&quot;A:2 {12}&quot;`
- **使用於**：S6F3 S6F8 S6F9 data set ID, akin to a report type

### DSNAME

- **格式**：`A:50`
- **範例**：`&quot;A:28 {process program save archive}&quot;`
- **使用於**：S7F37 S7F39 S7F41 S7F43 S13F1 S13F2 S13F3 S13F4 S15F49 S15F51 the name of a dataset such as a PPID

### DSPER

- **格式**：`A:6`
- **範例**：`&quot;A:6 000500&quot;`
- **使用於**：S2F23 data sample period, hhmmss is always supported, A:8 hhmmsscc may be supported

### DUTMS

- **格式**：`A:n`
- **說明**：Used by: S12F1 S12F4 die units of measure (per E5 Section 12)
- **範例**：`&quot;A:2 {mm}&quot;`
- **使用於**：S12F1 S12F4 die units of measure (per E5 Section 12)

### DVNAME

- **格式**：`U4:1`
- **範例**：`&quot;U4:1 10&quot;`
- **使用於**：S6F3 S6F8 data value name, generically a VID, therefore GEM requires Un type

### DVVAL

- **格式**：`A:n`
- **範例**：`&quot;A:2 {54}&quot;`
- **使用於**：S6F3 S6F8 S6F9 data value, any format including list

### DVVALNAME

- **格式**：`A:n`
- **說明**：Used by: S1F22 a descriptive name for a Data Value variable (DVVAL)
- **範例**：`&quot;A EventName&quot;`
- **使用於**：S1F22 a descriptive name for a Data Value variable (DVVAL)

### EAC

- **格式**：`B:1`
- **說明**：Used by: S2F16 equipment acknowledge code, 0 ok
- **範例**：`&quot;B:1 0x00&quot; 0 - ok 1 - one or more constants does not exist 2 - busy 3 - one or more values`
- **使用於**：S2F16 equipment acknowledge code, 0 ok

### ECDEF

- **格式**：`A:n`
- **範例**：`&quot;A:1 {0}&quot;`
- **使用於**：S2F30 equipment constant default value

### ECID

- **格式**：`U4:1`
- **範例**：`&quot;U4:1 220&quot;`
- **使用於**：S2F13 S2F15 S2F29 S2F30 equipment constant ID, GEM requires U4

### ECMAX

- **格式**：`A:n`
- **範例**：`&quot;A:3 100&quot;`
- **使用於**：S2F30 equipment constant maximum value, any scalar type

### ECMIN

- **格式**：`A:n`
- **範例**：`&quot;A:1 0&quot;`
- **使用於**：S2F30 equipment constant minimum value, any scalar type

### ECNAME

- **格式**：`A:n`
- **說明**：Used by: S2F30 equipment constant name
- **範例**：`&quot;A:10 TimeFormat&quot;`
- **使用於**：S2F30 equipment constant name

### ECV

- **格式**：`A:n`
- **範例**：`&quot;A:1 1&quot;`
- **使用於**：S2F14 S2F15 equipment constant value, any scalar type (constant is a misnomer)

### EDID

- **格式**：`A:80`
- **範例**：`&quot;A:12 {example PPID}&quot;`
- **使用於**：S9F13 expected data identification, PPID or SPID or PTN

### EMID

- **格式**：`A:16`
- **範例**：`&quot;A:1 {1}&quot;`
- **使用於**：S3F9 equivalent material ID

### EPD

- **格式**：`B:n`
- **說明**：Used by: S8F4 executive program data, the fantasy of using SECS for this has been revived
- **範例**：`&quot;B:0&quot;`
- **使用於**：S8F4 executive program data, the fantasy of using SECS for this has been revived

### EQID

- **格式**：`A:256`
- **說明**：Used by: S20F12 S20F13 S20F15 S20F17 S20F18 S20F20 S20F21 S20F23 S20F27 S20F30 S20F32 recipe specification of compatible equipment
- **範例**：`&quot;A:11 {Nimbus 2000}&quot;`
- **使用於**：S20F12 S20F13 S20F15 S20F17 S20F18 S20F20 S20F21 S20F23 S20F27 S20F30 S20F32 recipe specification of compatible equipment

### EQNAME

- **格式**：`A:80`
- **說明**：Used by: S4F27 factory assigned equipment identifier
- **範例**：`&quot;A:14 {nadasorter 103}&quot;`
- **使用於**：S4F27 factory assigned equipment identifier

### EQUSERID

- **格式**：`A:64`
- **說明**：Used by: S20F5 Equipment userID for recipe use authentication
- **範例**：`&quot;A:8 sysadmin&quot;`
- **使用於**：S20F5 Equipment userID for recipe use authentication

### ERACK

- **格式**：`B:1`
- **說明**：Used by: S2F38 enable/disable event report acknowledge
- **範例**：`&quot;B:1 0x00&quot; 0 - ok 1 - denied`
- **使用於**：S2F38 enable/disable event report acknowledge

### ERRCODE

- **格式**：`U4:1`
- **使用於**：S1F20 S3F18 S3F20 S3F22 S3F24 S3F26 S3F28 S3F30 S3F32 S3F34 S3F36 S4F20 S4F22 S4F23 <a href="msgs.ht

### ERRTEXT

- **格式**：`A:80`
- **說明**：Used by: S1F20 S3F18 S3F20 S3F22 S3F24 S3F26 S3F28 S3F30 S3F32 S3F34 S3F36 S4F20 S4F22 S4F23 <a href="msgs
- **使用於**：S1F20 S3F18 S3F20 S3F22 S3F24 S3F26 S3F28 S3F30 S3F32 S3F34 S3F36 S4F20 S4F22 S4F23 <a href="msgs

### ERRW7

- **格式**：`A:n`
- **範例**：`&quot;A:27 {MODLN value is inconsistent}&quot;`
- **使用於**：S7F27 process program error description

### EVNTSRC

- **格式**：`A:n`
- **說明**：Used by: S17F5 S17F9 S17F10 S17F11 S17F12 identifies an event source, use length 0 to specify the default
- **範例**：`&quot;A:0&quot;`
- **使用於**：S17F5 S17F9 S17F10 S17F11 S17F12 identifies an event source, use length 0 to specify the default

### EVNTSRC2

- **格式**：`A:n`
- **說明**：Used by: S17F5 a second event source EVNTSRC
- **範例**：`&quot;A:0&quot;`
- **使用於**：S17F5 a second event source EVNTSRC

### EXID

- **格式**：`A:20`
- **說明**：Used by: S5F9 S5F11 S5F13 S5F14 S5F15 S5F17 S5F18 exception identifier
- **範例**：`&quot;A:10 {out of ink}&quot;`
- **使用於**：S5F9 S5F11 S5F13 S5F14 S5F15 S5F17 S5F18 exception identifier

### EXMESSAGE

- **格式**：`A:n`
- **說明**：Used by: S5F9 S5F11 exception description
- **範例**：`&quot;A:30 {ink not sensed at nozzle inlet}&quot;`
- **使用於**：S5F9 S5F11 exception description

### EXRECVRA

- **格式**：`A:40`
- **說明**：Used by: S5F9 S5F13 exception recovery action description
- **範例**：`&quot;A:33 {manually insert new ink cartridge}&quot;`
- **使用於**：S5F9 S5F13 exception recovery action description

### EXTYPE

- **格式**：`A:5`
- **說明**：Used by: S5F9 S5F11 exception type, "ALARM" or "ERROR"
- **範例**：`&quot;A:5 {ALARM}&quot;`
- **使用於**：S5F9 S5F11 exception type, "ALARM" or "ERROR"

### FCNID

- **格式**：`U1:1`
- **說明**：Used by: S2F43 S2F44 S2F60 message type function value
- **範例**：`&quot;U1:1 13&quot;`
- **使用於**：S2F43 S2F44 S2F60 message type function value

### FFROT

- **格式**：`U2:1`
- **說明**：Used by: S12F1 S12F3 film frame location in degrees clockwise from bottom
- **範例**：`&quot;U2:1 0&quot;`
- **使用於**：S12F1 S12F3 film frame location in degrees clockwise from bottom

### FILDAT

- **格式**：`B`
- **使用於**：S13F6 Data Set Data, binary or ascii. Max length is the RECLEN from open.

### FNLOC

- **格式**：`U2:1`
- **說明**：Used by: S12F1 S12F3 S12F4 flat/notch location in degrees clockwise from bottom
- **範例**：`&quot;U2:1 0&quot;`
- **使用於**：S12F1 S12F3 S12F4 flat/notch location in degrees clockwise from bottom

### FRMLEN

- **格式**：`U4:1`
- **範例**：`&quot;U4:1 0&quot;`
- **使用於**：S7F34 formatted process program length if available, else 0

### GETRSPSTAT

- **格式**：`U1:1`
- **說明**：Used by: S19F6 S19F8 Response code for PDE queries, non-zero indicates failure
- **範例**：`&quot;U1:1 0&quot; 0 - OK 1 - not found 2 - locked 3 - not allowed 255 - other error`
- **使用於**：S19F6 S19F8 Response code for PDE queries, non-zero indicates failure

### GOILACK

- **格式**：`U1:1`
- **說明**：Used by: S20F4 completion code
- **範例**：`&quot;U1:1 0&quot; 0 - ok 1 - Error`
- **使用於**：S20F4 completion code

### GRANT

- **格式**：`B:1`
- **說明**：Used by: S2F2 S2F40 S3F16 S4F26 S13F12 S14F24 S16F2 S19F20 multiblock grant code
- **範例**：`&quot;B:1 0x00&quot; 0 - ok 1 - busy 2 - insufficient space 3 - duplicate DATAID`
- **使用於**：S2F2 S2F40 S3F16 S4F26 S13F12 S14F24 S16F2 S19F20 multiblock grant code

### GRANT6

- **格式**：`B:1`
- **說明**：Used by: S6F6 multblock permission grant
- **範例**：`&quot;B:1 0x00&quot; 0 - ok 1 - busy 2 - not interested`
- **使用於**：S6F6 multblock permission grant

### GRNT1

- **格式**：`B:1`
- **說明**：Used by: S12F6 grant code
- **範例**：`&quot;B:1 0x00&quot; 0 - ok 1 - retryable busy 2 - no space 3 - map too large 4 - duplicate ID 5 - m`
- **使用於**：S12F6 grant code

### GRXLACK

- **格式**：`U1:1`
- **說明**：Used by: S20F12 service completion code
- **範例**：`&quot;U1:1 0&quot; 0 - done 1 - error`
- **使用於**：S20F12 service completion code

### HANDLE

- **格式**：`U4`
- **範例**：`&quot;U4: 1&quot;`
- **使用於**：S13F3 S13F4 S13F5 S13F6 S13F7 S13F8 logical unit or handle for a data set

### HCACK

- **格式**：`B:1`
- **說明**：Used by: S2F42 S2F50 remote command acknowledge
- **範例**：`&quot;B:1 0x00&quot; 0 - ok, completed 1 - invalid command 2 - cannot do now 3 - parameter error 4 -`
- **使用於**：S2F42 S2F50 remote command acknowledge

### HOACK

- **格式**：`TF:1`
- **說明**：Used by: S4F31 S4F33 handoff success flag
- **範例**：`&quot;TF:1 1&quot;`
- **使用於**：S4F31 S4F33 handoff success flag

### HOCANCELACK

- **格式**：`U1:1`
- **說明**：Used by: S4F37 hand off cancel ack
- **範例**：`&quot;U1:1 0&quot; 0 - ok 1 - TRLINK value not known 2 - rejected - handoff started`
- **使用於**：S4F37 hand off cancel ack

### HOCMDNAME

- **格式**：`A:n`
- **範例**：`&quot;A:5 {allez}&quot;`
- **使用於**：S4F29 handoff command identifier

### HOHALTACK

- **格式**：`U1:1`
- **說明**：Used by: S4F41 hand off halt ack
- **範例**：`&quot;U1:1 0&quot; 0 - ok 1 - TRLINK value unknown`
- **使用於**：S4F41 hand off halt ack

### IACDS

- **格式**：`U2:n`
- **範例**：`&quot;U2:3 50 93 46&quot;`
- **使用於**：S7F22 vector of immediately after command codes

### IBCDS

- **格式**：`U2:n`
- **範例**：`&quot;U2:2 24 87&quot;`
- **使用於**：S7F22 vector of immediately before command codes

### IDTYP

- **格式**：`B:1`
- **說明**：Used by: S12F1 S12F3 S12F4 S12F5 S12F7 S12F9 S12F11 S12F13 S12F14 S12F15 S12F16 S12F17 S12F18 ID type &nbs
- **使用於**：S12F1 S12F3 S12F4 S12F5 S12F7 S12F9 S12F11 S12F13 S12F14 S12F15 S12F16 S12F17 S12F18 ID type &nbs

### INPTN

- **格式**：`B:1`
- **說明**：Used by: S3F35 input material port number
- **範例**：`&quot;B:1 0x00&quot;`
- **使用於**：S3F35 input material port number

### ITEMACK

- **格式**：`B:1`
- **說明**：Used by: S21F2 S21F4 S21F6 S21F8 S21F10 S21F12 S21F12 S21F14 S21F16 S21F18 S21F20 item request return code
- **範例**：`&quot;B:1 0&quot; 0 - ok 1 - invalid item type 2`
- **使用於**：S21F2 S21F4 S21F6 S21F8 S21F10 S21F12 S21F12 S21F14 S21F16 S21F18 S21F20 item request return code

### ITEMERROR

- **格式**：`A:1024`
- **說明**：Used by: S21F2 S21F4 S21F6 S21F8 S21F10 S21F12 S21F14 S21F16 S21F18 S21F20 error description, empty on success
- **範例**：`&quot;A:31 {file system directory not found}&quot;`
- **使用於**：S21F2 S21F4 S21F6 S21F8 S21F10 S21F12 S21F14 S21F16 S21F18 S21F20 error description, empty on success

### ITEMID

- **格式**：`A:256`
- **說明**：Used by: S21F1 S21F3 S21F5 S21F6 S21F8 S21F11 S21F12 S21F13 S21F15 S21F16 S21F17 item identifier
- **範例**：`&quot;A:26 conditionHiTempChamber.rcp&quot;`
- **使用於**：S21F1 S21F3 S21F5 S21F6 S21F8 S21F11 S21F12 S21F13 S21F15 S21F16 S21F17 item identifier

### ITEMINDEX

- **格式**：`U4:1`
- **說明**：Used by: S21F17 1-based index of a component part, 0 means done, 0xFFFFFFFF means abort
- **範例**：`&quot;U4:1 1&quot;`
- **使用於**：S21F17 1-based index of a component part, 0 means done, 0xFFFFFFFF means abort

### ITEMLENGTH

- **格式**：`U4:1`
- **範例**：`&quot;U4:1 15440&quot;`
- **使用於**：S21F1 S21F3 S21F6 S21F8 S21F13 S21F16 S21F17 sum of item part lengths in bytes, not a message length, type U4 or U8

### ITEMPART

- **格式**：`A:n`
- **範例**：`&quot;A:0 &quot;`
- **使用於**：S21F3 S21F6 S21F17 component part of an item, may be data type A:n or B:n

### ITEMPARTCOUNT

- **格式**：`U4:1`
- **說明**：Used by: S21F13 S21F16 S21F17 total number of item parts as split for transfer
- **範例**：`&quot;U4: 16&quot;`
- **使用於**：S21F13 S21F16 S21F17 total number of item parts as split for transfer

### ITEMPARTLENGTH

- **格式**：`U4:1`
- **說明**：Used by: S21F17 length of a specific item part presumably in bytes
- **範例**：`&quot;U4: 1 1024&quot;`
- **使用於**：S21F17 length of a specific item part presumably in bytes

### ITEMTYPE

- **格式**：`A:n`
- **說明**：Used by: S21F1 S21F3 S21F5 S21F6 S21F7 S21F8 S21F10 S21F11 S21F12 S21F13 S21F15 S21F16 S21F17 S21F19<
- **使用於**：S21F1 S21F3 S21F5 S21F6 S21F7 S21F8 S21F10 S21F11 S21F12 S21F13 S21F15 S21F16 S21F17 S21F19<

### ITEMTYPESUPPORT

- **格式**：`U4:1`
- **說明**：Used by: S21F20 bitfield to specify which S21Fx messages accepted
- **範例**：`&quot;U4:1 0&quot; 1 - bit 1, S21F1 Item Load Inquire 2 - bit 2, S21F3 Item Send 4 - bit 3, S21F5 It`
- **使用於**：S21F20 bitfield to specify which S21Fx messages accepted

### ITEMVERSION

- **格式**：`A:n`
- **說明**：Used by: S21F1 S21F3 S21F6 S21F8 S21F13 S21F16 S21F17 version value, empty for unknown, default is time last modified YYYYMMDDhhmmsscc in the equipment timezone
- **範例**：`&quot;A:16 2020071613213423&quot;`
- **使用於**：S21F1 S21F3 S21F6 S21F8 S21F13 S21F16 S21F17 version value, empty for unknown, default is time last modified YYYYMMDDhhmmsscc in the equipment timezone

### JOBACTION

- **格式**：`A:n`
- **說明**：Used by: S3F35 reticle transfer command
- **範例**：`&quot;A:8 {simulate}&quot;`
- **使用於**：S3F35 reticle transfer command

### LENGTH

- **格式**：`U4:1`
- **說明**：Used by: S2F1 S7F1 S7F29 program length in bytes
- **範例**：`&quot;U4:1 322&quot;`
- **使用於**：S2F1 S7F1 S7F29 program length in bytes

### LIMITACK

- **格式**：`B:1`
- **說明**：Used by: S2F46 variable limit value error code
- **範例**：`&quot;B:1 0x01&quot; 1 - LIMITID does not exist 2 - UPPERDB> LIMITMAX 3 - LOWERDB 4 - UPPERDB 5 - il`
- **使用於**：S2F46 variable limit value error code

### LIMITID

- **格式**：`B:1`
- **說明**：Used by: S2F45 S2F46 S2F48 identifies a specific limit
- **範例**：`&quot;B:1 0x00&quot;`
- **使用於**：S2F45 S2F46 S2F48 identifies a specific limit

### LIMITMAX

- **格式**：`F4:1`
- **範例**：`&quot;F4:1 250.0&quot;`
- **使用於**：S2F48 The maximum value allowed for the upper dead band limit

### LIMITMIN

- **格式**：`F4:1`
- **範例**：`&quot;F4:1 20.0&quot;`
- **使用於**：S2F48 The minimum value allowed for the lower dead band limit

### LINKID

- **格式**：`U4:1`
- **說明**：Used by: S6F25 S14F20 S14F21 S15F22 S15F30 correlates the RMOPID value in a request to a completion report
- **範例**：`&quot;U4:1 1&quot;`
- **使用於**：S6F25 S14F20 S14F21 S15F22 S15F30 correlates the RMOPID value in a request to a completion report

### LOC

- **格式**：`B:1`
- **說明**：Used by: S2F27 S3F2 material location code
- **範例**：`&quot;B:1 0x00&quot;`
- **使用於**：S2F27 S3F2 material location code

### LOCID

- **格式**：`A:n`
- **範例**：`&quot;A {LP1}&quot;`
- **使用於**：S3F29 S3F31 logical ID of carrier location, E87 requires text

### LOWERDB

- **格式**：`F4:1`
- **範例**：`&quot;F4:1 183.0&quot;`
- **使用於**：S2F45 S2F48 the lower bound of a deadband limit

### LRACK

- **格式**：`B:1`
- **說明**：Used by: S2F36 link report acknowledge
- **範例**：`&quot;B:1 0x00&quot; 0 - ok 1 - out of space 2 - invalid format 3 - 1 or more CEID links already def`
- **使用於**：S2F36 link report acknowledge

### LVACK

- **格式**：`B:1`
- **說明**：Used by: S2F46 variable limit error code
- **範例**：`&quot;B:1 0x01&quot; 1 - no such VID 2 - limits not support for VID 3 - variable repeated 4 - limit `
- **使用於**：S2F46 variable limit error code

### MAPER

- **格式**：`B:1`
- **說明**：Used by: S12F19 map error
- **範例**：`&quot;B:1 0x00&quot; 0 - ID not found 1 - invalid data 2 - format error`
- **使用於**：S12F19 map error

### MAPFT

- **格式**：`B:1`
- **說明**：Used by: S12F3 S12F5 map data format type
- **範例**：`&quot;B:1 0x00&quot; 0 - row format 1 - array format 2 - coordinate format`
- **使用於**：S12F3 S12F5 map data format type

### MAXNUMBER

- **格式**：`U2:1`
- **說明**：Used by: S20F25 subspace maximum
- **範例**：`&quot;U2:1 128&quot;`
- **使用於**：S20F25 subspace maximum

### MAXTIME

- **格式**：`U2:1`
- **說明**：Used by: S20F25 maximum minutes for a PEM recipe to be preserved in PRC post use, 0 means NA
- **範例**：`&quot;U2:1 128&quot;`
- **使用於**：S20F25 maximum minutes for a PEM recipe to be preserved in PRC post use, 0 means NA

### MCINDEX

- **格式**：`U4:1`
- **範例**：`&quot;U4:1 1001&quot;`
- **使用於**：S4F29 S4F31 correlation value for handoff command

### MDACK

- **格式**：`B:1`
- **說明**：Used by: S12F8 S12F10 S12F12 map data ack
- **範例**：`&quot;B:1 0x00&quot; 0 - ok 1 - format error 2 - no ID match 3 - abort/discard map`
- **使用於**：S12F8 S12F10 S12F12 map data ack

### MDLN

- **格式**：`A:20`
- **說明**：Used by: S1F2 S1F13 S1F14 S7F22 S7F23 S7F26 S7F31 equipment model type
- **範例**：`&quot;A:6 gemsim&quot;`
- **使用於**：S1F2 S1F13 S1F14 S7F22 S7F23 S7F26 S7F31 equipment model type

### MEXP

- **格式**：`A:6`
- **說明**：Used by: S9F13 message expected in form of SxxFyy
- **範例**：`&quot;A:6 S07F02&quot;`
- **使用於**：S9F13 message expected in form of SxxFyy

### MF

- **格式**：`B:1`
- **範例**：`&quot;B:1 0x01&quot; 1 - wafers 2 - cassettes 3 - die 4 - boats 5 - ingots 6 - leadframes 7 - lots 8`
- **使用於**：S3F2 S3F4 S3F5 S3F7 S16F3 S16F11 S16F15 material format code, ASCII indicates generic units, E40 restricts to B:1

### MHEAD

- **格式**：`B:10`
- **說明**：Used by: S9F1 S9F3 S9F5 S9F7 S9F11 message header of received block
- **範例**：`&quot;B:10 0x00 0x00 0x01 0x02 0x00 0x00 0x00 0x00 0x00 0x0f &quot;`
- **使用於**：S9F1 S9F3 S9F5 S9F7 S9F11 message header of received block

### MID

- **格式**：`A:16`
- **使用於**：S2F27 S3F2 S3F4 S3F7 S3F9 S3F12 S3F13 S4F1 S4F3 S4F5 S4F7 S4F9 S4F11 S4F13 S4F

### MIDAC

- **格式**：`B:1`
- **說明**：Used by: S3F14 material ID ack
- **範例**：`&quot;B:1 0x00&quot; 0 - ok 1 - invalid port 2 - material not present`
- **使用於**：S3F14 material ID ack

### MIDRA

- **格式**：`B:1`
- **說明**：Used by: S3F12 material ID Ack code
- **範例**：`&quot;B:1 0x01&quot; 0 - MID follows 1 - no MID 2 - MID to be sent in S3F13`
- **使用於**：S3F12 material ID Ack code

### MLCL

- **格式**：`U4:1`
- **範例**：`&quot;U4:1 656&quot;`
- **使用於**：S12F4 S12F5 message length in bytes

### MMODE

- **格式**：`B:1`
- **說明**：Used by: S7F15 matrix mode selection
- **範例**：`&quot;B:1 0x00&quot; 1 - host source 2 - local source 3 - host immediate`
- **使用於**：S7F15 matrix mode selection

### NACDS

- **格式**：`U2:n`
- **範例**：`&quot;U2:0&quot;`
- **使用於**：S7F22 vector of not after command codes

### NBCDS

- **格式**：`U2:n`
- **範例**：`&quot;U2:0&quot;`
- **使用於**：S7F22 vector of not before command codes

### NULBC

- **格式**：`A:n`
- **範例**：`&quot;A:1 {x}&quot;`
- **使用於**：S12F1 S12F3 S12F4 null bin code value

### OBJACK

- **格式**：`U1:1`
- **說明**：Used by: S14F2 S14F4 S14F6 S14F8 S14F10 S14F12 S14F14 S14F16 S14F18 S14F26 S14F28 acknowledge code, 0 ok, 1 error
- **範例**：`&quot;U1:1 0&quot;`
- **使用於**：S14F2 S14F4 S14F6 S14F8 S14F10 S14F12 S14F14 S14F16 S14F18 S14F26 S14F28 acknowledge code, 0 ok, 1 error

### OBJCMD

- **格式**：`U1:1`
- **說明**：Used by: S14F15 S14F17 Specifies an action to be performed by an object
- **範例**：`&quot;U1:1 1&quot; 1 - Attach to requestor 2 - detach from requestor 3 - reattach to requestor 4 - s`
- **使用於**：S14F15 S14F17 Specifies an action to be performed by an object

### OBJID

- **格式**：`A:80`
- **使用於**：S1F19 S14F1 S14F2 S14F3 S14F4 S20F1 S20F3 S20F5 S20F7 S20F9 S20F11 S20F13 S20F15 S20F17 <a href="msg

### OBJSPEC

- **格式**：`A:n`
- **使用於**：S2F49 S13F11 S13F13 S13F15 S14F1 S14F3 S14F5 S14F7 S14F9 S14F10 S14F11 S14F13 S14F15 S14F17 <

### OBJTOKEN

- **格式**：`U4:1`
- **說明**：Used by: S14F14 S14F15 S15F37 S15F39 S15F41 token used for authorization
- **範例**：`&quot;U4:1 34896&quot;`
- **使用於**：S14F14 S14F15 S15F37 S15F39 S15F41 token used for authorization

### OBJTYPE

- **格式**：`A:40`
- **使用於**：S1F19 S14F1 S14F3 S14F6 S14F7 S14F8 S14F9 S14F25 S14F26 S14F27 S20F1 S20F3 S20F5 S20F7 <a href="m

### OCEACK

- **格式**：`U1:1`
- **說明**：Used by: S20F6 event completion code
- **範例**：`&quot;U1:1 0&quot; 0 - done 1 - OK 2 - NG 3 - Error`
- **使用於**：S20F6 event completion code

### OFLACK

- **格式**：`B:1`
- **說明**：Used by: S1F16 offline acknowledge, 0 ok
- **範例**：`&quot;B:1 0x00&quot;`
- **使用於**：S1F16 offline acknowledge, 0 ok

### ONLACK

- **格式**：`B:1`
- **說明**：Used by: S1F18 online acknowledge, 0 ok
- **範例**：`&quot;B:1 0x00&quot; 0 - ok 1 - refused 2 - already online`
- **使用於**：S1F18 online acknowledge, 0 ok

### OPEID

- **格式**：`A:16`
- **說明**：Used by: S20F4 S20F5 S20F6 S20F7 S20F8 S20F9 S20F11 S20F12 S20F13 S20F13 S20F15 S20F15 S20F17 S20F17 <
- **使用於**：S20F4 S20F5 S20F6 S20F7 S20F8 S20F9 S20F11 S20F12 S20F13 S20F13 S20F15 S20F15 S20F17 S20F17 <

### OPETYPE

- **格式**：`U1:1`
- **說明**：Used by: S20F3 S20F5 S20F7 S20F9 S20F11 S20F13 S20F15 S20F17 S20F19 S20F21 S20F23 S20F27 S20F29 S20F
- **使用於**：S20F3 S20F5 S20F7 S20F9 S20F11 S20F13 S20F15 S20F17 S20F19 S20F21 S20F23 S20F27 S20F29 S20F

### OPID

- **格式**：`U4:1`
- **範例**：`&quot;U4:1 1&quot;`
- **使用於**：S6F25 S14F19 S14F21 S15F21 S15F29 S15F30 S15F37 S15F41 S15F44 S15F46 operation identifier

### ORLOC

- **格式**：`B:1`
- **說明**：Used by: S12F1 S12F3 S12F4 origin location
- **範例**：`&quot;B:1 0x00&quot; 0 - center die of wafer 1 - upper right 2 - upper left 3 - lower left 4 - lower`
- **使用於**：S12F1 S12F3 S12F4 origin location

### OUTPTN

- **格式**：`B:1`
- **範例**：`&quot;B:1 0x02&quot;`
- **使用於**：S3F35 output port (PTN)

### PARAMNAME

- **格式**：`A:n`
- **說明**：Used by: S3F23 S3F25 argument name
- **範例**：`&quot;A ServiceStatus&quot;`
- **使用於**：S3F23 S3F25 argument name

### PARAMVAL

- **格式**：`U1:1`
- **範例**：`&quot;U1:1 1&quot;`
- **使用於**：S3F23 S3F25 argument value, only defined use is ServiceStatus, 0 = OUT OF SERVICE, 1 = IN SERVICE

### PDEATTRIBUTE

- **格式**：`U1:1`
- **說明**：Used by: S19F1 S19F2 a reportable PDE attribute type, not necessarily useable in a filter expression
- **範例**：`&quot;U1:1 1&quot; 0 - RESERVED 1 - name 2 - gid 3 - groupName 4 - description 5 - type 6 - executab`
- **使用於**：S19F1 S19F2 a reportable PDE attribute type, not necessarily useable in a filter expression

### PDEATTRIBUTENAME

- **格式**：`U1:1`
- **說明**：Used by: S19F1 identifies a PDE attribute type
- **範例**：`&quot;U1:1 1&quot; 0 - RESERVED 1 - name 2 - gid 3 - groupName 4 - description 5 - type 6 - executab`
- **使用於**：S19F1 identifies a PDE attribute type

### PDEATTRIBUTEVALUE

- **格式**：`A`
- **範例**：`&quot;A {2007-12-31T17:00:03}&quot;`
- **使用於**：S19F1 S19F2 contains the value of a PDE Attribute, may be type L, A, TF, U1

### PDEREF

- **格式**：`A:36`
- **說明**：Used by: S19F15 S19F16 S19F17 The UID of a PDE or of a PDE group formatted as a 36 character string with runs of 8, 4, 4, 4, and 12 characters joined by hyphens.
- **範例**：`&quot;A:36 {12345678-1234-1234-1234-123456789ABC}&quot;`
- **使用於**：S19F15 S19F16 S19F17 The UID of a PDE or of a PDE group formatted as a 36 character string with runs of 8, 4, 4, 4, and 12 characters joined by hyphens.

### PECEACK

- **格式**：`U1:1`
- **說明**：Used by: S20F32 event completion code
- **範例**：`&quot;U1:1 0&quot; 0 - Done 1 - Error`
- **使用於**：S20F32 event completion code

### PECRSLT

- **格式**：`U1:1`
- **說明**：Used by: S20F32 RMS result
- **範例**：`&quot;U1:1 0&quot; 0 - Pass 1 - Fail`
- **使用於**：S20F32 RMS result

### PFCD

- **格式**：`B:1`
- **說明**：Used by: S6F9 predefined form selector
- **範例**：`&quot;B:1 0x02&quot;`
- **使用於**：S6F9 predefined form selector

### PGRPACTION

- **格式**：`A:n`
- **說明**：Used by: S3F23 port group command, an alias for PORTACTION?
- **範例**：`&quot;A CancelReservationAtPort&quot;`
- **使用於**：S3F23 port group command, an alias for PORTACTION?

### PODID

- **格式**：`A:n`
- **說明**：Used by: S3F35 OBJSPEC for a Pod instance
- **範例**：`&quot;A:10 {0000000124}&quot;`
- **使用於**：S3F35 OBJSPEC for a Pod instance

### PORTACTION

- **格式**：`A:n`
- **說明**：Used by: S3F25 ChangeServiceStatus, CancelReservationAtPort or ReserveAtPort
- **範例**：`&quot;A ChangeServiceStatus&quot;`
- **使用於**：S3F25 ChangeServiceStatus, CancelReservationAtPort or ReserveAtPort

### PORTGRPNAME

- **格式**：`A:n`
- **說明**：Used by: S3F21 S3F23 name of a group of ports
- **範例**：`&quot;A:7 buffer1&quot;`
- **使用於**：S3F21 S3F23 name of a group of ports

### PPARM

- **格式**：`A:n`
- **範例**：`&quot;A:5 {185.0}&quot;`
- **使用於**：S7F23 S7F26 S7F31 process parameter, any scalar or vector

### PPBODY

- **格式**：`B:n`
- **範例**：`&quot;B:0&quot;`
- **使用於**：S7F3 S7F6 S7F36 process program data, any non-list type

### PPGNT

- **格式**：`B:1`
- **說明**：Used by: S7F2 S7F30 process program transfer grant status
- **範例**：`&quot;B:1 0x00&quot; 0 - Ok 1 - already have 2 - no space 3 - invalid PPID 4 - busy, try later 5 - w`
- **使用於**：S7F2 S7F30 process program transfer grant status

### PPID

- **格式**：`A:80`
- **使用於**：S2F27 S7F1 S7F3 S7F5 S7F6 S7F8 S7F10 S7F11 S7F13 S7F17 S7F20 S7F23 S7F25 S7F26 <a href="msgs.html#S7F27" target

### PRAXI

- **格式**：`B:1`
- **說明**：Used by: S12F1 S12F3 process access
- **範例**：`&quot;B:1 0x00&quot;`
- **使用於**：S12F1 S12F3 process access

### PRCMDNAME

- **格式**：`A:6`
- **說明**：Used by: S16F5 process job commands, START, STOP, PAUSE, RESUME, ABORT, CANCEL
- **範例**：`&quot;A:6 CANCEL&quot;`
- **使用於**：S16F5 process job commands, START, STOP, PAUSE, RESUME, ABORT, CANCEL

### PRCPREEXECHK

- **格式**：`U1:1`
- **說明**：Used by: S20F25 Enable Pre-Execution checking
- **範例**：`&quot;U1:1 0&quot; 0 - disabled 1 - enabled`
- **使用於**：S20F25 Enable Pre-Execution checking

### PRDCT

- **格式**：`U4:1`
- **範例**：`&quot;U4:1 432&quot;`
- **使用於**：S12F1 S12F4 process die count

### PREACK

- **格式**：`U1:1`
- **說明**：Used by: S20F24 event completion code
- **範例**：`&quot;U1:1 0&quot; 0 - done 1 - error`
- **使用於**：S20F24 event completion code

### PREVENTID

- **格式**：`U1:1`
- **範例**：`&quot;U1:1 1&quot; 1 - waiting for material 2 - job state change`
- **使用於**：S16F9 process job event ID

### PRJOBID

- **格式**：`A:n`
- **說明**：Used by: S16F4 S16F5 S16F6 S16F7 S16F9 S16F11 S16F12 S16F15 S16F16 S16F17 S16F18 S16F20 S16F23 S16F25<
- **使用於**：S16F4 S16F5 S16F6 S16F7 S16F9 S16F11 S16F12 S16F15 S16F16 S16F17 S16F18 S16F20 S16F23 S16F25<

### PRJOBMILESTONE

- **格式**：`U1:1`
- **範例**：`&quot;U1:1 1&quot; 1 - setup 2 - in process 3 - processing complete 4 - job complete 5 - waiting for`
- **使用於**：S16F7 process job status

### PRJOBSPACE

- **格式**：`U2:1`
- **說明**：Used by: S16F22 the number of process jobs that can be created
- **範例**：`&quot;U2:1 32&quot;`
- **使用於**：S16F22 the number of process jobs that can be created

### PRMTRLORDER

- **格式**：`U1:1`
- **說明**：Used by: S16F29 ordering method for pending process jobs
- **範例**：`&quot;U1:1&quot; 1 - arrival sequence 2 - maximize throughput 3 - listed sequence`
- **使用於**：S16F29 ordering method for pending process jobs

### PRPAUSEEVENTID

- **格式**：`U4:1`
- **範例**：`&quot;U4:1 1501&quot;`
- **使用於**：S16F11 S16F15 an event identifier for which a process job should be paused

### PRPROCESSSTART

- **格式**：`TF:1`
- **說明**：Used by: S16F3 S16F11 S16F15 S16F25 automatic start flag, false implies manual start
- **範例**：`&quot;TF:1 1&quot;`
- **使用於**：S16F3 S16F11 S16F15 S16F25 automatic start flag, false implies manual start

### PRRECIPEMETHOD

- **格式**：`U1:1`
- **說明**：Used by: S16F3 S16F11 S16F15 recipe type
- **範例**：`&quot;U1:1 0&quot; 1 - recipe without variable tuning 2 - recipe with variable tuning`
- **使用於**：S16F3 S16F11 S16F15 recipe type

### PRSTATE

- **格式**：`U1:1`
- **說明**：Used by: S16F20 process job state, E40 definition
- **範例**：`&quot;U1:1 1&quot; 0 - Queued/pooled 1 - setting up 2 - waiting for start 3 - processing 4 - process`
- **使用於**：S16F20 process job state, E40 definition

### PSRACK

- **格式**：`U1:1`
- **說明**：Used by: S20F28 service completion code
- **範例**：`&quot;U1:1 0&quot; 0 - done 1 - error`
- **使用於**：S20F28 service completion code

### PSREACK

- **格式**：`U1:1`
- **說明**：Used by: S20F34 event completion code
- **範例**：`&quot;U1:1 0&quot; 0 - done 1 - error`
- **使用於**：S20F34 event completion code

### PTN

- **格式**：`U1:1`
- **使用於**：S3F11 S3F12 S3F13 S3F17 S3F21 S3F25 S3F27 S3F28 S4F1 S4F3 S4F5 S4F7 S4F9 S4F11 <a href="msgs.html#S4F13" target="

### QPRKEACK

- **格式**：`U1:1`
- **說明**：Used by: S20F30 event completion code
- **範例**：`&quot;U1:1 0&quot; 0 - done 1 - error`
- **使用於**：S20F30 event completion code

### QREACK

- **格式**：`U1:1`
- **說明**：Used by: S20F22 event completion code
- **範例**：`&quot;U1:1 0&quot; 0 - done 1 - error`
- **使用於**：S20F22 event completion code

### QRXLEACK

- **格式**：`U1:1`
- **說明**：Used by: S20F20 event completion code
- **範例**：`&quot;U1:1 0&quot; 0 - done 1 - error`
- **使用於**：S20F20 event completion code

### QUA

- **格式**：`B:1`
- **說明**：Used by: S3F2 S3F4 S3F5 S3F7 quantity (format limits max to 255!)
- **範例**：`&quot;B:1 0x18&quot;`
- **使用於**：S3F2 S3F4 S3F5 S3F7 quantity (format limits max to 255!)

### RAC

- **格式**：`U1:1`
- **範例**：`&quot;U1:1 1&quot; 0 - ok 1 - denied`
- **使用於**：S2F20 reset acknowledge

### RCMD

- **格式**：`A:n`
- **範例**：`&quot;A:5 {pause}&quot;`
- **使用於**：S2F21 S2F41 S2F49 remote command, GEM requires a maximum length of 20 printable characters, taken from hex 21-7E (no spaces)

### RCPATTRDATA

- **格式**：`A:n`
- **範例**：`&quot;A:3 150&quot;`
- **使用於**：S6F25 S15F13 S15F15 S15F15 S15F18 S15F18 S15F27 S15F28 S15F30 S15F32 the value of a recipe attribute, any type of data including list

### RCPATTRID

- **格式**：`A:n`
- **說明**：Used by: S6F25 S15F13 S15F15 S15F15 S15F18 S15F18 S15F27 S15F28 S15F30 S15F32 the name of a recipe attribute, but not used to indicate the recipe identifier
- **範例**：`&quot;A:7 {author}&quot;`
- **使用於**：S6F25 S15F13 S15F15 S15F15 S15F18 S15F18 S15F27 S15F28 S15F30 S15F32 the name of a recipe attribute, but not used to indicate the recipe identifier

### RCPBODY

- **格式**：`B:n`
- **使用於**：S15F13 S15F15 S15F18 S15F27 S15F32 Recipe body

### RCPBODYA

- **格式**：`A:n`
- **範例**：`&quot;A:0&quot;`
- **使用於**：S20F15 S20F18 S20F23 S20F32 user defined recipe body, list allowed

### RCPCLASS

- **格式**：`A:n`
- **說明**：Used by: S15F11 Recipe class
- **使用於**：S15F11 Recipe class

### RCPCMD

- **格式**：`U1:1`
- **說明**：Used by: S15F21 S15F22 recipe action
- **範例**：`&quot;U1:1 5&quot; 5 - delete 8 - unprotect 9 - protect 10 - verify 11 - link 12 - unlink 13 - certi`
- **使用於**：S15F21 S15F22 recipe action

### RCPDEL

- **格式**：`U1:1`
- **說明**：Used by: S15F35 recipe action
- **範例**：`&quot;U1:1 0&quot; 0 - delete 1 - deselect`
- **使用於**：S15F35 recipe action

### RCPDESCLTH

- **格式**：`U4:1`
- **範例**：`&quot;U4:1 15645&quot;`
- **使用於**：S15F24 the byte length of a recipe

### RCPDESCNM

- **格式**：`A:n`
- **說明**：Used by: S15F24 Identifies a descriptor type "ASDesc", "BodyDesc", "GenDesc"
- **範例**：`&quot;A:6 ASDesc&quot;`
- **使用於**：S15F24 Identifies a descriptor type "ASDesc", "BodyDesc", "GenDesc"

### RCPDESCTIME

- **格式**：`A:16`
- **說明**：Used by: S15F24 timestamp of a recipe section "YYYYMMDDhhmmsscc"
- **範例**：`&quot;A:16 2006081515441233&quot;`
- **使用於**：S15F24 timestamp of a recipe section "YYYYMMDDhhmmsscc"

### RCPID

- **格式**：`A:n`
- **說明**：Used by: S15F21 S15F23 S15F28 S15F29 S15F30 S15F33 S15F35 S15F37 S15F41 S15F44 S15F53 recipe identifier conforming to OBJSPEC
- **使用於**：S15F21 S15F23 S15F28 S15F29 S15F30 S15F33 S15F35 S15F37 S15F41 S15F44 S15F53 recipe identifier conforming to OBJSPEC

### RCPNAME

- **格式**：`A:n`
- **說明**：Used by: S15F11 recipe name
- **範例**：`&quot;A:22 HU:me:cmos70nm-default&quot;`
- **使用於**：S15F11 recipe name

### RCPNEWID

- **格式**：`A:n`
- **說明**：Used by: S15F19 S15F41 S15F44 S15F45 the new recipe identifier
- **使用於**：S15F19 S15F41 S15F44 S15F45 the new recipe identifier

### RCPOWCODE

- **格式**：`TF:1`
- **說明**：Used by: S15F27 S15F49 recipe overwrite code, true=overwrite ok, false=do not overwrite
- **範例**：`&quot;TF:1 1&quot;`
- **使用於**：S15F27 S15F49 recipe overwrite code, true=overwrite ok, false=do not overwrite

### RCPPARNM

- **格式**：`A:256`
- **說明**：Used by: S15F25 S15F33 S16F3 S16F11 S16F15 S16F23 the name of a recipe variable parameter
- **範例**：`&quot;A:15 {AcclimationTime}&quot;`
- **使用於**：S15F25 S15F33 S16F3 S16F11 S16F15 S16F23 the name of a recipe variable parameter

### RCPPARRULE

- **格式**：`A:80`
- **說明**：Used by: S15F25 the restrictions applied to a recipe variable parameter setting
- **使用於**：S15F25 the restrictions applied to a recipe variable parameter setting

### RCPPARVAL

- **格式**：`A:80`
- **範例**：`&quot;A:4 0.52&quot;`
- **使用於**：S15F25 S15F33 S16F3 S16F11 S16F15 S16F23 the value of a recipe variable parameter, any scalar format type

### RCPRENAME

- **格式**：`TF:1`
- **說明**：Used by: S15F19 whether a recipe is to be renamed (TRUE) or copied (FALSE)
- **範例**：`&quot;TF:1 1&quot;`
- **使用於**：S15F19 whether a recipe is to be renamed (TRUE) or copied (FALSE)

### RCPSECCODE

- **格式**：`B:1`
- **說明**：Used by: S15F15 S15F16 S15F17 indicates the sections of a recipe
- **範例**：`&quot;B 3&quot; 1 - generic attributes only 3 - generic attributes and body 4 - all agent specific d`
- **使用於**：S15F15 S15F16 S15F17 indicates the sections of a recipe

### RCPSECNM

- **格式**：`A:n`
- **說明**：Used by: S15F15 S15F15 S15F18 S15F18 Recipe section name, "Generic", "Body", "ASDS"
- **範例**：`&quot;A:4 Body&quot;`
- **使用於**：S15F15 S15F15 S15F18 S15F18 Recipe section name, "Generic", "Body", "ASDS"

### RCPSPEC

- **格式**：`A:n`
- **說明**：Used by: S6F25 S15F1 S15F9 S15F13 S15F15 S15F17 S15F19 S15F27 S15F31 S15F32 S15F45 S15F53 S16F3 S16F1
- **使用於**：S6F25 S15F1 S15F9 S15F13 S15F15 S15F17 S15F19 S15F27 S15F31 S15F32 S15F45 S15F53 S16F3 S16F1

### RCPSTAT

- **格式**：`U1:1`
- **說明**：Used by: S15F10 Recipe status code
- **範例**：`&quot;U1:1 0&quot; 0 - does not exist 8 - unprotected 9 - protected`
- **使用於**：S15F10 Recipe status code

### RCPUPDT

- **格式**：`TF:1`
- **說明**：Used by: S15F13 true for a recipe update, false for create
- **範例**：`&quot;TF:1 1&quot;`
- **使用於**：S15F13 true for a recipe update, false for create

### RCPVERS

- **格式**：`A:n`
- **說明**：Used by: S15F10 S15F12 recipe version
- **使用於**：S15F10 S15F12 recipe version

### READLN

- **格式**：`U4:1`
- **範例**：`&quot;U4:1 16000&quot;`
- **使用於**：S13F5 maximum number of bytes or characters to read

### REAPER

- **格式**：`A:80`
- **說明**：Used by: S4F27 transfer hand-off reader PER configuration
- **範例**：`&quot;A:25 {thguac tog nc.moc.tobonis}&quot;`
- **使用於**：S4F27 transfer hand-off reader PER configuration

### RECLEN

- **格式**：`U4:1`
- **範例**：`&quot;U4:1 1024&quot;`
- **使用於**：S13F4 maximum number of bytes or characters in a discrete record

### REFP

- **格式**：`I4:2`
- **範例**：`&quot;I4:2 0 0&quot;`
- **使用於**：S12F1 S12F4 x y reference point

### REPGSZ

- **格式**：`U4:1`
- **範例**：`&quot;U4:1 5&quot;`
- **使用於**：S2F23 S17F5 reporting group size, TOTSMP modulo REPGSZ should be 0

### RESOLUTION

- **格式**：`A:36`
- **說明**：Used by: S19F15 S19F16 S19F17 the UID of a PDE
- **範例**：`&quot;A:36 {B62C4E8D-62CC-404B-BBBF-BF3E3BBB137}&quot;`
- **使用於**：S19F15 S19F16 S19F17 the UID of a PDE

### RESPDESTAT

- **格式**：`U1:1`
- **說明**：Used by: S19F16 status codes for PDE resolution
- **範例**：`&quot;U1:1 0&quot; 0 - OK 1 - invalid input map 2 - map not found 3 - PDE reference resolution was d`
- **使用於**：S19F16 status codes for PDE resolution

### RESPEC

- **格式**：`A:n`
- **說明**：Used by: S15F29 S15F33 S15F35 object specifier for the recipe executor
- **使用於**：S15F29 S15F33 S15F35 object specifier for the recipe executor

### RETAINRECIPE_DISABLE

- **格式**：`U1:1`
- **說明**：Used by: S20F1 Disable automatic retention of recipes on disconnect
- **範例**：`&quot;U1:1 0&quot; 0 - retention is not disabled 1 - retention is disabled`
- **使用於**：S20F1 Disable automatic retention of recipes on disconnect

### RETICLEID

- **格式**：`A:n`
- **說明**：Used by: S3F35 OBJSPEC value for a reticle
- **範例**：`&quot;A:0&quot;`
- **使用於**：S3F35 OBJSPEC value for a reticle

### RETICLEID2

- **格式**：`A:n`
- **說明**：Used by: S3F35 OBJSPEC value for a second reticle
- **範例**：`&quot;A:0&quot;`
- **使用於**：S3F35 OBJSPEC value for a second reticle

### RETPLACEINSTR

- **格式**：`U1:1`
- **說明**：Used by: S3F35 pod slot reticle place instruction
- **範例**：`&quot;U1:1 0&quot; 0 - place 1 - skip 2 - occupied`
- **使用於**：S3F35 pod slot reticle place instruction

### RETREMOVEINSTR

- **格式**：`U1:1`
- **說明**：Used by: S3F35 pod slot reticle remove instruction
- **範例**：`&quot;U1:1 0&quot; 0 - remove 1 - skip`
- **使用於**：S3F35 pod slot reticle remove instruction

### REVID

- **格式**：`A:256`
- **說明**：Used by: S20F12 S20F13 S20F15 S20F17 S20F18 S20F20 S20F21 S20F23 S20F27 S20F30 S20F32 recipe revision information related to SRO
- **範例**：`&quot;A:0&quot;`
- **使用於**：S20F12 S20F13 S20F15 S20F17 S20F18 S20F20 S20F21 S20F23 S20F27 S20F30 S20F32 recipe revision information related to SRO

### RIC

- **格式**：`U1:1`
- **範例**：`&quot;U1:1 1&quot;`
- **使用於**：S2F19 reset code, 1 means power up reset

### RMACK

- **格式**：`U1:1`
- **說明**：Used by: S6F25 S15F4 S15F6 S15F8 S15F10 S15F12 S15F14 S15F16 S15F18 S15F20 S15F22 S15F24 S15F26 S15F28</
- **使用於**：S6F25 S15F4 S15F6 S15F8 S15F10 S15F12 S15F14 S15F16 S15F18 S15F20 S15F22 S15F24 S15F26 S15F28</

### RMCHGSTAT

- **格式**：`U4:1`
- **範例**：`&quot;U4:1 1&quot; 0 - no change 1 - created 2 - updated 3 - stored (new) 4 - replaced 5 - deleted 6`
- **使用於**：S6F25 object change type

### RMCHGTYPE

- **格式**：`U4:1`
- **使用於**：S15F37 S15F41 S15F44 S15F45 S15F46 type of change for a recipe 0 - no change 1 - create 2 - update 5 - delete 6 - copy (new object) 7 - rename 8 - unprotect 9 - product 10 - verify 11 - link 12 - unlink 13 - certify 14 - de-certify 15 - change generic attribute 16 - change agent specific attribute 1

### RMDATASIZE

- **格式**：`U4:1`
- **範例**：`&quot;U4:1 125000&quot;`
- **使用於**：S15F1 the maximum total message body length of a SECS-II message

### RMGRNT

- **格式**：`B:1`
- **說明**：Used by: S15F2 S15F37 S15F46 grant code, 0 ok
- **範例**：`&quot;B:1 0&quot; 0 - OK 1 - not now 2 - no space 3 - request is on hold whatever that means`
- **使用於**：S15F2 S15F37 S15F46 grant code, 0 ok

### RMNEWNS

- **格式**：`A:n`
- **說明**：Used by: S15F5 new name for a recipe namespace
- **使用於**：S15F5 new name for a recipe namespace

### RMNSCMD

- **格式**：`U1:1`
- **說明**：Used by: S15F3 S15F39 S15F41 recipe namespace command
- **範例**：`&quot;U1:1 1&quot; 1 - create 5 - delete`
- **使用於**：S15F3 S15F39 S15F41 recipe namespace command

### RMNSSPEC

- **格式**：`A:n`
- **說明**：Used by: S15F3 S15F5 S15F11 S15F21 S15F25 S15F47 object id of a recipe namespace
- **使用於**：S15F3 S15F5 S15F11 S15F21 S15F25 S15F47 object id of a recipe namespace

### RMRECSPEC

- **格式**：`A:n`
- **說明**：Used by: S15F39 S15F41 S15F47 object id of a distributed recipe namespace recorder
- **使用於**：S15F39 S15F41 S15F47 object id of a distributed recipe namespace recorder

### RMREQUESTOR

- **格式**：`TF:1`
- **說明**：Used by: S15F41 S15F44 True when the initiator of a change request is an attached segment, otherwise false
- **範例**：`&quot;TF:1 1&quot;`
- **使用於**：S15F41 S15F44 True when the initiator of a change request is an attached segment, otherwise false

### RMSEGSPEC

- **格式**：`A:n`
- **說明**：Used by: S15F37 S15F39 S15F41 S15F44 S15F47 The object ID of a distributed recipe namespace segment
- **使用於**：S15F37 S15F39 S15F41 S15F44 S15F47 The object ID of a distributed recipe namespace segment

### RMSPACE

- **格式**：`U4:1`
- **使用於**：S15F8 the amount of storage available in bytes for at least one recipe

### RMSPWD

- **格式**：`A:64`
- **說明**：Used by: S20F5 password of SRO user
- **範例**：`&quot;A:4 qzzy&quot;`
- **使用於**：S20F5 password of SRO user

### RMSUSERID

- **格式**：`A:64`
- **說明**：Used by: S20F5 SRO userID
- **範例**：`&quot;A:5 admin&quot;`
- **使用於**：S20F5 SRO userID

### ROWCT

- **格式**：`U4:1`
- **範例**：`&quot;U4:1 24&quot;`
- **使用於**：S12F1 S12F4 row count in die increments

### RPMACK

- **格式**：`U1:1`
- **說明**：Used by: S3F36 reticle pod management ack code
- **範例**：`&quot;U1:1 0&quot; 0 - ok 1 - service does not exist 2 - cannot perform now 3 - non-existent paramet`
- **使用於**：S3F36 reticle pod management ack code

### RPSEL

- **格式**：`U1:1`
- **說明**：Used by: S12F1 S12F4 reference point select
- **範例**：`&quot;U1:1 0&quot;`
- **使用於**：S12F1 S12F4 reference point select

### RPTID

- **格式**：`U4:1`
- **使用於**：S2F33 S2F35 S2F52 S2F53 S2F54 S2F56 S6F11 S6F13 S6F16 S6F18 S6F19 S6F21 S6F27 S6F30 <a href="msgs.html#S

### RPTOC

- **格式**：`TF:1`
- **說明**：Used by: S17F5 send only changed data trace report flag
- **範例**：`&quot;TF:1 1&quot;`
- **使用於**：S17F5 send only changed data trace report flag

### RQCMD

- **格式**：`TF:1`
- **說明**：Used by: S7F22 flag that command is required
- **範例**：`&quot;TF:1 1&quot;`
- **使用於**：S7F22 flag that command is required

### RRACK

- **格式**：`B:1`
- **說明**：Used by: S4F18 request to receive acknowledge
- **範例**：`&quot;B:1 0x00&quot; 0 - ok 1 - invalid port 2 - material is not at port 3 - retryable busy 4 - send`
- **使用於**：S4F18 request to receive acknowledge

### RRACK_S20

- **格式**：`U1:1`
- **說明**：Used by: S20F18 service completion code
- **範例**：`&quot;U1:1 0&quot; 0 - done 1 - error`
- **使用於**：S20F18 service completion code

### RSACK

- **格式**：`B:1`
- **說明**：Used by: S4F2 ready to send acknowledge
- **範例**：`&quot;B:1 0x00&quot; 0 - ok 1 - invalid port 2 - port is occupied 3 - retryable busy 4 - receiver la`
- **使用於**：S4F2 ready to send acknowledge

### RSDA

- **格式**：`B:1`
- **說明**：Used by: S6F24 spool request reply
- **範例**：`&quot;B:1 0x00&quot; 0 - ok 1 - retryable busy 2 - no spool data`
- **使用於**：S6F24 spool request reply

### RSDC

- **格式**：`U1:1`
- **說明**：Used by: S6F23 spool request code
- **範例**：`&quot;U1:1 0&quot; 0 - transmit 1 - purge`
- **使用於**：S6F23 spool request code

### RSINF

- **格式**：`I4:3`
- **範例**：`&quot;I4:3 0 0 -1&quot;`
- **使用於**：S12F7 S12F14 starting location for row or column, x,y,direction triplet

### RSPACK

- **格式**：`B:1`
- **說明**：Used by: S2F44 spooling response
- **範例**：`&quot;B:1 0x00&quot; 0 - ok 1 - rejected`
- **使用於**：S2F44 spooling response

### RTSRSPSTAT

- **格式**：`U1:1`
- **說明**：Used by: S19F10 PDE transfer request reply code, non-zero means denied
- **範例**：`&quot;U1:1 0&quot; 0 - OK 1 - insufficient resources 2 - transfer container too large 3 - insufficie`
- **使用於**：S19F10 PDE transfer request reply code, non-zero means denied

### RTYPE

- **格式**：`U1:1`
- **範例**：`&quot;U1:1 0&quot; 0 - Stream 1 - Discrete`
- **使用於**：S13F4 type of data record

### RecID

- **格式**：`A:n`
- **說明**：Used by: S20F12 S20F13 S20F15 S20F17 S20F18 S20F20 S20F21 S20F23 S20F27 S20F30 S20F32 recipe spec or ppid
- **使用於**：S20F12 S20F13 S20F15 S20F17 S20F18 S20F20 S20F21 S20F23 S20F27 S20F30 S20F32 recipe spec or ppid

### SDACK

- **格式**：`B:1`
- **說明**：Used by: S12F2 setup data ack, 0 ok
- **範例**：`&quot;B:1 0x00&quot;`
- **使用於**：S12F2 setup data ack, 0 ok

### SDBIN

- **格式**：`B:1`
- **說明**：Used by: S12F17 send bin data flag, 0=send, else do not
- **範例**：`&quot;B:1 0x00&quot;`
- **使用於**：S12F17 send bin data flag, 0=send, else do not

### SENDRSPSTAT

- **格式**：`U1:1`
- **說明**：Used by: S19F13 Return codes for the Send PDE request, non-zero means failure
- **範例**：`&quot;U1:1 0&quot; 0 - OK 1 - insufficent resources 2 - no match for the execution target 3 - the PD`
- **使用於**：S19F13 Return codes for the Send PDE request, non-zero means failure

### SEQNUM

- **格式**：`U4:1`
- **範例**：`&quot;U4:1 1&quot;`
- **使用於**：S7F27 process program command number

### SFCD

- **格式**：`B:1`
- **說明**：Used by: S1F5 S1F7 status form code
- **範例**：`&quot;B:1 0x01&quot;`
- **使用於**：S1F5 S1F7 status form code

### SHEAD

- **格式**：`B:10`
- **說明**：Used by: S9F9 message header of sent block
- **範例**：`&quot;B:10 0x00 0x00 0x01 0x02 0x00 0x00 0x00 0x00 0x00 0x0f &quot;`
- **使用於**：S9F9 message header of sent block

### SLOTID

- **格式**：`U1:1`
- **說明**：Used by: S16F11 S16F15 slot position within a carrier
- **範例**：`&quot;U1:1 1&quot;`
- **使用於**：S16F11 S16F15 slot position within a carrier

### SMPLN

- **格式**：`U4:1`
- **範例**：`&quot;U4:1 10&quot;`
- **使用於**：S6F1 sample number

### SOFTREV

- **格式**：`A:20`
- **說明**：Used by: S1F2 S1F13 S1F14 S7F22 S7F23 S7F26 S7F31 software revision
- **範例**：`&quot;A 1.0&quot;`
- **使用於**：S1F2 S1F13 S1F14 S7F22 S7F23 S7F26 S7F31 software revision

### SPAACK

- **格式**：`U1:1`
- **說明**：Used by: S2F4 S20F26 service completion code
- **範例**：`&quot;U1:1 0&quot; 0 - done 1 - error`
- **使用於**：S2F4 S20F26 service completion code

### SPD

- **格式**：`B:n`
- **說明**：Used by: S2F3 S2F6 service program data
- **範例**：`&quot;B:3 1 2 3&quot;`
- **使用於**：S2F3 S2F6 service program data

### SPID

- **格式**：`A:6`
- **說明**：Used by: S2F1 S2F5 S2F7 S2F9 S2F12 service program identifier
- **範例**：`&quot;A:6 bin007&quot;`
- **使用於**：S2F1 S2F5 S2F7 S2F9 S2F12 service program identifier

### SPNAME

- **格式**：`A:n`
- **說明**：Used by: S14F19 S14F20 S14F21 S14F28 service parameter name
- **範例**：`&quot;A:10 {BatchLocID}&quot;`
- **使用於**：S14F19 S14F20 S14F21 S14F28 service parameter name

### SPR

- **格式**：`A:n`
- **範例**：`&quot;A:19 {shutdown -i5 -g0 -y}&quot;`
- **使用於**：S2F10 device dependent, any data type

### SPVAL

- **格式**：`A:n`
- **範例**：`&quot;A:1 {1}&quot;`
- **使用於**：S14F19 S14F20 S14F21 service parameter value, any format type

### SSAACK

- **格式**：`U1:1`
- **說明**：Used by: S20F2 service completion code
- **範例**：`&quot;U4:1 0&quot; 0 - done 1 - error`
- **使用於**：S20F2 service completion code

### SSACK

- **格式**：`A:2`
- **說明**：Used by: S18F2 S18F4 S18F6 S18F8 S18F10 S18F12 S18F14 S18F16 two character codes for success or failure, NO Normal, EE exec. err, CE comm. err, HE h/w err, TE tag err
- **範例**：`&quot;A:2 NO&quot;`
- **使用於**：S18F2 S18F4 S18F6 S18F8 S18F10 S18F12 S18F14 S18F16 two character codes for success or failure, NO Normal, EE exec. err, CE comm. err, HE h/w err, TE tag err

### SSCMD

- **格式**：`A:n`
- **說明**：Used by: S18F13 subsystem action command
- **使用於**：S18F13 subsystem action command

### STATUS

- **格式**：`A:n`
- **說明**：Used by: S18F2 S18F4 S18F6 S18F8 S18F10 S18F12 S18F14 S18F16 subsystem status data
- **使用於**：S18F2 S18F4 S18F6 S18F8 S18F10 S18F12 S18F14 S18F16 subsystem status data

### STATUSTXT

- **格式**：`A:80`
- **說明**：Used by: S19F2 S19F4 S19F6 S19F8 S19F10 S19F13 S19F16 S19F18 status response description
- **範例**：`&quot;A:2 {Ok}&quot;`
- **使用於**：S19F2 S19F4 S19F6 S19F8 S19F10 S19F13 S19F16 S19F18 status response description

### STIME

- **格式**：`A:32`
- **說明**：Used by: S6F1 ECV TimeFormat controls format, 0=A:12 YYMMDDHHMMSS, 1=A:16 YYYYMMDDHHMMSScc,2=YYYY-MM-DDTHH:MM:SS.s[s]*{Z|+hh:mm|-hh:mm}
- **範例**：`&quot;A:16 2005041209345240&quot;`
- **使用於**：S6F1 ECV TimeFormat controls format, 0=A:12 YYMMDDHHMMSS, 1=A:16 YYYYMMDDHHMMSScc,2=YYYY-MM-DDTHH:MM:SS.s[s]*{Z|+hh:mm|-hh:mm}

### STRACK

- **格式**：`B:1`
- **說明**：Used by: S2F44 spooling stream acknowledge
- **範例**：`&quot;B:1 0x00&quot; 1 - not allowed for stream 2 - unknown stream 3 - unknown function 4 - secondar`
- **使用於**：S2F44 spooling stream acknowledge

### STRID

- **格式**：`U1:1`
- **說明**：Used by: S2F43 S2F44 S2F60 stream value
- **範例**：`&quot;U1:1 6&quot;`
- **使用於**：S2F43 S2F44 S2F60 stream value

### STRP

- **格式**：`I2:2`
- **範例**：`&quot;I2:2 0 0&quot;`
- **使用於**：S12F9 S12F16 x y die coordinate starting position

### SV

- **格式**：`A:n`
- **範例**：`&quot;U1:1 65&quot;`
- **使用於**：S1F4 S1F6 S6F1 status variable value

### SV0

- **格式**：`A:0`
- **範例**：`&quot;A:0&quot;`
- **使用於**：S1F8 Zero length value used to convey format type

### SVCACK

- **格式**：`B:1`
- **說明**：Used by: S14F20 S14F20 S14F21 service acknowledge code
- **範例**：`&quot;B:1 0x00&quot; 0 - ok done 1 - unknown service 2 - cannot do now 3 - 1 or more parameters inva`
- **使用於**：S14F20 S14F20 S14F21 service acknowledge code

### SVCNAME

- **格式**：`A:n`
- **說明**：Used by: S14F19 S14F26 S14F27 S14F28 service name
- **範例**：`&quot;A:5 {pause}&quot;`
- **使用於**：S14F19 S14F26 S14F27 S14F28 service name

### SVID

- **格式**：`U4:1`
- **範例**：`&quot;U4:1 810&quot;`
- **使用於**：S1F3 S1F11 S1F12 S2F23 status variable ID

### SVNAME

- **格式**：`A:n`
- **說明**：Used by: S1F8 S1F12 status variable name
- **範例**：`&quot;A:13 {AlarmsEnabled}&quot;`
- **使用於**：S1F8 S1F12 status variable name

### TARGETID

- **格式**：`A:n`
- **說明**：Used by: S18F1 S18F2 S18F3 S18F4 S18F5 S18F6 S18F7 S18F8 S18F9 S18F10 S18F11 S18F12 S18F13 S18F14 <a
- **使用於**：S18F1 S18F2 S18F3 S18F4 S18F5 S18F6 S18F7 S18F8 S18F9 S18F10 S18F11 S18F12 S18F13 S18F14 <a

### TARGETPDE

- **格式**：`A:36`
- **說明**：Used by: S19F15 S19F17 the UID of the target PDE, a 36 character string with runs of 8,4,4,4, and 12 characters joined by hyphens.
- **範例**：`&quot;A:36 {ABCD0123-62CC-404b-BBBF-BF3E3BBB1374}&quot;`
- **使用於**：S19F15 S19F17 the UID of the target PDE, a 36 character string with runs of 8,4,4,4, and 12 characters joined by hyphens.

### TARGETSPEC

- **格式**：`A:40`
- **說明**：Used by: S14F17 S15F43 Specifier of target object
- **使用於**：S14F17 S15F43 Specifier of target object

### TBLACK

- **格式**：`U1:1`
- **說明**：Used by: S13F14 S13F16 acknowledge code
- **範例**：`&quot;U1:1 0&quot; 0 - ok 1 - failure`
- **使用於**：S13F14 S13F16 acknowledge code

### TBLCMD

- **格式**：`U1:1`
- **說明**：Used by: S13F13 S13F15 table command
- **範例**：`&quot;U1:1 0&quot; 0 - complete table 1 - new rows (add) 2 - new columns (append) 3 - replace rows 4`
- **使用於**：S13F13 S13F15 table command

### TBLELT

- **格式**：`A:n`
- **範例**：`&quot;A:4 {0001}&quot;`
- **使用於**：S13F13 S13F15 S13F16 table element any type, list types or array types are discouraged, first column type must be a primary key value and not be a list or array

### TBLID

- **格式**：`A:80`
- **範例**：`&quot;A:14 {Table:TableARAMSCode>}&quot;`
- **使用於**：S13F13 S13F15 S13F16 table identifier, a kind of OBJSPEC

### TBLTYP

- **格式**：`A:n`
- **說明**：Used by: S13F13 S13F15 S13F16 denotes the format and application of the table, conforms to OBJTYPE
- **範例**：`&quot;A:5 {Table}&quot;`
- **使用於**：S13F13 S13F15 S13F16 denotes the format and application of the table, conforms to OBJTYPE

### TCID

- **格式**：`A:36`
- **說明**：Used by: S19F6 S19F8 S19F9 S19F10 S19F11 The identity of a transfer container specified as a 36 character string with runs of 8, 4, 4, 4, and 12 characters joined by hyphens
- **範例**：`&quot;A:36 {ff478f2d-e398-449a-93d4-62b072789fed}&quot;`
- **使用於**：S19F6 S19F8 S19F9 S19F10 S19F11 The identity of a transfer container specified as a 36 character string with runs of 8, 4, 4, 4, and 12 characters joined by hyphens

### TEXT

- **格式**：`A:120`
- **範例**：`&quot;A:38 {chamber cleaning PM must be done NEXT!}&quot;`
- **使用於**：S10F1 S10F3 S10F5 S10F9 line of text for display, no standard max size

### TIAACK

- **格式**：`B:1`
- **說明**：Used by: S2F24 trace acknowledgement code
- **範例**：`&quot;B:1 0x00&quot; 0 - ok 1 - too many SVIDs 2 - no more traces allowed 3 - invalid period 4 - unk`
- **使用於**：S2F24 trace acknowledgement code

### TIACK

- **格式**：`B:1`
- **說明**：Used by: S2F32 time set acknowledge
- **範例**：`&quot;B:1 0x00&quot; 0 - ok 1 - not done`
- **使用於**：S2F32 time set acknowledge

### TID

- **格式**：`B:1`
- **說明**：Used by: S10F1 S10F3 S10F5 S10F7 terminal ID
- **範例**：`&quot;B:1 0x00&quot;`
- **使用於**：S10F1 S10F3 S10F5 S10F7 terminal ID

### TIME

- **格式**：`A:32`
- **說明**：Used by: S2F18 S2F31 ECV TimeFormat controls format, 0=A:12 YYMMDDHHMMSS, 1=A:16 YYYYMMDDHHMMSScc,2=YYYY-MM-DDTHH:MM:SS.s[s]*{Z|+hh:mm|-hh:mm}
- **範例**：`&quot;A:16 2008121708371902&quot;`
- **使用於**：S2F18 S2F31 ECV TimeFormat controls format, 0=A:12 YYMMDDHHMMSS, 1=A:16 YYYYMMDDHHMMSScc,2=YYYY-MM-DDTHH:MM:SS.s[s]*{Z|+hh:mm|-hh:mm}

### TIMESTAMP

- **格式**：`A:32`
- **說明**：Used by: S5F9 S5F11 S5F15 S15F41 S15F44 S16F7 S16F9 S20F12 S20F13 S20F15 S20F17 S20F18 S20F20 S20F
- **使用於**：S5F9 S5F11 S5F15 S15F41 S15F44 S16F7 S16F9 S20F12 S20F13 S20F15 S20F17 S20F18 S20F20 S20F

### TOTSMP

- **格式**：`U4:1`
- **範例**：`&quot;U4:1 100&quot;`
- **使用於**：S2F23 S17F5 total samples to be made, should be an even multiple of REPGSZ

### TRACK

- **格式**：`TF:1`
- **說明**：Used by: S4F20 S4F22 S4F23 transfer activity success flag
- **範例**：`&quot;TF:1 1&quot;`
- **使用於**：S4F20 S4F22 S4F23 transfer activity success flag

### TRANSFERSIZE

- **格式**：`U8:1`
- **說明**：Used by: S19F9 SIze in bytes of the TransferContainer. An 8 byte value but HSMS uses 4 byte message lengths!!!
- **範例**：`&quot;U8:1 0x0000007F&quot;`
- **使用於**：S19F9 SIze in bytes of the TransferContainer. An 8 byte value but HSMS uses 4 byte message lengths!!!

### TRATOMCID

- **格式**：`U4:1`
- **範例**：`&quot;U4:1 101&quot;`
- **使用於**：S4F20 assigned identifier for atomic transfer

### TRAUTOD

- **格式**：`TF:1`
- **說明**：Used by: S17F5 delete upon completion flag
- **範例**：`&quot;TF:1 1&quot;`
- **使用於**：S17F5 delete upon completion flag

### TRAUTOSTART

- **格式**：`TF:1`
- **說明**：Used by: S4F19 if true material transfer is initiated by the primary when ready
- **範例**：`&quot;TF:1 0&quot;`
- **使用於**：S4F19 if true material transfer is initiated by the primary when ready

### TRCMDNAME

- **格式**：`A:n`
- **說明**：Used by: S4F21 text enum, CANCEL, PAUSE, RESUME, ABORT, STOP, STARTHANDOFF
- **範例**：`&quot;A:5 {PAUSE}&quot;`
- **使用於**：S4F21 text enum, CANCEL, PAUSE, RESUME, ABORT, STOP, STARTHANDOFF

### TRDIR

- **格式**：`U1:1`
- **說明**：Used by: S4F19 S4F27 transfer direction
- **範例**：`&quot;U1:1 1&quot; 1 - send material 2 - receive material`
- **使用於**：S4F19 S4F27 transfer direction

### TRID

- **格式**：`A:n`
- **範例**：`&quot;A:1 1&quot;`
- **使用於**：S2F23 S2F62 S6F1 S6F27 S6F28 S6F30 S17F5 S17F6 S17F7 S17F8 S17F13 S17F14 trace request ID

### TRJOBID

- **格式**：`B:1`
- **說明**：Used by: S4F20 S4F21 S4F23 assigned identifier for transfer job
- **範例**：`&quot;B:1 0x60&quot;`
- **使用於**：S4F20 S4F21 S4F23 assigned identifier for transfer job

### TRJOBMS

- **格式**：`U1:1`
- **說明**：Used by: S4F23 transfer job milestone
- **範例**：`&quot;U1:1 1&quot; 1 - started 2 - completed`
- **使用於**：S4F23 transfer job milestone

### TRJOBNAME

- **格式**：`A:80`
- **說明**：Used by: S4F19 S4F23 host assigned id for transfer job
- **範例**：`&quot;A:13 TJH_U_M_E1086&quot;`
- **使用於**：S4F19 S4F23 host assigned id for transfer job

### TRLINK

- **格式**：`U4:1`
- **範例**：`&quot;U4:1 3&quot;`
- **使用於**：S4F19 S4F27 S4F29 S4F31 S4F33 S4F35 S4F37 S4F39 S4F41 task identifier correlation value

### TRLOCATION

- **格式**：`U4:1`
- **範例**：`&quot;U4:1 1&quot;`
- **使用於**：S4F19 S4F27 material transfer location

### TROBJNAME

- **格式**：`A:n`
- **範例**：`&quot;A:7 {c000678}&quot;`
- **使用於**：S4F19 S4F27 identifies material to be transferred

### TROBJTYPE

- **格式**：`U4:1`
- **範例**：`&quot;U4:1 7&quot;`
- **使用於**：S4F19 S4F27 identifies type of object to be transferred

### TRPORT

- **格式**：`U4:1`
- **範例**：`&quot;U4:1 1&quot;`
- **使用於**：S4F19 S4F27 port identifier

### TRPTNR

- **格式**：`A:n`
- **說明**：Used by: S4F19 S4F27 EQNAME of transfer partner equipment
- **範例**：`&quot;A:7 {AGV0001}&quot;`
- **使用於**：S4F19 S4F27 EQNAME of transfer partner equipment

### TRPTPORT

- **格式**：`U4:1`
- **範例**：`&quot;U4:1 1&quot;`
- **使用於**：S4F19 S4F27 transfer partner port

### TRRCP

- **格式**：`A:80`
- **說明**：Used by: S4F19 name of transfer recipe for this handoff
- **範例**：`&quot;A:17 {standard exchange}&quot;`
- **使用於**：S4F19 name of transfer recipe for this handoff

### TRROLE

- **格式**：`U1:1`
- **說明**：Used by: S4F19 S4F27 indicates equipment transfer role
- **範例**：`&quot;U1:1 1&quot; 1 - primary 2 - secondary`
- **使用於**：S4F19 S4F27 indicates equipment transfer role

### TRTYPE

- **格式**：`U1:1`
- **說明**：Used by: S4F19 S4F27 equipment is active or passive transfer participant
- **範例**：`&quot;U1:1 1&quot; 1 - active 2 - passive`
- **使用於**：S4F19 S4F27 equipment is active or passive transfer participant

### TSIP

- **格式**：`B:n`
- **說明**：Used by: S1F10 transfer status of input ports
- **範例**：`&quot;B:1 0x01&quot; 1 - idle 2 - prep 3 - track on 4 - stuck in receiver`
- **使用於**：S1F10 transfer status of input ports

### TSOP

- **格式**：`B:n`
- **說明**：Used by: S1F10 transfer status of output ports
- **範例**：`&quot;B:1 0x01&quot; 1 - idle 2 - prep 3 - track on 4 - stuck in sender 5 - completion`
- **使用於**：S1F10 transfer status of output ports

### TTC

- **格式**：`U4:1`
- **範例**：`&quot;U4:1 286&quot;`
- **使用於**：S3F4 time to completion, standard does not specify units, in seconds??

### TYPEID

- **格式**：`U1:1`
- **說明**：Used by: S20F12 S20F13 S20F15 S20F17 S20F18 S20F20 S20F21 S20F23 S20F27 S20F30 S20F32 recipe type
- **範例**：`&quot;U1:1 1&quot; 0 - not used 1 - ProcessLink 2 - Pr`
- **使用於**：S20F12 S20F13 S20F15 S20F17 S20F18 S20F20 S20F21 S20F23 S20F27 S20F30 S20F32 recipe type

### UID

- **格式**：`A:36`
- **說明**：Used by: S19F2 S19F3 S19F4 S19F5 S19F6 S19F7 S19F8 S19F13 S19F16 S19F18 See SEMI E139. A unique identifier for a PDE consisting of a 36 character string with runs of 8, 4, 4, 4, and 12 characters separated by hyphens
- **範例**：`&quot;A`
- **使用於**：S19F2 S19F3 S19F4 S19F5 S19F6 S19F7 S19F8 S19F13 S19F16 S19F18 See SEMI E139. A unique identifier for a PDE consisting of a 36 character string with runs of 8, 4, 4, 4, and 12 characters separated by hyphens

### UNFLEN

- **格式**：`U4:1`
- **範例**：`&quot;U4:1 0&quot;`
- **使用於**：S7F34 unformatted process program length if available, else 0

### UNITS

- **格式**：`A:n`
- **說明**：Used by: S1F12 S1F22 S2F30 S2F48 units identifier (see E5 Section 9)
- **範例**：`&quot;A:0&quot;`
- **使用於**：S1F12 S1F22 S2F30 S2F48 units identifier (see E5 Section 9)

### UPPERDB

- **格式**：`F4:1`
- **範例**：`&quot;F4:1 185.0&quot;`
- **使用於**：S2F45 S2F48 the upper bound of a deadband limit

### V

- **格式**：`A:n`
- **範例**：`&quot;A:1 {0}&quot;`
- **使用於**：S6F11 S6F13 S6F16 S6F18 S6F20 S6F22 S6F27 S6F30 S16F9 variable value, any type including list

### VERID

- **格式**：`A:n`
- **說明**：Used by: S20F12 S20F13 S20F15 S20F17 S20F18 S20F20 S20F21 S20F23 S20F27 S20F30 S20F32 composite key with RecipeID to identify a unique recipe
- **範例**：`&quot;A:5 1.0.5&quot;`
- **使用於**：S20F12 S20F13 S20F15 S20F17 S20F18 S20F20 S20F21 S20F23 S20F27 S20F30 S20F32 composite key with RecipeID to identify a unique recipe

### VERIFYDEPTH

- **格式**：`U1:1`
- **說明**：Used by: S19F17 whether to check only the target, or the target and all referenced PDEs
- **範例**：`&quot;U1:1 0&quot; 0 - target only 1 - target and all referenced, directly or indirectly`
- **使用於**：S19F17 whether to check only the target, or the target and all referenced PDEs

### VERIFYRSPSTAT

- **格式**：`U1:1`
- **說明**：Used by: S19F13 S19F18 PDE verification result, 0 success, 10 none, other error
- **範例**：`&quot;U1:1 0&quot; 0 - success 1 - invalid input map 2 - PDE reference not found 3 - PDE reference r`
- **使用於**：S19F13 S19F18 PDE verification result, 0 success, 10 none, other error

### VERIFYSUCCESS

- **格式**：`TF:1`
- **說明**：Used by: S19F18 True if no errors were found
- **範例**：`&quot;TF:1 1&quot;`
- **使用於**：S19F18 True if no errors were found

### VERIFYTYPE

- **格式**：`U1:1`
- **說明**：Used by: S19F17 chooses the type of verification
- **範例**：`&quot;U1:1 0&quot; 0 - checksum validation 1 - complete validation`
- **使用於**：S19F17 chooses the type of verification

### VID

- **格式**：`A:n`
- **使用於**：S1F21 S1F22 S1F24 S2F33 S2F45 S2F46 S2F47 S2F48 S2F54 S6F13 S6F18 S6F22 S16F9 S17F1 A var

### VLAACK

- **格式**：`B:1`
- **說明**：Used by: S2F46 variable limit attribute acknowledge
- **範例**：`&quot;B:1 0x00&quot; 0 - ok 1 - limit attribute definition error 2 - cannot perform now`
- **使用於**：S2F46 variable limit attribute acknowledge

### WRACK

- **格式**：`U1:1`
- **說明**：Used by: S20F16 service completion code
- **範例**：`&quot;U4:1 0&quot; 0 - done 1 - full 2 - error`
- **使用於**：S20F16 service completion code

### XDIES

- **格式**：`F4:1`
- **範例**：`&quot;F4 5.0&quot;`
- **使用於**：S12F1 S12F4 X-axis die size

### XYPOS

- **格式**：`I2:2`
- **範例**：`&quot;I2:2 0 0&quot;`
- **使用於**：S12F11 S12F18 x y coordinate position

### YDIES

- **格式**：`F4:1`
- **範例**：`&quot;F4 7.2&quot;`
- **使用於**：S12F1 S12F4 Y-axis die size
