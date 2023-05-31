startup();
load('OFDM_Parameter.mat')

% call system/board_init.m see top of said script for current parameters
board_init();

% check type of model (non silent)
% decide upon the number of multiplexed RX streams
[model,rev,param,rtdexwords]=siso_recv_model_identify(0);
rxstreams = 1;
if model > 0
 if param == 4
     rxstreams = 4;
 else
  if param == 2
      rxstreams = 2;
  end
 end
end

training_freq(training_freq==0)=1e-12;
%
% Main: read 100 frames
%
rec_frame = cell(1,100);

loop=1;
while( loop<=100 )

dinmux = uint8( VHS('Read_RTDEx_ADC', 131*80*4*rxstreams ));
dinmux2= typecast(dinmux,'uint32');
din = typecast( dinmux2( 1 : rxstreams : end ),'uint8'); % demux first stream
din2 = typecast( dinmux2( 2 : rxstreams : end ),'uint8'); % demux 2nd stream...
din3 = typecast( dinmux2( 3 : rxstreams : end ),'uint8'); % demux 2nd stream...
din4 = typecast( dinmux2( 4 : rxstreams : end ),'uint8'); % demux 2nd stream...

rec_frame{loop}=siso_recv_byte_to_complex(din,1);

txt = ['read frame ', num2str(loop)];
disp(txt);
loop = loop+1;
end

save('rec_frame.mat','rec_frame');

%% Debug Routines/Output
if 0
    disp('----------------------------------');
    while 1
        VHS('Write_User_Reg_ADC', 2, 1 );
        VHS('Write_User_Reg_ADC', 2, 0 );
        VHS('Write_User_Reg_ADC', 2, 8 ); % enable CFO, delay 0
        print_out_register;
        din = uint8( VHS('Read_RTDEx_ADC', 131*80*4 ));
        pause(0.5)
        %    a = VHS('Read_User_Reg_ADC', 3)
        %    b = VHS('Read_User_Reg_ADC', 5)
    end
        % VHS model stress test: read 100 times 16 frames per request
        % if the sample counting in the FIFO output would not work
        % correctly, this sniplet would likely trigger it...
        tic
        for i=1:100
        din = uint8( VHS('Read_RTDEx_ADC', 131*80*4*16 ));
        end
        toc
        print_out_register;
end
 

