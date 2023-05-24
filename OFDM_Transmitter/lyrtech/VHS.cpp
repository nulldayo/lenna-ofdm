// VHS Control Utility for Matlab
// Authors: Sebastian Vorköper, Tom Reinartz (Transceiver/RFFE Host API), Hani Samara (optional Memory Module API)
//          Henryk Richter (bugfixes in RTDEx ADC/DAC,READ_SDRAM_ADC, addition SET_EXTERNAL, addition RX/TX Low Pass Filter)
//
#define datestring "28.03.2017"
// 
// changelog:
//  v0.92 22.03.2017 - started to implement separate RX/TX configuration of RF frontend
//  v0.91
//  v0.9  30.07.2015 - further attempts to improve SDRAM capture stability
//  v0.8  02.07.2015 - introduced 'Version' command to identify binary by version
//                   - expanded the RX Lowpass filter corner frequency selection (9975,10450,12600,19800)
//  v0.7  03.06.2015 - switched RXHP_OFF and RXHP_ON to make the RX highpass work as expected,
//                     in addition set the HP filter to 100 Hz when set off by transceiver (30 kHz when all Transceivers are set at once)
//  v0.6  02.06.2015 - RX/TX Lowpass filter control (coarse frequencies only for now)
//
// for compiling use
// mex -v -g VHS.cpp -IC:\Lyrtech\ADP\HostSdk\inc -IC:\Lyrtech\ADP\VHS\sdk\common\inc -IC:\Lyrtech\ADP\VHS\examples\VHSV4\app_ex\VHSV4_rffe -LC:\Lyrtech\ADP\HostSdk\lib\ -lVhsadac16_host_api

#define versionstring "VHS.cpp, v0.92a, date %s, %s %s\n"
#define copyrightstring " "
#define copyrightstring2 " "

#ifdef _WIN32
#define WIN32
#endif

#ifdef __WIN32__
#define WIN32
#endif

//#define DEBUG

//System includes
#include <stdio.h>
#include <conio.h>
#include <windows.h>
#include <string>
#include <map>
#include <sys/stat.h>
#include <time.h>
//Lyrtech includes
// #include "rffectrl.h"
#ifndef far
#define far
#endif
#include "vhsdac16ctrl_v4.h"
#include "vhsadc16ctrl_v4.h"
#include "vhsadc16ctrl.h"
#include "vhsadac16_host_api.h"
#include "vhsadac16_regs.h"
#include "rffectrl.h"
// #include "rffe.h"
#include "vhsmemctrl.h"
//Matlab includes
#include "mex.h"
#include "matrix.h"

#include "JSregs.h" // default register value


using namespace std;

//Definitions
#define VHS_ID       0 //Board 0
#define NUM_OF_TRANS 4

static HANDLE                m_h_ADC[8];
static HANDLE                m_h_DAC[8];
static HANDLE                m_h_board;
static HANDLE                m_h_board_dac;
static HANDLE                m_h_board_adc;

VHSADC16CONTEXT              m_context;

static unsigned int          rffe_base = 0x1200;
static unsigned int          transceiver_selected = 0;
static unsigned int          transceiver_lowpass[4] = {0x2a,0x2a,0x2a,0x2a}; // def: 9.5 MHz on recv, 12 MHz on Send

unsigned long int            MAX_FRAME_SIZE = 40*1024*1024;
static bool                  external_trig = false;

// Arrays of values to write in the registers of the Quad Dual Band RF Transceiver by SPI
unsigned int *pChFreq=0;
unsigned int chfreq1[]={2412,2417,2422,2427,2432,2437,2442,2447,2452,2457,2462,2467,2472,2484};
unsigned int chfreq2[]={5180,5200,5220,5240,5260,5280,5300,5320,5500,5520,5540,5560,5580,5600,5620,5640,5660,5680,5700,5745,5765,5785,5805};

// Register address table : used to set register values in a 'for' loop
REGISTER_SPI_NUMBER rffe_spi_num_tab[13] = {RFFE_REGISTER0, RFFE_REGISTER1, RFFE_STANDBY, RFFE_INTEGER_DIVIDER_RATIO,RFFE_FRACTIONAL_DIVIDER_RATIO,RFFE_BAND_SELECT_PLL,RFFE_CALIBRATION,RFFE_LOWPASS_FILTER,RFFE_RX_CONTROL_RSSI,RFFE_TX_LINEARITY_BB_GAIN,RFFE_PA_BIAS_DAC,RFFE_RX_GAIN,RFFE_TX_VGA_GAIN};
TRANSCEIVER_NUMBER transceiver_number_tab[4] = {TRANSCEIVER_1, TRANSCEIVER_2, TRANSCEIVER_3, TRANSCEIVER_4};
PABS_STATE pabs_st;
DSW_STATE dsw_st;
OSCS_STATE oscs_st;

// Help functions to manipulate files
// function prototype
int load_file(const char *filename, char **result);
int save_record_file(const char *result,char *source_buffer, LONG *offset, unsigned long frame_bytes); 
int load_pb_file(const char *filename, char **result);
char* files_available ( char** files, unsigned n );
int join_files ( char* joined, char** files, unsigned n );
VHS_PLAYBACK_STATE getPbState(HANDLE h);
VHS_RECORD_STATE getRecState(HANDLE h);

void SetChannel(unsigned int ch,unsigned int band,TRANSCEIVER_NUMBER transceiver_num, ULONG mode);

//       VOID WINAPI Sleep(
//             _In_  DWORD 1000
//             );
//             
enum task_type_{
    LOAD_DAC_BITFILE,
    LOAD_ADC_BITFILE,
    WRITE_SDRAM_DAC,
    READ_SDRAM_ADC,
    SET_PLAYBACK_SDRAM_DAC,
    SET_RECORD_SDRAM_ADC,
    START_PLAYBACK_SDRAM_DAC,
    STOP_PLAYBACK_SDRAM_DAC,    
    START_RECORD_SDRAM_ADC,
    PLAYBACK_MEM_SDRAM_INIT,
    PLAYBACK_MEM_SDRAM,
    RECORD_MEM_SDRAM_INIT,
    RECORD_MEM_SDRAM,
    START_MEM_PLAYBACK,
    STOP_MEM_PLAYBACK,
    GET_MEM_RUN,
    GET_STATE,
    QUERY_DAC_BOARD,
    QUERY_ADC_BOARD,
    CONNECT_DAC_BOARD,
    CONNECT_ADC_BOARD,
    DISCONNECT_DAC_BOARD,
    DISCONNECT_ADC_BOARD,
    WRITE_USER_REG_DAC,
    WRITE_USER_REG_ADC,
    READ_USER_REG_DAC,
    READ_USER_REG_ADC,
    WRITE_RTDEX_DAC,
    WRITE_RTDEX_ADC,
    READ_RTDEX_DAC,
    READ_RTDEX_ADC,
    OPEN_RTDEX_DAC,
    OPEN_RTDEX_ADC,
    CLOSE_RTDEX_DAC,
    CLOSE_RTDEX_ADC,
    RUN_DAC,
    RUN_ADC,
    INIT_RFFE,
	TRANS_MODE,
	SET_PABS_DSW_OSCS,
    INIT_SPI_REGISTER,
    SET_GAIN,
    SET_POWER_AMPLIFIER,
    QUIT,
    SET_CHANNEL,
    SET_TX_LOW_PASS_FILTER,
    SET_RX_LOW_PASS_FILTER,
    SET_RX_HIGH_PASS_FILTER,
    READ_TX_POWER,
    SET_EXTERNAL,
    VERSION,
};

//starting point
void mexFunction(int n_output, mxArray *output[], int n_input, const mxArray *input[]){
    int                     i,i2;
    double                  *tmp;

    static bool rtdex_open_dac = false;
    static bool rtdex_open_adc = false;

    static int              number_vhs_dac = 0;
    static int              number_vhs_adc = 0;
    static unsigned int     data;
    static unsigned int     ch = 0;
    static char             c[256];
    static int              cmd, band;
    
    static unsigned int     timeout = 2000; // time out in milliseconds
    
    static MODE_EXTCTRL external_mode_control = MODE_EXTCTRL_OFF;
    static ADD_EXTCTRL external_add_control   = ADD_EXTCTRL_OFF;
    static GAIN_EXTCTRL external_gain_control = GAIN_EXTCTRL_OFF;
    static FREQ_EXTCTRL external_freq_control = FREQ_EXTCTRL_OFF;
    static unsigned int extctrl;

    //TRANSCEIVER_NUMBER transceiver_num;
    LOCK_DETECT_STATE lockState;

    static ULONG ulBoardID_DAC = -1;
    static ULONG ulBoardID_ADC = -1;
    static bool rffe_init      = false;
    static bool power_amp[NUM_OF_TRANS];
    static char transmodes[NUM_OF_TRANS] = {-1,-1,-1,-1}; // -1 = unconfigured, 0 = RX, 1 = TX
//    static ULONG ulTransmode   = -1; //-1 -->no mode, 0 --> RX, 1 --> TX // deprecated
    static int pabs            = -1;
    static int dsw             = -1;
    static int oscs            = -1;
    static unsigned int spi_reg_value, gain_value;

    CLOCK_SOURCE Clock;
    std::map<std::string, unsigned int> task;
    task ["Load_DAC_Bitfile"]        = LOAD_DAC_BITFILE;
    task ["Load_ADC_Bitfile"]        = LOAD_ADC_BITFILE;
    task ["Write_SDRAM_DAC"]         = WRITE_SDRAM_DAC;
    task ["Read_SDRAM_ADC"]          = READ_SDRAM_ADC;
    task ["Set_Playback_SDRAM_DAC"]  = SET_PLAYBACK_SDRAM_DAC;
    task ["Set_Record_SDRAM_ADC"]    = SET_RECORD_SDRAM_ADC;
    task ["Start_Playback_SDRAM_DAC"]= START_PLAYBACK_SDRAM_DAC;
    task ["Stop_Playback_SDRAM_DAC"]  = STOP_PLAYBACK_SDRAM_DAC;    
    task ["Start_Record_SDRAM_ADC"]  = START_RECORD_SDRAM_ADC;
    task ["Playback_MEM_SDRAM_Init"] = PLAYBACK_MEM_SDRAM_INIT;
    task ["Playback_MEM_SDRAM"]      = PLAYBACK_MEM_SDRAM;
    task ["Record_MEM_SDRAM_Init"]   = RECORD_MEM_SDRAM_INIT;
    task ["Record_MEM_SDRAM"]        = RECORD_MEM_SDRAM; 
    task ["Start_MEM_Playback"]      = START_MEM_PLAYBACK;
    task ["Stop_MEM_Playback"]       = STOP_MEM_PLAYBACK;
    task ["GET_MEM_RUN"]             = GET_MEM_RUN,
    task ["Get_State"]               = GET_STATE,
    task ["Query_DAC_Board"]         = QUERY_DAC_BOARD;
    task ["Query_ADC_Board"]         = QUERY_ADC_BOARD;
    task ["Connect_DAC_Board"]       = CONNECT_DAC_BOARD;
    task ["Connect_ADC_Board"]       = CONNECT_ADC_BOARD;
    task ["Disconnect_DAC_Board"]    = DISCONNECT_DAC_BOARD;
    task ["Disconnect_ADC_Board"]    = DISCONNECT_ADC_BOARD;
    task ["Write_User_Reg_DAC"]      = WRITE_USER_REG_DAC;
    task ["Write_User_Reg_ADC"]      = WRITE_USER_REG_ADC;
    task ["Read_User_Reg_DAC"]       = READ_USER_REG_DAC;
    task ["Read_User_Reg_ADC"]       = READ_USER_REG_ADC;
    task ["Write_RTDEx_DAC"]         = WRITE_RTDEX_DAC;
    task ["Write_RTDEx_ADC"]         = WRITE_RTDEX_ADC;
    task ["Read_RTDEx_DAC"]          = READ_RTDEX_DAC;
    task ["Read_RTDEx_ADC"]          = READ_RTDEX_ADC;
    task ["Open_RTDEx_DAC"]          = OPEN_RTDEX_DAC;
    task ["Open_RTDEx_ADC"]          = OPEN_RTDEX_ADC;
    task ["Close_RTDEx_DAC"]         = CLOSE_RTDEX_DAC;
    task ["Close_RTDEx_ADC"]         = CLOSE_RTDEX_ADC;
    task ["RUN_DAC"]                 = RUN_DAC;
    task ["RUN_ADC"]                 = RUN_ADC;
    task ["Init_RFFE"]               = INIT_RFFE;
    task ["Transceiver_Mode"]	       = TRANS_MODE;
    task ["Set_PABS_DSW_OSCS"]       = SET_PABS_DSW_OSCS;
    task ["Init_SPI_Register"]       = INIT_SPI_REGISTER;
    task ["Set_Gain"]                = SET_GAIN;
    task ["Set_Power_Amplifier"]     = SET_POWER_AMPLIFIER;
    task ["Quit"]                    = QUIT;
    task ["Set_Channel"]             = SET_CHANNEL;
    task ["Set_TX_Low_Pass"]         = SET_TX_LOW_PASS_FILTER;
    task ["Set_RX_Low_Pass"]         = SET_RX_LOW_PASS_FILTER;
    task ["Set_RX_High_Pass"]        = SET_RX_HIGH_PASS_FILTER;
    task ["Read_TX_Power"]           = READ_TX_POWER;
    task ["Set_External"]            = SET_EXTERNAL;
    task ["Version"]                 = VERSION;

    /* Check the number of inputs and output arguments */
    if(n_input<1) {
        mexErrMsgTxt("Wrong number of input variables! \n");
    }
    /* Decide which task has to be done */
    switch( task[mxArrayToString(input[0])] ) {
//---------------------------------------------------------------------------
        case LOAD_DAC_BITFILE:{
            int error;
            int size;
            char *bitstream_file = NULL;
            char *bitstream      = NULL;

            if(n_input!=2) mexErrMsgTxt("Wrong number of input variables! \n");
            // Check, if connected to Board
            if(ulBoardID_DAC < 0 || ulBoardID_DAC > 7){
                if (number_vhs_dac == 0){
                    mexErrMsgTxt("Please execute \"HF ('Query_DAC_Board')\" first and then connect to the Board\n");
                    mexErrMsgTxt("before loading a Bitfile on the FPGA (\"HF ('Connect_Board', ID)\")!\n");
                }else{
                    mexErrMsgTxt("Please connect to Board first (\"HF ('Connect_Board', ID)\")! \n");
                }
                break;
            }

            bitstream_file = (char *)mxArrayToString(input[1]);
            size = load_file(bitstream_file, &bitstream);
            if(size<0){
                mexErrMsgTxt("Error loading Bitstream file! \n");
                break;
            }
            printf("-->Loading %s with %u Bytes to Board ID[%u]\n", bitstream_file, size, ulBoardID_DAC);

            error = Vhsadac16_FpgaReset (m_h_board, (UCHAR *)bitstream, size);

            free(bitstream);
            if(error != 0) {
                mexErrMsgTxt("Error uploading bitstream file!\n");
                break;
            }
            break;
        }
//---------------------------------------------------------------------------
        case LOAD_ADC_BITFILE:{
            int error;
            int size;

            char *bitstream_file = NULL;
            char *bitstream      = NULL;

            if(n_input!=2) mexErrMsgTxt("Wrong number of input variables! \n");
            // Check, if connected to Board
            if(ulBoardID_ADC < 0 || ulBoardID_ADC > 7){
                if (number_vhs_adc == 0){
                    mexErrMsgTxt("Please execute \"HF ('Query_ADC_Board')\" first and then connect to the Board\n");
                    mexErrMsgTxt("before loading a Bitfile on the FPGA (\"HF ('Connect_Board', ID)\")!\n");
                }else{
                    mexErrMsgTxt("Please connect to Board first (\"HF ('Connect_Board', ID)\")! \n");
                }
                break;
            }

            bitstream_file = (char *)mxArrayToString(input[1]);
            size = load_file(bitstream_file, &bitstream);
            if(size<0){
                mexErrMsgTxt("Error loading Bitstream file! \n");
                break;
            }
            printf("-->Loading %s with %u Bytes to Board ID[%u]\n", bitstream_file, size, ulBoardID_ADC);

            error = Vhsadac16_FpgaReset (m_h_board, (UCHAR *)bitstream, size);

            free(bitstream);
            if(error != 0) {
                mexErrMsgTxt("Error uploading bitstream file!\n");
                break;
            }
            
            break;
        }
//---------------------------------------------------------------------------
        case WRITE_SDRAM_DAC:{
            int m = 0, n = 0;
            char *buffer_in;
            ULONG  status, mem_size, buffer_length;
            LONG   bytes_send;
            PUCHAR buffer_out;
            
            if(n_input!=2) mexErrMsgTxt("Wrong number of input variables! \n");
            
            // Check, if connected to Board and FPGA
            if(ulBoardID_DAC == -1) mexErrMsgTxt("Please connect to DAC Board first! \n");

            // Check if the SDRAM Module is present?
            status = Vhsadac16_Sdram_AutoInit( m_h_board_dac );
            if(status != 1) mexErrMsgTxt("No SDRAM Module found!\n Was it installed in the Simulink Model?\n");
            
            /* Check if Data is really uint8 */
            if(!mxIsUint8(input[1])) mexErrMsgTxt("Frame must ba a uint8 array! \n");
            /* Copy Matlab Data to VHS-ADAC Data */
            m = mxGetM(input[1]);
            n = mxGetN(input[1]);

            if(m>n) { buffer_length = m; } else { buffer_length = n; };

            /* Allocate memory for transfer */
            buffer_out = (PUCHAR) malloc(buffer_length);
            if ( buffer_out == NULL ) mexErrMsgTxt("Error allocating input buffer. Exiting...\n");
            memset(buffer_out, 0, buffer_length); // Erstmal mit Nullen befuellen

            buffer_in = (char *)mxGetData(input[1]);
            for(int i = 0; i < (int)buffer_length; i++) {
                buffer_out[i] = buffer_in[i];
            }

            #ifdef DEBUG
                printf("Length = %u, %u, %u \n", buffer_length, m, n);
            #endif

            // read the Memory size
            mem_size = Vhsadac16_Sdram_Size( m_h_board_dac );
            #ifdef DEBUG
                printf("-->SDRAM with Memory size: %6.3f MB found.\n", (4.0*(double)mem_size)/(1024.0 * 1024.0));
            #endif
            if(buffer_length > mem_size) {
                mexErrMsgTxt("Error, SDRAM size is to small for Data. Exiting...\n");
            }
            
            #ifdef DEBUG
                printf("-->Write Data to SDRAM, starting at address 0.\n");
            #endif
            
            bytes_send = Vhsadac16_Sdram_Write( m_h_board_dac, 0, buffer_out, buffer_length);
   
            break;
        }
//---------------------------------------------------------------------------
        case READ_SDRAM_ADC:{
            int    length;
            PUCHAR buffer_in,buffer_aligned;
            ULONG  status, bytes_read;
            
            if(n_input!=2) mexErrMsgTxt("Wrong number of input variables! \n");
            
            // Check, if connected to Board and FPGA
            if(ulBoardID_ADC == -1) mexErrMsgTxt("Please connect to ADC Board first! \n");

            // Check if the SDRAM Module is present?
            status = Vhsadac16_Sdram_AutoInit( m_h_board_adc );
            if(status != 1) mexErrMsgTxt("No SDRAM Module found!\n Was it installed in the Simulink Model?\n");
            
            /* Retain length to be read */
            length = (int)mxGetScalar(input[1]);
            if(length < 1) mexErrMsgTxt("Length must be at least greater then zero!\n");

            /* Allocate memory for transfer */
            /* HR: added safety alignment to n*page size */
            buffer_in = (PUCHAR) malloc( (((length+8191) & ~8191) )*4 );
            if ( buffer_in == NULL ) mexErrMsgTxt("Error allocating input buffer. Exiting...\n");
            //memset(buffer_in, 0, length); // Erstmal mit Nullen befuellen (HR: irrelevant)
            
            #ifdef DEBUG
                printf("-->Reading Data from SDRAM, starting at address 0.\n");
            #endif
            { // align buffer to multiple of 4096
              int off = (int)buffer_in;
              off &= 4095;
              off  = 4096-off;
              buffer_aligned = buffer_in + off;
            }
            
            // Read the SDRAM into buffer
            bytes_read = (ULONG)Vhsadac16_Sdram_Read( m_h_board_adc, 0, buffer_aligned, length);
            if( bytes_read > 0x7fffffff )
            {
              fprintf(stderr,"Error: Vhsadac16_Sdram_Read returned %x\n",bytes_read );
            }
            else
            {
              int err = 1;
	            // Copy SDRAM Data to Matlab output buffer
        	    output[0] = mxCreateDoubleMatrix(bytes_read, 1, mxREAL);
              if( output[0] != NULL )
              {
  	            tmp = mxGetPr(output[0]);
                if( tmp != NULL )
                {
                  err = 0;
                  for(int i=0; i < bytes_read ; i++)
                  {
                      tmp[i] = (double)buffer_aligned[i];
                  }
                }
              }
              if( err )
                fprintf(stderr,"Error: cannot obtain output buffer from Matlab\n");
            }
            free(buffer_in);
            #ifdef DEBUG
                printf("Bytes read = %u \n", bytes_read );
            #endif

            break;
        }
//---------------------------------------------------------------------------
        case SET_PLAYBACK_SDRAM_DAC:{

            int m = 0, n = 0, channel_length;
            int trig_mode, trig_source;
            bool trig_enable = false;
            unsigned int *buffer_in;
            ULONG frame_size = 0;
            VHS_TRIGGER_SOURCE trigger_source;
            VHS_TRIGGER_MODE trigger_mode;
            RETURN_STATUS status;
            
            if(n_input!=5) {
                mexErrMsgTxt("Wrong number of input variables! \n");
                break;
            }
            
            m = mxGetM(input[1]);
            n = mxGetN(input[1]);

            if(m>n) { channel_length = m; } else { channel_length = n; };
            
            if(channel_length==0){
                mexErrMsgTxt("No channels for playback specified! \n");
            }
                
            buffer_in = (unsigned int *)mxGetData(input[1]);
            for(int i = 0; i < channel_length; i++) {
                #ifdef DEBUG
                    printf("Channel_Num = %d, Channel = %u\n", channel_length, buffer_in[i] );
                #endif
                status = VHSDAC_EnablePlayback( m_h_board_dac, buffer_in[i] );
                if(status != RETURN_NORMAL) mexErrMsgTxt("-->Playback Error!\n");
            }
            
            trig_mode   = (int) mxGetScalar(input[2]);
            trig_source = (int) mxGetScalar(input[3]);
            //clk_s = (int) mxGetScalar(input[5]);
            
            (trig_mode == 0)? trigger_mode = CONTINUOUS : trigger_mode = SINGLE_SHOT;
            (trig_source == 0)? trigger_source = TRIGGER_MANUAL : trigger_source = TRIGGER_EXTERNAL;
            
            if(trig_source == 1){
                trig_enable = true;
                external_trig = true;
            }
            else 
                trig_enable = false;
            
            status = VHSDAC_SetTriggerEnable( m_h_board_dac, trig_enable );
            if(status != RETURN_NORMAL) mexErrMsgTxt("-->Error setting Trigger Enable!\n");
            
            status = VHSDAC_SetTriggerSource( m_h_board_dac, trigger_source );
            if(status != RETURN_NORMAL) mexErrMsgTxt("-->Error setting Trigger Source!\n");
            
            status = VHSDAC_SetTriggerMode( m_h_board_dac, trigger_mode );
            if(status != RETURN_NORMAL) mexErrMsgTxt("-->Error setting Trigger Mode!\n");
            
            frame_size = (ULONG) mxGetScalar(input[4]);
            #ifdef DEBUG
                printf("Frame Size = %u\n", frame_size );
            #endif
            status = VHSDAC_SetPlaybackFramesize( m_h_board_dac, frame_size );
            if(status != RETURN_NORMAL) mexErrMsgTxt("-->Error setting Frame Size!\n");
            
            #ifdef DEBUG
                printf("-->Playback is initialized.\n");
            #endif

            break;
        }
//---------------------------------------------------------------------------
        case START_PLAYBACK_SDRAM_DAC:{
            RETURN_STATUS status;

            if(n_input!=1) {
                mexErrMsgTxt("Wrong number of input variables! \n");
                break;
            }

            status = VHSDAC_StartPlayback( m_h_board_dac );
            if(status != RETURN_NORMAL) mexErrMsgTxt("-->Error starting playback of the SDRAM! Is the DAC board running?\n");

            #ifdef DEBUG
                printf("-->Playback started.\n");
            #endif

            break;
        }
//---------------------------------------------------------------------------
        case STOP_PLAYBACK_SDRAM_DAC:{
            RETURN_STATUS status;

            if(n_input!=1) {
                mexErrMsgTxt("Wrong number of input variables! \n");
                break;
            }

            
            status = VHSDAC_StopPlayback( m_h_board_dac );
            if(status != RETURN_NORMAL) mexErrMsgTxt("-->Error stopping playback of the SDRAM! Was it already running?\n");

            #ifdef DEBUG
                printf("-->Playback stopped.\n");
            #endif

            break;
        }
//---------------------------------------------------------------------------
        case SET_RECORD_SDRAM_ADC:{

            int m = 0, n = 0, channel_length;
            int trig_mode, trig_source;
            bool trig_enable = false;
            unsigned int *buffer_in;
            ULONG frame_size = 0;
            VHS_TRIGGER_SOURCE trigger_source;
            RETURN_STATUS status;
            
            if(n_input!=4) {
                mexErrMsgTxt("Wrong number of input variables! \n");
                break;
            }
            
            m = mxGetM(input[1]);
            n = mxGetN(input[1]);

            if(m>n) { channel_length = m; } else { channel_length = n; };
            
            if(channel_length==0){
                mexErrMsgTxt("No channels for recording specified! \n");
            }
                
            buffer_in = (unsigned int *)mxGetData(input[1]);
            for(int i = 0; i < channel_length; i++) {
                #ifdef DEBUG
                    printf("Channel_Num = %d, Channel = %u\n", channel_length, buffer_in[i] );
                #endif
                status = VHSADC_EnableRecord( m_h_board_adc, buffer_in[i] );
                if(status != RETURN_NORMAL) mexErrMsgTxt("-->Record Error!\n");
            }
            
            trig_source = (int) mxGetScalar(input[2]);
            
            (trig_source == 0)? trigger_source = TRIGGER_MANUAL : trigger_source = TRIGGER_EXTERNAL;
            
            if(trig_source == 1){
                trig_enable = true;
                external_trig = true;
            }
            else 
                trig_enable = false;
            
            status = VHSADC_SetTriggerEnable( m_h_board_adc, trig_enable );
            if(status != RETURN_NORMAL) mexErrMsgTxt("-->Error setting Trigger Enable!\n");
            
            status = VHSADC_SetTriggerSource( m_h_board_adc, trigger_source );
            if(status != RETURN_NORMAL) mexErrMsgTxt("-->Error setting Trigger Source!\n");
            
            frame_size = (ULONG) mxGetScalar(input[3]);
            #ifdef DEBUG
                printf("Frame Size = %u\n", frame_size );
            #endif
            status = VHSADC_SetRecordFramesize( m_h_board_adc, frame_size );
            if(status != RETURN_NORMAL) mexErrMsgTxt("-->Error setting Frame Size!\n");
            
            #ifdef DEBUG
                printf("-->Record is initialized.\n");
            #endif

            break;
        }
//---------------------------------------------------------------------------
        case START_RECORD_SDRAM_ADC:{
            RETURN_STATUS status;

            if(n_input!=1) {
                mexErrMsgTxt("Wrong number of input variables! \n");
                break;
            }

            status = VHSADC_StartRecord( m_h_board_adc );
            if(status != RETURN_NORMAL) mexErrMsgTxt("-->Error starting recording of the SDRAM! Is the ADC board running?\n");

            #ifdef DEBUG
                printf("-->Recording started.\n");
            #endif

            break;
        }
//---------------------------------------------------------------------------
        case PLAYBACK_MEM_SDRAM_INIT:{
            
            printf("Initializing the Playback process ...\n");
            
            RETURN_STATUS status;
            VHS_TRIGGER_SOURCE trigger_source;
            VHS_TRIGGER_MODE trigger_mode;
            unsigned long ch_sel_dac;
            unsigned long pb_frame_size;
            int trig_mode, trig_source;
            bool trig_enable = false;
            VHS_PLAYBACK_STATE *pbstate = NULL;
            unsigned short plevel[] = {8};
            PROG_GAINS_STATUS gain_status;
    
            
            if(n_input!=6) mexErrMsgTxt("-->Wrong number of input variables! \n");
            
            
            ch_sel_dac = (unsigned long) mxGetScalar(input[1]);
            pb_frame_size = (unsigned long) mxGetScalar(input[2]);
           
            trig_mode =  (int) mxGetScalar(input[3]);
            trig_source =  (int) mxGetScalar(input[4]);
            //clk_s = (int) mxGetScalar(input[5]);
            
            (trig_mode == 0)? trigger_mode = CONTINUOUS : trigger_mode = SINGLE_SHOT;
            (trig_source == 0)? trigger_source = TRIGGER_MANUAL : trigger_source = TRIGGER_EXTERNAL;
            
            if(trig_source == 1){
                trig_enable = true;
                external_trig = true;
            }
            else 
                trig_enable = false;
            
            VHSMEM_FPGAInternalReset(m_h_board_dac);
            status = VHSDAC_SetTriggerEnable(m_h_board_dac, trig_enable);
            
            status = VHSMEM_EnablePlaybackChannels(m_h_board_dac, 0);
            if(status == RETURN_NORMAL){
                printf("-->Channel selcetion was succssesful\n");
            }
            else mexErrMsgTxt("-->Error setting channel selection\n");
            
            
            status = VHSMEM_SetPlaybackFramesize(m_h_board_dac, pb_frame_size);
            if(status == RETURN_NORMAL){
                printf("-->Frame size is setting to: %luK samples per Channel\n", pb_frame_size/1024);
            }
            else mexErrMsgTxt("-->Error setting frame size\n");
            
            status = VHSMEM_SetPlaybackTriggerSource (m_h_board_dac, trigger_source);
            if(status == RETURN_NORMAL){
               (trig_source == 0)? printf("-->Trigger source setting to: TRIGGER_MANUAL\n"):printf("-->Trigger source setting to: TRIGGER_EXTERNAL\n");;                

            }
            else mexErrMsgTxt("-->Error setting trigger source\n");
            
            status = VHSMEM_SetPlaybackTriggerMode(m_h_board_dac, trigger_mode);
            if(status == RETURN_NORMAL){
               (trig_mode == 0)? printf("-->Trigger mode setting to: CONTINUOUS\n") : printf("-->Trigger mode setting to: SINGLE_SHOT\n");                

            }
            else mexErrMsgTxt("-->Error setting trigger mode\n");
            

            
            
           break;
        
        
        }
//---------------------------------------------------------------------------
        case PLAYBACK_MEM_SDRAM:{
            
            printf("Writing data to the DDR-SDRAM memory ...\n");
            ULONG mem_size;
            unsigned long size, data_size;
            RETURN_STATUS status;
            VHS_PLAYBACK_STATE pbState;
            unsigned long data_size_to_write,size_readed;
            unsigned long ch_sel_dac;
            BYTE pbRunBit;
            char *buffer_in = NULL; 
            char *buffer_out = NULL;
            int i=0;
            bool segmentation = false;
            unsigned long size_to_write,copy_size;
            
            if(n_input!=4) mexErrMsgTxt("-->Wrong number of input variables! \n");
           
            // read the Memory size
            mem_size = VHSMEM_Sdram_Size(m_h_board_dac);
            printf("-->Memory size is: %luGB\n",mem_size);
            
            buffer_in = (char *)mxArrayToString(input[1]);   
            data_size_to_write = (unsigned long) mxGetScalar(input[2]);
            ch_sel_dac = (unsigned long) mxGetScalar(input[3]);
            
            FILE * pFile;
            pFile = fopen (buffer_in , "rb" );
            if (pFile==NULL) {fputs ("File error",stderr); exit (1);}

            // obtain file size:
            fseek (pFile , 0 , SEEK_END);
            size = ftell (pFile);
            fseek(pFile, 0, SEEK_SET);
            
            printf("-->Generated file size is %lu\n",size);
            
            if (data_size_to_write != size)
                printf("Erro reading file!\n");
            
            if (size > MAX_FRAME_SIZE)
                segmentation = true;
            else 
                segmentation = false;
            
            if(segmentation == false){
                printf("-->Writing Playback (%lu Bytes) Data to Memory\n",data_size_to_write); 
                data_size = load_pb_file(buffer_in, &buffer_out); 
                    // write data to the Memory
                size_readed = VHSMEM_Sdram_Write(m_h_board_dac,(UCHAR*)buffer_out, data_size);
                if(size_readed ==0){
                    mexErrMsgTxt("-->MEM could not be written\n");
                }
            }
           
            else
            {
                printf("-->Writing Playback (%lu Bytes) Data to Memory\n",data_size_to_write); 
                size_to_write = size - MAX_FRAME_SIZE;
                bool eof = false;
                i=1;
                while(size_to_write > 0 )
                {
                    
                    buffer_out = (char*) malloc (sizeof(char)*MAX_FRAME_SIZE);
                    if (buffer_out == NULL) {fputs ("Memory error",stderr); exit (2);}
                    // copy the file into the buffer:
                    copy_size = fread (buffer_out,sizeof(char),MAX_FRAME_SIZE,pFile);
                    if(copy_size == 0){
                        mexErrMsgTxt("-->FILE could not be readed\n");
                    }
                    // write data to the Memory
                    printf("-->Writing block %d from HD to Memory module\n",i);
                    size_readed = VHSMEM_Sdram_Write(m_h_board_dac,(UCHAR*)buffer_out, MAX_FRAME_SIZE);
                    if(size_readed == 0){
                        mexErrMsgTxt("-->MEM could not be written\n");
                    }
                   
                    if(size_to_write <= MAX_FRAME_SIZE && eof == false){
                        MAX_FRAME_SIZE = size_to_write;
                        eof = true;
                    }
                    else
                        size_to_write = size_to_write - MAX_FRAME_SIZE;
                    
                    
                    i++;
                    free(buffer_out);
                    buffer_out = NULL;
                    copy_size = 0;
                    size_readed = 0;
                    
                }
                
            }
            
            fclose(pFile);
            free(buffer_out);
            size = 0;
            buffer_out = NULL;           
            
            //pbState = getPbState(m_h_board_dac);
    
            if(external_trig){
                status = VHSMEM_StartPlayback(m_h_board_dac);
                if(status == RETURN_NORMAL){
                    printf("-->Playback is starting\n");
                }
                else mexErrMsgTxt("-->Playback Error!\n");
            
            
                //pbState = getPbState(m_h_board_dac);

                status = VHSMEM_GetPlaybackState(m_h_board_dac, &pbState);
                while(pbState == PLAYBACK_ARMED){
                     VHSMEM_GetPlaybackState(m_h_board_dac, &pbState);
                }
               //pbState = getPbState(m_h_board_dac);
               status = VHSMEM_StopPlayback(m_h_board_dac);
            }
            
            status = VHSMEM_EnablePlaybackChannels(m_h_board_dac, ch_sel_dac);
            if(status == RETURN_NORMAL){
                printf("-->Channel selcetion was succssesful\n");
            }
            else mexErrMsgTxt("-->Error setting channel selection\n");
            
            status = VHSMEM_StartPlayback(m_h_board_dac);
            if(status == RETURN_NORMAL){
                printf("-->Playback is starting\n");
            }
            else mexErrMsgTxt("-->Playback Error!\n");
            
            break;
        }
//---------------------------------------------------------------------------
        case RECORD_MEM_SDRAM_INIT:{
            
            printf("Initializing the Record process ...\n");
            
            RETURN_STATUS status;
            
            BOOL bResetAddress = true;
            VHS_ADC_REC_SRC record_source = ADC_NORMAL_ACQUISITION;
            VHS_TRIGGER_SOURCE trigger_source;  
            ULONG sdram_ini;
            ULONG size = 0;
            
            unsigned long ch_sel_adc;
            unsigned long rec_frame_size;
            unsigned long ch_num_adc;
            static unsigned int record_duration;
            int trig_source;
            bool trig_enable = false;        
            long recOffset;
            
            if(n_input!=6) mexErrMsgTxt("-->Wrong number of input variables! \n");
            
            ch_sel_adc = (unsigned long) mxGetScalar(input[1]);
            rec_frame_size = (unsigned long) mxGetScalar(input[2]);
            ch_num_adc = (unsigned long) mxGetScalar(input[3]);
            trig_source =  (int) mxGetScalar(input[4]);
            recOffset = (long) mxGetScalar(input[5]);
            size = VHSMEM_Sdram_Size(m_h_board);
            printf("-->The size of the MEM is: %luGB\n",size);
            
            
            (trig_source == 0)? trigger_source = TRIGGER_MANUAL : trigger_source = TRIGGER_EXTERNAL;
            
            if(trig_source == 0)
                trig_enable = false;
            else{
                trig_enable = true;
                external_trig = true;
            }

            status = VHSADC_SetTriggerEnable(m_h_board_adc, trig_enable);
            
            status = VHSMEM_SetRecordFramesize(m_h_board_adc, rec_frame_size);
            if(status == RETURN_NORMAL){
                printf("-->frame size is setting to: %luK samples per Channel\n",rec_frame_size/1024);
            }
            else mexErrMsgTxt("-->Error setting the frame size\n");

            status = VHSMEM_SetRecordInputSource(m_h_board_adc,record_source);
            if(status == RETURN_NORMAL){
                printf("-->Memory record source data sets to: ADC_NORMAL_ACQUISITION\n");
            }
            else mexErrMsgTxt("-->Error setting memory record source data\n");
            
            status = VHSMEM_SetRecordTriggerSource(m_h_board_adc, trigger_source);
            if(status == RETURN_NORMAL){
                (trig_source == 0)? printf("-->Memory trigger source setting to: TRIGGER_MANUAL\n") : printf("-->Memory trigger source setting to: TRIGGER_EXTERNAL\n");
            }
            else mexErrMsgTxt("-->Error setting memory trigger source data\n");

            status = VHSMEM_EnableRecordChannels(m_h_board_adc, ch_sel_adc);
            if(status == RETURN_NORMAL){
                printf("-->Channel selection was succssesful\n");
            }
            else mexErrMsgTxt("-->Error setting channel selection\n");
            
            status = VHSMEM_SetRecordTriggerOffset(m_h_board_adc,recOffset);
   
            break;
            
        }
//---------------------------------------------------------------------------
        case RECORD_MEM_SDRAM:{
            
            printf("Recording data from ADC to DDR-SDRAM memory ...\n");
            ULONG size = 0;
            ULONG data_size;
            RETURN_STATUS status;
            VHS_RECORD_STATE recState;
            char *buffer_in = NULL;
            char *tmp_buffer = NULL;
            char *file_name;
            char rec_file[30];
            VHS_LOCK_STATUS lock;
            unsigned long ch_sel_adc;
            int size_2 = 0;            
            LONG offset;
            bool segmentation = false;
           
            unsigned long data_size_to_read,duration;            
            bool first_one = true;

            unsigned long size_to_read;
            
            if(n_input!=5) mexErrMsgTxt("-->Wrong number of input variables! \n");
            
            file_name = (char *)mxArrayToString(input[1]);        
            data_size_to_read = (unsigned long) mxGetScalar(input[2]);
            duration = (unsigned long) mxGetScalar(input[3]);
            
            if (data_size_to_read > MAX_FRAME_SIZE)
                segmentation = true;
            else
                segmentation = false;
            
            ch_sel_adc = (unsigned long) mxGetScalar(input[4]);
            FILE *rxFile_big = fopen(file_name, "wb");
            if( rxFile_big == NULL )
            {
                printf("ERROR OPENING %s FILE!!!\n",file_name);
                mexErrMsgTxt("-->ERROR !!!");
            }


            VHSMEM_Sdram_Transfert_Reset(m_h_board_adc, true);  
            
            status = VHSMEM_StartRecord(m_h_board_adc);
            VHSMEM_WaitEndofRecord(m_h_board_adc,duration);
            
            printf("-->Reading %lu Bytes from the Memory ...\n",data_size_to_read);
            
            if (segmentation == false){
                FILE *rxFile = fopen(file_name, "wb");
                if( rxFile == NULL )
                {
                    printf("ERROR OPENING %s FILE!!!\n",file_name);
                    mexErrMsgTxt("-->ERROR !!!");
                }

                buffer_in = (char *)malloc(data_size_to_read);
                if(buffer_in == NULL) mexErrMsgTxt("-->Error allocating input buffer! \n");
                memset(buffer_in, 0, data_size_to_read);
                tmp_buffer = buffer_in; 
                
                data_size = VHSMEM_Sdram_Read(m_h_board_adc,(UCHAR*)tmp_buffer, data_size_to_read);
                if(data_size==0)
                    mexErrMsgTxt("-->Error in reading the MEM\n");
              
                status = VHSMEM_GetRecordBufferOffset(m_h_board_adc,&offset);
                if(status == RETURN_NORMAL){
                    printf("-->Get record buffer offsset was succssesful!\n");
                    printf("-->Record offset is: %ld Bytes\n",offset);
                }
                else mexErrMsgTxt("-->Error Getting Record buffer offset!\n");

                //offset the first buffer of the the recorded frame to only use valid data
                buffer_in = tmp_buffer + offset;
                data_size_to_read = data_size_to_read - offset;
                printf("-->Saving recorded Data to %s\n",file_name); 
                size_2 = fwrite(buffer_in, sizeof(char),data_size_to_read, rxFile);
                fclose(rxFile);
            }
            
            else
            {
                printf("-->Saving recorded Data to %s\n",file_name);                
                size_to_read = data_size_to_read - MAX_FRAME_SIZE;
                bool eof = false;
                int i = 1;
                while((size_to_read) > 0){
                    sprintf(rec_file,"data_recorded_%d.bin",i);
                    FILE *rxFile = fopen(rec_file, "wb");
                    buffer_in = (char *)malloc(MAX_FRAME_SIZE);
                    if(buffer_in == NULL) mexErrMsgTxt("-->Error allocating input buffer! \n");
                    memset(buffer_in, 0, MAX_FRAME_SIZE);
                    tmp_buffer = buffer_in; 
                
                    data_size = VHSMEM_Sdram_Read(m_h_board_adc,(UCHAR*)tmp_buffer, MAX_FRAME_SIZE);
                    if(data_size==0)
                        mexErrMsgTxt("-->Error in reading the MEM\n");
                    
                    // offset just the first buffer of the recorded data
                    if(first_one){
                        status = VHSMEM_GetRecordBufferOffset(m_h_board_adc,&offset);
                        if(status == RETURN_NORMAL){
                            printf("-->Get record buffer offset was succssesful!\n");
                            printf("-->Record offset is: %ld Bytes\n",offset);
                        }
                        else mexErrMsgTxt("-->Error Getting Record buffer offset!\n");
                        buffer_in = tmp_buffer + offset;
                        first_one = false;
                    }

                    size_2 = fwrite(buffer_in, sizeof(char),(MAX_FRAME_SIZE - offset), rxFile);
                    size_2 = 0;
                    size_2 = fwrite(buffer_in, sizeof(char),(MAX_FRAME_SIZE - offset), rxFile_big);
                    if(size_2 == 0)
                        mexErrMsgTxt("-->Error on writing to the file!\n");
                    
                    if(size_to_read <= MAX_FRAME_SIZE && eof == false){
                        MAX_FRAME_SIZE = size_to_read;
                        eof = true;
                    }
                    else
                        size_to_read = size_to_read - MAX_FRAME_SIZE;
                    
                    offset = 0;
                    free(buffer_in);
                    free(tmp_buffer);
                    buffer_in = NULL;
                    tmp_buffer = NULL;
                    data_size = 0;
                    size_2 = 0;
                    size = 0;
                    i++;
                    fclose(rxFile);
                }
            }

            fclose(rxFile_big);
            break;
             
        }
 //---------------------------------------------------------------------------       
        case START_MEM_PLAYBACK:{
            
            RETURN_STATUS status;
            
            if(n_input!=1) {
                mexErrMsgTxt("Wrong number of input variables! \n");
                break;
            }
            
            status = VHSMEM_StartPlayback(m_h_board_dac);
            if(status == RETURN_NORMAL){
                printf("-->Playback is started\n");
            }
            else mexErrMsgTxt("-->Playback Error!\n");
           
            break;
            
        
        }
//---------------------------------------------------------------------------        
        case STOP_MEM_PLAYBACK:{
            
            RETURN_STATUS status;
            
            if(n_input!=1) {
                mexErrMsgTxt("Wrong number of input variables! \n");
                break;
            }
            
            status = VHSMEM_StopPlayback(m_h_board_dac);
            if(status == RETURN_NORMAL){
                printf("-->Playback is stoped\n");
            }
            else mexErrMsgTxt("-->Playback Error!\n");
           
            break;
            
        
        }
//---------------------------------------------------------------------------
        case GET_STATE:{
                       
            
            VHS_RECORD_STATE recState;
            recState = getRecState(m_h_board_adc);

            VHS_PLAYBACK_STATE pbState;
            pbState  = getPbState(m_h_board_dac);
            break;
        }
//---------------------------------------------------------------------------        
        case GET_MEM_RUN:{
            RETURN_STATUS status;
            BYTE pbRunBit, recbit;
            
            status = VHSMEM_GetPlaybackRunBitValue(m_h_board_dac,&pbRunBit);
            printf("-->Pb Run bit: %d (DAC ON/OFF)\n",pbRunBit);
            status = VHSMEM_GetRecordRunBitValue(m_h_board_adc,&recbit);
            printf("-->Rec Run bit: %d (ADC ON/OFF)\n",recbit);
            break;
        
        
        }
//---------------------------------------------------------------------------
        case QUERY_DAC_BOARD:{
            int board_num_dac;

            if(n_input!=1) {
                mexErrMsgTxt("Wrong number of input variables! \n");
                break;
            }
            if(n_output !=1) {
                 mexErrMsgTxt("Wrong number of output variables! \n");
                 break;
            }
            
            //Board detection
            if (!Vhsadac16_Open(&m_context)){
                mexErrMsgTxt("Could not open VHS BOARD! You have one? \n");
                break;
            }
            
            
            if (number_vhs_dac > 0){
                for (board_num_dac = 0; board_num_dac < 8; board_num_dac++){
                    Vhsadac16_Close(m_h_DAC[board_num_dac]);
                }
            }
            number_vhs_dac = 0;
            
            for (board_num_dac = 0; board_num_dac < 8; board_num_dac++){
                //get Handle and Slot-ID for board
                m_h_DAC[board_num_dac] = Vhsadac16_getHandle(BT_VHSDAC16, (UCHAR)board_num_dac);
                
                if (m_h_DAC[board_num_dac] != INVALID_HANDLE_VALUE){
                    printf("-->Board VHS-DAC-8/16 found! -> Slot ID[%d] \n", board_num_dac );
                    number_vhs_dac++;
                }
            }
            if (number_vhs_dac == 0){
                mexErrMsgTxt("No boards found! Are you sure, you are using a VHS Board?\n");
                break;
            }else{
                printf("-->Found %d Board(s)...\n", number_vhs_dac);
                
                /* Create Output Array with Slot IDs */
                output[0] = mxCreateDoubleMatrix(1, 1, mxREAL);
                tmp = mxGetPr(output[0]);
                tmp[0] = number_vhs_dac - 1;
            }
            
            
            break;

        }
//---------------------------------------------------------------------------
        case QUERY_ADC_BOARD:{
            int board_num_adc;
            
            if(n_input!=1) {
                mexErrMsgTxt("Wrong number of input variables! \n");
                break;
            }
            if(n_output !=1) {
                 mexErrMsgTxt("Wrong number of output variables! \n");
                 break;
            }
            
            //Board detection
            if (!Vhsadac16_Open(&m_context)){
                mexErrMsgTxt("Could not open VHS BOARD! You have one? \n");
                break;
            }
            
            if (number_vhs_adc > 0){
                for (board_num_adc = 0; board_num_adc < 8; board_num_adc++){
                    Vhsadac16_Close(m_h_ADC[board_num_adc]);
                }
            }
            number_vhs_adc = 0;
           
            for (board_num_adc = 0; board_num_adc < 8; board_num_adc++){
                //get Handle and Slot-ID for board  
                m_h_ADC[board_num_adc] = Vhsadac16_getHandle(BT_VHSADC16, (UCHAR)board_num_adc);
                
                if (m_h_ADC[board_num_adc] != INVALID_HANDLE_VALUE){
                    printf("-->Board VHS-ADC-8/16 found! -> Slot ID[%d] \n", board_num_adc );
                    number_vhs_adc++;
                }
            }
            if (number_vhs_adc == 0){
                mexErrMsgTxt("No boards found! Are you sure, you are using a VHS Board?\n");
                break;
            }else{
                printf("-->Found %d Board(s)...\n", number_vhs_adc);
                
                /* Create Output Array with Slot IDs */
                output[0] = mxCreateDoubleMatrix(1, 1, mxREAL);
                tmp = mxGetPr(output[0]);
                tmp[0] = number_vhs_adc - 1;
            }
            break;
        }
 //---------------------------------------------------------------------------
        case CONNECT_DAC_BOARD: {
            if(number_vhs_dac <= 0){
                mexErrMsgTxt("Please execute QUERY_DAC_BOARD first!\n");
                break;
            }
            if(n_input!=2){
                mexErrMsgTxt("Wrong number of input variables!\n");
                break;
            }
            ulBoardID_DAC = (ULONG)mxGetScalar(input[1]);

            if(ulBoardID_DAC < 0 || ulBoardID_DAC > 7) {
                printf("Wrong Board ID! %d \n", ulBoardID_DAC);
                ulBoardID_DAC = -1;
            }else{
                printf("-->Connecting to Board with Slot ID[%u]\n", ulBoardID_DAC);
                m_h_board_dac = m_h_board = m_h_DAC[ulBoardID_DAC];
                
            }
            break;
        }
//---------------------------------------------------------------------------
        case CONNECT_ADC_BOARD: {
            if(number_vhs_adc <= 0){
                mexErrMsgTxt("Please execute QUERY_ADC_BOARD first!\n");
                break;
            }
            if(n_input!=2){
                mexErrMsgTxt("Wrong number of input variables!\n");
                break;
            }
            ulBoardID_ADC = (ULONG)mxGetScalar(input[1]);

            if(ulBoardID_ADC < 0 || ulBoardID_ADC > 7) {
                printf("Wrong Board ID! %d \n", ulBoardID_ADC);
                ulBoardID_ADC = -1;
            }else{
                printf("-->Connecting to Board with Slot ID[%u]\n", ulBoardID_ADC);
                m_h_board_adc = m_h_board = m_h_ADC[ulBoardID_ADC];
            }
            break;
        }
//---------------------------------------------------------------------------
        case DISCONNECT_DAC_BOARD: {
			int board_num_dac;
            //Closes all DAC_Board_Handles
            for (board_num_dac = 0; board_num_dac < 8; board_num_dac++){
                Vhsadac16_Close(m_h_DAC[board_num_dac]);
                ulBoardID_DAC = -1;
            }
            
            break;
            
        }
//---------------------------------------------------------------------------
        case DISCONNECT_ADC_BOARD: {
            int board_num_adc;
            //Closes all ADC_Board_Handles
            for (board_num_adc = 0; board_num_adc < 8; board_num_adc++){
                Vhsadac16_Close(m_h_ADC[board_num_adc]);
                ulBoardID_ADC = -1;                
            }
            
            break;
        } 
/* ------------------------------------------------------------------------------ */
        case READ_USER_REG_DAC: {
          int offset;
          int REGISTER;
          unsigned int ulData;

          if(n_input!=2) mexErrMsgTxt("Wrong number of input variables! \n");
          if(n_output !=1) mexErrMsgTxt("Wrong number of output variables! \n");
          // Check, if connected to Board and FPGA
          if(ulBoardID_DAC == -1) mexErrMsgTxt("Please connect to DAC Board first! \n");

          offset = (int)mxGetScalar(input[1]);
          if( (offset<0)||(offset>11) ) mexErrMsgTxt("Wrong user register number! \n");
          
          if( offset > 3 ) {
              REGISTER = VHSVIRTEX_USER_CTRL4 + (4 * ( offset - 4 ));
          } else {
              REGISTER = VHSVIRTEX_USER_CTRL0 + (4 * offset);
          }

          #ifdef DEBUG
            printf("Reading 32 Bit number!\n");
          #endif
          ulData = Vhsadac16_ReadFpgaReg(m_h_board_dac, REGISTER);

          /* Create Output Array with Slot IDs */
          output[0] = mxCreateDoubleMatrix(1, 1, mxREAL);
          tmp = mxGetPr(output[0]);
          tmp[0] = ulData;

          break;
        }
 /* ------------------------------------------------------------------------------ */
        case READ_USER_REG_ADC: {
          int offset;
          int REGISTER;
          unsigned int ulData;

          if(n_input!=2) mexErrMsgTxt("Wrong number of input variables! \n");
          if(n_output !=1) mexErrMsgTxt("Wrong number of output variables! \n");
          // Check, if connected to Board and FPGA
          if(ulBoardID_ADC == -1) mexErrMsgTxt("Please connect to ADC Board first! \n");

          offset = (int)mxGetScalar(input[1]);
          if( (offset<0)||(offset>11) ) mexErrMsgTxt("Wrong user register number! \n");
          
          if( offset > 3 ) {
              REGISTER = VHSVIRTEX_USER_CTRL4 + (4 * ( offset - 4 ));
          } else {
              REGISTER = VHSVIRTEX_USER_CTRL0 + (4 * offset);
          }
          
          #ifdef DEBUG
            printf("Reading 32 Bit number!\n");
          #endif
          ulData = Vhsadac16_ReadFpgaReg(m_h_board_adc, REGISTER);

          /* Create Output Array with Slot IDs */
          output[0] = mxCreateDoubleMatrix(1, 1, mxREAL);
          tmp = mxGetPr(output[0]);
          tmp[0] = ulData;

          break;
        }
/* ------------------------------------------------------------------------------ */
        case WRITE_USER_REG_DAC: {
          int offset;
          int REGISTER;
          ULONG ulData;

          if(n_input!=3) mexErrMsgTxt("Wrong number of input variables! \n");
          if(n_output !=0) mexErrMsgTxt("Wrong number of output variables! \n");
          // Check, if connected to Board and FPGA
          if(ulBoardID_DAC == -1) mexErrMsgTxt("Please connect to DAC Board first! \n");

          offset = (int)mxGetScalar(input[1]);
          if( (offset<0)||(offset>11) ) mexErrMsgTxt("Wrong user register number! \n");
          
          if( offset > 3 ) {
              REGISTER = VHSVIRTEX_USER_CTRL4 + (4 * ( offset - 4 ));
          } else {
              REGISTER = VHSVIRTEX_USER_CTRL0 + (4 * offset);
          }

          ulData = (ULONG)mxGetScalar(input[2]);

          #ifdef DEBUG
            printf("Writing 32 Bit number %x to board %x\n",ulData,m_h_board_dac);
          #endif
          Vhsadac16_WriteFpgaReg(m_h_board_dac, REGISTER, ulData);

          break;
        }
 /* ------------------------------------------------------------------------------ */
        case WRITE_USER_REG_ADC: {
          int offset;
          int REGISTER;
          ULONG ulData;

          if(n_input!=3) mexErrMsgTxt("Wrong number of input variables! \n");
          if(n_output !=0) mexErrMsgTxt("Wrong number of output variables! \n");
          // Check, if connected to Board and FPGA
          if(ulBoardID_ADC == -1) mexErrMsgTxt("Please connect to ADC Board first! \n");

          offset = (int)mxGetScalar(input[1]);
          if( (offset<0)||(offset>11) ) mexErrMsgTxt("Wrong user register number! \n");
          
          if( offset > 3 ) {
              REGISTER = VHSVIRTEX_USER_CTRL4 + (4 * ( offset - 4 ));
          } else {
              REGISTER = VHSVIRTEX_USER_CTRL0 + (4 * offset);
          }

          ulData = (ULONG)mxGetScalar(input[2]);

          #ifdef DEBUG
            printf("Writing 32 Bit number %x to board %x\n",ulData,m_h_board_adc);
          #endif
          Vhsadac16_WriteFpgaReg(m_h_board_adc, REGISTER, ulData);

          break;
        }       
/* ------------------------------------------------------------------------------ */
        case WRITE_RTDEX_DAC: {
          int m = 0, n = 0;
          char *buffer_in;
          int length;
          static int bufidx=-1;
// must be 2^n
#define bufidx_ct 4
          static PUCHAR buffer_out[bufidx_ct];
          static int buffer_sizes[bufidx_ct];
          unsigned long rtdexRet;
          VHSADAC_RTDEX_ERROR_INFO errInfo;

          if( bufidx==-1)
          {
           int i;
           for(i=0;i<bufidx_ct;i++)
           {
             buffer_out[i]   = (0);
             buffer_sizes[i] = 0;
           }
           bufidx=0;
          }
          
          if(n_input!=2) mexErrMsgTxt("Wrong number of input variables! \n");
          // Check, if connected to Board and FPGA
          if(ulBoardID_DAC == -1) mexErrMsgTxt("Please connect to DAC Board first! \n");
          if(rtdex_open_dac == false) mexErrMsgTxt("RTDEx must be opened first! \n");

          /* Check if Data is really uint8 */
          if(!mxIsUint8(input[1])) mexErrMsgTxt("Frame must ba a uint8 array! \n");
          /* Copy Matlab Data to VHS-ADAC Data */
          m = mxGetM(input[1]);
          n = mxGetN(input[1]);

          if(m>n) { length = m; } else { length = n; };

          /* Allocate memory for transfer */
          if( (buffer_out[bufidx] == (0)) || (buffer_sizes[bufidx] != length) )
          {
            if( buffer_out[bufidx] != (0))
              free( buffer_out[bufidx]);
            
            buffer_out[bufidx]   = (PUCHAR) malloc(length);
            buffer_sizes[bufidx] = length;
          }
          if ( buffer_out[bufidx] == NULL ) mexErrMsgTxt("Error allocating input buffer. Exiting...\n");
          //memset(buffer_out[bufidx], 0, length); // Erstmal mit Nullen befuellen (was soll das ?)

          buffer_in = (char *)mxGetData(input[1]);
          memcpy( buffer_out[bufidx], buffer_in, length );
          /* for(int i=0; i<length; i++) {(buffer_out[bufidx])[i] = buffer_in[i];}*/

          #ifdef DEBUG
            printf("Length = %u, %u, %u \n", length, m, n);
          #endif

          /* Start the Tx */
          Vhsadac16_RTDExReset(m_h_board_dac, FALSE, TRUE);

          /* Write the buffer to the FPGA. */
          // rtdexRet = 
          Vhsadac16_RTDExWriteBuffer(m_h_board_dac, buffer_out[bufidx], length);
          bufidx = (bufidx+1)&(bufidx_ct-1);

          // if (rtdexRet != 0) mexErrMsgTxt("Unable to configure TX buffer");

          /* Get error information from RTDEx */
          memset(&errInfo, 0, sizeof(errInfo));
          rtdexRet = Vhsadac16_GetRTDExError(m_h_board_dac, &errInfo);

          if (rtdexRet != 0) mexErrMsgTxt("Unable to get error information");

          #ifdef DEBUG
            /* Show information to the user. */
            printf("\n");
            printf("----------------------------\n");
            printf("RTDEx REPORT:               \n");
            printf("----------------------------\n");
            printf("BYTES SENT             = %u\n", (UINT) length);
            printf("RX OVERRUN  COUNT      = %u\n", errInfo.rxOverrun);
            printf("TX UNDERRUN COUNT      = %u\n", errInfo.txUnderrun);
            printf("----------------------------\n");
          #endif

          break;
        }
 /* ------------------------------------------------------------------------------ */
        case WRITE_RTDEX_ADC: {
          int m = 0, n = 0;
          static int bufidx=-1;
// must be 2^n, see define at DAC
//#define bufidx_ct 4
          char *buffer_in;
          int length;
          static PUCHAR buffer_out[bufidx_ct];
          unsigned long rtdexRet;
          VHSADAC_RTDEX_ERROR_INFO errInfo;

          if( bufidx==-1)
          {
           int i;
           for(i=0;i<bufidx_ct;i++)
             buffer_out[i]=(0);
           bufidx=0;
          }
          
          if(n_input!=2) mexErrMsgTxt("Wrong number of input variables! \n");
          // Check, if connected to Board and FPGA
          if(ulBoardID_ADC == -1) mexErrMsgTxt("Please connect to ADC Board first! \n");
          if(rtdex_open_adc == false) mexErrMsgTxt("RTDEx must be opened first! \n");

          /* Check if Data is really uint8 */
          if(!mxIsUint8(input[1])) mexErrMsgTxt("Frame must ba a uint8 array! \n");
          /* Copy Matlab Data to VHS-ADAC Data */
          m = mxGetM(input[1]);
          n = mxGetN(input[1]);

          if(m>n) { length = m; } else { length = n; };

          /* Allocate memory for transfer */
          if( buffer_out[bufidx] != (0) )
          {
            free( buffer_out[bufidx] );
          }
          buffer_out[bufidx] = (PUCHAR) malloc(length);
          if ( buffer_out[bufidx] == NULL ) mexErrMsgTxt("Error allocating input buffer. Exiting...\n");
          memset(buffer_out[bufidx], 0, length); // Erstmal mit Nullen befuellen

          buffer_in = (char *)mxGetData(input[1]);
          for(int i=0; i<length; i++) {
            (buffer_out[bufidx])[i] = buffer_in[i];
          }

          #ifdef DEBUG
            printf("Length = %u, %u, %u \n", length, m, n);
          #endif

          /* Start the Tx */
          Vhsadac16_RTDExReset(m_h_board_adc, FALSE, TRUE);

          /* Write the buffer to the FPGA. */
          rtdexRet = Vhsadac16_RTDExWriteBuffer(m_h_board_adc, buffer_out[bufidx], length);
          bufidx = (bufidx+1)&(bufidx_ct-1);

          if (rtdexRet != 0) mexErrMsgTxt("Unable to configure TX buffer");

          /* Get error information from RTDEx */
          memset(&errInfo, 0, sizeof(errInfo));
          rtdexRet = Vhsadac16_GetRTDExError(m_h_board_adc, &errInfo);

          if (rtdexRet != 0) mexErrMsgTxt("Unable to get error information");

          #ifdef DEBUG
            /* Show information to the user. */
            printf("\n");
            printf("----------------------------\n");
            printf("RTDEx REPORT:               \n");
            printf("----------------------------\n");
            printf("BYTES SENT             = %u\n", (UINT) length);
            printf("RX OVERRUN  COUNT      = %u\n", errInfo.rxOverrun);
            printf("TX UNDERRUN COUNT      = %u\n", errInfo.txUnderrun);
            printf("----------------------------\n");
          #endif

          break;
        }       
/* ------------------------------------------------------------------------------ */
        case READ_RTDEX_DAC: {
          int length = 0;
          PUCHAR buffer_in;
          unsigned long rtdexRet;
          VHSADAC_RTDEX_ERROR_INFO errInfo;

          if(n_input!=2) mexErrMsgTxt("Wrong number of input variables! \n");
          if(n_output!=1) mexErrMsgTxt("Wrong number of output variables! \n");
          // Check, if connected to Board and FPGA
          if(ulBoardID_DAC == -1) mexErrMsgTxt("Please connect to DAC Board first! \n");
          if(rtdex_open_dac == false) mexErrMsgTxt("RTDEx must be opened first! \n");
          
          length = (int)mxGetScalar(input[1]);
          if(length < 1) mexErrMsgTxt("Length must at least be greater then zero! \n");
          
          /* Allocate memory for transfer */
          buffer_in = (PUCHAR) malloc(length);
          if ( buffer_in == NULL ) mexErrMsgTxt("Error allocating input buffer. Exiting...\n");
          memset(buffer_in, 0, length); // Erstmal mit Nullen befuellen

          /* Start the Rx */
          Vhsadac16_RTDExReset(m_h_board_dac, TRUE, FALSE);

          /* RX buffer from the FPGA. */
          // rtdexRet = Vhsadac16_RTDExReadBuffer(m_h_board_dac, buffer_in, length);
          Vhsadac16_RTDExReadBuffer(m_h_board_dac, buffer_in, length);
          
		  // if (rtdexRet != 0) mexErrMsgTxt("Error while Read buffer");

          /* Get error information. */
          memset(&errInfo, 0, sizeof(errInfo));
          rtdexRet = Vhsadac16_GetRTDExError(m_h_board_dac, &errInfo);
          if (rtdexRet != 0) mexErrMsgTxt("Unable to get error information");

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

          break;
        }
/* ------------------------------------------------------------------------------ */
        case READ_RTDEX_ADC: {
          int length = 0;
          PUCHAR buffer_in;
          unsigned long rtdexRet;
          VHSADAC_RTDEX_ERROR_INFO errInfo;

          if(n_input!=2) mexErrMsgTxt("Wrong number of input variables! \n");
          if(n_output!=1) mexErrMsgTxt("Wrong number of output variables! \n");
          // Check, if connected to Board and FPGA
          if(ulBoardID_ADC == -1) mexErrMsgTxt("Please connect to ADC Board first! \n");
          if(rtdex_open_adc == false) mexErrMsgTxt("RTDEx must be opened first! \n");

          length = (int)mxGetScalar(input[1]);
          if(length < 1) mexErrMsgTxt("Length must at least be greater then zero! \n");

          /* Allocate memory for transfer */
          buffer_in = (PUCHAR) malloc(length);
          if ( buffer_in == NULL ) mexErrMsgTxt("Error allocating input buffer. Exiting...\n");
          memset(buffer_in, 0, length); // Erstmal mit Nullen befuellen

          /* Start the Rx */
          Vhsadac16_RTDExReset(m_h_board_adc, TRUE, FALSE);

          /* RX buffer from the FPGA. */
          // rtdexRet = Vhsadac16_RTDExReadBuffer(m_h_board_adc, buffer_in, length);
		  Vhsadac16_RTDExReadBuffer(m_h_board_adc, buffer_in, length);

          // if (rtdexRet != 0) mexErrMsgTxt("Error while Read buffer");

          /* Get error information. */
          memset(&errInfo, 0, sizeof(errInfo));
          rtdexRet = Vhsadac16_GetRTDExError(m_h_board_adc, &errInfo);
          if (rtdexRet != 0) mexErrMsgTxt("Unable to get error information");

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
          break;
        }       
/* ------------------------------------------------------------------------------ */
        case OPEN_RTDEX_DAC: {
          unsigned long rtdexRet;

          if(n_input!=1) mexErrMsgTxt("Wrong number of input variables! \n");
          // Check, if connected to Board and FPGA
          if(ulBoardID_DAC == -1) mexErrMsgTxt("Please connect to DAC Board first! \n");
          if(rtdex_open_dac == true) mexErrMsgTxt("RTDEx is already opened! \n");

          rtdexRet = Vhsadac16_isRTDExDetected(m_h_board_dac);

          if (rtdexRet != 1) {
            printf("RTDEx Return Code: %d\n", rtdexRet);
            mexErrMsgTxt("Unable to open RTDEx");
          }

          printf("RTDEx present!\n");
          rtdex_open_dac = true;

          break;
        }
/* ------------------------------------------------------------------------------ */
        case OPEN_RTDEX_ADC: {
          unsigned long rtdexRet;

          if(n_input!=1) mexErrMsgTxt("Wrong number of input variables! \n");
          // Check, if connected to Board and FPGA
          if(ulBoardID_ADC == -1) mexErrMsgTxt("Please connect to ADC Board first! \n");
          if(rtdex_open_adc == true) mexErrMsgTxt("RTDEx is already opened! \n");

          rtdexRet = Vhsadac16_isRTDExDetected(m_h_board_adc);

          if (rtdexRet != 1) {
            printf("RTDEx Return Code: %d\n", rtdexRet);
            mexErrMsgTxt("Unable to open RTDEx");
          }

          printf("RTDEx present!\n");
          rtdex_open_adc = true;

          break;
        }        
/* ------------------------------------------------------------------------------ */
        case CLOSE_RTDEX_DAC: {

          if(n_input!=1) mexErrMsgTxt("Wrong number of input variables! \n");
          // Check, if connected to Board and FPGA
          if(ulBoardID_DAC == -1) mexErrMsgTxt("Please connect to DAC Board first! \n");
          if(rtdex_open_dac == false) mexErrMsgTxt("RTDEx is not opened! \n");

          Vhsadac16_RTDExReset(m_h_board_dac, false, false);
          printf("RTDEx closed!\n");

          rtdex_open_dac = false;

          break;
        }
/* ------------------------------------------------------------------------------ */
        case CLOSE_RTDEX_ADC: {

          if(n_input!=1) mexErrMsgTxt("Wrong number of input variables! \n");
          // Check, if connected to Board and FPGA
          if(ulBoardID_ADC == -1) mexErrMsgTxt("Please connect to ADC Board first! \n");
          if(rtdex_open_adc == false) mexErrMsgTxt("RTDEx is not opened! \n");

          Vhsadac16_RTDExReset(m_h_board_adc, false, false);
          printf("RTDEx closed!\n");

          rtdex_open_adc = false;

          break;
        }
/* ------------------------------------------------------------------------------ */
        case RUN_DAC:{
            // int errorstat;
            // VHS_LOCK_STATUS clock_status;
            CLOCK_SOURCE clk_src;
            RETURN_STATUS status;
            if(n_input != 2) mexErrMsgTxt("Wrong number of input variables! \n");
            if(ulBoardID_DAC == -1) mexErrMsgTxt("Please connect to DAC Board first! \n");
            
            int clk_s = (int)mxGetScalar(input[1]);
            switch (clk_s){
                case 0:
                    clk_src = ONBOARD;
                    break;
                case 1:
                    clk_src = ONBOARD_DIVIDED;
                    break;
                case 2:
                    clk_src = EXTERNAL_FRONT;
                    break;
                 default:
                    clk_src = ONBOARD;
            }
            
            /*
            // set clock to programmable clock
            printf("\nSlot ID[%d]: Using DAC V4 ONBOARD FIXED clock...\n", ulBoardID_DAC);
            VHSDAC_DivideDIFClock(m_h_board_dac, true);
            VHSADC_SetClockSource(m_h_board_dac, ONBOARD);
            
            clock_status = VHSADC_GetLockStatus(m_h_board_dac);
            printf("Lock status: %d \n", clock_status);
            printf("DCM-Status: %d\n", VHSADC_DCMsReset (m_h_board_dac));
            printf("Slot ID[%d]: Setting DAC Run State\n", ulBoardID_DAC);
            errorstat = VHSDAC_SetTransmissionStatus(m_h_board_dac, ACQUISITION_ON);
            
            
            clock_status = VHSADC_GetLockStatus(m_h_board_dac);
            printf("Lock status: %d \n", clock_status);
            
            if(errorstat==RETURN_NORMAL)
                printf("DAC Run State successfully set to ON\n");
            else if (errorstat==RETURN_ERROR)
                printf("Error while setting DAC Run State to ON\n");
             */
            if(Vhsadac16_isVhsBoardV4(m_h_board_dac)){
                //V4...
                VHSDAC_SetClockSource(m_h_board_dac, ONBOARD);	//104MHz
                
                VHSDAC_DCMsReset(m_h_board_dac);
            }
            
            // set ADC run state
            status = VHSADC_SetAcquisitionStatus(m_h_board_dac,ACQUISITION_ON);
            if(status==RETURN_NORMAL)
                printf("-->VHSDAC_SetAcquisitionStatus() is set to ON.\n");
            else
                printf("ERROR: VHSDAC_SetAcquisitionStatus()\n");
            
            //initialise control clock
            status = VHSDAC_HostControlClock(m_h_board_dac, TRUE);
            if(status==RETURN_NORMAL)
                printf("-->VHSDAC_HostControlClock() is set to TRUE.\n");
            else
                printf("ERROR: VHSDAC_HostControlClock()\n");
            break;
        }       
/* ------------------------------------------------------------------------------ */
        case RUN_ADC:{
            // int errorstat;
            CLOCK_SOURCE clk_src;
            int clk_s;
            if(n_input!=2) mexErrMsgTxt("Wrong number of input variables! \n");
            if(ulBoardID_ADC == -1) mexErrMsgTxt("Please connect to ADC Board first! \n");
            clk_s = (int)mxGetScalar(input[1]);
            
            switch (clk_s){
                case 0:
                    clk_src = ONBOARD;
                    break;
                case 1:
                    clk_src = ONBOARD_DIVIDED;
                    break;
                case 2:
                    clk_src = EXTERNAL_FRONT;
                    break;
                 default:
                    clk_src = ONBOARD;
            }
            
  
            
            if(Vhsadac16_isVhsBoardV4(m_h_board_adc)){
                //V4...
                VHSADC_SetClockSource(m_h_board_adc, ONBOARD);	//104MHz
                
                VHSADC_DCMsReset(m_h_board_adc);
            }
//             
//             // set ADC run state
            RETURN_STATUS status = VHSADC_SetAcquisitionStatus(m_h_board_adc,ACQUISITION_ON);
            if(status==RETURN_NORMAL)
                printf("-->VHSADC_SetAcquisitionStatus() is set to ON.\n");
            else
                printf("ERROR: VHSADC_SetAcquisitionStatus()\n");
            
//             //initialise control clock
            status = VHSADC_HostControlClock(m_h_board_adc, TRUE);
            if(status==RETURN_NORMAL)
                printf("-->VHSADC_HostControlClock() is set to TRUE.\n");
            else
                printf("ERROR: VHSADC_HostControlClock()\n");
            
            
            break;            



//          set clock to programmable clock
//             printf("\nSlot ID[%d]: Using DAC V4 ONBOARD FIXED clock...\n", ulBoardID_ADC);
//             VHSADC_SetClockSource(m_h_board_adc, clk_src);
//             
//             printf("Slot ID[%d]: Setting DAC Run State\n", ulBoardID_ADC);
//             errorstat = VHSDAC_SetTransmissionStatus(m_h_board_adc, ACQUISITION_ON);
//             
//             if(errorstat==RETURN_NORMAL)
//                 printf("DAC Run State successfully set to ON\n");
//             else if (errorstat==RETURN_ERROR)
//                 printf("Error while setting DAC Run State to ON\n");
//             
            
            break;
        }
        
//---------------------------------------------------------------------------               
        case INIT_RFFE:{            
            if(n_input!=1){     
                mexErrMsgTxt("Wrong number of input variables!\n");
                break;
            }
            if(ulBoardID_DAC == -1 && ulBoardID_ADC == -1){
                mexErrMsgTxt("No Board found!\n");
                break;
            }
            
            if (RETURN_ERROR == RFFE_Init(m_h_board)){ //Initialization of the RFFE
                mexErrMsgTxt("Quad_Dual_Band_RF_Transceiver_Init Failed!\n");
                mexErrMsgTxt("Please check your Bitfile. It must contain a RFFE-Block!\n");
                rffe_init = false;
            }else{
                printf("Quad_Dual_Band_RF_Transceiver_Init was succesfull!\n");
                rffe_init = true;
                Vhsadac16_WriteFpgaReg(m_h_board, RFFE_EXTCTLR, 0); //External Control is deactivated
            }
            
            break;
        }
//---------------------------------------------------------------------------               
        case TRANS_MODE:{
          // transceiver mode for transceiver 1-4 or individual
            if (n_input == 2){
                int transmode = (int)mxGetScalar(input[1]);

                if (transmode == 0){
                    printf("Quad Dual Band RF Transceiver Rx!\n");
                } else {
                    if (transmode == 1){
                        printf("Quad Dual Band RF Transceiver Tx!\n");
                    }else{
                        printf ("Invalid Transceiver Mode Value!\n");
                        transmode = -1;
                    }
                }
                int i;
                for(i = 0 ; i < 3 ; i++ )
                    transmodes[i] = transmode;
            } 
            else if(n_input == 3) {
                int i;
                
                int transmode = (int)mxGetScalar(input[1]);
                i = (int)mxGetScalar(input[2]);            

                if (transmode == 0){
                    printf("Quad Dual Band RF Transceiver %d Rx!\n",i);
                } else {
                    if (transmode == 1){
                        printf("Quad Dual Band RF Transceiver %d Tx!\n",i);
                    }else{
                        printf ("Invalid Transceiver %d Mode Value!\n",i);
                        transmode = -1;
                    }
                }
                transmodes[i-1] = transmode;
            }
            break;
        }
//---------------------------------------------------------------------------       
        case SET_PABS_DSW_OSCS:{
            if (n_input != 4){
                mexErrMsgTxt("Wrong number of input variables!\n");
                break;    
            }
            
            if(ulBoardID_DAC == -1 && ulBoardID_ADC == -1){
                mexErrMsgTxt("No Board found!\n");
                break;
            }
            
            band = (int)mxGetScalar(input[1]);
            dsw  = (int)mxGetScalar(input[2]);
            oscs = (int)mxGetScalar(input[3]);
            
            if (band == 1){
                pabs    = 0;
                pabs_st = PABS_OFF; //5 GHz   
                pChFreq = chfreq2;
            } else if (band == 0){
                pabs    = 1;
                pabs_st = PABS_ON; //2.4 GHz
                pChFreq = chfreq1;
            } else {
                mexErrMsgTxt("Invalid frenquency band value.\n");
                pabs = -1;
                band = -1;
            }
            
            if (dsw == 0)    
                dsw_st = DSW_ON; //Antenna A
            else if (dsw == 1)    
                dsw_st = DSW_OFF; //Antenna B
            else {
                mexErrMsgTxt("Invalid DSW value.\n");
                dsw = -1;
            }
            
            if (oscs == 0)    
                oscs_st = OSCS_OFF; //External
            else if (oscs == 1)        
                oscs_st = OSCS_ON;  //Internal
            else{
                mexErrMsgTxt("Invalid OSCS value.\n");
                oscs = -1;
            }
            
            if(pabs == -1 || dsw == -1 || oscs == -1){
                mexErrMsgTxt("Could not set Pabs/Dsw/Oscs...pls try again!\n");
                break;
            }
            
            RFFE_SetPabsDswOscs(m_h_board, pabs_st, dsw_st, oscs_st);  
            printf("PABS: %d, DSW: %d, OSCS: %d\n", pabs_st, dsw_st, oscs_st);
            
            break;
        }        
//---------------------------------------------------------------------------       
        case INIT_SPI_REGISTER:{
            
            i2=i=0;
            
            if (n_input == 1){
                
                if(ulBoardID_DAC == -1 && ulBoardID_ADC == -1){
                    mexErrMsgTxt("No Board found!\n");
                    break;
                }
                
                for (i=0; i < NUM_OF_TRANS; i++){
                    for (i2=2; i2<13; i2++){
                        if (transmodes[i] == 1 ) { //ulTransmode == 1){
                            RFFE_SetRegisterSPI(m_h_board, transceiver_number_tab[i], rffe_spi_num_tab[i2], rffe_JStx[i2]);
                        }else{
                            RFFE_SetRegisterSPI(m_h_board, transceiver_number_tab[i], rffe_spi_num_tab[i2], rffe_JSrx[i2]);
                        }
                        Sleep(25);
                    }
                    if (transmodes[i] == 1 ) { //(ulTransmode == 1){
                        RFFE_SetMode(m_h_board, transceiver_number_tab[i], TRANSCEIVER_TX);
                        printf("Transceiver %d is in mode Tx\n", i+1);
                    }else{
                        RFFE_SetMode(m_h_board, transceiver_number_tab[i], TRANSCEIVER_RX);
                        printf("Transceiver %d is in mode Rx\n", i+1);

                        // Setting Rx Gain at maximum through the SPI Register
                        gain_value = 0x00;
                        RFFE_SetRegisterSPI(m_h_board,transceiver_number_tab[i], RFFE_RX_CONTROL_RSSI, 0x125);
                        Sleep(25);
                        RFFE_SetParallelGain(m_h_board,transceiver_number_tab[i], gain_value);
                        printf("\nTransceiver Rx Gain set to minimum through SPI communication\n");
                    }
                    SetChannel(ch, band, transceiver_number_tab[i], transmodes[i] );//ulTransmode);
                }
                
            } else if (n_input == 2) {
                
              i = (int)mxGetScalar(input[1]);
                
                if(i < 1 || i > 4){
                    mexErrMsgTxt("ERROR! Transceiver-Number must be between 1 and 4!\n");
                    break;
                }
                
                for (i2=2; i2<13;i2++){
                        if (transmodes[i-1] == 1 ){ //ulTransmode == 1){
                            RFFE_SetRegisterSPI(m_h_board, transceiver_number_tab[i-1], rffe_spi_num_tab[i2], rffe_JStx[i2]);
                        }else{
                            RFFE_SetRegisterSPI(m_h_board, transceiver_number_tab[i-1], rffe_spi_num_tab[i2], rffe_JSrx[i2]);
                        }
                        Sleep(25);
                    }
                    if (transmodes[i-1] == 1 ){ //ulTransmode == 1){
                        RFFE_SetMode(m_h_board, transceiver_number_tab[i-1], TRANSCEIVER_TX);
                        printf("Transceiver %d is in mode Tx\n", i);
                    }else{
                        RFFE_SetMode(m_h_board, transceiver_number_tab[i-1], TRANSCEIVER_RX);
                        printf("Transceiver %d is in mode Rx\n", i);

                        // Setting Rx Gain at maximum through the SPI Register
                        gain_value = 0x00;
                        RFFE_SetRegisterSPI(m_h_board,transceiver_number_tab[i-1], RFFE_RX_CONTROL_RSSI, 0x125);                        
                        Sleep(25);
                        RFFE_SetParallelGain(m_h_board,transceiver_number_tab[i-1], gain_value);
                        printf("Transceiver Rx Gain set to minimum through SPI communication\n");
                    }
                    SetChannel(ch, band, transceiver_number_tab[i-1], transmodes[i-1] );//ulTransmode);
                    
            } else {
                mexErrMsgTxt("Wrong number of input variables! \n");
                break;
            }
            
            printf("\nReading the Lock-Detect(LD) Digital Output of the Frequency Synthesizer\n");
            for(i=0; i<NUM_OF_TRANS; i++){
                lockState = RFFE_GetFreqSynthLockDetect(m_h_board, transceiver_number_tab[i]);
                printf("LD%d",i+1);
                printf("B: %d\n",lockState);
            }
            break;
        }
//---------------------------------------------------------------------------       
        case SET_GAIN:{
            if (n_input == 2){
                gain_value = (int)mxGetScalar(input[1]);
                
                if (gain_value < 0 || gain_value > 127){
                    mexErrMsgTxt("ERROR! Gain must be between 0 and 127!\n");
                    break;
                }
                for (i=0; i<NUM_OF_TRANS; i++){
                    printf("Setting Gain of Transceiver %d to 0x%x\n", i+1, gain_value);
                    RFFE_SetParallelGain( m_h_board, transceiver_number_tab[i], gain_value );    
                }
                
            } else if(n_input == 3) {
                gain_value = (int)mxGetScalar(input[1]);
                
                i = (int)mxGetScalar(input[2]);
                if (gain_value < 0 || gain_value > 127){
                    mexErrMsgTxt("ERROR! Gain must be between 0 and 127!\n");
                    break;
                }
                if(i < 1 || i > 4){
                    mexErrMsgTxt("ERROR! Transceiver-Number must be between 1 and 4!\n");
                    break;
                }
                printf("Setting Gain of Transceiver %d to 0x%x\n", i, gain_value);
                RFFE_SetParallelGain( m_h_board, transceiver_number_tab[i-1], gain_value );               
                
            } else mexErrMsgTxt("Wrong number of input variables!\n");
            
            break;
        }
//---------------------------------------------------------------------------   
        case SET_POWER_AMPLIFIER:{
            int power_status;
            if (n_input > 3 || n_input < 2){
                mexErrMsgTxt("Wrong number of input variables!\n");
                break;
            }
            
            power_status = (int)mxGetScalar(input[1]);
            
            if (rffe_init == false){
                mexErrMsgTxt("Please initialise RFFE first\n");
            }
            
            if (power_status > 1 || power_status < 0)
                mexErrMsgTxt("Wrong power amplifier status! The power amplifier can only be set on(1) or off(0)!\n");
            if (n_input == 2){
                for (i = 0; i < NUM_OF_TRANS; i++){
                    if( transmodes[i] == 1 ) { /* proceed only if the current transceiver is set to "TX" */
                        if(power_status == 1){
                         printf("Power amplifier will be set to ON (Transceiver %d)...",i+1);
                         if(RETURN_ERROR == RFFE_SetPaen(m_h_board, transceiver_number_tab[i], PAEN_ON)){
                             printf("\nPower amplifier on Transceiver %d could not bet set to ON!\n", i+1);
                         }else{
                             printf("done!\n");
                         }
                        }else{
                         printf("Power amplifier will be set to OFF (Transceiver %d)...",i+1);
                         if(RETURN_ERROR == RFFE_SetPaen(m_h_board, transceiver_number_tab[i], PAEN_OFF)){
                             printf("\nPower amplifier on Transceiver %d could not bet set to OFF!\n", i+1); 
                         }else{
                             printf("done!\n");
                         }
                        }
                    } /* if(transmodes[i] == 1 */
                }
                    
                
            } else {
              
                i = (int)mxGetScalar(input[2]);

                if(i < 1 || i > 4){
                    mexErrMsgTxt("ERROR! Transceiver-Number must be between 1 and 4!\n");
                    break;
                }

                if (transmodes[i-1] == 0){
                    printf("RX Transceiver detected, cannot proceed!\n");
                    break;
                }else if(transmodes[i-1] == -1){
                    printf("Unknown Transceivermode. Please define the Transceiver-Mode first!\n");
                    break;
                }
                
                if(power_status == 1){
                    printf("Power amplifier will be set to ON (Transceiver %d)...",i);
                    if(RETURN_ERROR == RFFE_SetPaen(m_h_board, transceiver_number_tab[i-1], PAEN_ON)){
                        printf("\nPower amplifier on Transceiver %d could not bet set to ON!\n", i);
                    }else{
                        printf("done!\n");
                    }
                } else {
                    printf("Power amplifier will be set to OFF (Transceiver %d)...",i);
                    if(RETURN_ERROR == RFFE_SetPaen(m_h_board, transceiver_number_tab[i-1], PAEN_OFF)){
                        printf("\nPower amplifier on Transceiver %d could not bet set to OFF!\n", i);
                    }else{
                        printf("done!\n");
                    }
                }
            }
            
            break;
        }
//---------------------------------------------------------------------------  
        case QUIT:{ //all handles are released and/or closed
            if(n_input != 1) { 
                mexErrMsgTxt("Wrong number of input variables! \n");
                break;
            }
            if (rffe_init == true){               
                for (i=0; i< NUM_OF_TRANS; i++){
                    printf("Setting transceiver %d in shutdown mode...\n", i+1);
                    RFFE_SetMode(m_h_board, transceiver_number_tab[i], TRANSCEIVER_SHUTDOWN);
                }                
                data = 0x02;
                /*
                 *On 1, disables the output enable latch of I/O 1 and 
                 * 2. Forces pins SHDN, TXEN, RXEN, PAEN, and RXHP of each
                 * channel to 1, placing the complete unit on standby. 
                 * Must be set to 0 to control the Quad Dual Band RF Transceiver.
                 */
                Vhsadac16_WriteFpgaReg(m_h_board, rffe_base + 0x6C, data);
                rffe_init = false;
                printf("Done.\n");
            }            
            break;            
        }
//---------------------------------------------------------------------------   
        
        case SET_CHANNEL:{
            if (rffe_init == false){
                mexErrMsgTxt("Error! No rffe_handle found!\n");
                mexErrMsgTxt("Please execute \"Search_Board\" and then \"Connect_Board\"! \n"); 
                break;
            }
            
            if (n_input < 2 || n_input > 3){
                mexErrMsgTxt("Wrong number of input variables! \n");
                break;
            }
            
            int channel = (int)mxGetScalar(input[1]);
            
            if ( (pabs == 1 && channel > 13) || (pabs == 1 && channel < 0) || (pabs == 0 && channel > 22) || (pabs == 0 && channel < 0) )
            {
                mexErrMsgTxt("Invalid Channel value entered!\n");
                break;
            }
     
                
            if (n_input == 3){
                int i = (int)mxGetScalar(input[2]);
                
                if(i < 1 || i > 4){
                    mexErrMsgTxt("ERROR! Transceiver-Number must be between 1 and 4!\n");
                    break;
                }
                
                SetChannel(channel, band, transceiver_number_tab[i-1], transmodes[i-1]);// ulTransmode);          
            }else{
                for (i = 0; i < NUM_OF_TRANS; i++){
                    SetChannel(channel, band, transceiver_number_tab[i], transmodes[i]);//ulTransmode);
                }
            }
            break;
        }
//------------------------------------------------------------------------ 
        case SET_TX_LOW_PASS_FILTER:
        {
            if (n_input > 3 || n_input < 2){
                mexErrMsgTxt("Arguments to low pass filter <corner_freq>,[transceiver], where corner_freq is in kHz - 12000,18000,24000 transceiver is 1...4 when not omitted\n");
                break;
            }
            if (n_input == 2){
                
                if(ulBoardID_DAC == -1 && ulBoardID_ADC == -1){
                    mexErrMsgTxt("No Board found!\n");
                    break;
                }

                for( i = 0 ; i < 4 ; i++ )
                  transceiver_lowpass[i] &= ~96; // clear D5,D6 receiver bits

                int freq = (int)mxGetScalar(input[1]);
                int fbits = 0;
                switch(freq)
                {
                  case 12000:
                    fbits = (1)<<5;break;
                  case 18000:
                    fbits = (2)<<5;break;
                  case 24000:
                    fbits = (3)<<5;break;
                  default:
                    fbits = (2)<<5;break;
                }
                for( i = 0 ; i < 4 ; i++ )
                  transceiver_lowpass[i] |= fbits; // set D3,D4 receiver bits

                for (i=0; i < NUM_OF_TRANS; i++)
                {
                    RFFE_SetRegisterSPI(m_h_board, transceiver_number_tab[i], rffe_spi_num_tab[7], transceiver_lowpass[i] );
                    printf("setting transceiver %d low-pass filter to 0x%x\n",i,transceiver_lowpass[i]);
                    Sleep(25);
                }
                
            } else {
                i = (int)mxGetScalar(input[2]);
                if(i < 1 || i > 4){
                    mexErrMsgTxt("ERROR! Transceiver-Number must be between 1 and 4!\n");
                    break;
                }
                i--;

                transceiver_lowpass[i] &= ~96; // clear D5,D6 receiver bits

                int freq = (int)mxGetScalar(input[1]);
                int fbits = 0;
                switch(freq)
                {
                  case 12000:
                    fbits = (1)<<5;break;
                  case 18000:
                    fbits = (2)<<5;break;
                  case 24000:
                    fbits = (3)<<5;break;
                  default:
                    printf("invalid frequency %d kHz, using default\n",freq);
                    fbits = (2)<<5;break;
                }
                transceiver_lowpass[i] |= fbits; // set D3,D4 receiver bits
                RFFE_SetRegisterSPI(m_h_board, transceiver_number_tab[i], rffe_spi_num_tab[7], transceiver_lowpass[i] );
                printf("setting transceiver %d low-pass filter to 0x%x\n",i,transceiver_lowpass[i]);
                Sleep(25);
            }
        }
        break;        
        //------------------------------------------------------------------------ 
        /*
         * RX Lowpass filter, ask VHS library about available corner frequencies by
         *  VHS('Set_RX_Low_Pass')
         * example:
         *  VHS('Set_RX_Low_Pass',9500,1); % set receiver 1 to 9.5 GHz corner freq
        */
        case SET_RX_LOW_PASS_FILTER:
        {
          int imin,imax;
          
            if (n_input > 3 || n_input < 2){
                printf("Arguments to low pass filter <corner_freq>,[transceiver],\n where corner_freq is in kHz - 7500,9500,9975,10450,12600,14000,18000,19800\n transceiver is 1...4 when not omitted\n");
                break;
            }
            //printf("Arguments %s %d %d\n",mxArrayToString(input[0]),(int)mxGetScalar(input[1]),(int)mxGetScalar(input[2]));
            imin = 0;
            imax = NUM_OF_TRANS;
            if( n_input == 3)
            {
              imin = (int)mxGetScalar(input[2]) - 1; // transceiver passed as 1...4, used as 0..3 here
              imax = imin+1; // loop says "<"...
              if( (imin < 0) || (imax>4) )
              {
                printf("Transceiver number %d out of range (%d) ",imax,imin);
                mexErrMsgTxt("Valid Range for Transceiver number is 1...4 !\n");
                break;
              }
            }
               
            if(ulBoardID_DAC == -1 && ulBoardID_ADC == -1){
                mexErrMsgTxt("No Board found or connected!\n");
                break;
            }

            for( i = imin ; i < imax ; i++ )
             transceiver_lowpass[i] &= ~31; // clear D0,D1,D2,D3,D4 receiver bits

            int freq = (int)mxGetScalar(input[1]);
            int fbits = 0;
            switch(freq)
            {
                  case 7500:
                    fbits = ((0)<<3)|2;break; // coarse 7500, fine 100%
                  case 9500:
                    fbits = ((1)<<3)|2;break; // coarse 9500, fine 100%
                  case 9975:
                    fbits = ((1)<<3)|3;break; // coarse 9500, fine 105%
                  case 10450:
                    fbits = ((1)<<3)|4;break; // coarse 9500, fine 110%
                  case 12600:
                    fbits = ((2)<<3)|0;break; // coarse 14000, fine 90%
                  case 14000:
                    fbits = ((2)<<3)|2;break; // coarse 14000, fine 100%
                  case 18000:
                    fbits = ((3)<<3)|2;break; // coarse 18000, fine 100%
                  case 19800:
                    fbits = ((3)<<3)|4;break; // coarse 18000, fine 110%
                  default:
                    printf("invalid frequency %d kHz, using default 9.5 MHz\n",freq);
                    fbits = ((1)<<3)|2;break;
            }
            for( i = imin ; i < imax ; i++ )
                  transceiver_lowpass[i] |= fbits; // set D3,D4 receiver bits

            for (i=imin; i < imax; i++)
            {
                  RFFE_SetRegisterSPI(m_h_board, transceiver_number_tab[i], rffe_spi_num_tab[7], transceiver_lowpass[i] );
                  printf("setting transceiver %d low-pass filter to 0x%x\n",i,transceiver_lowpass[i]);
                  Sleep(25);
            }
        }
        break;
//------------------------------------------------------------------------ 
        case SET_RX_HIGH_PASS_FILTER:{
            int high_pass_status;
            if (n_input > 3 || n_input < 2){
                mexErrMsgTxt("Wrong number of input variables! \n");
                break;
            }
            high_pass_status = (int)mxGetScalar(input[1]);
            if (rffe_init == false){
                mexErrMsgTxt("Please initialise RFFE first\n");
            }
            
            if (high_pass_status > 1 || high_pass_status < 0)
                mexErrMsgTxt("Wrong RX high pass filter status! The RX high pass filter can only be set to on(1) or off(0)!\n");
            
            if (n_input == 2){
                for (i = 0; i < NUM_OF_TRANS; i++){
                    if( transmodes[i] == 0 ) {
                     if(high_pass_status == 1){
                        printf("RX high pass filter will be set to ON (Transceiver %d)...",i+1);
                        if(RETURN_ERROR == RFFE_SetRxhp(m_h_board, transceiver_number_tab[i], RXHP_OFF)){
                            printf("\nRX high pass filter on Transceiver %d could not bet set to ON!\n", i+1);
                        }else{
                            printf("done!\n");
                        }
                     }else{
                        printf("RX high pass filter will be set to OFF (Transceiver %d)...",i+1);
                        if(RETURN_ERROR == RFFE_SetRxhp(m_h_board, transceiver_number_tab[i], RXHP_ON)){
                            printf("\nRX high pass filter on Transceiver %d could not bet set to OFF!\n", i+1); 
                        }else{
                            printf("done!\n");
                        }
                     }
                    } /* if( transmodes[i] == 0 ) */
                }
            }else{
                i = (int)mxGetScalar(input[2]);              

                if (transmodes[i-1] == 1){
                    printf("TX Transceiver detected, cannot proceed!\n");
                    break;
                }else if(transmodes[i-1] == -1){
                    printf("Unknown Transceivermode. Please define the Transceiver-Mode first!\n");
                    break;
                }            
                
                if(high_pass_status == 1){
                    printf("RX high pass filter will be set to ON (Transceiver %d)...",i);
                    if(RETURN_ERROR == RFFE_SetRxhp(m_h_board, transceiver_number_tab[i-1], RXHP_OFF)){
                        printf("\nRX high pass filter on Transceiver %d could not bet set to ON!\n", i);
                    }else{
                      // set RFFE_RX_CONTROL_RSSI to highpass at 30 kHz (600 kHz)
                      RFFE_SetRegisterSPI(m_h_board, transceiver_number_tab[i-1], rffe_spi_num_tab[8], 37 );
                        printf("done!\n");
                    }
                }else{
                    printf("RX high pass filter will be set to OFF (Transceiver %d)...",i);
                    if(RETURN_ERROR == RFFE_SetRxhp(m_h_board, transceiver_number_tab[i-1], RXHP_ON)){
                        printf("\nRX high pass filter on Transceiver %d could not bet set to OFF!\n", i);
                    }else{
                      // set RFFE_RX_CONTROL_RSSI to highpass at 100 Hz
                      RFFE_SetRegisterSPI(m_h_board, transceiver_number_tab[i-1], rffe_spi_num_tab[8], 33 );
                        printf("done!\n");
                    }
                }
            } 
            break;
        }
//------------------------------------------------------------------------
        case READ_TX_POWER:{
            unsigned int ulData; 
            //int REGISTER;
            //REGISTER = rffe_base + 0x5C;
            ulData = Vhsadac16_ReadFpgaReg(m_h_board, rffe_base + 0x5C);
            printf("Power output to antenna 1: 0x%x\n", ulData);
            ulData = Vhsadac16_ReadFpgaReg(m_h_board, rffe_base + 0x60);
            printf("Power output to antenna 2: 0x%x\n", ulData);
            ulData = Vhsadac16_ReadFpgaReg(m_h_board, rffe_base + 0x64);
            printf("Power output to antenna 3: 0x%x\n", ulData);
            ulData = Vhsadac16_ReadFpgaReg(m_h_board, rffe_base + 0x68);
            printf("Power output to antenna 4: 0x%x\n", ulData);
            break;
         }
//------------------------------------------------------------------------
/*!
   \brief  turn RFFE control over to FPGA board (external) or keep at PC (internal)
   \param  ext_control external mode is set on (1=FPGA) or off (0=Host PC)
   \param  ext_tran    external transceiver control on (1=FPGA) or off (0=Host PC) 
   \param  ext_gain    external gain (1=FPGA) or off (0=Host PC)
   \param  ext_freq    external frequency control on (1=FPGA) or off (0=Host PC)
*/
         case SET_EXTERNAL:{
		if (n_input < 5)
		{
                	mexErrMsgTxt("Wrong number of input variables! usage: 'Set_External',ext_control,ext_tran,ext_gain,ext_freq \n");
                	break;
		}
		int ext_control = (int)mxGetScalar(input[1]);
		int ext_tran = (int)mxGetScalar(input[2]);
		int ext_gain = (int)mxGetScalar(input[3]);
		int ext_freq = (int)mxGetScalar(input[4]);
		

	 	printf("External Mode is set to %s\n", (ext_control==1) ? "FPGA" : "Host" );
 		printf("External Transceiver is set to %s\n", (ext_tran==1) ? "FPGA" : "Host" );
	 	printf("External Gain is set to %s\n", (ext_gain==1) ? "FPGA" : "Host" );
 		printf("External Frequency is set to %s\n", (ext_freq==1) ? "FPGA" : "Host" );

    ext_control = (ext_control==1) ? MODE_EXTCTRL_ON : MODE_EXTCTRL_OFF;
		ext_tran    = (ext_tran==1)    ? ADD_EXTCTRL_ON  : ADD_EXTCTRL_OFF;
		ext_gain    = (ext_gain==1)    ? GAIN_EXTCTRL_ON : GAIN_EXTCTRL_OFF;
		ext_freq    = (ext_freq==1)    ? FREQ_EXTCTRL_ON : FREQ_EXTCTRL_OFF;
#if 0
{ unsigned int tst = ext_control|ext_tran|ext_gain|ext_freq;
		RFFE_SetExternalControl(m_h_board, (MODE_EXTCTRL)tst,(ADD_EXTCTRL)tst, (GAIN_EXTCTRL)tst, (FREQ_EXTCTRL)tst);
}
#else
		RFFE_SetExternalControl(m_h_board, (MODE_EXTCTRL)ext_control, (ADD_EXTCTRL)ext_tran, (GAIN_EXTCTRL)ext_gain, (FREQ_EXTCTRL)ext_freq);
#endif
#if 0
int i;
for(i=0;i<4;i++)
{
    RFFE_SetRegisterSPI(m_h_board, transceiver_number_tab[i], (REGISTER_SPI_NUMBER)RFFE_EXTCTLR, ext_control|ext_tran|ext_gain|ext_freq );
}
#endif    
    break;
	}
  /*
     print version of VHS library
  */
  case VERSION:
    printf(versionstring,datestring,copyrightstring,copyrightstring2); // versionstring contains a %s for additional strings
    break;
//         
//         case ADACSync:{
//             
//             E_ADACSync_InputClock inClock = ADACSync_Onboard;
//             
//             Result result;
//             result = ADACSync_SetInputClock(hHandle, inClock);
//             result = ADACSync_EnableOutputClock(hHandle,ADACSync_Clock_0,true);
//             
//             break;
//         }
 
    }
}

int load_file(const char *filename, char **result){
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
    

    if (size != fread(*result, sizeof(char),size, f))
    {
        free(*result);
        return -2; // -2 means file reading fail
    }
    fclose(f);
    (*result)[size] = 0;
    return size;
}

int load_pb_file(const char *filename, char **result){
    
  int size = 0,copy_size=0;
  FILE * pFile;

  pFile = fopen (filename , "rb" );
  if (pFile==NULL) {fputs ("File error",stderr); exit (1);}

  // obtain file size:
  fseek (pFile , 0 , SEEK_END);
  size = ftell (pFile);
  fseek(pFile, 0, SEEK_SET);
    
  //printf("-->Size of the playback file is: %d MB\n",size/1024/1024);

  // allocate memory to contain the whole file:
  *result = (char*) malloc (sizeof(char)*size);
  if (result == NULL) {fputs ("Memory error",stderr); exit (2);}

  // copy the file into the buffer:
  copy_size = fread (*result,sizeof(char),size,pFile);
  
  if (size != copy_size) {
       printf("-->%d Bytes Copying to the memory buffer\n",copy_size); 
       mexErrMsgTxt("-->Error: copying file to memory buffer!\n");
  }
 
  /* the whole file is now loaded in the memory buffer. */
  // terminate
  fclose (pFile);
  
  return copy_size; 
    
}

int save_record_file(const char *result,char *source_buffer, LONG *offset, unsigned long data_recorded_bytes){

    int size = 0;
    unsigned long file_size_bytes = data_recorded_bytes - *offset;
    //printf("file_size_bytes = %ld\n",file_size_bytes);
    
   // printf("saving file...\n");
    FILE *rxFile = fopen(result, "wb");
	if( rxFile == NULL )
	{
		printf("ERROR OPENING %s FILE!!!\n",result);
		return -1;
	}
    
    size = fwrite(source_buffer, sizeof(char),file_size_bytes, rxFile);
    fclose(rxFile);
	//printf("End saving file...\n");
    return size;

}

char* files_available ( char** files, unsigned n ) { 
    FILE* fp; unsigned i; 
  
    for ( i=0; i<n; i++ ) { 
        if ( NULL == ( fp = fopen ( files[i], "rb" ))) 
            return files[i]; 
        else 
            fclose ( fp ); 
    } 
    return NULL; 
} 
  
int join_files ( char* joined, char** files, unsigned n ) { 
    FILE* in, *out; unsigned bytes; char buf [4096] = {0}; 
    unsigned i; char* fname = NULL; 
  
    if ( NULL != ( fname = files_available ( files, n ))) { // Sind wir alle daaa?! 
        printf ( "Can't open %s for reading!\n", fname ); 
        return 1; // Nein! 
    } 
    
    if ( NULL == ( out = fopen ( joined, "wb" ))) { 
        printf ( "Can't open %s for writing!\n", joined ); 
        return 1; 
    } 
        
    for ( i = 0; i < n; i++ ) { 
        if ( NULL == ( in = fopen ( files[i], "rb"))) { 
            printf ( "Can't open %s for reading!\n", files[i] ); 
            fclose ( out ); 
            return 1; 
        } 
        while ( 0 != ( bytes = fread ( buf, sizeof(char), sizeof(buf), in ))) { 
            fwrite ( buf, sizeof(char), bytes, out ) ; 
        } 
        if ( ferror (in) || ferror (out) ) { 
                printf ( "IO Error!" ); 
                fclose (in), fclose (out); 
                return 1; 
        } 
        fclose ( in ); 
    } 
  
    fclose ( out ); 
    return 0; 
} 

void SetChannel(unsigned int ch,unsigned int band,TRANSCEIVER_NUMBER transceiver_num, ULONG mode)
{
    // Valeurs des registres Integer Divider Ratio et Fractional divider Ratio    
    // Band 2.4 GHz
    unsigned int IDRreg1[]={0x00A0,0x20A1,0x30A1,0x00A1,0x20A2,0x30A2,0x00A2,0x20A3,0x030A3,0x00A3,0x20A4,0x30A4,0x00A4,0x10A5};
    unsigned int FDRreg1[]={0x3333,0x0888,0x1DDD,0x3333,0x0888,0x1DDD,0x3333,0x0888,0x1DDD,0x3333,0x0888,0x1DDD,0x3333,0x2666};
    // Band 5GHz
    unsigned int IDRreg2[]={0x30CF,0x00D0,0x00D0,0x10D1,0x20D2,0x30D3,0x00D4,0x00D4,0x00DC,0x00DC,0x10DD,0x20DE,0x30DF,0x00E0,0x00E0,0x10E1,0x20E2,0x30E3,0x00E4,0x00E5,0x10E6,0x20E7,0x30E8};
    unsigned int FDRreg2[]={0x0CCC,0x0000,0x3333,0x2666,0x1999,0x0CCC,0x0000,0x3333,0x0000,0x3333,0x2666,0x1999,0x0CCC,0x0000,0x3333,0x2666,0x1999,0x0CCC,0x0000,0x3333,0x2666,0x1999,0x0CCC};


    if(band==0)
    {
        RFFE_SetRegisterSPI(m_h_board, transceiver_num, rffe_spi_num_tab[5], 0x1824);
        RFFE_SetRegisterSPI(m_h_board, transceiver_num, rffe_spi_num_tab[3], IDRreg1[ch]);
        RFFE_SetRegisterSPI(m_h_board, transceiver_num, rffe_spi_num_tab[4], FDRreg1[ch]);
        if (mode==1)
        {
            rffe_JStx[3]=IDRreg1[ch];
            rffe_JStx[4]=FDRreg1[ch];
        }
        else 
        {
            rffe_JSrx[3]=IDRreg1[ch];
            rffe_JSrx[4]=FDRreg1[ch];
        }
    }
    else
    {
        RFFE_SetRegisterSPI(m_h_board, transceiver_num, rffe_spi_num_tab[3], IDRreg2[ch]);
        RFFE_SetRegisterSPI(m_h_board, transceiver_num, rffe_spi_num_tab[4], FDRreg2[ch]);
        
        if (mode==1)
        {
            rffe_JStx[3]=IDRreg2[ch];
            rffe_JStx[4]=FDRreg2[ch];
        }
        else 
        {
            rffe_JSrx[3]=IDRreg2[ch];
            rffe_JSrx[4]=FDRreg2[ch];
        }
        if (ch<8)
            RFFE_SetRegisterSPI(m_h_board, transceiver_num, rffe_spi_num_tab[5], 0x1825);
        else
            RFFE_SetRegisterSPI(m_h_board, transceiver_num, rffe_spi_num_tab[5], 0x1865);
    }
    
    #ifdef DEBUG
        if(band==0) {
        	printf("-->Set channel %d, freq. %d, IDReg %d, FDReg %d, band %d\n", ch, chfreq1[ch], IDRreg1[ch], FDRreg1[ch], band);
        } else {
            printf("-->Set channel %d, freq. %d, IDReg %d, FDReg %d, band %d\n", ch, chfreq2[ch], IDRreg2[ch], FDRreg2[ch], band);
        }
    #endif
}

VHS_PLAYBACK_STATE getPbState(HANDLE h){
    
    
    VHS_PLAYBACK_STATE state;
    RETURN_STATUS status;
    status = VHSMEM_GetPlaybackState(h, &state);
    
    switch (state){
        case PLAYBACK_IDLE:
            printf("PLAYBACK state is PLAYBACK_IDLE!\n");
            break;
        case PLAYBACK_ARMED:
            printf("PLAYBACK state is PLAYBACK_ARMED!\n");
            break;
        case PLAYBACK_WAITING:
            printf("PLAYBACK state is PLAYBACK_WAITING!\n");
            break;
        case PLAYBACK_READY:
            printf("PLAYBACK state is PLAYBACK_READY!\n");
            break;
        case PLAYBACK_PLAYING:
            printf("PLAYBACK state is PLAYBACK_PLAYING!\n");
            break;
        case PLAYBACK_PRELOAD:
            printf("PLAYBACK state is PLAYBACK_PRELOAD!\n");
            break; 
        default: printf("Playback State not detected\n");    
            
    } 
    return state;
}
VHS_RECORD_STATE getRecState(HANDLE h){
    
    
    VHS_RECORD_STATE state;
    RETURN_STATUS status;
    status = VHSMEM_GetRecordState(h, &state);
    
    switch (state){
        case RECORD_IDLE:
            printf("RECORD state is RECORD_IDLE!\n");
            break;
        case RECORD_ARMED:
            printf("RECORD state is RECORD_ARMED!\n");
            break;
        case RECORD_WAITING:
            printf("RECORD state is RECORD_WAITING!\n");
            break;
        case RECORD_STORING:
            printf("RECORD state is RECORD_STORING!\n");
            break;
        case RECORD_READY:
            printf("RECORD state is RECORD_READY!\n");
            break;
        case RECORD_PRE_READY:
            printf("RECORD state is RECORD_READY!\n");
            break;
        default: printf("Record State not detected!\n");

    }
    
    return state;
}
    