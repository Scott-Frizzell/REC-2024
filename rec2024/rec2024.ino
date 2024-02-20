class Task {
  private int state;
  private int states;
  void Task(_states) {
    this.state = 0
    this.states = _states
  }
  int getState() {
    return this.state;
  }
  void exec() {
    
  }
}

void setup() {

}

void loop() {
  // TASK 1 | GENERAL RIDE FLOW
  switch (state1) {
    case 0:
      // STATE 0 | INIT
      break;
    case 1:
      // STATE 1 | STOPPED
      break;
    case 2:
      // STATE 2 | RUNNING
      break;
    case 3:
      // STATE 3 | E-STOPPED
      break;
  }
  
  // TASK 2 | NRF SEND
  switch (state2) {
    case 0: // STATE 0 | INIT
      break;
    case 1: // STATE 1 | SEND
      break;
  }

  // TASK 3 | NRF RECEIVE
  switch (state3) {
    case 0: // STATE 0 | INIT
      break;
    case 1: // STATE 1 | RECEIVE
      break;
  }

  // TASK 4 | BRAKE RUN 1
  switch (state4) {
    case 0: // STATE 0 | INIT
      break;
    case 1: // STATE 1 | CLOSED
      break;
    case 2: // STATE 2 | OPEN
      break;
  }

  // TASK 5 | BRAKE RUN 2
  switch (state5) {
    case 0: // STATE 0 | INIT
      break;
    case 1: // STATE 1 | CLOSED
      break;
    case 2: // STATE 2 | OPEN
      break;
  }

  // TASK 6 | BRAKE RUN 3
  switch (state6) {
    case 0: // STATE 0 | INIT
      break;
    case 1: // STATE 1 | CLOSED
      break;
    case 2: // STATE 2 | OPEN
      break;
  }

  // TASK 7 | SERIAL WRITE
  switch (state7) {
    case 0: // STATE 0 | INIT
      break;
    case 1: // STATE 1 | WRITE
      break;
  }

  // TASK 8 | THEMING
  switch (state8) {
    case 0: // STATE 0 | INIT
      break;
    case 1: // STATE 1 | STOPPED
      break;
    case 2: // STATE 2 | RUNNING
      break;
    case 3: // STATE 2 | ESTOPPED
      break;
  }

  // TASK 9 | OPERATOR INPUT
  switch (state9) {
    case 0: // STATE 0 | INIT
      break;
    case 1: // STATE 1 | REGULAR OPERATION
      break;
    case 2: // STATE 2 | ESTOPPED
      break;
  }

  
}

void estop_interrupt() {
  
}
