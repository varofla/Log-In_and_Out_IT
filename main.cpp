#include <SoftwareSerial.h>
#include <Keyboard.h>

SoftwareSerial bt(9, 10);

String __UUID = "********************************";
String __MAC = "************";

String __PSWD = "****";

void setup() {
  bt.begin(9600);
  pinMode(SCK, INPUT_PULLUP);
  if(!digitalRead(SCK))
    while(true) {}
}

int read_ble() {
  bt.print("AT+DISI?");
  char buff[50], count = 0, flag = 0;
  int buff_pointer = 0, rssi = 999;
  while(true) {
    while(!bt.available()) {}
    buff[buff_pointer++] = bt.read();
    buff[buff_pointer] = 0; // println 오작동 방지
    if(count == 5 && buff[buff_pointer - 1] == '+') { // RSSI
      buff[buff_pointer - 3] = 0;
      if(flag == 2) {
        sscanf(buff, "%d", &rssi);
        flag++;
      } else {
        flag = 0;
      }
      buff_pointer = 0;
      count = 0;
    }
    else if(count == 0 && buff[buff_pointer - 1] == 'C') {
      while(!bt.available()) {}
      char _temp = bt.read();
      if(_temp == 'E') // DISCE - 스캔 데이터 종료
        break;
      else // DISC: - 스캔 데이터 시작 문자
        count++;
      buff_pointer = 0;
    }
    else if(buff[buff_pointer - 1] == ':') {
      buff[buff_pointer - 1] = 0;
      String string_buff = buff;
      switch(count) {
        case 2: // UUID
          if(__UUID.equals(string_buff)) {
            flag++;
          }
          break;
        case 4: // MAC
          if(__MAC.equals(string_buff)) {
            flag++;
          }
          break;
      }
      buff_pointer = 0;
      count++;
    }
  }
  return rssi;
}

unsigned long time_scan;
bool is_login = true;
int cnt_disconnect = 0;

void loop() {
  if(time_scan <= millis()) {
    int out = read_ble();
    if(out == 999) {
      cnt_disconnect++;
      if(cnt_disconnect == 2) {
        Keyboard.press(KEY_LEFT_GUI);
        Keyboard.write('l');
        Keyboard.releaseAll();
        is_login = false;
      }
    } else {
      if(out >=-90) {
        cnt_disconnect = 0;
        if(!is_login) {
          Keyboard.write(KEY_ESC);
          delay(1000);
          Keyboard.println(__PSWD);
          is_login = true;
        }
      } else {
        cnt_disconnect = 10;
        if(is_login) {
          Keyboard.press(KEY_LEFT_GUI);
          Keyboard.write('l');
          Keyboard.releaseAll();
          is_login = false;
        }
      }
    }
    time_scan = millis() + 1000;
  }
}
