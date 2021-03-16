import hypermedia.net.*;
import controlP5.*;

UDP udp;
ControlP5 cp5;

final String IP = "127.0.0.1";
final int PORT = 13251;//送信側のポート番号

String msg = "test_messege";   //UDPで送るコマンド

void setup() {
  size(700, 700);

  cp5 = new ControlP5(this);
  udp = new UDP( this, 1800 );

  text("left", 5,5);
  cp5.addButton("UDP_Msg")
    .setLabel("Gogo-setrefpos")
    .setPosition(10,10)
    .setSize(100, 30);
  cp5.addSlider("x-CD-l")
     .setPosition(10,50)
     .setSize(200,20)
     .setRange(0,5.0)
     .setValue(3.0);
  cp5.addSlider("y-CD-l")
     .setPosition(10,90)
     .setSize(200,20)
     .setRange(0,5.0)
     .setValue(3.0);
  cp5.addSlider("z-CD-l")
     .setPosition(10,130)
     .setSize(200,20)
     .setRange(0,5.0)
     .setValue(3.0);
    
}

void draw() {
  background(200);
}

void UDP_Msg(){
  udp.send(msg,IP,PORT);
}
