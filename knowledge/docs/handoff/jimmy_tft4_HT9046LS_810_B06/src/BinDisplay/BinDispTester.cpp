//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
USERES("BinDispTester.res");
USEFORM("BinDisp.cpp", Form1);
USEFORM("MyBinDisp.cpp", DataModule3); /* TDataModule: File Type */
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    try
    {
         Application->Initialize();
         Application->CreateForm(__classid(TForm1), &Form1);
         Application->CreateForm(__classid(TDataModule3), &DataModule3);
         Application->Run();
    }
    catch (Exception &exception)
    {
         Application->ShowException(&exception);
    }
    return 0;
}
//---------------------------------------------------------------------------
