% ------------------------------------------------------------
% RFO Correction
% ------------------------------------------------------------
function [frame_no_RFO] = correctRFO(frame_equalized,res_cfo,method,do_averaging,Nused_data_idx,Nused_pilot_idx,Nofdm)

if strcmp(method,'simple')
    t  = Nused_pilot_idx' * ones(1,length(Nused_data_idx));
    t2 = ones(4,1) * Nused_data_idx;
    [foo,Next_Pilot]=min(abs(t2-t)); % which pilot for which subcarrier
    if do_averaging
        % average over 4 pilots for each OFDM Symbol separately
        cfmean=[mean(res_cfo);...
            mean(res_cfo);...
            mean(res_cfo);...
            mean(res_cfo)];
        tx=cfmean(Next_Pilot,:);
    else
        % no average, each pilot corrects neighbouring carriers
        tx=res_cfo(Next_Pilot,:);
    end
    
    txa=angle(tx);
    frame_no_RFO=frame_equalized(Nused_data_idx,:) .* exp(-1j*txa);
    
    %frame_no_RFO=frame_equalized(Nused_data_idx,:); % no pilot cfo
    
    %frame_no_RFO=frame_equalized(Nused_data_idx,:) .* conj(tx);
end

if strcmp(method,'polifit') 

    pfit = zeros( length(Nused_pilot_idx), 2 );
    for idx = 1:length(Nused_pilot_idx)
        tmp = polyfit( 1:length(res_cfo(idx,:)), phase(res_cfo(idx,:)), 1);
        pfit(idx,:) = tmp;
    end
    frame_no_RFO = zeros( size(frame_equalized) );
    
    if do_averaging
        t  = Nused_pilot_idx' * ones(1,length(Nused_data_idx));
        t2 = ones(4,1) * Nused_data_idx;
        [foo,Next_Pilot]=min(abs(t2-t)); % which pilot for which subcarrier
        
        phaserot_tmp = ones(length(Nused_pilot_idx),1)*(1:Nofdm);
        phaserot = repmat(pfit(:,2),1,Nofdm) + repmat(pfit(:,1),1,Nofdm).*phaserot_tmp;
        
        phasetab = zeros( length(Nused_data_idx), Nofdm );
        for idx = 1:length(Nused_data_idx)
            phasetab(idx,:) = phaserot(Next_Pilot(idx),:);
        end
        
        frame_no_RFO=frame_equalized(Nused_data_idx,:) .* conj(exp(1j.*phasetab));
       
    else
        pfit_mean = mean(pfit(:,1));
        for idx = 1:Nofdm
            phase_mean = exp(1j*pfit_mean*(idx-1));
            frame_no_RFO(:,idx) = frame_equalized(:,idx) .* conj(phase_mean);
        end
    end
end

end

