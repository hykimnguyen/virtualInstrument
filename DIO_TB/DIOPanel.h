/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  DIOPANEL                         1
#define  DIOPANEL_QUITBUTTON              2       /* control type: command, callback function: quitPanel */
#define  DIOPANEL_CONFIGPANEL             3       /* control type: command, callback function: configPanel */
#define  DIOPANEL_DEBUG                   4       /* control type: command, callback function: DebugPanel */
#define  DIOPANEL_INPUT                   5       /* control type: numeric, callback function: (none) */
#define  DIOPANEL_OUTPUT                  6       /* control type: numeric, callback function: (none) */
#define  DIOPANEL_TIMER                   7       /* control type: timer, callback function: Timer_Display */


     /* Control Arrays: */

          /* (no control arrays in the resource file) */


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */

int  CVICALLBACK configPanel(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK DebugPanel(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK quitPanel(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK Timer_Display(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
