// C MATLAB MEX File.
/*
	SMQ_Query returns the number of installed SMQV4-Boards
	Matlab: num_smq_boards = SMQ_Query();
*/
/*
 * version history
 *  - 27-Mar-2015: H.R. - include RTDEx in QUIT command
 *  - 26-Mar-2015: H.R. - Fix RTDEX read function to free() allocated memory
 *  - initial: Vorkoeper et al.
*/

// Command to compile: mex SMQ.cpp -IC:\Lyrtech\ADP\HostSdk\inc -LC:\Lyrtech\ADP\HostSdk\lib\ -lSignalMasterQuad

#ifdef _WIN32
#define WIN32
#endif

#ifdef __WIN32__
#define WIN32
#endif

// #define DEBUG


// System includes
#include <stdio.h>
#include <conio.h>
#include <windows.h>
#include <string>
#include <map>
#include <sys/stat.h>
// Lyrtech includes
#include "smquadcxx.h"
#include "rtdex.h"
// #include "rapidchannel.h" // Nur vom DSP aus Möglich!
// Matlab includes
#include "mex.h"

// Globals
int (*sm_printf)(const char *,...) = printf; // Needed for the library

// Definitions
#define CUSTOM_REG_START            0x90000040 // 32 Bit

enum task_type_{
  DEFAULT,
  INIT,
  QUIT,
  QUERY_BOARD,
  CONNECT_BOARD,
  CONNECT_FPGA,
  RESET_FPGA,
  RESET_AND_PROGRAM_FPGA,
  WRITE_RTDEX,
  READ_RTDEX,
  OPEN_RTDEX,
  CLOSE_RTDEX,
  WRITE_USER_REG,
  READ_USER_REG,
  PROGRAM_DSP,
  RESET_DSP,
  SHUTDOWN,
//  RESET_RC,
};

// Aus dem Internet kopiert
int load_file(const char *filename, char **result) 
{ 
	int size = 0;
	FILE *f = fopen(filename, "rb");
	if (f == NULL) 
	{ 
		*result = NULL;
		return -1; // -1 means file opening fail 
	} 
	fseek(f, 0, SEEK_END);
	size = ftell(f);
	fseek(f, 0, SEEK_SET);
	*result = (char *)malloc(size+1);
	if (size != fread(*result, sizeof(char), size, f)) 
	{ 
		free(*result);
		return -2; // -2 means file reading fail 
	} 
	fclose(f);
	(*result)[size] = 0;
	return size;
}

void mexFunction(int n_output, mxArray *output[], int n_input, const mxArray *input[])
{
  double *tmp;
  static bool init_done  = false;
  static bool quit_done  = true;
  static bool rtdex_open = false;
  // Lyrtech specific variables
  static ULONG               ulFpgaId  = -1;
  static ULONG               ulBoardid = -1;
  static ULONG               ulNumberOfSMQ;
  static PSMQUADC6XX_DEVICE  SMQTable[SMQUADC6XX_MAX_DEVICES];

  std::map<std::string, unsigned int> task;
  task["Init"]                = INIT;
  task["Quit"]                = QUIT;
  task["Query_Board"]         = QUERY_BOARD;
  task["Connect_Board"]       = CONNECT_BOARD;
  task["Connect_FPGA"]        = CONNECT_FPGA;
  task["Reset_FPGA"]          = RESET_FPGA;
  task["Reset_Program_FPGA"]  = RESET_AND_PROGRAM_FPGA;
  task["Write_RTDEx"]         = WRITE_RTDEX;
  task["Read_RTDEx"]          = READ_RTDEX;
  task["Open_RTDEx"]          = OPEN_RTDEX;
  task["Close_RTDEx"]         = CLOSE_RTDEX;
  task["Write_User_Reg"]      = WRITE_USER_REG;
  task["Read_User_Reg"]       = READ_USER_REG;
  task["Program_DSP"]         = PROGRAM_DSP;
  task["Reset_DSP"]           = RESET_DSP;
  task["Shutdown"]            = SHUTDOWN;
//   task["Reset_RC"]            = RESET_RC; // Nur vom DSP aus möglich!

  /* Check the number of inputs and output arguments */
  if(n_input<1) {
    mexErrMsgTxt("Wrong number of input variables! \n");
  }

  /* Decide which task has to be done */
  switch( task[mxArrayToString(input[0])] ) {
    /* ------------------------------------------------------------------------------ */
    case INIT: {
      if(n_input!=1) mexErrMsgTxt("Wrong number of input variables! \n");

	    /* Initialize the library */
	    if ((init_done == false) && (quit_done == true)) {
		    printf("Initializing the library! \n");
		    SmQuadcxxInit();
	    }

      init_done = true;
      break;
    }
    /* ------------------------------------------------------------------------------ */
    case QUIT: {
      if(n_input!=1) mexErrMsgTxt("Wrong number of input variables! \n");

      if( (quit_done == false) && (rtdex_open != false) )
      {
          smquadcxx_RTDExReset(SMQTable[ulBoardid], ulFpgaId);
          smquadcxx_RTDExClose(SMQTable[ulBoardid]);
          rtdex_open = false;
      }

	    /* Close the library */
	    if ((init_done == true) && (quit_done == false)) 
      {
		    printf("Freeing all resources! \n");
		    SmQuadcxxQuit();
	    }

      quit_done = true;
      ulFpgaId  = -1;
      ulBoardid = -1;
      ulNumberOfSMQ = NULL;

      break;
    }
    /* ------------------------------------------------------------------------------ */
    case QUERY_BOARD: {
		  if(n_input!=1) mexErrMsgTxt("Wrong number of input variables! \n");
      if(n_output !=1) mexErrMsgTxt("Wrong number of output variables! \n");

      ulNumberOfSMQ   = 0;

		  /* Get the slot ID */
      memset(SMQTable, 0, sizeof(SMQTable[SMQUADC6XX_MAX_DEVICES]));
      
      for (UINT i=0; i<SMQUADC6XX_MAX_DEVICES; i++) //Find every device (until Find device return NULL)
      {
        SMQTable[i] = SmQuadcxxFindDevice(i);
        if (SMQTable[i] == NULL) break;
        ulNumberOfSMQ++;
        printf("SignalMaster Quad found -> Slot ID[%d]\n",i);
      }

      /* Create Output Array with Slot IDs */
      output[0] = mxCreateDoubleMatrix(1, 1, mxREAL);
      tmp = mxGetPr(output[0]);
      tmp[0] = ulNumberOfSMQ - 1;

      if (ulNumberOfSMQ == 0){
        puts("No SignalMaster Quad found!");
      }

      break;
    }
    /* ------------------------------------------------------------------------------ */
    case CONNECT_BOARD: {
      if(n_input!=2) mexErrMsgTxt("Wrong number of input variables! \n");
      ulBoardid = (ULONG)mxGetScalar(input[1]);
      printf("Connecting to Board with Slot ID[%u]\n", ulBoardid);
      if((ulBoardid < 0)||(ulBoardid>(ulNumberOfSMQ-1))) {
        ulBoardid = -1;
        mexErrMsgTxt("Wrong Board ID! \n");
      }

      break;
    }
    /* ------------------------------------------------------------------------------ */
    case CONNECT_FPGA: {
      if(n_input!=2) mexErrMsgTxt("Wrong number of input variables! \n");
      ulFpgaId = (ULONG)mxGetScalar(input[1]);
      printf("Connecting to FPGA ID[%u]\n", ulFpgaId);
      if((ulFpgaId < 0)||(ulFpgaId>1)) {
        ulFpgaId = -1;
        mexErrMsgTxt("Wrong FPGA ID! \n");
      }

      break;
    }
    /* ------------------------------------------------------------------------------ */
    case READ_USER_REG: {
      int offset;
      int REGISTER;
      unsigned long ulData;

      if(n_input!=2) mexErrMsgTxt("Wrong number of input variables! \n");
      if(n_output !=1) mexErrMsgTxt("Wrong number of output variables! \n");
      // Check, if connected to Board and FPGA
      if(ulBoardid == -1) mexErrMsgTxt("Please connect to Board first! \n");
      if(ulFpgaId == -1) mexErrMsgTxt("Please connect to FPGA first! \n");

      offset = (int)mxGetScalar(input[1]);
      if( (offset<0)||(offset>11) ) mexErrMsgTxt("Wrong user register number! \n");
      REGISTER = CUSTOM_REG_START + (4 * offset);

      #ifdef DEBUG
        printf("Reading 32 Bit number!\n");
      #endif
      SmQuadcxxReadFpgaDword(SMQTable[ulBoardid], ulFpgaId, REGISTER, 1, 1, &ulData );

      /* Create Output Array with Slot IDs */
      output[0] = mxCreateDoubleMatrix(1, 1, mxREAL);
      tmp = mxGetPr(output[0]);
      tmp[0] = ulData;

      break;
    }
    /* ------------------------------------------------------------------------------ */
    case WRITE_USER_REG: {
      int offset;
      int REGISTER;
      ULONG ulData;

      if(n_input!=3) mexErrMsgTxt("Wrong number of input variables! \n");
      if(n_output !=0) mexErrMsgTxt("Wrong number of output variables! \n");
      // Check, if connected to Board and FPGA
      if(ulBoardid == -1) mexErrMsgTxt("Please connect to Board first! \n");
      if(ulFpgaId == -1) mexErrMsgTxt("Please connect to FPGA first! \n");

      offset = (int)mxGetScalar(input[1]);
      if( (offset<0)||(offset>11) ) mexErrMsgTxt("Wrong user register number! \n");
      REGISTER = CUSTOM_REG_START + (4 * offset);

      ulData = (ULONG)mxGetScalar(input[2]);

	  #ifdef DEBUG
		printf("Writing 32 Bit number!\n");
      #endif
      SmQuadcxxWriteFpgaDword(SMQTable[ulBoardid], ulFpgaId, REGISTER, 1, 1, &ulData );

      break;
    }
    /* ------------------------------------------------------------------------------ */
    case RESET_FPGA: {
      int error;
      int size;

      if(n_input!=1) mexErrMsgTxt("Wrong number of input variables! \n");
      // Check, if connected to Board and FPGA
      if(ulBoardid == -1) mexErrMsgTxt("Please connect to Board first! \n");
      if(ulFpgaId == -1) mexErrMsgTxt("Please connect to FPGA first! \n");

      printf("Reseting FPGA ID[%u]\n", ulFpgaId );
      error = SmQuadcxxHardwareModuleFpgaReset( SMQTable[ulBoardid], ulFpgaId);

      if(error == 0) {
		  printf("Error = %d\n", error);
		  mexErrMsgTxt("Error reseting the FPGA!\n");
	  }

      break;
    }
    /* ------------------------------------------------------------------------------ */
    case RESET_AND_PROGRAM_FPGA: {
      int error;
      int size;
      char *bitstream_file     = NULL;
      char *bitstream = NULL;

      if(n_input!=2) mexErrMsgTxt("Wrong number of input variables! \n");
      // Check, if connected to Board and FPGA
      if(ulBoardid == -1) mexErrMsgTxt("Please connect to Board first! \n");
      if(ulFpgaId == -1) mexErrMsgTxt("Please connect to FPGA first! \n");

      bitstream_file = (char *)mxArrayToString(input[1]);
      size = load_file(bitstream_file, &bitstream);
      if(size<0) mexErrMsgTxt("Error loading Bitstream file! \n");
      printf("Loading %s with %u Bytes to Board ID[%u] and FPGA ID[%u] \n", bitstream_file, size, ulBoardid, ulFpgaId);

      error = SmQuadcxxFpgaReset(SMQTable[ulBoardid], ulFpgaId+1, VIRTEX, SELECT_MAP_MODE, (UCHAR *)bitstream, size );

      free(bitstream);
      if(error == 0) mexErrMsgTxt("Error uploading bitstream file!\n");


      break;
    }
    /* ------------------------------------------------------------------------------ */
    case READ_RTDEX: {
      int length = 0;
      PUCHAR buffer_in;
      RTDEX_RETURN rtdexRet;
      RTDEX_ERROR_INFO errInfo;

      if(n_input!=2) mexErrMsgTxt("Wrong number of input variables! \n");
      if(n_output!=1) mexErrMsgTxt("Wrong number of output variables! \n");
      // Check, if connected to Board and FPGA
      if(ulBoardid == -1) mexErrMsgTxt("Please connect to Board first! \n");
      if(ulFpgaId == -1) mexErrMsgTxt("Please connect to FPGA first! \n");
      if(rtdex_open == false) mexErrMsgTxt("RTDEx must be opened first! \n");

      length = (int)mxGetScalar(input[1]);
      if(length < 1) mexErrMsgTxt("Length must at least be greater then zero! \n");

      /* Allocate memory for transfer */
      buffer_in = (PUCHAR) malloc(length);
      if ( buffer_in == NULL ) mexErrMsgTxt("Error allocating input buffer. Exiting...\n");
      //memset(buffer_in, 0, length); // Erstmal mit Nullen befuellen

      /* Start the Rx */
      smquadcxx_RTDExReset(SMQTable[ulBoardid], ulFpgaId);

      /* RX buffer from the FPGA. */    
      rtdexRet = smquadcxx_RTDExReadBuffer(SMQTable[ulBoardid], ulFpgaId, buffer_in, length);
      if (rtdexRet != RTDEX_RETURN_NORMAL) mexErrMsgTxt("Error while Read buffer");

      /* Get error information. */
      memset(&errInfo, 0, sizeof(errInfo));
      rtdexRet = smquadcxx_RTDExGetError(SMQTable[ulBoardid], ulFpgaId, &errInfo);
      if (rtdexRet != RTDEX_RETURN_NORMAL) mexErrMsgTxt("Unable to get error information");

	  #ifdef DEBUG
		/* Show information to the user. */
		printf("\n");
		printf("----------------------------\n");
		printf("RTDEx REPORT:           \n");
		printf("----------------------------\n");
		printf("BYTES RECEIVED      = %u \n", length);
		printf("RX OVERRUN  COUNT   = %u\n", errInfo.rxOverrun);
		printf("TX UNDERRUN COUNT   = %u\n", errInfo.txUnderrun);
		printf("----------------------------\n");
      #endif

      /* Write uploaded data to Matlab output buffer */
      output[0] = mxCreateDoubleMatrix(length, 1, mxREAL);
      tmp = mxGetPr(output[0]);
      for(int i=0; i<length; i++) {
        tmp[i] = (double)buffer_in[i];
      }
      free(buffer_in);

      break;
    }
    /* ------------------------------------------------------------------------------ */
    case WRITE_RTDEX: {
      int m = 0, n = 0;
      char *buffer_in;
      int length;
      PUCHAR buffer_out;
      RTDEX_RETURN rtdexRet;
      RTDEX_ERROR_INFO errInfo;

      if(n_input!=2) mexErrMsgTxt("Wrong number of input variables! \n");
      // Check, if connected to Board and FPGA
      if(ulBoardid == -1) mexErrMsgTxt("Please connect to Board first! \n");
      if(ulFpgaId == -1) mexErrMsgTxt("Please connect to FPGA first! \n");
      if(rtdex_open == false) mexErrMsgTxt("RTDEx must be opened first! \n");

      /* Check if Data is really uint8 */
      if(!mxIsUint8(input[1])) mexErrMsgTxt("Frame must ba a uint8 array! \n");
      /* Copy Matlab Data to SMQ Data */
      m = mxGetM(input[1]);
      n = mxGetN(input[1]);
      
      if(m>n) { length = m; } else { length = n; };

      /* Allocate memory for transfer */
      buffer_out = (PUCHAR) malloc(length);
      if ( buffer_out == NULL ) mexErrMsgTxt("Error allocating input buffer. Exiting...\n");
      memset(buffer_out, 0, length); // Erstmal mit Nullen befuellen

      buffer_in = (char *)mxGetData(input[1]);
      for(int i=0; i<length; i++) {
        buffer_out[i] = buffer_in[i];
      }

      #ifdef DEBUG
        printf("Length = %u, %u, %u", length, m, n);
      #endif

      /* Write the buffer to the FPGA. */
      rtdexRet = smquadcxx_RTDExWriteBuffer(SMQTable[ulBoardid], ulFpgaId, buffer_out, length);
      if (rtdexRet != RTDEX_RETURN_NORMAL) mexErrMsgTxt("Unable to configure TX buffer");
    
      /* Get error information from RTDEx */
      memset(&errInfo, 0, sizeof(errInfo));
      rtdexRet = smquadcxx_RTDExGetError(SMQTable[ulBoardid], ulFpgaId, &errInfo);
      if (rtdexRet != RTDEX_RETURN_NORMAL) mexErrMsgTxt("Unable to get error information");

      #ifdef DEBUG
        /* Show information to the user. */
        printf("\n");
        printf("----------------------------\n");
        printf("RTDEx REPORT:               \n");
        printf("----------------------------\n");
        printf("BYTES SENT             = %u\n", (UINT) length);
        printf("RX OVERRUN  COUNT      = %u\n", (UINT) errInfo.rxOverrun);
        printf("TX UNDERRUN COUNT      = %u\n", (UINT) errInfo.txUnderrun);
        printf("----------------------------\n");
      #endif

      smquadcxx_RTDExReset(SMQTable[ulBoardid],ulFpgaId);

      break;
    }
    /* ------------------------------------------------------------------------------ */
    case OPEN_RTDEX: {
      RTDEX_RETURN rtdexRet;

      if(n_input!=1) mexErrMsgTxt("Wrong number of input variables! \n");
      // Check, if connected to Board and FPGA
      if(ulBoardid == -1) mexErrMsgTxt("Please connect to Board first! \n");
      if(ulFpgaId == -1) mexErrMsgTxt("Please connect to FPGA first! \n");
      if(rtdex_open == true) mexErrMsgTxt("RTDEx is already opened! \n");

      rtdexRet = smquadcxx_RTDExOpen(SMQTable[ulBoardid]);
      if (rtdexRet != RTDEX_RETURN_NORMAL) {
        rtdexRet = smquadcxx_RTDExOpen(SMQTable[ulBoardid]);
        if (rtdexRet != RTDEX_RETURN_NORMAL) {
          printf("RTDEx Return Code: %d\n", rtdexRet);
          mexErrMsgTxt("Unable to open RTDEx");
        }
      }

      printf("RTDEx opened!\n");
      
      rtdex_open = true;

      break;
    }
    /* ------------------------------------------------------------------------------ */
    case CLOSE_RTDEX: {
      RTDEX_RETURN rtdexRet;

      if(n_input!=1) mexErrMsgTxt("Wrong number of input variables! \n");
      // Check, if connected to Board and FPGA
      if(ulBoardid == -1) mexErrMsgTxt("Please connect to Board first! \n");
      if(ulFpgaId == -1) mexErrMsgTxt("Please connect to FPGA first! \n");
      if(rtdex_open == false) mexErrMsgTxt("RTDEx is not opened! \n");

      smquadcxx_RTDExClose(SMQTable[ulBoardid]);
      printf("RTDEx closed!\n");

      rtdex_open = false;

      break;
    }
    /* ------------------------------------------------------------------------------ */
    case PROGRAM_DSP: {
      int error;
      int size;
      int dsp_id;
      char *executable_file = NULL;
      char *executable      = NULL;

      if(n_input!=3) mexErrMsgTxt("Wrong number of input variables! \n");
      // Check, if connected to Board and FPGA
      if(ulBoardid == -1) mexErrMsgTxt("Please connect to Board first! \n");
      if(ulFpgaId == -1) mexErrMsgTxt("Please connect to FPGA first! \n");

      dsp_id = (int)mxGetScalar(input[2]);
      if( (dsp_id<0) || (dsp_id>3) ) mexErrMsgTxt("DSP ID must be between 0..3! \n");

      executable_file = (char *)mxArrayToString(input[1]);
      size = load_file(executable_file, &executable);
      if(size<0) mexErrMsgTxt("Error loading Executable file! \n");

      printf("Loading %s with %u Bytes to Board ID[%u] and DSP ID[%u] \n", executable_file, size, ulBoardid, dsp_id);

      error = SmQuadcxxDspLoad(SMQTable[ulBoardid], dsp_id + 1, (UCHAR *)executable, size );

      free(executable);
      if(error == 0) mexErrMsgTxt("Error uploading executable file!\n");

      break;
    }
          
    /* ------------------------------------------------------------------------------ */
    case RESET_DSP: {
      int error;
      int size;
      int dsp_id;

      if(n_input!=2) mexErrMsgTxt("Wrong number of input variables! \n");
      // Check, if connected to Board and FPGA
      if(ulBoardid == -1) mexErrMsgTxt("Please connect to Board first! \n");
      if(ulFpgaId == -1) mexErrMsgTxt("Please connect to FPGA first! \n");

      dsp_id = (int)mxGetScalar(input[1]);
      if( (dsp_id<0) || (dsp_id>3) ) mexErrMsgTxt("DSP ID must be between 0..3! \n");

      printf("Reseting DSP ID[%u] on Board ID[%u]\n", dsp_id, ulBoardid );

      error = SmQuadcxxDspReset( SMQTable[ulBoardid], dsp_id + 1 );

      if(error == 0) mexErrMsgTxt("Error reseting the DSP!\n");

      break;
    }
    /* ------------------------------------------------------------------------------ */
    /*
    case RESET_RC: {
      int error;
      int size;
      int rc_rx_tx;

      if(n_input!=2) mexErrMsgTxt("Wrong number of input variables! \n");
      // Check, if connected to Board and FPGA
      if(ulBoardid == -1) mexErrMsgTxt("Please connect to Board first! \n");
      if(ulFpgaId == -1) mexErrMsgTxt("Please connect to FPGA first! \n");

      rc_rx_tx = (int)mxGetScalar(input[1]);
      if( (rc_rx_tx<0) || (rc_rx_tx>1) ) mexErrMsgTxt("RapidCHANNEL number must be between 0..1! (RX = 0, TX = 1) \n");

      if( rc_rx_tx == 0 ) {
		printf("Reseting RapidCHANNEL in RX Mode on Board ID[%u]\n", ulBoardid );
		error = rc_reset( SMQTable[ulBoardid], ulFpgaId, RAPIDCHANNELRX );
	  } else {
		printf("Reseting RapidCHANNEL in TX Mode on Board ID[%u]\n", ulBoardid );
		error = rc_reset( SMQTable[ulBoardid], ulFpgaId, RAPIDCHANNELTX );
	  }

      if(error == 0) {
		  printf("Error = %d\n", error);
		  mexErrMsgTxt("Error reseting the RapidCHANNEL!\n");
	  }

      break;
    }
    */
    /* ------------------------------------------------------------------------------ */
    default: {
      mexErrMsgTxt("Invalid task! \n");
      break;
    }
  }

}

