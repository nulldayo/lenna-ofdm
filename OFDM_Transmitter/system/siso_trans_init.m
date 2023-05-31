%
% Initialize SISO transmitter
% Parameters to modify, depending on working group and Lyrtech hardware
%  - rf_channel (default 11)
%  - noadc (default=0, old Lyrtech sender without ADC board needs 1)
%parameter();

% -------------------------------------------------------------
%
% 1. Define files to be uploaded
%              VHS only     -> nosmq=1 (naturally, no DSP processing
%                                       available)
% -------------------------------------------------------------
nosmq=1;
noadc=0; 
rf_channel = 11;
def_oversampling = 5; % 104 MHz / 5 = 20.8 MHz signal clock (valid range: 1..128)
vhsdacfilename='vhs_fpga_tx_10k_nosmq_nodsp_cw.bit';
vhsadcfilename=siso_trans_search_file('vhs_fpga_rx_sdram_8k_13xcic_cw.bit');


vhsdacfilename=siso_trans_search_file(vhsdacfilename);
vhsadcfilename=siso_trans_search_file(vhsadcfilename);

% -------------------------------------------------------------
%
% 2. Program DAC Board
%
% -------------------------------------------------------------
% Query VHS Boards
disp('----------------------------------');
disp('Query Boards');
[DAC_Slot_ID] = VHS('Query_DAC_Board');

% Initialize VHS-DAC
disp('----------------------------------');
disp('Initialize VHS-DAC Board')
VHS('Connect_DAC_Board', DAC_Slot_ID);
VHS('Load_DAC_Bitfile', vhsdacfilename);

if noadc == 0
  % -------------------------------------------------------------
  %
  % 3. Program ADC Board (ADC model used for RFFE init, nothing else)
  %
  % -------------------------------------------------------------
  % Query VHS Boards
  disp('----------------------------------');
  disp('Query Boards');
  [ADC_Slot_ID] = VHS('Query_ADC_Board');

  % Initialize VHS-ADC
  disp('----------------------------------');
  disp('Initialize VHS-ADC Board')
  VHS('Connect_ADC_Board', ADC_Slot_ID);
  VHS('Load_ADC_Bitfile', vhsadcfilename);
end

% -------------------------------------------------------------
%
% 4. Set transmitter parameter
%
% -------------------------------------------------------------
disp('----------------------------------');
VHS('Init_RFFE');
VHS('Transceiver_Mode', 1);
VHS('Set_Gain', 90 );
VHS('Set_PABS_DSW_OSCS', 0, 0, 1);
VHS('Init_SPI_Register');
VHS('Set_Gain', 127 );
VHS('Set_Power_Amplifier', 1,1);
%VHS('Set_Power_Amplifier', 1);
VHS('Set_Channel', rf_channel );


% Run DAC Board, with onboard clock (0) (104 MHz)
% Other clocks are:
%   (0) .. Onboard Clock (104 MHz)
%   (1) .. Onboard Clock devided by 2 (52 MHz)
%   (2) .. External Front Clock
disp('----------------------------------');
VHS('RUN_DAC', 0);
% Open RTDEx
VHS('Open_RTDEx_DAC');

% get VHS model type and revision, print gathered data (verbose=0,silent=1)
[model,rev,param,rtdexwords]=siso_trans_model_identify(0);
if( model ~= 3 )
  fprintf('ERROR: expected VHS model number 3, got %u\n',model);
end
  
% Zero - initialize frame
init_frame_bits = typecast(uint32(zeros(rtdexwords,1)), 'uint8');

% Default Oversampling Factor
VHS('Write_User_Reg_DAC', 5, def_oversampling-1 );
% Reset
VHS('Write_User_Reg_DAC', 1, 1 );
pause(0.1);
VHS('Write_User_Reg_DAC', 1, 0 );
pause(0.1);

% Reset Phase: write frame and capture result from VHS board
VHS('Write_RTDEx_DAC', uint8(init_frame_bits(:)));
pause(0.3);
VHS('Write_User_Reg_DAC', 1, 1 );
pause(0.2);
d=1;
f=5;
while( d ~= 0 )
 VHS('Write_User_Reg_DAC', 1, 0 );
 pause(0.3);
 d = VHS('Read_User_Reg_DAC', 4 );
 e = VHS('Read_User_Reg_DAC', 5 );
 d = d+e;
 f=f-1;
 if( f==0 ) d=0; end;
 VHS('Write_User_Reg_DAC', 1, 1 );
 pause(0.2);
end

VHS('Write_User_Reg_DAC', 1, 1 );
pause(0.1);
VHS('Write_User_Reg_DAC', 1, 0 );
% End Board Reset and Initialization

