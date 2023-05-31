rf_channel=11;
initial_gain=16;        % AGC default gain
threshold_plus1=500;    % AGC lower bound
threshold_minus1=600;   % AGC upper bound
threshold_preamble=400; % preamble detection threshold
CFOEnable=0; % (0) = no CFO on board, (1) = CFO on VHS board

% -------------------------------------------------------------
%
% 2. Program DAC Board
%
% -------------------------------------------------------------

% Query VHS Boards
disp('----------------------------------');
disp('Query Boards');
[ADC_Slot_ID] = VHS('Query_ADC_Board');

%vhsfpga=siso_recv_search_file('vhs_fpga_rx_to_sdram_time_share_cw.bit');
%vhsfpga=siso_recv_search_file('vhs_fpga_rx_to_sdram_time_share_agc_cw.bit');
%vhsfpga=siso_recv_search_file('vhs_fpga_rx_to_sdram_time_share_agc_cfo_cw.bit');
vhsfpga=siso_recv_search_file('vhs_fpga_rx_to_sdram_time_share_agc_cfo_4chan_cw.bit');

% Initialize VHS-ADC
disp('----------------------------------');
disp('Initialize VHS-ADC Board')
VHS('Connect_ADC_Board', ADC_Slot_ID);
%VHS('Load_ADC_Bitfile', '../models/fpga/vhs_fpga_rx_to_sdram_no_fifo_agc_with_stp_cw.bit');
%VHS('Load_ADC_Bitfile', '../models/fpga/vhs_fpga_rx_to_sdram_time_share_cw.bit');
%VHS('Load_ADC_Bitfile', '../models/fpga/vhs_fpga_rx_to_sdram_time_share_double_buffer_cw.bit');
VHS('Load_ADC_Bitfile',vhsfpga);
VHS('Write_User_Reg_ADC', 2, 1 );

pause(0.25);

% Set HF parameter
disp('----------------------------------');
VHS('Init_RFFE');
VHS('Transceiver_Mode', 0);
%VHS('Set_PABS_DSW_OSCS', 0, 0, 0); % ext clock, no CFO
VHS('Set_PABS_DSW_OSCS', 0, 0, 1); % OSCS = 0 external, OSCS = 1 internal
VHS('Init_SPI_Register');
VHS('Set_Gain', 0 );
VHS('Set_Gain',initial_gain,1 );
VHS('Set_Channel', rf_channel);
% VHS('Set_RX_High_Pass', 1 );
% VHS('Set_RX_Low_Pass',7500); %7500,9500,9975,10450,12600,14000,18000,19800
%VHS('Set_RX_High_Pass',0,1); % disable high-pass entirely for first transceiver (100 Hz)
VHS('Set_RX_High_Pass',0); % lower high-pass for all transceivers (30 kHz)
VHS('Set_RX_Low_Pass',10450)

% Open RTDEx
VHS('Open_RTDEx_ADC');

% Reset FIFO's ADC-Board
pause(0.1);

VHS('Write_User_Reg_ADC', 1, threshold_preamble ); % preamble threshold
% AGC Parameters (if 1, else default gain)
VHS('Set_Gain', initial_gain );
VHS('Set_External', 1, 1, 1, 1 ); % Ext, Gain, Tran, Freq
VHS('Write_User_Reg_ADC', 8, initial_gain ); % AGC current gain
VHS('Write_User_Reg_ADC', 9, threshold_plus1 ); % AGC lower bound
VHS('Write_User_Reg_ADC',10, threshold_minus1 ); % AGC upper bound

% Run DAC Board, with onboard clock (0) (104 MHz)
% Other clocks are:
%   (0) .. Onboard Clock (104 MHz)
%   (1) .. Onboard Clock devided by 2 (52 MHz)
%   (2) .. External Front Clock
disp('----------------------------------');
VHS('RUN_ADC', 0);
pause(0.1);
VHS('Write_User_Reg_ADC', 2, (CFOEnable*8) ); % enable VHS model (clear reset) and
                                  % set preferred additonal FIFO write delay (delay 0,1,2,3 -> value 0,2,4,6)
                                  % set CFO enable (8) (... e.g. delay1 + active CFO = 2+8=10 ...)
pause(1.5); % wait for AGC to settle...                                
