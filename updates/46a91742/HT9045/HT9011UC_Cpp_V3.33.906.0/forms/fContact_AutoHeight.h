// =============================================================================
//  forms/fContact_AutoHeight.h -- E-042 (SAFETY): the golden cContact.cpp:93-98 file-scope globals that
//  forms/fContact_AutoHeight.cpp defines (golden keeps them in cContact.cpp; the port had none).
//
//  AI(W906-E042) 20261004 (St01 / ST01-E): NEW FILE, externs only.  iIndexStatus is written by
//  Do_Z1_AutoGetHeight case 3010 (golden :7673, only for CC_KYEC_LEE / CC_HONPREC_QC) and read by the Contact
//  page Index jog (golden :16961-17032, the laptop's CT-3c) -- exported so that jog can see it.
// =============================================================================
#ifndef fContact_AutoHeightH
#define fContact_AutoHeightH

extern int    iIndexStatus;                       // golden cContact.cpp:93
extern int    iContactJogMove, iContactJogSpeed;  // golden cContact.cpp:94
extern bool   bContactJogFlag;                    // golden cContact.cpp:95
extern double iTotalOffset_1;                     // golden cContact.cpp:97 (double despite the i- prefix -- golden)
extern double iTotalOffset_2;                     // golden cContact.cpp:98

//  AI(W906-E042) 20261005 (B4, St01): the contact-mode single entry (FileRW/DeviceForm_File.cpp EOF): V912 DF_SetContactMode
//  on the web's radio state, mirrored onto the TfContact facade.  rbForTest >= 0 = test seam (selects that radio).
void FileRW_Contact_SetContactModeSingleEntry(int rbForTest = -1);

#endif  // fContact_AutoHeightH
