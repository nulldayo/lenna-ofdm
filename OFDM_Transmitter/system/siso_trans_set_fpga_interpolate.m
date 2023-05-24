function siso_trans_set_fpga_interpolate(interp_mode)
% set FPGA interpolation method
% 'zero'   = insert zeros between samples
% 'nearest'= nearest neighbor / sample&hold (sinc in freq. domain)
% 'cic'    = 3 stage CIC filter (sinc^3 in freq. domain)
%
mode = 2;
if( strcmp(interp_mode,'zero')==1 )
  mode = 0;
end
if( strcmp(interp_mode,'nearest')==1 )
  mode = 1;
end
if( strcmp(interp_mode,'cic')==1 )
  mode = 2;
end

fprintf('setting FPGA interpolation to mode %d\n',mode);
VHS('Write_User_Reg_DAC', 2, mode );

end