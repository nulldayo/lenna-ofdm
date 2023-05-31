function [model,rev,param,rtdexwords]=siso_recv_model_identify(silent);
%% read VHS ADC custom register 7 and report model parameters if applicable
%
% CR7 layout on VHS ADC board (MSB to LSB listed here)
%  4 Bit MAGIC 0110 (==6), identifier for active models 
%                          (uninitialized would be either 0xffffffff or 0x0)
%  6 Bit parameters (model specific, unspecified here)
%  3 Bit model revision (0,....)
%  4 Bit Model ID
%        0 == unspecified
%        1 == SISO OFDM Receiver
%        2 == MIMO OFDM Receiver
%        3 == SISO Praktikum Receiver
%        4 == SDRAM sampler
% 15 Bit RTDEX read length per frame
%
% rev: 20.03.2015, Henryk Richter - added 4
%
modelnames={'unknown','SISO OFDM','MIMO OFDM','SISO Praktikum','104 MHz ADC Sampler'};

reg = uint32(VHS('Read_User_Reg_ADC', 7 ));
magic=bitshift(reg,-28);
param=bitand(bitshift(reg,-22),63);
rev=bitand(bitshift(reg,-19),7);
model=bitand(bitshift(reg,-15),15);
rtdexwords=bitand(reg,32767);

if( magic == 6 )
 mdl = model;
 if( model >= size( modelnames, 2 ) )
   mdl = 0;
 end
 if( silent == 0 )
   fprintf('ADC model %s rev %u param %u loaded, RTDEX read size %d\n',char(modelnames(mdl+1)),rev,param,rtdexwords);
 end
else
 if( silent == 0 )
  fprintf('no MAGIC string in VHS ADC register 7, cannot identify model\n');
 end
 model=0;
end

end