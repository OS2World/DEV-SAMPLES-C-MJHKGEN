/***************************************************************************/
/*                                                                         */
/* PROGRAM NAME: MJHKGEN - Generic System Input Queue Hook                 */
/* -------------                                                           */
/*  Allow a Generic Hook to be installed.  This program accepts three      */
/*  parameters: 1) Key To Intercept (only will intercept F1 through F9),   */
/*              2) Command To Execute,                                     */
/*              3) Argument String to pass to the Command To Execute.      */
/*  The parameters are then put in a shared memory segment for access by   */
/*  the MJHKGENP.DLL hook procedure "GenericHookProc".  If the hook is     */
/*  "set" and the Key To Intercept is pressed then GenericHookProc will    */
/*  execute the Command To Execute via DosExecPgm passing the Argument     */
/*  String as parameters.                                                  */
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
/*                       a sample System Queue Input Hook for the IdleNews */
/*                       Newsletter. It works but isn't production ready.  */
/***************************************************************************/

#define INCL_DOS
#define INCL_ERRORS
#define INCL_PM
#include <os2.h>

#include <stdio.h>
#include <string.h>

#include "mjhkgen.h"

HAB              hab;
HMODULE          hmodule = (HMODULE)NULL;
PFN              pfnInputHook;
SEL              selSharedMJHKGEN = (SEL)NULL;

MRESULT EXPENTRY GenericHookWndProc( HWND, USHORT, MPARAM, MPARAM );
VOID APIENTRY ProgramTermination( VOID );

VOID cdecl main( argc, argv, envp )
int       argc;
char      **argv;
char      **envp;
  {
  HMQ              hmq;
  HPOINTER         hptrIcon;
  HWND             hwndDlg;
  PBYTE            pbyte;
  PSZ              pszPeriod;
  QMSG             qmsg;
  USHORT           usBaseErrorCode;          /* Base (DOS) error code.     */
  USHORT           usArgSize[4];

  if( argc < 3 ) return;       /* At least the first 2 arguments are req'd */

  pszPeriod = strstr( argv[2], "." );     /* Find the period in executable */

  usArgSize[0]   = strlen( argv[1] ) + 1;    /* Get the sizes of all the   */
  usArgSize[1]   = strlen( argv[2] ) + 1;    /* arguments including an     */
  if( pszPeriod == (PSZ)NULL )               /* argument constructed here  */
    usArgSize[2] = usArgSize[1];             /* representing the simple    */
  else                                       /* command name per CMD.EXE   */
    usArgSize[2] = pszPeriod - argv[2] + 1;  /* conventions.  The total of */
  if( argc > 3 )                             /* the sizes becomes the size */
    usArgSize[3] = strlen( argv[3] ) + 1;    /* of the shared memory       */
  else                                       /* allocated.                 */
    usArgSize[3] = 1;

  usBaseErrorCode
    = DosAllocShrSeg( usArgSize[0]           /* Include room for an extra  */
                        + usArgSize[1]       /* NULL which terminates the  */
                        + usArgSize[2]       /* argument block used by     */
                        + usArgSize[3]       /* DosExecPgm in the hook     */
                        + 1,                 /* procedure.                 */
                      "\\SHAREMEM\\MJHKGEN", /* Shared Memory Name         */
                      &selSharedMJHKGEN );   /* Selector for shared memory */

  if( usBaseErrorCode                        /* Only allow one copy of     */
       == ERROR_ALREADY_EXISTS ) return;     /* this program to run.       */

  strcpy( MAKEP(selSharedMJHKGEN, 0),        /* Load F1 through F9 argument*/
          argv[1] );

  strcpy( MAKEP(selSharedMJHKGEN,            /* Load Command To Execute    */
                usArgSize[0]),
          argv[2] );

  memcpy( MAKEP(selSharedMJHKGEN,            /* Load Simple Command Name   */
                usArgSize[0] + usArgSize[1]),
          argv[2],
          usArgSize[2] - 1 );

  pbyte = MAKEP(selSharedMJHKGEN,            /* Add trailing NULL          */
                usArgSize[0]
                 + usArgSize[1]
                 + usArgSize[2]
                 - 1);
  *pbyte = NULL;

  if( argc > 3 )                             /* Load Argument String       */
    strcpy( MAKEP(selSharedMJHKGEN,
                  usArgSize[0]
                   + usArgSize[1]
                   + usArgSize[2]),
            argv[3] );
  else
    {
    pbyte = MAKEP(selSharedMJHKGEN,          /* Load NULL for Arg. String  */
                  usArgSize[0]
                   + usArgSize[1]
                   + usArgSize[2]);
    *pbyte = NULL;
    }

  pbyte = MAKEP(selSharedMJHKGEN,            /* End Argument Block w/ NULL */
                usArgSize[0]
                 + usArgSize[1]
                 + usArgSize[2]
                 + usArgSize[3]);
  *pbyte = NULL;

  hab = WinInitialize( NULL );
  hmq = WinCreateMsgQueue( hab, 0 );

  DosExitList( EXLST_ADD | 0xFF00,           /* Ensure proper pgm. ending  */
               (PFNEXITLIST)ProgramTermination );

  DosLoadModule( NULL,                       /* Load the DLL module        */
                 0,                          /* containing the Hook        */
                 "MJHKGENP",                 /* procedure (must use a DLL).*/
                 &hmodule );                 /* Get module handle          */

  DosGetProcAddr( hmodule,                   /* Get address of Hook Proc.  */
                  "GENERICHOOKPROC",         /* for the WinSetHook and     */
                  &pfnInputHook );           /* WinReleaseHook calls.      */

  WinSetHook( hab,                           /* Anchor block handle        */
              (HMQ)NULL,                     /* NULL means hook all PM apps*/
              HK_INPUT,                      /* Type of hook: Input Queue. */
              pfnInputHook,                  /* Address of hook procedure. */
              hmodule );                     /* DLL module handle.         */

  WinRegisterClass( hab,
                    CLIENTCLASS,
                    GenericHookWndProc,
                    (ULONG)0,                /* No class styles needed     */
                    (USHORT)0 );             /* No window words needed     */

  hwndDlg = WinLoadDlg( HWND_DESKTOP,        /* Load the dialog window     */
                        HWND_DESKTOP,        /* which allows turning the   */
                        NULL,                /* hook on and off.  This     */
                        NULL,                /* window also provides a     */
                        ID_MJHKGENDLG,       /* visual notification that a */
                        NULL );              /* hook may be active (if set)*/

  hptrIcon = WinQuerySysPointer( HWND_DESKTOP, 
                                 SPTR_APPICON,  /* Get standard icon       */
                                 FALSE );       /* No icon copy needed     */
 
  WinSendMsg( hwndDlg,                       /* Attach the icon to the     */
              WM_SETICON,                    /* frame window.              */
              (MPARAM)hptrIcon,
              (MPARAM)NULL );

  while( WinGetMsg( hab, &qmsg, (HWND)NULL, 0, 0 ) )
    WinDispatchMsg( hab, &qmsg );

  WinReleaseHook( hab,                       /* Make sure hook is released */
                  (HMQ)NULL,
                  HK_INPUT,
                  pfnInputHook,
                  hmodule );

  WinDestroyWindow( hwndDlg );

  WinDestroyMsgQueue( hmq );

  WinTerminate( hab );

  return;
  }


/***************************************************************************|
| Generic Hook Window Procedure                                             |
|***************************************************************************/

MRESULT EXPENTRY GenericHookWndProc( hwnd, msg, mp1, mp2 )
HWND         hwnd;
USHORT       msg;
MPARAM       mp1;
MPARAM       mp2;
  {
  HWND             hwndCheckBox;
  HWND             hwndFrame;
  USHORT           usCheckedState;

  switch(msg)
    {

    case WM_CONTROL:
      if(    (SHORT1FROMMP(mp1) != ID_MJHKGENCB)  /* Only care about CLICK */
          || (SHORT2FROMMP(mp1) != BN_CLICKED) )  /* Check Box messages.   */
        break;
      usCheckedState                               /* Query if check box is*/
        = SHORT1FROMMR( WinSendMsg( hwndCheckBox,  /* set or not after     */
                                    BM_QUERYCHECK, /* activity in the check*/
                                    (MPARAM)NULL,  /* box.                 */
                                    (MPARAM)NULL ) );
      if( usCheckedState == 1 )
        WinSetHook( hab,                   /* Check box is checked so make */
                    (HMQ)NULL,             /* sure hook is "set" to capture*/
                    HK_INPUT,              /* input messages for filtering.*/
                    pfnInputHook,
                    hmodule );
      else
        WinReleaseHook( hab,               /* Check box is not checked so  */
                        (HMQ)NULL,         /* make sure hook is not "set"  */
                        HK_INPUT,          /* so input messages are not    */
                        pfnInputHook,      /* intercepted by the hook      */
                        hmodule );         /* procedure.                   */
      break;

    case WM_ERASEBACKGROUND:
      return( (MRESULT)TRUE );

    case WM_INITDLG:
      hwndFrame = WinQueryWindow( hwnd,       /* Get dialog frame handle   */
                                  QW_PARENT,
                                  FALSE );

      WinSetWindowText( hwndFrame,            /* Set title bar text        */
                        "Generic Hook" );

      hwndCheckBox
        = WinWindowFromID( hwnd,              /* Get Check Box handle      */
                           ID_MJHKGENCB );

      WinSendMsg( hwndCheckBox,               /* Set initial state of the  */
                  BM_SETCHECK,                /* check box to be "checked".*/
                  MPFROMSHORT(1),
                  (MPARAM)NULL );
      break;

    }

  return( WinDefWindowProc( hwnd, msg, mp1, mp2 ) );
  }


/***************************************************************************|
| Exit List processing at program termination                               |
|***************************************************************************/

VOID APIENTRY ProgramTermination( )
  {

  if( hmodule != (HMODULE)NULL )       /* Unfortunately does not free the  */
    DosFreeModule( hmodule );          /* DLL because System Input Queue   */
                                       /* hooks cannot be freed.           */

  if( selSharedMJHKGEN != (SEL)NULL )  /* Free shared memory segment.      */
    DosFreeSeg( selSharedMJHKGEN );    /* At least this works!             */

  DosExitList( EXLST_EXIT,             /* Required to end ExitList process.*/
               (PFNEXITLIST)ProgramTermination );
  }
