class GasSensor {
  private:
    int pin;
    float a;
    float b;
    float rl;
    float ro;
    float vc;
    
  public:
    GasSensor(int pin, float a, float b, float rl, float ro, float vc) 
      : pin(pin), a(a), b(b), rl(rl), ro(ro), vc(vc) {}

    float readVoltage() {
      int analogValue = analogRead(pin);
      return analogValue * (vc / 4095.0);
    }

    float readResistance() {
      float voltage = readVoltage();
      return rl * ((vc / voltage) - 1);
    }

    float readConcentration() {
      float rs = readResistance();
      float ratio = rs / ro;
      return a * pow(ratio, b);
    }
};
