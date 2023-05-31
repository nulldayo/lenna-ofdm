function sel = state_machine_tx(RST, FIFO_RCH_dcount, N_FB_RX_frame, F_BufferSwap)

  persistent state, state = xl_state(0, {xlUnsigned,2,0});
  
  % Counter laeuft unabhaengig
  if RST
    % Nach einen Reset sind die FIFOs leer und können wieder beschrieben
    % werden
    state = 1;
  elseif FIFO_RCH_dcount == N_FB_RX_frame
    % Wenn der FIFO von der RTDEx-Schnittstelle gefüllt wurde, dann
    % den FIFO über den RapidChannel (RCH) auslesen
    state = 2;
  elseif  state == 2 && FIFO_RCH_dcount == 0 % F_BufferSwap % 
    % Wenn FIFO vollgeschrieben und anderer FIFO leer (2. State Machine),
    % dann Signal dass ein neues Frame übernommen wurde und somit 
    % kann neues Frame in den anderen FIFO geschrieben werden
    state = 3;
  elseif state == 3 && F_BufferSwap
    % Frame abgeschickt und Buffer Swap Signal aus dem DAC Teil
    state = 1;
  else
    state = state;
  end
  
 sel = state;
