#include "Arduino.h"
#include "SegDisp.h"

const int SegDisp::numbers[10][8] = {{1, 1, 1, 1, 1, 0, 1, 0}, {0, 0, 1, 1, 0, 0, 0, 0},
                                     {1, 1, 0, 1, 1, 0, 0, 1}, {0, 1, 1, 1, 1, 0, 0, 1},
                                     {0, 0, 1, 1, 0, 0, 1, 1}, {0, 1, 1, 0, 1, 0, 1, 1},
                                     {1, 1, 1, 0, 1, 0, 1, 1}, {0, 0, 1, 1, 1, 0, 1, 0},
                                     {1, 1, 1, 1, 1, 0, 1, 1}, {0, 1, 1, 1, 1, 0, 1, 1}};

const int SegDisp::alphabets[7][8] = {{0, 1, 1, 0, 0, 0, 0, 0}, {0, 0, 1, 0, 0, 0, 0, 0},
                                      {0, 0, 1, 1, 1, 1, 0, 0}, {0, 0, 1, 0, 1, 0, 0, 0},
                                      {0, 0, 1, 1, 0, 1, 0, 0}, {0, 1, 1, 1, 0, 1, 0, 0},
                                      {0, 0, 1, 0, 0, 1, 0, 0}};

const int SegDisp::small_b[8] = {0, 0, 1, 0, 0, 1, 0, 1};

const int SegDisp::err[8] = {1, 1, 1, 1, 0, 1, 1, 1};

const int SegDisp::none[8] = {0, 0, 0, 0, 0, 0, 0, 0};

char SegDisp::dispArray[3];

static void enable_light_pin(int count, int one_pin, int two_pin, int three_pin);
static void countup_light_pin(int& count);
static int calc_disp_number(int count, int dispNumber);

// コンストラクタ
SegDisp::SegDisp(int pins[8]){
  int i;

  for (i = 0; i < 8; i++){
    output_pins[i] = pins[i];
    pinMode(pins[i], OUTPUT);
  }

  for (i = 0; i < 3; i++){
    dispArray[i] = 'D';
  }
}

void SegDisp::lightNumber(int number){
  int i;

  for (int i = 0; i < (sizeof(output_pins) / sizeof(int)); i++){
    digitalWrite(output_pins[i], numbers[number][i]);
  }
}

void SegDisp::lightAlphabet(char alphabet){
  int i;

  // 引数チェック
  if (alphabet >= 'A' && alphabet <= 'G'){
    for (int i = 0; i < (sizeof(output_pins) / sizeof(int)); i++){
      digitalWrite(output_pins[i], alphabets[alphabet - 'A'][i]);
    }
  }else if (alphabet == 'b'){
    for (int i = 0; i < (sizeof(output_pins) / sizeof(int)); i++){
      digitalWrite(output_pins[i], small_b[i]);
    }
  }else{
    for (int i = 0; i < (sizeof(output_pins) / sizeof(int)); i++){
      digitalWrite(output_pins[i], err[i]);
    }
  }
}

void SegDisp::lightClear(void){
  for(int i = 0; i < (sizeof(output_pins) / sizeof(int)); i++){
    digitalWrite(output_pins[i], none[i]);
  }
}

void SegDisp::setDispArray(char out_one, char out_two, char out_three){
  dispArray[0] = out_one;
  dispArray[1] = out_two;
  dispArray[2] = out_three;
  //Serial.println(dispArray[0]);
}

int SegDisp::lightThreeCharByDynamic(int one_pin, int two_pin, int three_pin){
  static int count = 1;
  enable_light_pin(count, one_pin, two_pin, three_pin);
  lightAlphabet(dispArray[count - 1]);
  //Serial.println(dispArray[count - 1]);
  countup_light_pin(count);
  return count;
}

void SegDisp::setDispNumber(int number){
  SegDisp::dispNumber = number;
}

int SegDisp::lightNumberByDynamic(int one_pin, int two_pin, int three_pin){
  static int count = 1;
  int disp_num_now = calc_disp_number(count, dispNumber);
  enable_light_pin(count, one_pin, two_pin, three_pin);
  if(disp_num_now == -1){
    lightClear();
  }else{
    lightNumber(disp_num_now);
  }
  countup_light_pin(count);
  return count;
}

static void enable_light_pin(int count, int one_pin, int two_pin, int three_pin){
   switch(count){
    case 1:
      digitalWrite(one_pin, HIGH);
      digitalWrite(two_pin, LOW);
      digitalWrite(three_pin, LOW);
      break;
    case 2:
      digitalWrite(one_pin, LOW);
      digitalWrite(two_pin, HIGH);
      digitalWrite(three_pin, LOW);
      break;
    case 3:
      digitalWrite(one_pin, LOW);
      digitalWrite(two_pin, LOW);
      digitalWrite(three_pin, HIGH);
      break;
    default:
      break;
  }
}

static void countup_light_pin(int& count){
  if(count != 3){
    count++;
  }else{
    count = 1;
  }
}

static int calc_disp_number(int count, int dispNumber){
  switch(count){
    case 1:
      if(dispNumber >= 100){
        return (dispNumber % 1000) / 100;
      }else{
        return (-1);
      }
      break;
    case 2:
      if(dispNumber >= 10){
        return (dispNumber % 100) / 10;
      }else{
        return (-1);
      }
      break;
    case 3:
      if(dispNumber >= 0){
        return (dispNumber % 10);
      }else{
        return (-1);
      }
      break;
    default:
      break;
  }
  return (-1);
}
