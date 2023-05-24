load('ofdm_frame_received_qpsk.mat')

din_bin8=de2bi(din).'; 

din_bin32=reshape(din_bin8,32,[]).';

din32=bi2de(uint32(din_bin32));

save('ofdm_frame_received_64qam_uint32_14August')


%% testen

old=load('ofdm_frame_received_qpsk_uint32_old.mat');

new=load('ofdm_frame_received_qpsk_uint32_new.mat');

old.din32==new.din32