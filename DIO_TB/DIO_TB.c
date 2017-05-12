#include <userint.h>

//==============================================================================
//
// Title:       DIO_TB
// Purpose:     Test bench platform to test DIO configurations
//
// Created on:  5/01/2017 at 1:29:25 PM by Michael Foe.
// Copyright:   Customer Service Associates. All Rights Reserved.
//
//==============================================================================
#define MAXCHARARRAYLENGTH 400
#define MAXNUMBEROFDIOPORTS 500
#define DIO_TB_Rev "1.0"
//==============================================================================
// Include files

#include "cvixml.h"
#include "toolbox.h"
#include <ansi_c.h>
#include <utility.h>
//#include <NIDAQmx.h>
//#include <DAQmxIOctrl.h>
//#include "DIO_LIB.h"
#include "DIOPanel.h"
#include "DIO_TB.h"

//#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else
//==============================================================================
// Global variables
typedef struct
{
    char        FullIOPath[MAXCHARARRAYLENGTH];
    char        Direction[MAXCHARARRAYLENGTH];
    char        InverseLogic[MAXCHARARRAYLENGTH];
    char        SignalName[MAXCHARARRAYLENGTH];
    //TaskHandle  DAQMxTaskHandle;
    int         Status; // 0=Off; 1=On
} DIOFileInfoStruct;

DIOFileInfoStruct DIOFileInfo[MAXNUMBEROFDIOPORTS]= {0};

char DIOParamName[4][20]= {"FullIOPath","Direction","InverseLogic","SignalName"};
int DIOConfigurationPanelHandle;
int DIOConfigTableHandle;
int DIODebugPanelHandle;
int DIOCmdButtonHandle[MAXNUMBEROFDIOPORTS];

static CmtThreadPoolHandle DIOInputThreadHandle;

int ThreadID=0;
int KillThread;
char PathToDIOConfigFile[MAXCHARARRAYLENGTH];
int MainPanelHandlebk;
int NumOfDIOPorts=0;
int DIOUpdateStarted=0;
static int notInitialized=1;
static int DIOPanelLoaded;


/********************************************//**
* Create the DIO configuration table. Read from the specified file and populates to DIOFileInfo struct as well as the Dio config table.
***********************************************/
void __stdcall CreateDIOConfigurationTable(char *filePath, int MainPanelHandle) ;
/********************************************//**
* Create the DIO debug Panel used to manually turn on and off signals.
***********************************************/
void __stdcall CreateDIODebugPanel(int MainPanelHandle) ;
/********************************************//**
* DIO config table cb function for toggling between input and output
***********************************************/
int CVICALLBACK  DIOTableCBFunction (int panel, int control, int event, void *callbackData, int eventData1, int eventData2) ;
/********************************************//**
* Quit/hide DIO config table
***********************************************/
int CVICALLBACK  QuitDIOConfigTableCB (int panel, int control, int event, void *callbackData, int eventData1, int eventData2) ;
/********************************************//**
* Quit/hide DIO Debug panel
***********************************************/
int CVICALLBACK  QuitDIODebugCB (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
/********************************************//**
 *  Callback for the Save button in DIO Configuration panel
 *  This function create a back of the current Test Configuration xml file and saves the new config into the file.
 ***********************************************/
int CVICALLBACK  SaveDIOConfigTableCB (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
/********************************************//**
 *  Save backup copy of the existing DIO config xml file
 ***********************************************/
int SaveBackupXmlFilenameDIO(const char *filename)  ;
/********************************************//**
 * read the xml DIO configuration from specified path and populate DIOFileInfo structure
 ***********************************************/
int  ReadDIOConfigurationFile(char *filePath);
/********************************************//**
 *  This function takes the data previously loaded into DIO struct and populats the test configuration table.
 ***********************************************/
void LoadDIOConfigFile(void);
/********************************************//**
 *  find the full IO path and direction given a signal name;
 ***********************************************/
//int FindChanelFromSignalName(char* SignalName, char* channel,char * Direction, char *InverseLogic, TaskHandle  *DAQMxTaskHandle);
int FindChanelFromSignalName_TB(char *SignalName, char *Direction, char *InverseLogic);
/********************************************//**
* Get Number of DIO inputs
***********************************************/
int GetNumOfDIOInputs(void);
/********************************************//**
* Get Number of DIO outputs
***********************************************/
int GetNumOfDIOOutputs(void);
/********************************************//**
 *  Sort DIOFileInfo array inputs first then outputs
 ***********************************************/
void SortDIOFileInfo(void);
/********************************************//**
* Callback on output button clicked
***********************************************/
int CVICALLBACK ToogleDIOOutput (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
/********************************************//**
*Create new worker using the assigned workerData
***********************************************/
//int UpdateDIOInputs(void *dummy);
int ISValidXMLDIO (char * string) ;
/********************************************//**
 *  Create and Start DAQMx task
 ***********************************************/
//int CreateAndInitDAQMxTask(void);
/********************************************//**
 *  Stop and Clear DAQMx task
 ***********************************************/
int CloseAndStopDAQMxTask(void);
/********************************************//**
*Stop updating DIO Inputs
***********************************************/
//void StopDIOInputUpdate(void);
/********************************************//**
 * Callback function for delete row button. Delete the slected test row from DIO configuration table
 ***********************************************/
int CVICALLBACK  DelRowDIOConfigTableCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)   ;
/********************************************//**
 * Callback function for add test button. Adds a test row below the selected row in the DIO configuration table.
 ***********************************************/
int CVICALLBACK  AddRowDIOConfigTableCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2) ;
int CVICALLBACK DIOHwCtrlPanelEvent (int panelHandle, int event, void *callbackData, int eventData1, int eventData2) ;
/********************************************//**
*Starts updating DIO Inputs
***********************************************/
//void __stdcall StartDIOInputUpdate(void);
/********************************************//**
* Update all DIO outputs. Set DIO outputs to green if high else set to white
***********************************************/
void __stdcall UpdateDIOOutputs(void);

void WriteDigitalLines(char *SignalName, int Data);
int ReadDigitalLines(char *SignalName);
void DIOOutputLink(char *SignalName);
void ToggleSignal_TB(char *SignalName);

/********************************************//**
* Initialize the DIO configuration table panel and the DIO debug panel
***********************************************/
void __cdecl InitializeDIO_TB(char *DIOConfigurationFile, int MainPanelHandle)
{
    CreateDIOConfigurationTable(DIOConfigurationFile,MainPanelHandle);
    CreateDIODebugPanel(MainPanelHandle);
}
/********************************************//**
*Starts updating DIO Inputs
***********************************************
void __stdcall StartDIOInputUpdate(void)
{
    ThreadID=0;
    KillThread=1;
    CmtNewThreadPool(1,&DIOInputThreadHandle);
    CmtScheduleThreadPoolFunction(DIOInputThreadHandle,UpdateDIOInputs,0,&ThreadID);

}*/

void __stdcall UpdateDIOInputs_TB(void){
    // Init input DIO
    for(int i=0; i<GetNumOfDIOInputs(); i++){
        if (ReadDIOSignal(DIOFileInfo[i].SignalName) == 1){
            SetCtrlAttribute(DIODebugPanelHandle,DIOCmdButtonHandle[i],ATTR_CMD_BUTTON_COLOR,VAL_GREEN);
        }else{
            SetCtrlAttribute(DIODebugPanelHandle,DIOCmdButtonHandle[i],ATTR_CMD_BUTTON_COLOR,VAL_RED);
        }
    }
}
/********************************************//**
*Stop updating DIO Inputs
***********************************************
void StopDIOInputUpdate(void)
{
    KillThread=0;
    //CmtReleaseThreadPoolFunctionID(DIOInputThreadHandle,ThreadID);
    //CmtDiscardThreadPool(DIOInputThreadHandle);
    ThreadID=0;
}*/
/********************************************//**
*Create new worker using the assigned workerData
***********************************************
int UpdateDIOInputs(void *dummy)
{
    int value=0;
    while(KillThread==1 && (GetPanelAttribute(DIODebugPanelHandle,ATTR_HEIGHT,&value)>-1) )
    {
        for(int i=0; i<GetNumOfDIOInputs(); i++)
        {
            if( ReadDIOSignal(DIOFileInfo[i].SignalName) )
                SetCtrlAttribute(DIODebugPanelHandle,DIOCmdButtonHandle[i],ATTR_CMD_BUTTON_COLOR,VAL_GREEN);
            else
                SetCtrlAttribute(DIODebugPanelHandle,DIOCmdButtonHandle[i],ATTR_CMD_BUTTON_COLOR,VAL_RED);
        }
        //DelayWithEventProcessing(1);
    }
    return 0;
} // end NewWorker */
/********************************************//**
* Create the DIO debug Panel used to manually turn on and off signals.
***********************************************/
void __stdcall CreateDIODebugPanel(int MainPanelHandle)
{
    int Top=70;
    int Left=25;
    int TopDelta=60;
    int LeftDelta=120;
    int i;
    
    MainPanelHandlebk = MainPanelHandle;
    DIODebugPanelHandle = NewPanel (MainPanelHandle, "DIO Debug Utility", 160, 150, 850/*730*/, 1600/*1000*/);
    InstallPanelCallback (DIODebugPanelHandle, DIOHwCtrlPanelEvent, 0);
    int DebugPanelExitButtonHandle = NewCtrl (DIODebugPanelHandle, CTRL_SQUARE_COMMAND_BUTTON, "Exit", 820/*700*/, 1500/*900*/);
    SetCtrlAttribute (DIODebugPanelHandle,DebugPanelExitButtonHandle, ATTR_CALLBACK_FUNCTION_POINTER,QuitDIODebugCB);
    for(i=0; i< GetNumOfDIOInputs(); i++)
    {
        DIOCmdButtonHandle[i]=NewCtrl(DIODebugPanelHandle,CTRL_SQUARE_COMMAND_BUTTON,"Signal Name",Top,Left);
        SetCtrlAttribute(DIODebugPanelHandle,DIOCmdButtonHandle[i],ATTR_LABEL_POINT_SIZE, 9);
        SetCtrlAttribute(DIODebugPanelHandle,DIOCmdButtonHandle[i],ATTR_HEIGHT,30);
        SetCtrlAttribute(DIODebugPanelHandle,DIOCmdButtonHandle[i],ATTR_WIDTH,100);
        SetCtrlAttribute(DIODebugPanelHandle,DIOCmdButtonHandle[i],ATTR_AUTO_SIZING,VAL_NEVER_AUTO_SIZE);
        SetCtrlAttribute(DIODebugPanelHandle,DIOCmdButtonHandle[i],ATTR_LABEL_TEXT,DIOFileInfo[i].SignalName);
        Left=Left+LeftDelta;
        if(Left>1540/*900*/)
        {
            Top = Top + TopDelta;
            Left = 25;
        }
    }
    int TextHandle=NewCtrl(DIODebugPanelHandle,CTRL_TEXT_MSG,"INPUTS",30,800);
    SetCtrlAttribute(DIODebugPanelHandle,TextHandle,ATTR_TEXT_BOLD,1);
    SetCtrlAttribute(DIODebugPanelHandle,TextHandle,ATTR_TEXT_POINT_SIZE,20);
    int DecoHandle=NewCtrl(DIODebugPanelHandle,CTRL_RECESSED_FRAME,"Input",20,10);
    SetCtrlAttribute(DIODebugPanelHandle,DecoHandle,ATTR_HEIGHT,Top+30);
    SetCtrlAttribute(DIODebugPanelHandle,DecoHandle,ATTR_WIDTH,1580/*980*/);

    Top = Top + 40;
    Left = 25;

    int TextHandle2=NewCtrl(DIODebugPanelHandle,CTRL_TEXT_MSG,"OUTPUTS",Top+10,800);
    SetCtrlAttribute(DIODebugPanelHandle,TextHandle2,ATTR_TEXT_BOLD,1);
    SetCtrlAttribute(DIODebugPanelHandle,TextHandle2,ATTR_TEXT_POINT_SIZE,20);
    int DecoHandle2=NewCtrl(DIODebugPanelHandle,CTRL_RECESSED_FRAME,"Output",Top,10);
    SetCtrlAttribute(DIODebugPanelHandle,DecoHandle2,ATTR_HEIGHT,810-Top);  //690-Top
    SetCtrlAttribute(DIODebugPanelHandle,DecoHandle2,ATTR_WIDTH,1580/*980*/);
    for(i=GetNumOfDIOInputs(); i< GetNumOfDIOOutputs()+GetNumOfDIOInputs(); i++)
    {
        DIOCmdButtonHandle[i]=NewCtrl(DIODebugPanelHandle,CTRL_SQUARE_COMMAND_BUTTON,"Signal Name",Top+50,Left);
        SetCtrlAttribute(DIODebugPanelHandle,DIOCmdButtonHandle[i],ATTR_LABEL_POINT_SIZE, 9);
        SetCtrlAttribute(DIODebugPanelHandle,DIOCmdButtonHandle[i],ATTR_HEIGHT,30);
        SetCtrlAttribute(DIODebugPanelHandle,DIOCmdButtonHandle[i],ATTR_WIDTH,100);
        SetCtrlAttribute(DIODebugPanelHandle,DIOCmdButtonHandle[i],ATTR_AUTO_SIZING,VAL_NEVER_AUTO_SIZE);
        SetCtrlAttribute(DIODebugPanelHandle,DIOCmdButtonHandle[i],ATTR_LABEL_TEXT,DIOFileInfo[i].SignalName);
        SetCtrlAttribute(DIODebugPanelHandle,DIOCmdButtonHandle[i],ATTR_CALLBACK_FUNCTION_POINTER,ToogleDIOOutput);
        Left=Left+LeftDelta;
        if(Left>1540/*900*/)
        {
            Top = Top + TopDelta;
            Left = 25;
        }
    }
    
}
/********************************************//**
 * Callback function for add test button. Adds a test row below the selected row in the DIO configuration table.
 ***********************************************/
int CVICALLBACK  AddRowDIOConfigTableCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    if(event == EVENT_LEFT_CLICK_UP)
    {
        Point SelectedCell;
        int SelectedRow=0;
        GetActiveTableCell(panel,DIOConfigTableHandle,&SelectedCell);
        SelectedRow = SelectedCell.y;
        if (SelectedRow == 0)
        {
            InsertTableRows(panel,DIOConfigTableHandle,-1,1,VAL_USE_MASTER_CELL_TYPE);  // insert top row
            SetTableCellVal(panel, DIOConfigTableHandle,MakePoint(2,SelectedRow),"Output");
        }
        else
        {
            InsertTableRows(panel,DIOConfigTableHandle,SelectedRow+1,1,VAL_USE_MASTER_CELL_TYPE);
            SetTableCellVal(panel, DIOConfigTableHandle,MakePoint(2,SelectedRow+1),"Output");
        }
    }
    return 0;
}
/********************************************//**
 * Callback function for delete row button. Delete the slected test row from DIO configuration table
 ***********************************************/
int CVICALLBACK  DelRowDIOConfigTableCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    if(event == EVENT_LEFT_CLICK_UP)
    {
        Point SelectedCell;
        int SelectedRow=0;
        GetActiveTableCell(panel,DIOConfigTableHandle,&SelectedCell);
        SelectedRow = SelectedCell.y;
        if (SelectedRow > 0)
        {
            DeleteTableRows(panel,DIOConfigTableHandle,SelectedRow,1);
        }
        else
        {
            char msg[256];
            sprintf(msg,"Unable to delete row %i",SelectedRow);
            MessagePopup("Error",msg);
        }
    }
    return 0;
}
/********************************************//**
* Update all DIO outputs. Set DIO outputs to green if high else set to white
***********************************************
void __stdcall UpdateDIOOutputs(void)
{
    for(int i=GetNumOfDIOInputs(); i<GetNumOfDIOOutputs()+GetNumOfDIOInputs(); i++)
    {
        if( ReadDIOSignal(DIOFileInfo[i].SignalName) )
            SetCtrlAttribute(DIODebugPanelHandle,DIOCmdButtonHandle[i],ATTR_CMD_BUTTON_COLOR,VAL_GREEN);
        else
            SetCtrlAttribute(DIODebugPanelHandle,DIOCmdButtonHandle[i],ATTR_CMD_BUTTON_COLOR,VAL_RED);
    }
}*/

void __stdcall UpdateDIOOutputs_TB(void){
    
    for (int i=GetNumOfDIOInputs(); i<GetNumOfDIOOutputs()+GetNumOfDIOInputs(); i++){
        if( ReadDIOSignal(DIOFileInfo[i].SignalName) == 1)
            SetCtrlAttribute(DIODebugPanelHandle,DIOCmdButtonHandle[i],ATTR_CMD_BUTTON_COLOR,VAL_GREEN);
        else
            SetCtrlAttribute(DIODebugPanelHandle,DIOCmdButtonHandle[i],ATTR_CMD_BUTTON_COLOR,VAL_RED);
    }   
}
/********************************************//**
* Callback on output button clicked
***********************************************/
int CVICALLBACK ToogleDIOOutput (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    if(event == EVENT_COMMIT )
    {
        int i=0;
        for(i=0; i<NumOfDIOPorts; i++)
        {
            if(DIOCmdButtonHandle[i] ==  control)
                break;
        }
        int color=0;
        GetCtrlAttribute(panel, control,ATTR_CMD_BUTTON_COLOR, &color);
        if(color==VAL_GREEN)
        {
            SetCtrlAttribute(panel,control,ATTR_CMD_BUTTON_COLOR,VAL_RED);
            WriteDIOSignal(DIOFileInfo[i].SignalName, 0);
            DIOOutputLink(DIOFileInfo[i].SignalName);
        }
        else
        {
            SetCtrlAttribute(panel,control,ATTR_CMD_BUTTON_COLOR,VAL_GREEN);
            WriteDIOSignal(DIOFileInfo[i].SignalName, 1);
            DIOOutputLink(DIOFileInfo[i].SignalName);
        }
    }
    return 0;
}

void DIOOutputLink(char *SignalName){
    
    int state;
    
    if(stricmp(SignalName,"CTRL_POWER_24VDC") == 0){
        // E-Stop
        ToggleSignal_TB("E-Stop");
        
        // PSI-Sense
        ToggleSignal_TB("PSI_SENSE");
        
        // FixID
        ToggleSignal_TB("FixID_0");
        ToggleSignal_TB("FixID_2");
        
    }else if(stricmp(SignalName,"NEST1_LOCK_VALVE") == 0){
        // Nest1 Lock
        ToggleSignal_TB("NEST1_LOCK");
        
    }else if(stricmp(SignalName,"NEST2_LOCK_VALVE") == 0){
        // Nest2 Lock
        ToggleSignal_TB("NEST2_LOCK");
        
    }
    
}

/********************************************//**
* Toggles the Signal of the given SignalName
************************************************/
void ToggleSignal_TB(char *SignalName){
    int state;
    state = ReadDigitalLines(SignalName);
    WriteDIOSignal(SignalName, !state);
}
/********************************************//**
* Get Number of DIO inputs
***********************************************/
int GetNumOfDIOInputs(void)
{
    int input=0;
    for(int i=0; i<NumOfDIOPorts; i++)
    {
        if(stricmp(DIOFileInfo[i].Direction,"input")==0)
            input++;
    }

    return input;
}
/********************************************//**
* Get Number of DIO outputs
***********************************************/
int GetNumOfDIOOutputs(void)
{
    int output=0;
    for(int i=0; i<NumOfDIOPorts; i++)
    {
        if(stricmp(DIOFileInfo[i].Direction,"output")==0)
            output++;
    }
    
    return output;
}

int CVICALLBACK DIOHwCtrlPanelEvent (int panelHandle, int event, void *callbackData, int eventData1, int eventData2)
{
    if((DIOUpdateStarted==0) && (event == EVENT_GOT_FOCUS))
    {
        //UpdateDIOOutputs();
        UpdateDIOOutputs_TB();
        //StartDIOInputUpdate();
        UpdateDIOInputs_TB();
        DIOUpdateStarted=1;
    }
    return 0;
}
/********************************************//**
* Create the DIO configuration table. Read from the specified file and populates to DIOFileInfo struct as well as the Dio config table.
***********************************************/
void __stdcall CreateDIOConfigurationTable(char *filePath, int MainPanelHandle)
{
    sprintf(PathToDIOConfigFile,"%s",filePath);
    NumOfDIOPorts = ReadDIOConfigurationFile(PathToDIOConfigFile); //Returns the number of child elements in DIO XML
    if (notInitialized)
    {
        //CreateAndInitDAQMxTask();
        notInitialized = 0;
    }
    DIOConfigurationPanelHandle = NewPanel (MainPanelHandle, "DIO Configuration", 56, 8, 700, 805);

    DIOConfigTableHandle = NewCtrl (DIOConfigurationPanelHandle, CTRL_TABLE, "", 0, 0);
    SetCtrlAttribute(DIOConfigurationPanelHandle,DIOConfigTableHandle,ATTR_HEIGHT,650);
    SetCtrlAttribute(DIOConfigurationPanelHandle,DIOConfigTableHandle,ATTR_WIDTH,800); 
    SetCtrlAttribute( DIOConfigurationPanelHandle,DIOConfigTableHandle,ATTR_DATA_TYPE,VAL_INTEGER);
    SetCtrlAttribute( DIOConfigurationPanelHandle,DIOConfigTableHandle,ATTR_CALLBACK_FUNCTION_POINTER ,DIOTableCBFunction);

    int SaveButtonHandle = NewCtrl (DIOConfigurationPanelHandle, CTRL_SQUARE_COMMAND_BUTTON, "Save", 660, 625);
    SetCtrlAttribute (DIOConfigurationPanelHandle,SaveButtonHandle, ATTR_CALLBACK_FUNCTION_POINTER,SaveDIOConfigTableCB);

    int ExitButtonHandle = NewCtrl (DIOConfigurationPanelHandle, CTRL_SQUARE_COMMAND_BUTTON, "Exit", 660, 700);
    SetCtrlAttribute (DIOConfigurationPanelHandle,ExitButtonHandle, ATTR_CALLBACK_FUNCTION_POINTER,QuitDIOConfigTableCB);
    int DIOAddButtonHandle = NewCtrl (DIOConfigurationPanelHandle, CTRL_SQUARE_COMMAND_BUTTON, "Add Row", 660, 10);
    SetCtrlAttribute (DIOConfigurationPanelHandle,DIOAddButtonHandle, ATTR_CALLBACK_FUNCTION_POINTER,AddRowDIOConfigTableCB);

    int DIODelButtonHandle = NewCtrl (DIOConfigurationPanelHandle, CTRL_SQUARE_COMMAND_BUTTON, "Delete Row", 660, 75);
    SetCtrlAttribute (DIOConfigurationPanelHandle,DIODelButtonHandle, ATTR_CALLBACK_FUNCTION_POINTER,DelRowDIOConfigTableCB);

    InsertTableRows(DIOConfigurationPanelHandle,DIOConfigTableHandle,-1,NumOfDIOPorts,VAL_CELL_STRING);

    InsertTableColumns (DIOConfigurationPanelHandle, DIOConfigTableHandle, 1, 1, VAL_CELL_STRING);
    SetTableColumnAttribute (DIOConfigurationPanelHandle, DIOConfigTableHandle, 1, ATTR_LABEL_TEXT,DIOParamName[0]);
    SetTableColumnAttribute (DIOConfigurationPanelHandle, DIOConfigTableHandle, 1, ATTR_USE_LABEL_TEXT,1);
    SetTableColumnAttribute (DIOConfigurationPanelHandle, DIOConfigTableHandle, 1, ATTR_COLUMN_WIDTH,305);
    SetTableColumnAttribute (DIOConfigurationPanelHandle, DIOConfigTableHandle, 1, ATTR_CELL_TYPE,VAL_CELL_STRING);

    InsertTableColumns (DIOConfigurationPanelHandle, DIOConfigTableHandle, 2, 1, VAL_CELL_BUTTON);
    SetTableColumnAttribute (DIOConfigurationPanelHandle, DIOConfigTableHandle, 2, ATTR_LABEL_TEXT,DIOParamName[1]);
    SetTableColumnAttribute (DIOConfigurationPanelHandle, DIOConfigTableHandle, 2, ATTR_USE_LABEL_TEXT,1);
    SetTableColumnAttribute (DIOConfigurationPanelHandle, DIOConfigTableHandle, 2, ATTR_COLUMN_WIDTH,50);
    SetTableColumnAttribute (DIOConfigurationPanelHandle, DIOConfigTableHandle, 2, ATTR_CELL_TYPE,VAL_CELL_BUTTON);

    InsertTableColumns (DIOConfigurationPanelHandle, DIOConfigTableHandle, 3, 1, VAL_CELL_BUTTON);
    SetTableColumnAttribute (DIOConfigurationPanelHandle, DIOConfigTableHandle, 3, ATTR_LABEL_TEXT,DIOParamName[2]);
    SetTableColumnAttribute (DIOConfigurationPanelHandle, DIOConfigTableHandle, 3, ATTR_USE_LABEL_TEXT,1);
    SetTableColumnAttribute (DIOConfigurationPanelHandle, DIOConfigTableHandle, 3, ATTR_COLUMN_WIDTH,70);
    SetTableColumnAttribute (DIOConfigurationPanelHandle, DIOConfigTableHandle, 3, ATTR_CELL_TYPE,VAL_CELL_BUTTON);
    
    InsertTableColumns (DIOConfigurationPanelHandle, DIOConfigTableHandle, 4, 1, VAL_CELL_STRING);
    SetTableColumnAttribute (DIOConfigurationPanelHandle, DIOConfigTableHandle, 4, ATTR_LABEL_TEXT,DIOParamName[3]);
    SetTableColumnAttribute (DIOConfigurationPanelHandle, DIOConfigTableHandle, 4, ATTR_USE_LABEL_TEXT,1);
    SetTableColumnAttribute (DIOConfigurationPanelHandle, DIOConfigTableHandle, 4, ATTR_COLUMN_WIDTH,310);
    SetTableColumnAttribute (DIOConfigurationPanelHandle, DIOConfigTableHandle, 4, ATTR_CELL_TYPE,VAL_CELL_STRING);
    
    LoadDIOConfigFile();
    
    //DisplayPanel(DIOConfigurationPanelHandle);
}
/********************************************//**
* DIO config table cb function for toggling betweehn input and output
***********************************************/
int CVICALLBACK  DIOTableCBFunction (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    if(event == EVENT_COMMIT && eventData2==2)
    {
        char CellVal[MAXCHARARRAYLENGTH];
        GetTableCellVal(DIOConfigurationPanelHandle, DIOConfigTableHandle,MakePoint(eventData2,eventData1),CellVal);
        if(!stricmp(CellVal,"Input"))
            SetTableCellVal(DIOConfigurationPanelHandle, DIOConfigTableHandle,MakePoint(eventData2,eventData1),"Output");
        if(!stricmp(CellVal,"Output"))
            SetTableCellVal(DIOConfigurationPanelHandle, DIOConfigTableHandle,MakePoint(eventData2,eventData1),"Input");
    }
    if(event == EVENT_COMMIT && eventData2==3)
    {
        char CellVal2[MAXCHARARRAYLENGTH];
        GetTableCellVal(DIOConfigurationPanelHandle, DIOConfigTableHandle,MakePoint(eventData2,eventData1),CellVal2);
        if(!stricmp(CellVal2,"No"))
            SetTableCellVal(DIOConfigurationPanelHandle, DIOConfigTableHandle,MakePoint(eventData2,eventData1),"Yes");
        if(!stricmp(CellVal2,"Yes"))
            SetTableCellVal(DIOConfigurationPanelHandle, DIOConfigTableHandle,MakePoint(eventData2,eventData1),"No");
    }
    return 0;
}
/********************************************//**
* Quit/hide DIO config table
***********************************************/
int CVICALLBACK  QuitDIOConfigTableCB (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    if(event == EVENT_COMMIT )
    {
        HidePanel(DIOConfigurationPanelHandle);
    }
    return 0;
}
/********************************************//**
* Quit/hide DIO Debug panel
***********************************************/
int CVICALLBACK  QuitDIODebugCB (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    if(event == EVENT_COMMIT )
    {
        //StopDIOInputUpdate();
        DIOUpdateStarted=0;
        HidePanel(DIODebugPanelHandle);
    }
    return 0;
}
/********************************************//**
 *  Callback for the Save button in DIO Configuration panel
 *  This function create a back of the current Test Configuration xml file and saves the new config into the file.
 ***********************************************/
int CVICALLBACK  SaveDIOConfigTableCB (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{

    if(event == EVENT_COMMIT )
    {
        int response = ConfirmPopup  ("Please Confirm", "The GUI will need to be restarted if you save any changes.\n Continue Saving?");

        if(!response)
            return 0;
        int error;
        char msg[MAXCHARARRAYLENGTH];
        CVIXMLDocument   doc;
        CVIXMLElement    curElem = 0;
        CVIXMLElement    childElem = 0;
        CVIXMLElement    childElem1 = 0;
        char rootTag[]= {"DIOHW"};
        int numRows;
        char tag[120];
        char value[120];

        error = SaveBackupXmlFilenameDIO(PathToDIOConfigFile);

        //CVIXMLLoadDocument(filePath,&doc);
        CVIXMLNewDocument(rootTag,&doc);
        CVIXMLGetRootElement(doc,&curElem);

        // get number of table rows
        GetNumTableRows(panel,DIOConfigTableHandle,&numRows);
        if (numRows <= 0)
        {
            sprintf(msg,"Invalid number of row %i",numRows);
            MessagePopup("Error",msg);
            return(-1);
        }
        for (int i=numRows-1; i>=0; i--)
        {
            CVIXMLNewElement(curElem,0,"DIO",&childElem);
            for(int j=3; j>=0; j--)
            {
                GetTableColumnAttribute(panel,DIOConfigTableHandle,j+1,ATTR_LABEL_TEXT,tag);
                GetTableCellVal(panel,DIOConfigTableHandle,MakePoint(j+1,i+1),value);
                if (ISValidXMLDIO(tag))
                {
                    CVIXMLNewElement(childElem,0,tag,&childElem1);
                    CVIXMLSetElementValue(childElem1,value);
                }
                else
                {
                    MessagePopup("Invalid XML tag",tag);
                }
                if (childElem1)
                {
                    CVIXMLDiscardElement(childElem1);
                    childElem1=0;
                }
            }
            if (childElem1)
            {
                CVIXMLDiscardElement(childElem1);
                childElem1=0;
            }
            if (childElem)
            {
                CVIXMLDiscardElement(childElem);
                childElem=0;
            }
        }
        error = CVIXMLSaveDocument(doc,1,PathToDIOConfigFile);
        if (error == 0)
        {
            sprintf(msg,"%s is saved successfully.",PathToDIOConfigFile);
            MessagePopup("Message",msg);
        }
        else
        {
            sprintf(msg,"Unable to save xml file %s",PathToDIOConfigFile);
            MessagePopup("Error",msg);
            return(-1);
        }
        // release and free xml memory
        if (curElem)
        {
            CVIXMLDiscardElement(curElem);
        }
        if (doc)
        {
            CVIXMLDiscardDocument(doc);
        }
        //CloseAndStopDAQMxTask();
        ReadDIOConfigurationFile(PathToDIOConfigFile);
        CreateDIODebugPanel(MainPanelHandlebk);
        //CreateAndInitDAQMxTask();
        QuitUserInterface(0);
    }

    return 0;
}
/********************************************//**
 *  Save backup copy of the existing DIO config xml file
 ***********************************************/
int SaveBackupXmlFilenameDIO(const char *filename)
{
    int error;
    char backUpFilename[MAXCHARARRAYLENGTH];
    char tempStr[MAXCHARARRAYLENGTH];
    int year,month,day,hour,min,sec;
    GetSystemTime(&hour,&min,&sec);
    GetSystemDate(&month,&day,&year);
    strcpy(backUpFilename,filename);
    char * pch;
    pch = strstr(backUpFilename,".xml");
    sprintf(tempStr,"%04d%02d%02d%02d%02d%02d.bak",year,month,day,hour,min,sec);
    strncpy(pch,tempStr,19);
    error = CopyFile(filename,backUpFilename);
    return(error);
} // end SaveBackupXmlFilename
/********************************************//**
 * read the xml DIO configuration from specified path and populate DIOFileInfo structure
 ***********************************************/
int  ReadDIOConfigurationFile(char *filePath)
{
    CVIXMLElement    curElem = 0;
    CVIXMLDocument   doc;
    CVIXMLElement    childElem = 0;
    CVIXMLElement    childElem1 = 0;
    sprintf(PathToDIOConfigFile,"%s",filePath);
    
    int numChildren = 0;
    int numSubChildren = 0;
    char value[120];
    int address;
    // load document
    CVIXMLLoadDocument(filePath,&doc);
    CVIXMLGetRootElement(doc,&curElem);

    CVIXMLGetNumChildElements(curElem,&numChildren);
    for (int i=0; i<numChildren; i++)
    {
        CVIXMLGetChildElementByIndex(curElem,i,&childElem);
        CVIXMLGetNumChildElements(childElem,&numSubChildren);
        for(int j=0; j<numSubChildren; j++)
        {
            CVIXMLGetChildElementByTag(childElem,DIOParamName[j],&childElem1);
            CVIXMLGetElementValue(childElem1,value);

            //Take the address DIOFileInfo[k].testNum[0] and cast it to int. Since all elements of this
            //DIOInfo[0] struct are the same size to go to the next element we simply add the size of it.
            address =  ((int)&(DIOFileInfo[i].FullIOPath[0]))+(j*sizeof(DIOFileInfo[0].FullIOPath)) ;
            sprintf((char*)address,"%s",value);

            if(childElem1){
                CVIXMLDiscardElement(childElem1);
            }
        }
        if(childElem){
            CVIXMLDiscardElement(childElem);
        }
    }
    if(curElem){
        CVIXMLDiscardElement(curElem);
    }
    if(doc){
        CVIXMLDiscardDocument(doc);
    }
    SortDIOFileInfo();

    return(numChildren);
}
/********************************************//**
 *  Sort DIOFileInfo array inputs first then outputs
 ***********************************************/
void SortDIOFileInfo(void)
{
    int cnt=0;
    int i;
    
    DIOFileInfoStruct tempDIOFileInfo[MAXNUMBEROFDIOPORTS]= {0};
    for(i=0; i<NumOfDIOPorts; i++)
    {
        if(stricmp(DIOFileInfo[i].Direction,"Input")==0)
        {
            sprintf(tempDIOFileInfo[cnt].FullIOPath,"%s",DIOFileInfo[i].FullIOPath);
            sprintf(tempDIOFileInfo[cnt].Direction,"%s",DIOFileInfo[i].Direction);
            sprintf(tempDIOFileInfo[cnt].InverseLogic,"%s",DIOFileInfo[i].InverseLogic);
            sprintf(tempDIOFileInfo[cnt].SignalName,"%s",DIOFileInfo[i].SignalName);
            //tempDIOFileInfo[cnt].DAQMxTaskHandle=DIOFileInfo[i].DAQMxTaskHandle;
            cnt++;
        }
    }
    for(i=0; i<NumOfDIOPorts; i++)
    {
        if(stricmp(DIOFileInfo[i].Direction,"Output")==0)
        {
            sprintf(tempDIOFileInfo[cnt].FullIOPath,"%s",DIOFileInfo[i].FullIOPath);
            sprintf(tempDIOFileInfo[cnt].Direction,"%s",DIOFileInfo[i].Direction);
            sprintf(tempDIOFileInfo[cnt].InverseLogic,"%s",DIOFileInfo[i].InverseLogic);
            sprintf(tempDIOFileInfo[cnt].SignalName,"%s",DIOFileInfo[i].SignalName);
            //tempDIOFileInfo[cnt].DAQMxTaskHandle=DIOFileInfo[i].DAQMxTaskHandle;
            cnt++;
        }
    }
    for(i=0; i<NumOfDIOPorts; i++)
    {

        sprintf(DIOFileInfo[i].FullIOPath,"%s",tempDIOFileInfo[i].FullIOPath);
        sprintf(DIOFileInfo[i].Direction,"%s",tempDIOFileInfo[i].Direction);
        sprintf(tempDIOFileInfo[cnt].InverseLogic,"%s",DIOFileInfo[i].InverseLogic);
        sprintf(DIOFileInfo[i].SignalName,"%s",tempDIOFileInfo[i].SignalName);
        //DIOFileInfo[i].DAQMxTaskHandle=tempDIOFileInfo[i].DAQMxTaskHandle;

    }

}
/********************************************//**
 *  This function takes the data previously loaded into DIO struct and populats the test configuration table.
 ***********************************************/
void LoadDIOConfigFile(void)
{
    for(int i=0; i<NumOfDIOPorts; i++)
    {
        SetTableCellVal (DIOConfigurationPanelHandle, DIOConfigTableHandle, MakePoint (1, i+1), DIOFileInfo[i].FullIOPath);
        SetTableCellVal (DIOConfigurationPanelHandle, DIOConfigTableHandle, MakePoint (2, i+1), DIOFileInfo[i].Direction);
        SetTableCellVal (DIOConfigurationPanelHandle, DIOConfigTableHandle, MakePoint (3, i+1), DIOFileInfo[i].InverseLogic);
        SetTableCellVal (DIOConfigurationPanelHandle, DIOConfigTableHandle, MakePoint (4, i+1), DIOFileInfo[i].SignalName);
    }
}
/********************************************//**
 *  Write specified value to specified signal name
 ***********************************************/
int __stdcall WriteDIOSignal(char * SignalName, int Value)
{
    //TaskHandle  taskHandle=0;
    //uInt8       data[8];
    char chan[256]= {0};
    char direction[10]= {0};
    char inverselogic[10] = {0};
    int data = -1;
    //FindChanelFromSignalName(SignalName,chan, direction, &inverselogic, &taskHandle);
    FindChanelFromSignalName_TB(SignalName, direction, &inverselogic);
    if(stricmp(inverselogic,"yes") == 0)
        data = !Value;
    else
        data = Value;
    //if(stricmp(inverselogic,"yes") == 0)
        //data[0]=(uInt8)!Value;
    //else
        //data[0]=(uInt8)Value;
    //if(taskHandle)
        //DAQmxWriteDigitalLines(taskHandle,1,1,10.0,DAQmx_Val_GroupByChannel,&data[0],NULL,NULL);
    WriteDigitalLines(SignalName, data);
    return data;//data[0];
}
/********************************************//**
 *  Read specified signal name
 ***********************************************/
int __stdcall ReadDIOSignal(char * SignalName)
{
    //TaskHandle  taskHandle=0;
    //uInt8       data[100]= {0};
    char chan[256]= {0};
    char direction[10]= {0};
    char inverselogic[10] = {0};
    int data = -1;
    //int32       read,bytesPerSamp;
    //FindChanelFromSignalName(SignalName,chan,direction, &inverselogic, &taskHandle);
    FindChanelFromSignalName_TB(SignalName, direction, &inverselogic);
    
    data = ReadDigitalLines(SignalName);
    //if(taskHandle)
        //DAQmxReadDigitalLines(taskHandle,1,10.0,DAQmx_Val_GroupByChannel,data,100,&read,&bytesPerSamp,NULL);
    
    if(stricmp(inverselogic,"yes") == 0)
        return !data;
    else
        return data;
    //if(stricmp(inverselogic,"yes") == 0)
        //return !data[0];
    //else
        //return //data[0];
}
/********************************************//**
 *  find the full IO path and direction given a signal name;
 ***********************************************
int FindChanelFromSignalName(char *SignalName, char *channel,char *Direction, char *InverseLogic, TaskHandle  *DAQMxTaskHandle)
{
    for(int i=0; i<NumOfDIOPorts; i++)
    {
        if(stricmp(DIOFileInfo[i].SignalName,SignalName)==0)
        {
            sprintf(channel,"%s",DIOFileInfo[i].FullIOPath);
            sprintf(Direction,"%s",DIOFileInfo[i].Direction);
            sprintf(InverseLogic,"%s",DIOFileInfo[i].InverseLogic);
            *DAQMxTaskHandle = DIOFileInfo[i].DAQMxTaskHandle;
            return 0;
        }
    }
    return -1;
}*/

int FindChanelFromSignalName_TB(char *SignalName, char *Direction, char *InverseLogic){
    
    for(int i=0; i<NumOfDIOPorts; i++){
        if(stricmp(DIOFileInfo[i].SignalName,SignalName) == 0){
            sprintf(Direction, "%s", DIOFileInfo[i].Direction);
            sprintf(InverseLogic, "%s", DIOFileInfo[i].InverseLogic);
            //Status = DIOFileInfo[i].Status;
            return 0;
        }
    }
}

int ReadDigitalLines(char *SignalName){
    
    for(int i=0; i<NumOfDIOPorts; i++){
        if(stricmp(DIOFileInfo[i].SignalName,SignalName) == 0)
            return DIOFileInfo[i].Status;
    }
}

void WriteDigitalLines(char *SignalName, int Data){
    
    for(int i=0; i<NumOfDIOPorts; i++){
        if(stricmp(DIOFileInfo[i].SignalName,SignalName) == 0)
            DIOFileInfo[i].Status = Data;
    }
}
/********************************************//**
 *  Get DioDebug panel handle
 ***********************************************/
int __stdcall GetDIODebugPanelHandle(void)
{
    return  DIODebugPanelHandle;
}
/********************************************//**
 *  Get DioConfiguration panel handle
 ***********************************************/
int __stdcall GetDIOConfigurationPanelHandle(void)
{
    return  DIOConfigurationPanelHandle;
}

int __cdecl GetDioPanelLoadStatus(void){
    return DIOPanelLoaded;
}


int ISValidXMLDIO (char * string)
{
    int     i = 0;
    size_t  len = 0;
    int     allNums = 0;

    if (string[0] == '\0')
        return 0;

    len = strlen(string);
    for (i = 0; i < len; i++)
    {
        if (isdigit (string[i]))
        {
            if (i == 0)
                allNums = 1;
        }
        else
        {
            allNums = 0;
        }

        if (!isalnum (string[i]) || isspace (string[i]))
            return 0;
    }

    if (allNums)
        return 0;

    return 1;
}  // end IsValidXML
/********************************************//**
 *  get library revision
 ***********************************************
char __stdcall * getDIOLibRevision (void)
{
    return DIOLIBREV;
}*/

/********************************************//**
 *  Loads DIO Panel
 ***********************************************/
int __cdecl LoadDIOPanel(char *DIODirectory){
    
    char dir[MAXCHARARRAYLENGTH], uirPath[MAXCHARARRAYLENGTH];
    int panelHandle;
    DIOPanelLoaded = 1;
    //GetProjectDir(dir);
    sprintf(dir,"%s",DIODirectory);
    //strcpy(dir, "C:\\Users\\ted\\Documents\\GitHub\\Universal_TestStand\\Source\\DIO");
    sprintf(uirPath, "%s%s",dir,"\\DIOPanel.uir");
    
    if((panelHandle = LoadPanelEx(0, uirPath, DIOPANEL, __CVIUserHInst))<0)
        return -1;
    
    DisplayPanel(panelHandle);
    SetCtrlVal(panelHandle,DIOPANEL_INPUT,GetNumOfDIOInputs());
    SetCtrlVal(panelHandle,DIOPANEL_OUTPUT,GetNumOfDIOOutputs());
    RunUserInterface();
    DiscardPanel(panelHandle);
    
    return 0;
}
/********************************************************************************//**
 *  ALL Callback functions for the DIO Panel
 ***********************************************************************************/
/********************************************//**
 *  Loads the Debug Panel
 ***********************************************/
int CVICALLBACK DebugPanel (int panel, int control, int event,
                            void *callbackData, int eventData1, int eventData2) {
    switch (event) {
        case EVENT_COMMIT:
            DisplayPanel(GetDIODebugPanelHandle());
            break;
        case EVENT_RIGHT_CLICK_UP:

            break;
    }
    return 0;
}

int CVICALLBACK Timer_Display (int panel, int control, int event,
                               void *callbackData, int eventData1, int eventData2) {
    
    //GetDIOConfigurationPanelHandle();
    UpdateDIOOutputs_TB();
    UpdateDIOInputs_TB();
    return 0;
}

/********************************************//**
 *  Quits the DIO Panel
 ***********************************************/
int CVICALLBACK quitPanel (int panel, int control, int event,
                           void *callbackData, int eventData1, int eventData2) {
    switch (event) {
        case EVENT_COMMIT:
            //CloseAndStopDAQMxTask();
            HidePanel(GetDIOConfigurationPanelHandle());
            HidePanel(GetDIODebugPanelHandle());
            DIOPanelLoaded = 0;
            QuitUserInterface(0);
            break;
        case EVENT_RIGHT_CLICK_UP:

            break;
    }
    return 0;
}

/********************************************//**
 *  Opens the DIO Config Panel
 ***********************************************/
int CVICALLBACK configPanel (int panel, int control, int event,
                             void *callbackData, int eventData1, int eventData2) {
    switch (event) {
        case EVENT_COMMIT:
            DisplayPanel(GetDIOConfigurationPanelHandle());
            break;
        case EVENT_RIGHT_CLICK_UP:

            break;
    }
    return 0;
}



