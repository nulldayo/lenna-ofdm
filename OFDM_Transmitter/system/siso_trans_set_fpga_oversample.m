function siso_trans_set_fpga_oversample(oversampling_factor)
% set FPGA oversampling factor
% 
% values range from 1 to 128 (i.e. signal baseband frequency 104 MHz...0.812 MHz)
%
osf=fix(oversampling_factor);

if( osf < 1 )
 fprintf('error: oversampling factor must be >=1\n');
else
 if( osf > 128 ) 
  fprintf('error: oversampling factor must be <=128\n');
 else

  fprintf('setting FPGA oversampling factor to %d\n',osf);
  VHS('Write_User_Reg_DAC', 5, osf-1 );

 end
end

end