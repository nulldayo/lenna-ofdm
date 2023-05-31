function [model,rev,param,rtdexwords]=siso_trans_model_identify(silent);
%% read VHS DAC custom register 7 and report model parameters if applicable
%
% CR7 layout on VHS DAC board (MSB to LSB listed here)
%  4 Bit MAGIC 0110 (==6), identifier for active models 
%                          (uninitialized would be either 0xffffffff or 0x0)
%  6 Bit parameters (model specific, unspecified here)
%  3 Bit model revision (0,....)
%  4 Bit Model ID
%        0 == unspecified
%        1 == SISO OFDM Sender
%        2 == MIMO OFDM Sender
%        3 == SISO Praktikum Sender
% 15 Bit RTDEX write length per frame
modelnames={'unknown','SISO OFDM','MIMO OFDM','SISO Praktikum'};

reg = uint32(VHS('Read_User_Reg_DAC', 7 ));
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
   fprintf('model %s rev %u param %u loaded, RTDEX write size %d\n',char(modelnames(mdl+1)),rev,param,rtdexwords);
 end
else
 if( silent == 0 )
  fprintf('no MAGIC string in VHS DAC register 7, cannot identify model\n');
 end
 model=0;
end

end