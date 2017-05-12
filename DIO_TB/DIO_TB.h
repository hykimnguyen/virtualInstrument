//==============================================================================
//
// Title:       DIO_LIB
// Purpose:     A short description of the interface.
//
// Created on:  4/26/2014 at 1:29:25 PM by dell.
// Copyright:   Customer Service Associates. All Rights Reserved.
//
//==============================================================================

#ifndef __DIO_LIB_H__
#define __DIO_LIB_H__

#ifdef __cplusplus
    extern "C" {
#endif

//==============================================================================
// Include files

#include "cvidef.h"

//==============================================================================
// Constants

//==============================================================================
// Types

//==============================================================================
// External variables

//==============================================================================
// Global functions
        
// Returns the dll Handle to TestStand
__declspec(dllexport) void * ReturnHandle(void);

/********************************************//**
* Initialize the DIO configuration table panel and the DIO debug panel
***********************************************/
void __cdecl InitializeDIO_TB(char *DIOConfigurationFile, int MainPanelHandle);
/********************************************//**
 *  Write specified value to specified signal name
 ***********************************************/
int __stdcall WriteDIOSignal(char * SignalName, int Value);
/********************************************//**
 *  Read specified signal name
 ***********************************************/
int __stdcall ReadDIOSignal(char * SignalName);
/********************************************//**
 *  Get DioDebug panel handle
 ***********************************************/
int __stdcall GetDIODebugPanelHandle(void);
/********************************************//**
 *  Get DioDebug panel handle
 ***********************************************/
int __stdcall GetDIOConfigurationPanelHandle(void) ;
/********************************************//**
 *  get library revision
 ***********************************************/
char __stdcall * getDIOLibRevision (void);


/********************************************//**
 *  get Number of DIO Inputs
 ***********************************************/
int GetNumOfDIOInputs(void);
/********************************************//**
 *  get Number of DIO Outputs
 ***********************************************/
int GetNumOfDIOOutputs(void);

/********************************************//**
 *  Load/Initialize Panel
 ***********************************************/
int __cdecl LoadDIOPanel(char *DIODirectory);

/********************************************//**
 *  Unload all Task
 ***********************************************/
int CloseAndStopDAQMxTask(void);

int __cdecl GetDioPanelLoadStatus(void);

#ifdef __cplusplus
    }
#endif

#endif  /* ndef __DIO_LIB_H__ */
