import hypermedia.net.*;
import controlP5.*;

UDP udp;
ControlP5 cp5;

final String IP = "127.0.0.1";
final int PORT = 13251;//送信側のポート番号

String msg = "test_messege";   //UDPで送るコマンド

void setup() {
  size(800, 600);
  background(100);

  cp5 = new ControlP5(this);
  udp = new UDP( this, 1800 );

  text("left", 5,5);
  int gui_y_offset = 10;
  int gui_x_offset = 10;
  cp5.addButton("setRefPos")
    .setLabel("Set ref for gogo")
    .setPosition(gui_x_offset,gui_y_offset)
    .setSize(100, 20);
  cp5.addButton("sendParams")
    .setLabel("Send parameters")
    .setPosition(150,gui_y_offset)
    .setSize(100, 20);
   cp5.addButton("activateGogo")
    .setLabel("Activate gogo")
    .setPosition(300,gui_y_offset)
    .setSize(100, 20);
   cp5.addButton("deactivateGogo")
    .setLabel("Deactivate gogo")
    .setPosition(450,gui_y_offset)
    .setSize(100, 20);
   cp5.addButton("saveParams")
    .setLabel("save parameters")
    .setPosition(600,gui_y_offset)
    .setSize(100, 20); gui_y_offset+=60;

  // left
  cp5.addSlider("x-CD-l")
     .setPosition(gui_x_offset,gui_y_offset)
     .setSize(200,20)
     .setRange(0,5.0)
     .setValue(3.0);gui_y_offset+=30;
  cp5.addSlider("y-CD-l")
     .setPosition(gui_x_offset,gui_y_offset)
     .setSize(200,20)
     .setRange(0,5.0)
     .setValue(3.0);gui_y_offset+=30;
  cp5.addSlider("z-CD-l")
     .setPosition(gui_x_offset,gui_y_offset)
     .setSize(200,20)
     .setRange(0,5.0)
     .setValue(3.0);gui_y_offset+=60;

  cp5.addSlider("x-ofs-l")
     .setPosition(gui_x_offset,gui_y_offset)
     .setSize(200,20)
     .setRange(-1000,1000.0)
     .setValue(0);gui_y_offset+=30;
  cp5.addSlider("y-ofs-l")
     .setPosition(gui_x_offset,gui_y_offset)
     .setSize(200,20)
     .setRange(-1000,1000.0)
     .setValue(0);gui_y_offset+=30;
  cp5.addSlider("z-ofs-l")
     .setPosition(gui_x_offset,gui_y_offset)
     .setSize(200,20)
     .setRange(-1000,1000.0)
     .setValue(0);gui_y_offset+=60;

  cp5.addSlider("rotx-ofs-l")
     .setPosition(gui_x_offset,gui_y_offset)
     .setSize(200,20)
     .setRange(0,1000.0)
     .setValue(0);gui_y_offset+=30;
  cp5.addSlider("roty-ofs-l")
     .setPosition(gui_x_offset,gui_y_offset)
     .setSize(200,20)
     .setRange(0,1000.0)
     .setValue(0);gui_y_offset+=30;
  cp5.addSlider("rotz-ofs-l")
     .setPosition(gui_x_offset,gui_y_offset)
     .setSize(200,20)
     .setRange(0,1000.0)
     .setValue(0);gui_y_offset+=30;

  gui_x_offset = 400;
  gui_y_offset = 70;

  // right
  cp5.addSlider("x-CD-r")
     .setPosition(gui_x_offset,gui_y_offset)
     .setSize(200,20)
     .setRange(0,5.0)
     .setValue(3.0);gui_y_offset+=30;
  cp5.addSlider("y-CD-r")
     .setPosition(gui_x_offset,gui_y_offset)
     .setSize(200,20)
     .setRange(0,5.0)
     .setValue(3.0);gui_y_offset+=30;
  cp5.addSlider("z-CD-r")
     .setPosition(gui_x_offset,gui_y_offset)
     .setSize(200,20)
     .setRange(0,5.0)
     .setValue(3.0);gui_y_offset+=60;

  cp5.addSlider("x-ofs-r")
     .setPosition(gui_x_offset,gui_y_offset)
     .setSize(200,20)
     .setRange(-1000,1000.0)
     .setValue(0);gui_y_offset+=30;
  cp5.addSlider("y-ofs-r")
     .setPosition(gui_x_offset,gui_y_offset)
     .setSize(200,20)
     .setRange(-1000,1000.0)
     .setValue(0);gui_y_offset+=30;
  cp5.addSlider("z-ofs-r")
     .setPosition(gui_x_offset,gui_y_offset)
     .setSize(200,20)
     .setRange(-1000,1000.0)
     .setValue(0);gui_y_offset+=60;

  cp5.addSlider("rotx-ofs-r")
     .setPosition(gui_x_offset,gui_y_offset)
     .setSize(200,20)
     .setRange(0,1000.0)
     .setValue(0);gui_y_offset+=30;
  cp5.addSlider("roty-ofs-r")
     .setPosition(gui_x_offset,gui_y_offset)
     .setSize(200,20)
     .setRange(0,1000.0)
     .setValue(0);gui_y_offset+=30;
  cp5.addSlider("rotz-ofs-r")
     .setPosition(gui_x_offset,gui_y_offset)
     .setSize(200,20)
     .setRange(0,1000.0)
     .setValue(0);gui_y_offset+=30;
}

JSONObject setObject(){
  System.out.println("sending params");
  System.out.println(cp5.getController("x-CD-r").getValue());

  JSONObject json = new JSONObject();
  json.setFloat("x-CD-l",cp5.getController("x-CD-l").getValue());
  json.setFloat("y-CD-l",cp5.getController("y-CD-l").getValue());
  json.setFloat("z-CD-l",cp5.getController("z-CD-l").getValue());
  json.setFloat("x-ofs-l",cp5.getController("x-ofs-l").getValue());
  json.setFloat("y-ofs-l",cp5.getController("y-ofs-l").getValue());
  json.setFloat("z-ofs-l",cp5.getController("z-ofs-l").getValue());
  json.setFloat("rotx-ofs-l",cp5.getController("rotx-ofs-l").getValue());
  json.setFloat("roty-ofs-l",cp5.getController("roty-ofs-l").getValue());
  json.setFloat("rotz-ofs-l",cp5.getController("rotz-ofs-l").getValue());

  json.setFloat("x-CD-r",cp5.getController("x-CD-r").getValue());
  json.setFloat("y-CD-r",cp5.getController("y-CD-r").getValue());
  json.setFloat("z-CD-r",cp5.getController("z-CD-r").getValue());
  json.setFloat("x-ofs-r",cp5.getController("x-ofs-r").getValue());
  json.setFloat("y-ofs-r",cp5.getController("y-ofs-r").getValue());
  json.setFloat("z-ofs-r",cp5.getController("z-ofs-r").getValue());
  json.setFloat("rotx-ofs-r",cp5.getController("rotx-ofs-r").getValue());
  json.setFloat("roty-ofs-r",cp5.getController("roty-ofs-r").getValue());
  json.setFloat("rotz-ofs-r",cp5.getController("rotz-ofs-r").getValue());

  String str = json.toString();
  System.out.println(str);
  return json;
}
void draw() {
  background(50);
}
void saveParams(){
  JSONObject json = setObject();
  saveJSONObject(json, "param.json");
  System.out.println("data saved");
}
void activateGogo(){
  udp.send("ActivateGoGo", IP, PORT);
  System.out.println("activate gogo");
}
void deactivateGogo(){
  udp.send("DeactivateGoGo", IP, PORT);
  System.out.println("deactivate gogo");
}
void sendParams(){
  JSONObject json = setObject();
  String str = json.toString();
  udp.send(str, IP, PORT);
}

void setRefPos(){
  udp.send("SetRefPos", IP, PORT);
  System.out.println("SetRefPos");
}
void UDP_Msg(){
  udp.send(msg,IP,PORT);
}
