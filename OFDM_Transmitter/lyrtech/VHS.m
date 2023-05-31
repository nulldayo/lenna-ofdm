%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%  MexFile HF
%   function output = VHS(task,varargin)
%
%   This function communicate with the Quad Dual Band RF Transceiver Board,
%   see below for the different tasks. 
%   For additional help see also:
%   - VHS.cpp
%   - VHS-ADC/DAC Low-level Host API
%   - VHS-ADC/DAC High-level Host API
%   - RFFE Host API
%
%
% INPUTS
%   tasks:   
%   1. [DAC-Slot-ID] = VHS('Query_DAC_Board');  
%      [ADC-Slot-ID] = VHS('Query_ADC_Board');
%       This function searchs for a DAC/ADC-Board, gets the board handle
%       and gives back the Slot-ID.
%       (uses VHS-ADC/DAC Low-level Host API: HANDLE Vhsadac16_getHandle  
%       (VHSADAC16_BOARD_TYPE board_type, UCHAR board_num))
%       ATTENTION! Has to be used first!
%
%   2. [] = VHS('Connect_DAC_Board',DAC-Slot-ID);
%      [] = VHS('Connect_ADC_Board',ADC-Slot-ID);
%       This Function sets the Board ID
%       (uses no Host API)
%
%   3. [] = VHS('Load_DAC_Bitfile','BITFILE.bit');
%      [] = VHS('Load_ADC_Bitfile','BITFILE.bit');
%       if the board is connected, you can use this function to write a 
%       Bitfile "BITFILE.bit" to the FPGA. 
%       Attention: This function will reset the FPGA!
%       (uses VHS-ADC/DAC Low-level Host API: int LONG Vhsadac16_FpgaReset
%        (HANDLE h, UCHAR ?pBitstream, ULONG length))
%
%   4. [] = VHS('Disconnect_DAC_Board');
%      [] = VHS('Disconnect_ADC_Board');
%       This function releases all board handles
%       (uses VHS-ADC/DAC High-level Host API: void Vhsadac16_Close (HANDLE h))
%
%   5. [] = VHS('Init_RFFE');
%       THis function initializes the RFFE-Tool. The Bitfile must contain a
%       RFFE-Block! The External Control will be deactivated
%       (uses RFFE Host API: RETURN_STATUS RFFE_Init (HANDLE h))
%
%   6. [] = VHS('Transceiver_Mode', Transmode);
%       This function sets the Transceiver mode. '0' sets the Transceiver 
%       to Tx-Mode and '1' to Rx-Mode
%       (uses no Host API)
%
%   7. [] = VHS('Set_PABS_DSW_OSCS',pabs, dsw,oscs);
%       This functions sets the PABS-State, DSW-State and the OSCS-State of
%       the transceivers.
%       The PABS-state '0' stands for the 2.4 GHz band and '1' for the 5
%       GHz Band. The DSW-state '0' stands for antenna B and '1' stands for
%       antenna A. The OSCS-state '0' stands for an external oscillator and
%       '1' for an internal oscillator.
%       (uses RFFE Host API:RETURN_STATUS RFFE_SetPabsDswOscs (HANDLE h, 
%        PABS_STATE pabs, DSW_STATE dsw,OSCS_STATE oscs))
%
%   8. [] = VHS('Init_SPI_Register', Transceiver-Number);
%       This function sets the transceiver registers through the serial
%       parallel interface. If no transceiver-number is given, all
%       transceiver registers will be set. The Parallel Gain will be set to '0' by
%       default.
%       (uses RFFE Host API: RETURN_STATUS RFFE_SetRegisterSPI (HANDLE h, 
%        TRANSCEIVER_NUMBER transceiver, REGISTER_-SPI_NUMBER spi, unsigned
%        int data), RETURN_STATUS RFFE_SetParallelGain (HANDLE h, 
%        TRANSCEIVER_NUMBER transceiver, unsigned int data))
%
%   9. [] = VHS('Set_Gain', Gain, Transceiver-Number);
%       This function sets the transceiver's gain. If no transceiver-number
%       is given, all transceivers are programmed with the given gain.
%       Attention: Gain must be entered in decimal-form
%       (uses RFFE Host API: RETURN_STATUS RFFE_SetPaen (HANDLE h, 
%        TRANSCEIVER_NUMBER transceiver, PAEN_STATE state))
%
%  10. [] = VHS('Set_Power_Amplifier', Transceiver-Number);
%       This function activates or deactivates the Power-Amplifier of a 
%       transceiver if the transceiver is in TX-Mode. If no Transceiver 
%       number is given, all Transceivers are programmed.
%       (uses RFFE Host API: RETURN_STATUS RFFE_SetPaen (HANDLE h, 
%        TRANSCEIVER_NUMBER transceiver, PAEN_STATE state))
%
%  11. [] = VHS('Quit');
%       This function releases and closes all handles and resources of the 
%       RFFE-Tool and places the unit on standby.
%       (uses RFFE Host API: RETURN_STATUS RFFE_SetMode (HANDLE h, 
%        TRANSCEIVER_NUMBER transceiver, TRANSCEIVER_-MODE data))
%
%  12. [] = VHS('Set_Channel', Channel, Transceiver-Number);
%       This function sets the channel of the transceivers. If no
%       transceiver-Number is given, all transceivers will be programmed.
%       (uses no Host API)
%   
%       
% OUTPUTS
%   look at tasks
%
% NEEDED FUNCTIONS
%               mexfile VHS.mex32
%--------------------------------------------------------------------------
% created:      04.07.2012 (T.Reinartz) 
%..........................................................................
% CHANGELOG
%
%   04.07.2012
%       - created file
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%