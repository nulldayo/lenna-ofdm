parameter();

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
vhsfpga=siso_recv_search_file('vhs_fpga_rx_to_sdram_time_share_agc_cw.bit');

initial_gain=18;        % AGC default gain
threshold_plus1=350;    % AGC lower bound
threshold_minus1=450;   % AGC upper bound
threshold_preamble=800; % preamble detection threshold

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
VHS('Set_PABS_DSW_OSCS', 0, 0, 1); % OSCS = 0 external, OSCS = 1 internal
VHS('Init_SPI_Register');
VHS('Set_Gain', initial_gain );
VHS('Set_Channel', 13 );


% Open RTDEx
VHS('Open_RTDEx_ADC');

% Reset FIFO's ADC-Board
pause(0.1);

VHS('Write_User_Reg_ADC', 1, threshold_preamble ); % preamble threshold
% AGC Parameters (if 1, else default gain)
VHS('Set_Gain', initial_gain );
VHS('Write_User_Reg_ADC', 8, initial_gain ); % AGC current gain
VHS('Write_User_Reg_ADC', 9, threshold_plus1 ); % AGC lower bound
VHS('Write_User_Reg_ADC',10, threshold_minus1 ); % AGC upper bound
VHS('Set_External', 1, 1, 1, 1 ); % Ext, Gain, Tran, Freq


% Run DAC Board, with onboard clock (0) (104 MHz)
% Other clocks are:
%   (0) .. Onboard Clock (104 MHz)
%   (1) .. Onboard Clock devided by 2 (52 MHz)
%   (2) .. External Front Clock
disp('----------------------------------');
VHS('RUN_ADC', 0);
pause(0.25);
VHS('Write_User_Reg_ADC', 2, 0 ); % enable VHS model (clear reset)

training_freq(training_freq==0)=1e-12;
%
% Main: read 10 frames
%
t=0;
while( t<10 )

din = uint8( VHS('Read_RTDEx_ADC', 131*80*4 ));

rec_frame=siso_recv_byte_to_complex(din,1);

s_guard = (reshape(rec_frame, Nfft + Nguard, []));
% 
% Estimate CFO, FFO
[gamma_est_cp, dF_est_cp] = FFO_CFO_estimator( s_guard(Nguard+1:end,:), ...
                            s_guard(1:Nguard,:), Nfft, Nguard, Nskip, Fs / Nfft );
% Reduce CFO
s = rec_frame .* exp(-1j * (2*pi/Nfft * gamma_est_cp * (1:length(rec_frame)) ) ).';
% 
s_guard = (reshape(s, Nfft + Nguard, []));
s_noguard = s_guard( (Nguard+1):end , :);
S = fft( s_noguard, Nfft );
% 
% Channel estimation
H = S(:,1) ./ training_freq.';
% 
% Equalization
for idx = 1:Nofdm
    E(:,idx) = S(:,1+idx) ./ H;
end
% 
% Fine phase offset estimation
phase_cfo = zeros( length(Nused_pilot_idx), Nofdm );
for idx = 1:Nofdm
     phase_cfo(:,idx) = ...
        E(Nused_pilot_idx, idx) ./ training_freq(Nused_pilot_idx).';
end
% 
pfit = zeros( length(Nused_pilot_idx), 1 );
    for idx = 1:length(Nused_pilot_idx)
      tmp = polyfit( 1:length(phase_cfo(idx,:)), phase(phase_cfo(idx,:)), 1);
      pfit(idx) = tmp(1);
    end
   pfit_mean = mean(pfit);
 
D = zeros( size(E) );
for idx = Nused_data_idx %1:Nofdm
      phase_mean = exp(1j*pfit_mean*(idx-1));
      D(:,idx) = E(:,idx) .* conj(phase_mean);
end
%D(Nused_data_idx)=E(Nused_data_idx);


figure(2);
plot(D, '+'); axis([-2 2 -2 2]); grid on; pause(0.2);

t=t+1;
end

% example: convert to 32 bit packed (again), assumes normalized signal with
% amplitude 1
bytestream=siso_trans_complex_to_byte(D(:),0);


% Debug Output
if 0
    disp('----------------------------------');
    while 1
        print_out_register;
        pause(0.5)
        %    a = VHS('Read_User_Reg_ADC', 3)
        %    b = VHS('Read_User_Reg_ADC', 5)
    end

end
 