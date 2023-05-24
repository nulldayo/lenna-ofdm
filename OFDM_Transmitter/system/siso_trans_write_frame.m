function res=siso_trans_write_frame( input_frame )
%% write single frame via RTDEX to VHS DAC board
%

a=3; % wait ready signal in register 0
while a ~= 1
  a = VHS('Read_User_Reg_DAC', 0 ); % sync wait loop option (b) observe status
end
 
 VHS('Write_RTDEx_DAC', typecast(input_frame(:),'uint8'));

end