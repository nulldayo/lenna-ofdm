function frame_no_RFO = remove_RFO(frame_equalized, training_freq, H, parameter, mode)
%REMOVE_RFO This function removes the residual frequency offset (RFO)
%
% Note: Remove unused carriers -> Only data carriers should remain
%
% Input:    frame_equalized -> 64x130 complex double
%           training_freq -> training data in frequency domain
%           H -> channel coefficients 64x1
%           parameter -> struct with predefined OFDM parameter
%           mode -> 'average', 'next' or 'none'
% Output:   frame_no_RFO -> 48x130 complex double

aRFO=frame_equalized(parameter.Nused_pilot_idx,:)./training_freq(parameter.Nused_pilot_idx).';

switch lower(mode)

    case 'weighted_average'
        eRFO=1/(2*pi)*angle(sum(abs(H(parameter.Nused_pilot_idx)).^2.*aRFO/sum(abs(H(parameter.Nused_pilot_idx)).^2,1),1));
    case 'average'
        eRFO=1/(2*pi)*angle(1/(length(parameter.Nused_pilot_idx))*sum(aRFO,1));
    case 'none'
        eRFO=0;
    otherwise
        warning('Unknown method, utilizing none instead')
        eRFO=0;
end
    frame_no_RFO=frame_equalized(parameter.Nused_data_idx,:).*repmat(exp(-1j*2*pi*eRFO),length(parameter.Nused_data_idx),1);
end

