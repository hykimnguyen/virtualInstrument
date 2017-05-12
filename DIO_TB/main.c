#include <userint.h>
#include <utility.h>
#include <ansi_c.h>
//#include <DIO_LIB.h>
#include <cvirte.h>
#include "DIOPanel.h"
#include "toolbox.h"
#include <DIO_TB.h>


#define MAXCHARARRAYLENGTH 400

static int panelHandle;

/*-------------------------------------//
 * main is used for .exe test
/*-------------------------------------*/
int main(int argc, char*argv[]){
    //Variables
    char configFilePath[MAXCHARARRAYLENGTH];
    char finalPath[MAXCHARARRAYLENGTH];
    char userInput[64];

    int result=0;
    
    //Find project directory and file name
    GetProjectDir(configFilePath);
    printf("--Current directory: %s\n", configFilePath);
    printf("Enter the .XML file you want to use: ('exit' to quit)\n");
    while(1){
        scanf("%s", userInput);
    
        MakePathname(configFilePath, userInput,finalPath);
        result = FileExists(finalPath, 0);
        if(strcmp(userInput, "exit") == 0){
            break;
        }
        if(result == 0){
            printf("File was not found. (%d) -- Make sure file is in the above directory --\n", result);
        }else {
            break;
        }
        
    }
    //Loading Panel and initialize DIOLibrary
    //Display Panels
    if(result != 0){
        if(InitCVIRTE(0, argv, 0) == 0)
            return -1;  //Out of Memory
        if((panelHandle = LoadPanel(0,"DIOPanel.uir",DIOPANEL))<0)
            return -1;

        InitializeDIO_TB(finalPath, 0);
        DisplayPanel(panelHandle);
        //DisplayPanel(GetDIOConfigurationPanelHandle());
    
        SetCtrlVal(panelHandle,DIOPANEL_INPUT,GetNumOfDIOInputs());
        SetCtrlVal(panelHandle,DIOPANEL_OUTPUT,GetNumOfDIOOutputs());
    
        //Run the Panel
        RunUserInterface();

        //Discard Panel upon exiting the UI
        DiscardPanel(panelHandle);
    }
    return 0;
}

/*-------------------------------------//
 * Call to load panel
/*-------------------------------------*/
//int LoadDIOPanel(char *DIODirectory){
//    
//    char dir[MAXCHARARRAYLENGTH], uirPath[MAXCHARARRAYLENGTH];
//    
//    //GetProjectDir(dir);
//    sprintf(dir,"%s",DIODirectory);
//    //strcpy(dir, "C:\\Users\\ted\\Documents\\GitHub\\Universal_TestStand\\Source\\DIO");
//    sprintf(uirPath, "%s%s",dir,"\\DIOPanel.uir");
//    
//    if((panelHandle = LoadPanelEx(0, uirPath, DIOPANEL, __CVIUserHInst))<0)
//        return -1;
//    
//    DisplayPanel(panelHandle);
//    SetCtrlVal(panelHandle,DIOPANEL_INPUT,GetNumOfDIOInputs());
//    SetCtrlVal(panelHandle,DIOPANEL_OUTPUT,GetNumOfDIOOutputs());
//    RunUserInterface();
//    DiscardPanel(panelHandle);
//    
//    return 0;
//}
//
//int CVICALLBACK DebugPanel (int panel, int control, int event,
//                            void *callbackData, int eventData1, int eventData2) {
//    switch (event) {
//        case EVENT_COMMIT:
//            DisplayPanel(GetDIODebugPanelHandle());
//            break;
//        case EVENT_RIGHT_CLICK_UP:
//
//            break;
//    }
//    return 0;
//}
//
//int CVICALLBACK Timer_Display (int panel, int control, int event,
//                               void *callbackData, int eventData1, int eventData2) {
//    
//    GetDIOConfigurationPanelHandle();
//    return 0;
//}
//
//int CVICALLBACK quitPanel (int panel, int control, int event,
//                           void *callbackData, int eventData1, int eventData2) {
//    switch (event) {
//        case EVENT_COMMIT:
//            CloseAndStopDAQMxTask();
//            QuitUserInterface(0);
//            break;
//        case EVENT_RIGHT_CLICK_UP:
//
//            break;
//    }
//    return 0;
//}
//
//int CVICALLBACK configPanel (int panel, int control, int event,
//                             void *callbackData, int eventData1, int eventData2) {
//    switch (event) {
//        case EVENT_COMMIT:
//            DisplayPanel(GetDIOConfigurationPanelHandle());
//            break;
//        case EVENT_RIGHT_CLICK_UP:
//
//            break;
//    }
//    return 0;
//}
