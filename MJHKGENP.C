/***************************************************************************/
/*                                                                         */
/* PROGRAM NAME: MJHKGENP - Generic System Wide Input Hook Procedure       */
/* -------------                                                           */
/*  Process input messages.  Start the Command To Execute if the           */
/*  Key To Intercept is pressed.  Pass the Argument String to the          */
/*  Command To Execute as parameters.                                      */
/*                                                                         */
/* IN ORDER TO BE ABLE TO READ ALL THE COMMENTS IN THIS                    */
/* PROGRAM, READ THE SOURCE FILES INCLUDED SEPARATELY.                     */
/*                                                                         */
/* AUTHOR:  Michael R. Jones (Fidonet Net Mail address 1:202/204)          */
/* -------                                                                 */
/*                                                                         */
/* LOG:  04/29/91 Program Creation                                         */
/* ----                                                                    */
/*                                                                         */
/* RESTRICTIONS:  None.  This program is Public Domain.  Please note that  */
/* -------------         this program is not complete.  It was written as  */
/*                       a sample System Input Queue Hook for the IdleNews */
/*                       Newsletter. It works but isn't production ready.  */
/***************************************************************************/

#define INCL_DOS
#define INCL_ERRORS
#define INCL_PM
#include <os2.h>

#include <stdio.h>
#include <string.h>

int            _acrtused = 0;
BYTE           byteKeyToIntercept = ' ';

BOOL EXPENTRY GenericHookProc( HAB, PQMSG, USHORT );
BOOL EXPENTRY GenericHookStartProcess( HAB, PQMSG, USHORT );

BOOL EXPENTRY GenericHookProc( hab, pqmsg, usRemove )
HAB            hab;
PQMSG          pqmsg;
USHORT         usRemove;
  {
  PBYTE            pbyte;
  SEL              selSharedMJHKGEN;

  if( byteKeyToIntercept == ' ' )        /* Only get the Key To Intercept  */
    {                                    /* once.                          */
    DosGetShrSeg( "\\SHAREMEM\\MJHKGEN",
                  &selSharedMJHKGEN );
    pbyte = MAKEP(selSharedMJHKGEN, 1);
    byteKeyToIntercept = *pbyte;
    DosFreeSeg( selSharedMJHKGEN );
    }

  if( pqmsg->msg != WM_CHAR )          /* Only care about WM_CHAR messages */
    goto end_function;

  if( SHORT1FROMMP(pqmsg->mp1)         /* Ignore KEYUP, process down       */
        & KC_KEYUP )
    goto end_function;

  switch( SHORT2FROMMP(pqmsg->mp2) )   /* Check if the key pressed matches */
    {                                  /* the Key To Intercept.            */
    case VK_F1:
      if( byteKeyToIntercept == '1' )  /* Note that this construct is quite*/
        goto start_process;            /* crude.  A more generic, efficient*/
      break;                           /* routine should be used to see if */
    case VK_F2:                        /* the key pressed matches the Key  */
      if( byteKeyToIntercept == '2' )  /* To Intercept.  It would be better*/
        goto start_process;            /* to translate the Key To Intercept*/
      break;                           /* to the VK_ equivalent in the     */
    case VK_F3:                        /* MJHKGEN.C program so that this   */
      if( byteKeyToIntercept == '3' )  /* comparision could be done in one */
        goto start_process;            /* line of code.  Efficiency is very*/
      break;                           /* important inside this function   */
    case VK_F4:                        /* because it will most likely get  */
      if( byteKeyToIntercept == '4' )  /* executed for every input message */
        goto start_process;            /* for all applications.  For that  */
      break;                           /* matter, these hooks would be best*/
    case VK_F5:                        /* implemented for production use in*/
      if( byteKeyToIntercept == '5' )  /* assembler.  C language is best   */
        goto start_process;            /* for prototyping since debugging  */
      break;                           /* is easier.  Debugging is a major */
    case VK_F6:                        /* concern since you have to reboot */
      if( byteKeyToIntercept == '6' )  /* or kill PM entirely to free this */
        goto start_process;            /* DLL in order to replace it.      */
      break;
    case VK_F7:
      if( byteKeyToIntercept == '7' )
        goto start_process;
      break;
    case VK_F8:
      if( byteKeyToIntercept == '8' )
        goto start_process;
      break;
    case VK_F9:
      if( byteKeyToIntercept == '9' )
        goto start_process;
      break;
    }

  goto end_function;

  start_process:
   GenericHookStartProcess( hab, pqmsg, usRemove );

  end_function:
   return( FALSE ); /* Returning FALSE allows other hooks and the recipient*/
                    /* window to receive this message unless another hook  */
                    /* later in the chain returns TRUE which halts the     */
                    /* passing of the message down the line.  Returning    */
                    /* TRUE is one way of disabling system or application  */
                    /* functions.                                          */
  }


/***************************************************************************\
| Start the Command To Execute running as an asynchronous process.          |
\***************************************************************************/

BOOL EXPENTRY GenericHookStartProcess( hab, pqmsg, usRemove )
HAB            hab;
PQMSG          pqmsg;
USHORT         usRemove;
  {

  PSZ              pszArgumentString;
  PSZ              pszCommandToExecute;
  RESULTCODES      ResultCodes;
  SEL              selSharedMJHKGEN;

  DosGetShrSeg( "\\SHAREMEM\\MJHKGEN",  /* Get access to arguments.  Access*/
                &selSharedMJHKGEN );    /* should really be semaphored.    */

  pszCommandToExecute
    = MAKEP(selSharedMJHKGEN,
            strlen( MAKEP(selSharedMJHKGEN,0) ) + 1);

  pszArgumentString
    = pszCommandToExecute
       + strlen( pszCommandToExecute ) + 1;

  DosExecPgm( NULL,               /* This should really be DosStartSession */
              0,                  /* which can start executables which are */
              EXEC_ASYNC,         /* of a different executable type than   */
              pszArgumentString,  /* the calling session.  DosExecPgm will */
              (PSZ)NULL,          /* only successfully start PM programs.  */
              &ResultCodes,
              pszCommandToExecute );

  DosFreeSeg( selSharedMJHKGEN ); /* Keep shared memory freed for MJHKGEN.C*/

  return( TRUE );
  }
