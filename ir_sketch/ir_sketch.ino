//FIXME: If a beacon is detected it should override the
//IR sensors, and record a finding

int enter_gate_status = 0; // 0 is off, 1 is triggered
int exit_gate_status = 0; // 0 is off, 1 is triggered
int process_enter = 0;
int process_exit = 0;
int beacon_status = 0;
int enter_gate_timer = 0; // 10 = 1 second @ delay of 100
int exit_gate_timer = 0;
int a_trigger_count = 0;
int b_trigger_count = 0;

int state = 0;
int print_state = 1;

void setup() {
  Serial.begin(57600);

  pinMode(7, INPUT); //Channel A IR, we'll call it enter
  pinMode(6, INPUT); //Channel B IR, we'll call it exit
  pinMode(5, INPUT); //BCA Beacon Checker

  Serial.println("Starting up...");
}

void loop() {
  delay(100);

  if (state == 0) {
    //check which one gets triggered first
    read_ir_sensors();
    if (print_state) {
      Serial.println("Entering starting state...");
      print_state = 0;
    }
    if (process_enter == 1 && process_exit == 0) {
      state = 1;
      print_state = 1;
      return;
    }

    if (process_exit == 1 && process_enter == 0) {
      state = 2;
      print_state = 1;
      return;
    }

    if (process_enter == 1 && process_exit == 1) {
      enter_gate_status = 0;
      exit_gate_status = 0;
      process_enter = 0;
      process_exit = 0;
      state = 5;
      print_state = 1;
      return;
    }
  }

  if (state == 1) {
    //enter was triggered first, check for exit
    read_exit_sensor();
    if (print_state) {
      Serial.println("Entering state 1...");
      print_state = 0;
    }
    if (enter_gate_timer > 10) {
      //timer hit, reset
      enter_gate_timer = 0;
      process_enter = 0;
      state = 0;
      print_state = 1;
      return;
    }

    if (process_exit == 0) {
      enter_gate_timer += 1;
      return;
    }

    if (process_exit == 1) {
      enter_gate_timer = 0;
      print_state = 1;
      state = 3;
      return;
    }
  }

  if (state == 2) {
    //exit was triggered first, check for enter
    read_enter_sensor();
    if (print_state) {
      Serial.println("Entering state 2...");
      print_state = 0;
    }
    if (exit_gate_timer > 10) {
      //timer hit, reset
      exit_gate_timer = 0;
      process_exit = 0;
      state = 0;
      print_state = 1;
      return;
    }

    if (process_enter == 0) {
      exit_gate_timer += 1;
      return;
    }

    if (process_enter == 1) {
      exit_gate_timer = 0;
      state = 4;
      print_state = 1;
      return;
    }
  }

  if (state == 3) {
    if (print_state) {
      Serial.println("Forward");
      print_state = 0;
    }
    process_enter = 0;
    process_exit = 0;
    state = 0;
    print_state = 1;
    return;
  }

  if (state == 4) {
    if (print_state) {
      Serial.println("Backward");
      print_state = 0;
    }
    process_enter = 0;
    process_exit = 0;
    state = 0;
    print_state = 1;
    return;
  }

  if (state == 5) {
    if (print_state) {
      Serial.println("Triggered at same time");
      print_state = 0;
    }
    state = 0;
    print_state = 1;
    return;
  }
}

void read_ir_sensors() {
  read_enter_sensor();
  read_exit_sensor();
}

void read_enter_sensor() {
  int IR_A = digitalRead(7);

  if (IR_A == LOW && enter_gate_status == 0) {
    //Serial.println("A is low");
    if (process_enter == 0) {
      enter_gate_status = 1;
      process_enter = 1;
    }
  }
  else if (IR_A == HIGH && enter_gate_status == 1 && enter_gate_timer > 10) {
    //Serial.println("Resetting A");
    enter_gate_status = 0;
    enter_gate_timer = 0;
    //a_trigger_count += 1;
  }
  else if (IR_A == HIGH && enter_gate_status == 1) {
    //Serial.println("A dipped low");
    //Serial.print("Timer: ");
    //Serial.println(enter_gate_timer);
    enter_gate_timer += 1;
  }
}

void read_exit_sensor() {
  int IR_B = digitalRead(6);

  if (IR_B == LOW && exit_gate_status == 0) {
    //Serial.println("B is low");
    if (process_exit == 0) {
      exit_gate_status = 1;
      process_exit = 1;
    }
  }
  else if (IR_B == HIGH && exit_gate_status == 1 && exit_gate_timer > 10) {
    //Serial.println("Resetting B");
    exit_gate_status = 0;
    exit_gate_timer = 0;
    //b_trigger_count += 1;
  }
  else if (IR_B == HIGH && exit_gate_status == 1) {
    exit_gate_timer += 1;
  }
}

void read_beacon_sensor() {
  int BCA = digitalRead(5);

  if (BCA == LOW) {
    Serial.println("BCA is low");
  }
}




  // if (IR_A == HIGH) {
  //   Serial.println("A is high");
  // }

  // else if (IR_A == HIGH && enter_gate_status == 1 && enter_gate_timer > 150) {
  //   //Serial.println("Resetting A");
  //   enter_gate_status = 0;
  //   enter_gate_timer = 0;
  //   //a_trigger_count += 1;
  // }
  // else if (IR_A == HIGH && enter_gate_status == 1) {
  //   //Serial.println("A dipped low");
  //   //Serial.print("Timer: ");
  //   //Serial.println(enter_gate_timer);
  //   enter_gate_timer += 1;
  // }

  // if (IR_B == HIGH) {
  //   Serial.println("B is high");
  // }
  // else if (IR_B == HIGH && exit_gate_status == 1 && exit_gate_timer > 150) {
  //   //Serial.println("Resetting B");
  //   exit_gate_status = 0;
  //   exit_gate_timer = 0;
  //   //b_trigger_count += 1;
  // }
  // else if (IR_B == HIGH && exit_gate_status == 1) {
  //   exit_gate_timer += 1;
  // }
