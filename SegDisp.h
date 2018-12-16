#ifndef SegDisp_h
#define SegDisp_h

class SegDisp{
public:
  SegDisp(int pins[8]);
  void lightNumber(int number);
  void lightAlphabet(char alphabet);
  void lightClear(void);
  void setDispArray(char out_one, char out_two, char out_three);
  int lightThreeCharByDynamic(int one_pin, int two_pin, int three_pin);
  void setDispNumber(int number);
  int lightNumberByDynamic(int one_pin, int two_pin, int three_pin);

private:
  static const int numbers[10][8];
  static const int alphabets[7][8];
  static const int small_b[8];
  static const int err[8];
  static const int none[8];
  static char dispArray[3];
  int output_pins[8];
  int dispNumber;
};

#endif
