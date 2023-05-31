function siso_trans_set_tx_gain(gain)
% set transmit power by adjusting RF frontend amplifier
%
if( (gain < 1) || (gain > 127) )
  fprintf('siso_trans_set_tx_gain: gain value %d invalid, range is 1...127\n',gain);
else
  VHS('Set_Gain', int32(gain) );
end
