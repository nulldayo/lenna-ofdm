%Transmit script for OFDM

% initialize hardware
startup();


siso_trans_init(); % rf_channel currently 13
%% set FPGA oversampling clock divider -> 104 MHz / divider, hence 5 equals 20.8 MHz output clock
siso_trans_set_fpga_oversample(5); % default: 5
create_OFDM_frame; % get transmit frames

%[model,rev,param,rtdexwords]=siso_trans_model_identify(1);

%--------------------------------------------------------------------------
%% main: generate frame
Nframes = numel(frames);
send_frames = cell(1,Nframes);

% OFDM-Frame  durch create_OFDM_frame.m definiert
for i=1:Nframes
    send_frames{i} = typecast(siso_trans_complex_to_byte(...
                  frames{i}(1:rtdexwords),32700,32768),'uint32');
end

%--------------------------------------------------------------------------
% main loop
%--------------------------------------------------------------------------
fprintf('starting main demo loop\n');
siso_trans_set_fpga_interpolate( 'cic' ); % FPGA send filter mode 'zero','nearest','cic'
siso_trans_set_tx_gain( 127 ); % adjust transmit power (1...127)
VHS('Set_TX_Low_Pass',12000);

t=0;       % counter
tic        % display elapsed time with tic/toc
while t<2000000
 for i=1:Nframes
      siso_trans_write_frame( send_frames{i} ); % ex2: write manual frame
 end

 %pause(0.1);
 t=t+1;
 if( mod(t,1000) == 0 ) % print "." after 1000 frames
   fprintf('.');
   if( mod(t,20000) == 0 )
   fprintf('\n');
   end
 end
end
fprintf('\n');
toc

% close hardware
siso_trans_close();
