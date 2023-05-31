function sel = state_machine_tx(RST, FIFO_RCH_dcount, N_FB_RX_frame, RCH_FIFO_dcount, N_FB_TX_frame)

  persistent state, state = xl_state(0, {xlUnsigned,2,0});
  
  % Counter laeuft unabhaengig
  if RST
    % Nach einen Reset sind die FIFOs leer und k�nnen wieder beschrieben
    % werden
    state = 1;
  elseif FIFO_RCH_dcount == N_FB_RX_frame
    % Wenn der FIFO von der RTDEx-Schnittstelle gef�llt wurde, dann
    % den FIFO �ber den RapidChannel (RCH) auslesen
    state = 2;
  elseif RCH_FIFO_dcount == N_FB_TX_frame
    % Wenn der FIFO vom RapidChannel (RCH) gef�llt wurde,
    % dann den FIFO �ber den DAC auslesen
    state = 3;
  elseif  state == 3 && RCH_FIFO_dcount == 0
    % Wenn beide FIFOs leer sind und der letzte Zustand das auslesen �ber
    % den DAC war, dann kann wider eine neuer Frame geladen werden
    state = 1;
  else
    state = state;
  end
  
 sel = state;
