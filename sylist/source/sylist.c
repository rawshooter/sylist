/* $VER: SYList_Source 1.0 (24.4.94) © by Thomas Alexnat */
/* At this point of time SYList is GiftWare (look into the documentation what this
   means). All rights are reseved by Thomas Alexnat. */

#include <exec/types.h>
#include <dos/dos.h>
#include <dos/rdargs.h>
#include <clib/dos_protos.h>

#include <exec/libraries.h>
#include <libraries/asl.h>
#include <clib/exec_protos.h>
#include <clib/asl_protos.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define TEMPLATE "File/M,All/S,Voices/S,Performances/S,Songs/S,Rhythms/S,Samples/S,NoAbout/S,LaTeX/S,Version/S,NoInit/S,Rows/K/N"
#define OPT_FILE	  0
#define OPT_ALL		  1
#define OPT_VOICES        2
#define OPT_PERFORMANCES  3
#define OPT_SONGS         4
#define OPT_RHYTHMS       5
#define OPT_SAMPLES       6
#define OPT_NOABOUT       7
#define OPT_LATEX	  8
#define OPT_VERSION       9
#define OPT_NOINIT        10
#define OPT_ROWS          11
#define OPT_COUNT	  12

/* SY85 filestructure informations */
#define LENGTH_SYNAMES		8
#define POS_SAVEMODE		0
#define POS_PERFORMANCE_M1	379
#define POS_PERFORMANCE_M2	33147
#define POS_VOICE_M1B1		11291
#define POS_VOICE_M1B2		22049
#define POS_VOICE_M2B1		44059
#define	POS_VOICE_M2B2		54817
#define POS_SEQUENCER		65649
#define POS_RHYTHM		68006
#define POS_SAMPLE		114720
#define POS_KEY_SEQUENCER	65536
#define POS_KEY_SAMPLE		114688
#define LENGTH_PERFORMANCE	168
#define LENGTH_VOICE		162
#define	LENGTH_SONG		198
#define ENTRIES_PERFORMANCE	64
#define ENTRIES_VOICE		64
#define ENTRIES_RHYTHM		100
#define ENTRIES_SONG		10
#define ENTRIES_SAMPLE		64
#define KEY_SEQUENCER		"SY85SEQ"
#define KEY_SAMPLE		"SY85WVALL"
#define KEY_INIT		"Init"
#define SY85			"SY85ALL"

#define VERSTAG "$VER: SYList 1.0 (24.4.94) © by Thomas Alexnat\n"

/* defines for the ASL-Filerequester */
#define MYLEFTEDGE 0
#define MYTOPEDGE  0
#define MYWIDTH    320
#define MYHEIGHT   400


/* ----------------- */
/* --- Variables --- */
/* ----------------- */

LONG    opts[OPT_COUNT];	/* C guarantees this will be all 0's! */
LONG    rows = 4;
LONG    ERROR,counter;			/* ...to check my procedures */
BOOL    About = TRUE;
BOOL    Voices,
        Performances,
        Songs,
        Rhythms,
        Samples,
        ShowVersion,
        NoInit,
        OK = FALSE;
FILE   *fptr = NULL;
char   *Filename,
       *name,
       *file,
       *path;
char  **sptr;			/* Pointer auf das Filearray */
char    buffer[90];
int     rowcounter;
ldiv_t  teiler;
extern long _OSERR;


/* structs for ASL */
struct Library *AslBase = NULL;

struct TagItem frtags[] =
{
  ASL_Hail, (ULONG) "Select a (saved) Yamaha SY85 File",
  ASL_Height, MYHEIGHT,
  ASL_Width, MYWIDTH,
  ASL_LeftEdge, MYLEFTEDGE,
  ASL_TopEdge, MYTOPEDGE,
  ASL_OKText, (ULONG) "OKAY",
  ASL_CancelText, (ULONG) "CANCEL",
  TAG_DONE
};



/* prototypes of my functions */
void    Information (void);
void    Version (void);
int     openfile (void);
int     closefile (void);
void    handlevoices (void);
void    Read_Name (void);
int     Read_Performances (void);
int     Read_Voices (void);
int     Read_Rhythms (void);
int     Read_Sequencer (void);
int     Read_Samples (void);
void    Print_Bank (void);
void    DoRow (void);
void    Print_Name (void);


int
main (argc, argv)
     int     argc;
     char  **argv;
{
  struct RDArgs *argsptr;
  struct FileRequester *fr;	/* for ASL-requester */


  argsptr = ReadArgs (TEMPLATE, opts, NULL);


  if (argsptr == NULL)
    {
      PrintFault (IoErr (), NULL);	/* prints the appropriate err message */
      return RETURN_ERROR;
    }
  else
    {
      sptr = (char **) (opts[OPT_FILE]);
      if (!sptr)
	{
	  if (AslBase = OpenLibrary ("asl.library", 37L))
	    {
	      if (fr = (struct FileRequester *)
		  AllocAslRequest (ASL_FileRequest, frtags))
		{
		  if (AslRequest (fr, NULL))
		    {
		      file = fr->rf_File;
		      path = fr->rf_Dir;

		      if (AddPart (path, file, 512) != NULL)
			{
			  Filename = path;
			  /* printf("File: %s\n",path); */
			  Voices = TRUE;
			  Performances = TRUE;
			  Songs = TRUE;
			  Rhythms = TRUE;
			  Samples = TRUE;
			  OK = TRUE;
			}
		      else
			{
			  printf ("Error while creating internal File-string\n");
			}

		    }
		  FreeAslRequest (fr);
		}
	      else
	      CloseLibrary (AslBase);
	    }
	}
      else
	{
	  Filename = *sptr;	/* convert the specified file to my filename */
	  sptr++;
	}

      /* if a option was not specified, it will be NULL (since buffer started */

      /* ------------------------------ */
      /* --- check args for options --- */
      /* ------------------------------ */

      if (opts[OPT_NOABOUT])
	{
	  About = FALSE;
	}

      if (opts[OPT_VERSION])
	{
	  ShowVersion = TRUE;
	}

      if (opts[OPT_VOICES])
	{
	  Voices = TRUE;
	  OK = TRUE;
	}

      if (opts[OPT_PERFORMANCES])
	{
	  Performances = TRUE;
	  OK = TRUE;
	}

      if (opts[OPT_SONGS])
	{
	  Songs = TRUE;
	  OK = TRUE;
	}

      if (opts[OPT_RHYTHMS])
	{
	  Rhythms = TRUE;
	  OK = TRUE;
	}

      if (opts[OPT_SAMPLES])
	{
	  Samples = TRUE;
	  OK = TRUE;
	}

      if (opts[OPT_NOINIT])
	{
	  NoInit = TRUE;
	}

      if (opts[OPT_ROWS])
	{
	  rows = *((LONG *) opts[OPT_ROWS]);
	}


      if (opts[OPT_ALL])
	{
	  Voices = TRUE;
	  Performances = TRUE;
	  Songs = TRUE;
	  Rhythms = TRUE;
	  Samples = TRUE;
	  OK = TRUE;
	}

      /* normally the meat of the program would go here */

      /* ----------------------- */
      /* --- print out infos --- */
      /* ----------------------- */

      if (About)
	Information ();

      if (ShowVersion != FALSE)
	if (About == TRUE)
	  {
	    Version ();
	  }

      if (OK == FALSE)
	{
	  printf ("You MUST give at least one option !\n\n");
	  return RETURN_ERROR;
	}
      else
	{
	  /* -------------------- */
	  /* --- file opening --- */
	  /* -------------------- */

	  ERROR = openfile ();	/* let's open the stupid file ... */
	  if (ERROR != RETURN_OK)
	    {
	      PrintFault (ERROR, NULL);		/* prints the appropriate err message */
	      return RETURN_ERROR;
	    }

	  /* --------------------- */
	  /* --- file handling --- */
	  /* --------------------- */


	  Read_Name ();

	  if (About == TRUE)
	    {
	      if (stcpma (SY85, name) == 0)	/* compare the two strings */
		{
		  /* printf ("WARNING: This file-type is not recognized by SYList!\n"); */
		}
	    }

	  if (Performances == TRUE)
	    {
	      Read_Performances ();
	      printf ("\n");
	    }

	  if (Voices == TRUE)
	    {
	      Read_Voices ();
	      printf ("\n");
	    }

	  if (Songs == TRUE)
	    {
	      Read_Sequencer ();
	      printf ("\n");
	    }

	  if (Rhythms == TRUE)
	    {
	      Read_Rhythms ();
	      printf ("\n");
	    }

	  if (Samples == TRUE)
	    {
	      Read_Samples ();
	      printf ("\n");
	    }


	  /* -------------------------- */
	  /* --- cleanup procedures --- */
	  /* -------------------------- */

	  ERROR = closefile ();	/* ...and close this rubbish    */
	  if (ERROR != RETURN_OK)
	    {
	      PrintFault (ERROR, NULL);		/* prints the appropriate err message */
	      return RETURN_ERROR;
	    }

	  FreeArgs (argsptr);
	}

      return RETURN_OK;		/* program succeeded */
    }
}


void
Information (void)
{
  printf ("SYList Release 1.0\n");
  printf ("(C)opyright 1993-94 by Thomas Alexnat\n");
  printf ("All rights reserved. \n\n");
}

void
Version (void)
{
  printf ("%s\n", VERSTAG);
}


int
openfile (void)			/* ÷ffnet das File */
{
  fptr = fopen (Filename, "r");
  if (fptr == NULL)
    {
      printf ("Error while opening file !\n");
      PrintFault (_OSERR, NULL);	/* prints the appropriate err message */
      return RETURN_ERROR;
    }
  else
    {
      return RETURN_OK;
    }

}

int
closefile (void)		/* Schlieﬂt das File wieder */
{
  int     close;

  close = fclose (fptr);
  if (close == EOF)
    {
      printf ("Error while closing file !\n");
      PrintFault (_OSERR, NULL);	/* prints the appropriate err message */
      return RETURN_ERROR;
    }
  else
    {
      return RETURN_OK;
    }

}

void
Read_Name (void)
{
  name = fgets (buffer, 9, fptr);
}

int
Read_Performances (void)
{
  int     PosError;

  printf ("\nPerformances 1\n");
  counter = 0;
  rowcounter = 0;
  PosError = fseek (fptr, POS_PERFORMANCE_M1, 0);
  if (PosError != 0)
    {
      printf ("Error while positionating at location %ld !\n", ftell (fptr));
      PrintFault (_OSERR, NULL);	/* prints the appropriate err message */
      return RETURN_ERROR;
    }

  while (counter < ENTRIES_PERFORMANCE)
    {
      counter++;
      Read_Name ();
      Print_Bank ();
      Print_Name ();
      DoRow ();

      PosError = fseek (fptr, LENGTH_PERFORMANCE - LENGTH_SYNAMES, 1);
      if (PosError != 0)
	{
	  printf ("Error while positionating at location %ld !\n", ftell (fptr));
	  PrintFault (_OSERR, NULL);	/* prints the appropriate err message */
	  return RETURN_ERROR;
	}
    }




  printf ("\nPerformances 2\n");
  rowcounter = 0;
  counter = 0;

  PosError = fseek (fptr, POS_PERFORMANCE_M2, 0);
  if (PosError != 0)
    {
      printf ("Error while positionating at location %ld !\n", ftell (fptr));
      PrintFault (_OSERR, NULL);	/* prints the appropriate err message */
      return RETURN_ERROR;
    }

  while (counter < ENTRIES_PERFORMANCE)
    {
      counter++;
      Read_Name ();
      Print_Bank ();
      Print_Name ();
      DoRow ();

      PosError = fseek (fptr, LENGTH_PERFORMANCE - LENGTH_SYNAMES, 1);
      if (PosError != 0)
	{
	  printf ("Error while positionating at location %ld !\n", ftell (fptr));
	  PrintFault (_OSERR, NULL);	/* prints the appropriate err message */
	  return RETURN_ERROR;
	}
    }



  return RETURN_OK;
}


int
Read_Voices (void)
{
  int     PosError;

  printf ("\nVoices 1\n");
  rowcounter = 0;
  counter = 0;

  PosError = fseek (fptr, POS_VOICE_M1B1, 0);
  if (PosError != 0)
    {
      printf ("Error while positionating at location %ld !\n", ftell (fptr));
      PrintFault (_OSERR, NULL);	/* prints the appropriate err message */
      return RETURN_ERROR;
    }

  while (counter < ENTRIES_VOICE)
    {
      counter++;
      Read_Name ();
      Print_Bank ();
      Print_Name ();
      DoRow ();

      PosError = fseek (fptr, LENGTH_VOICE - LENGTH_SYNAMES, 1);
      if (PosError != 0)
	{
	  printf ("Error while positionating at location %ld !\n", ftell (fptr));
	  PrintFault (_OSERR, NULL);	/* prints the appropriate err message */
	  return RETURN_ERROR;
	}
    }

  printf ("\nVoices 2\n");
  rowcounter = 0;
  counter = 0;

  PosError = fseek (fptr, POS_VOICE_M1B2, 0);
  if (PosError != 0)
    {
      printf ("Error while positionating at location %ld !\n", ftell (fptr));
      PrintFault (_OSERR, NULL);	/* prints the appropriate err message */
      return RETURN_ERROR;
    }

  while (counter < ENTRIES_VOICE)
    {
      counter++;
      Read_Name ();
      Print_Bank ();
      Print_Name ();
      DoRow ();

      PosError = fseek (fptr, LENGTH_VOICE - LENGTH_SYNAMES, 1);
      if (PosError != 0)
	{
	  printf ("Error while positionating at location %ld !\n", ftell (fptr));
	  PrintFault (_OSERR, NULL);	/* prints the appropriate err message */
	  return RETURN_ERROR;
	}
    }

  printf ("\nVoices 3\n");
  rowcounter = 0;
  counter = 0;

  PosError = fseek (fptr, POS_VOICE_M2B1, 0);
  if (PosError != 0)
    {
      printf ("Error while positionating at location %ld !\n", ftell (fptr));
      PrintFault (_OSERR, NULL);	/* prints the appropriate err message */
      return RETURN_ERROR;
    }

  while (counter < ENTRIES_VOICE)
    {
      counter++;
      Read_Name ();
      Print_Bank ();
      Print_Name ();
      DoRow ();

      PosError = fseek (fptr, LENGTH_VOICE - LENGTH_SYNAMES, 1);
      if (PosError != 0)
	{
	  printf ("Error while positionating at location %ld !\n", ftell (fptr));
	  PrintFault (_OSERR, NULL);	/* prints the appropriate err message */
	  return RETURN_ERROR;
	}
    }

  printf ("\nVoices 4\n");
  rowcounter = 0;
  counter = 0;

  PosError = fseek (fptr, POS_VOICE_M2B2, 0);
  if (PosError != 0)
    {
      printf ("Error while positionating at location %ld !\n", ftell (fptr));
      PrintFault (_OSERR, NULL);	/* prints the appropriate err message */
      return RETURN_ERROR;
    }

  while (counter < ENTRIES_VOICE)
    {
      counter++;
      Read_Name ();
      Print_Bank ();
      Print_Name ();
      DoRow ();

      PosError = fseek (fptr, LENGTH_VOICE - LENGTH_SYNAMES, 1);
      if (PosError != 0)
	{
	  printf ("Error while positionating at location %ld !\n", ftell (fptr));
	  PrintFault (_OSERR, NULL);	/* prints the appropriate err message */
	  return RETURN_ERROR;
	}
    }
  return RETURN_OK;
}



int
Read_Rhythms (void)
{
  int     PosError;

  printf ("\nRhythms\n");
  rowcounter = 0;
  counter = 0;

  PosError = fseek (fptr, POS_RHYTHM, 0);
  if (PosError != 0)
    {
      printf ("Error while positionating at location %ld !\n", ftell (fptr));
      PrintFault (_OSERR, NULL);	/* prints the appropriate err message */
      return RETURN_ERROR;
    }

  while (counter < ENTRIES_RHYTHM)
    {
      Read_Name ();

      printf ("%2d", counter);
      Print_Name ();
      DoRow ();
      counter++;
    }
  return RETURN_OK;
}

int
Read_Sequencer (void)
{
  int     PosError;

  printf ("\nSongs\n");
  rowcounter = 0;
  counter = 0;

  PosError = fseek (fptr, POS_KEY_SEQUENCER, 0);
  if (PosError != 0)
    {
      printf ("Error while positionating at location %ld !\n", ftell (fptr));
      PrintFault (_OSERR, NULL);	/* prints the appropriate err message */
      return RETURN_ERROR;
    }

  name = fgets (buffer, 8, fptr);
  if (stcpma (KEY_SEQUENCER, name) != 0)	/* compare the two strings */
    {
      PosError = fseek (fptr, POS_SEQUENCER, 0);
      if (PosError != 0)
	{
	  printf ("Error while positionating at location %ld !\n", ftell (fptr));
	  PrintFault (_OSERR, NULL);	/* prints the appropriate err message */
	  return RETURN_ERROR;
	}

      while (counter < ENTRIES_SONG)
	{
	  counter++;
	  Read_Name ();
	  printf ("%2d", counter);
	  Print_Name ();
	  DoRow ();

	  PosError = fseek (fptr, LENGTH_SONG - LENGTH_SYNAMES, 1);
	  if (PosError != 0)
	    {
	      printf ("Error while positionating at location %ld !\n", ftell (fptr));
	      PrintFault (_OSERR, NULL);	/* prints the appropriate err message */
	      return RETURN_ERROR;
	    }
	}

      return RETURN_OK;
    }

  else
    {
      printf ("WARNING: No valid sequencer keyword found !\n");
      return RETURN_ERROR;
    }
}


int
Read_Samples (void)
{
  int     PosError;

  printf ("\nSamples\n");
  rowcounter = 0;
  counter = 0;

  PosError = fseek (fptr, POS_KEY_SAMPLE, 0);
  if (PosError != 0)
    {
      printf ("Error while positionating at location %ld !\n", ftell (fptr));
      PrintFault (_OSERR, NULL);	/* prints the appropriate err message */
      return RETURN_ERROR;
    }

  name = fgets (buffer, 8, fptr);


  if (stcpma (KEY_SAMPLE, name) != 0)	/* compare the two strings */
    {
      PosError = fseek (fptr, POS_SAMPLE, 0);
      if (PosError != 0)
	{
	  printf ("Error while positionating at location %ld !\n", ftell (fptr));
	  PrintFault (_OSERR, NULL);	/* prints the appropriate err message */
	  return RETURN_ERROR;
	}


      while (counter < ENTRIES_SAMPLE)
	{
	  Read_Name ();

	  printf ("%2d", counter);
	  Print_Name ();
	  DoRow ();
	  counter++;
	}
      return RETURN_OK;
    }
  else
    {
      printf ("WARNING: No valid sample-data keyword found !\n");
      return RETURN_ERROR;
    }


}



void
Print_Bank (void)
{
  teiler = ldiv (counter - 1, 8);
  switch (teiler.quot)
    {
    case 0:
      printf ("A");
      break;

    case 1:
      printf ("B");
      break;

    case 2:
      printf ("C");
      break;

    case 3:
      printf ("D");
      break;

    case 4:
      printf ("E");
      break;

    case 5:
      printf ("F");
      break;

    case 6:
      printf ("G");
      break;

    case 7:
      printf ("H");
      break;

    }
  printf ("%ld", teiler.rem + 1);
}

void
DoRow (void)
{
  rowcounter++;
  if (rowcounter >= rows)
    {
      printf ("\n");
      rowcounter = 0;
    }
}

void
Print_Name (void)
{
  if (NoInit == TRUE)
    {
      if (strncmp (KEY_INIT, name, 3) == 0)	/* compare the two strings */
	{
	  printf ("             ");
	}
      else
	{
	  printf (" %s    ", name);
	}
    }
  else
    {
      printf (" %s    ", name);
    }
}
