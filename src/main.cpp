#include <Arduino.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <Servo.h>

#define GEAR_PWM 5
#define FLIR_3_PWM 4
#define FLIR_2_PWM 3
#define FLIR_1_PWM 2

Servo gear;
Servo flir_1;
Servo flir_2;
Servo flir_3;

int state_1 = 0;
int state_2 = 0;
int state_3 = 0;

int state_g = 0;

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEB
};

IPAddress ip(192, 168, 10, 171);
IPAddress target(192, 168, 10, 170);

char packetBuffer[UDP_TX_PACKET_MAX_SIZE];  
char ReplyBuffer[] = "acknowledged"; 

unsigned int localPort = 3003;
EthernetUDP Udp;

void sendChar(char __c);

void setup() {
  Serial.begin(9600);

  Ethernet.init(6);
  Ethernet.begin(mac, ip);

  delay(1000);

  gear.attach(GEAR_PWM);
  flir_1.attach(FLIR_1_PWM);
  flir_2.attach(FLIR_2_PWM);
  flir_3.attach(FLIR_3_PWM);

  gear.write(0);
  flir_1.write(0);
  flir_2.write(0);
  flir_3.write(0);

  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); 
    }
  }

  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
    while (true) {
      delay(1); 
    }
  }

  Serial.println("Started");


  Udp.begin(localPort);
}

unsigned int timer_1 = 0;
unsigned int timer_2 = 0;

void loop() {
  int packetSize = Udp.parsePacket();

  if (packetSize) {
    Serial.print("Received packet of size ");
    Serial.println(packetSize);
    Serial.print("From ");
    IPAddress remote = Udp.remoteIP();

    for (int i=0; i < 4; i++) {
      Serial.print(remote[i], DEC);
      if (i < 3) {
        Serial.print(".");
      }
    }

    Serial.print(", port ");
    Serial.println(Udp.remotePort());

    Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    Serial.println("Contents:");
    Serial.println(packetBuffer);

    if (packetBuffer[0] == '1') {
      Serial.print("State_1 : ");
      Serial.println(state_1);

      if (state_1 == 2) {
        state_1 = 0;
      } else {
        state_1++;
      }

      flir_1.write(state_1 * 90);
    }

    if (packetBuffer[0] == '2') {
      Serial.print("State_2 : ");
      Serial.println(state_2);
      
      if (state_2 == 2) {
        state_2 = 0;
      } else {
        state_2++;
      }

      flir_2.write(state_2 * 90);
    }

    if (packetBuffer[0] == '3') {
      Serial.print("State_3 : ");
      Serial.println(state_3);
      
      if (state_3 == 1) {
        state_3 = 0;
      } else {
        state_3++;
      }

      flir_3.write(state_3 * 180);
    }

    if (packetBuffer[0] == 'u') {
      Serial.println("Gear UP");
      gear.write(90);
    }

    if (packetBuffer[0] == 'd') {
      Serial.println("Gear DOWN");
      gear.write(0);
    }

    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write(ReplyBuffer);
    Udp.endPacket();
  }

  timer_2 = millis();

  if ((timer_2 - timer_1) > 1000) {
    timer_1 = millis();
    sendChar('c');
  }

  delay(10);
}

void sendChar(char __c) {
  Udp.beginPacket(target, localPort);
  Udp.write(__c);
  Udp.endPacket();
}