function sel = state_machine_fifo(RST, FIFO_dcount, N_frame_size, Preamble_Det)

  persistent state, state = xl_state(0, {xlUnsigned,3,0});
  
  % Es existieren 4 Zustaende:
  % RST-State   -> state = 0
  % IDLE-State  -> state = 1
  % WRITE-State -> state = 2
  % READ-State  -> state = 3
  if RST
    % Nach einen Reset ist das FIFOs leer und kann wieder beschrieben
    % werden
    state = 0;
  elseif FIFO_dcount == 0 && Preamble_Det && state == 1
    % Wenn eine Preamble detektiert wurde und man im IDLE-State ist
    % und das FIFO leer ist (sollte hierbei immer der Fall sein)
    % wird der empfangene/detektierte Frame ins FIFO geschrieben
    % d.h. wechsle zum WRITE-State
    state = 2;
  elseif FIFO_dcount < N_frame_size - 1 && state == 2
    % Solange noch nicht alle Framedaten ins FIFO geschreiben wurden
    % bleibt man im WRITE-Zustand
    state = 2;
  elseif FIFO_dcount > N_frame_size && state == 2
    % spurious issue: more than N_frame_size samples in buffer ->
    % reset FIFO and go to state 1 = idle
    state = 4;
  elseif FIFO_dcount == N_frame_size - 1 && state == 2
    % Wenn ein kompletter Frame im FIFO vorliegt, kann in den READ-State
    % umgeschaltet werden
    state = 3;
  elseif FIFO_dcount ~=0 && state == 3
    % Solange noch nicht alle Framedaten aus dem FIFO ueber den RC
    % uebertragen wurden, bleibt man im READ-State
    state = 3;
  elseif FIFO_dcount > 0 && state == 4
    % FIFO reset complete ? -> no = wait in state 4, else move to state 1
    state = 4;
  else
    % Wenn alle obigen Bedingungen nicht erfuellt sind, uebergeht man in
    % den IDLE-State
    state = 1;
  end
  
 sel = state;
