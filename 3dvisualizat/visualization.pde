import processing.serial.*;

Serial myPort;
float xOffset = 0;
float yOffset = 0;
float zOffset = 0;
float yaw = 0;
float pitch = 0;
float roll = 0;
float vy = 0;
float vz = 0;
float yawps = 0;
float pitchps = 0;
float rollps = 0;
float ax = 0;
float ay = 0;
float az = 0;

void setup() {
  size(800, 600, P3D); // Create an 800x600 canvas in 3D Mode
  
  // Print a list of available serial ports to your console
  printArray(Serial.list());
                                                             
  // REPLACE THE 0 WITH THE INDEX OF YOUR ARDUINO PORT FROM THE PRINTED LIST
  String portName = Serial.list()[0];
  myPort = new Serial(this, portName, 115200);
  myPort.bufferUntil('\n'); // Only trigger a serial event when a new line arrives
}

void draw() {
  background(50); // Dark grey background
  lights();       // Turn on default 3D ambient lighting
  
  hint(DISABLE_DEPTH_TEST);
  fill(255);
  textSize(13);
  text("Data: ", 20, 30);
  text("X pos: " + nfc(xOffset, 2), 20, 50);
  text("Y pos: " + nfc(yOffset, 2), 20, 70);
  text("Z pos: " + nfc(zOffset, 2), 20, 90);
  text("yaw: " + nfc(yaw, 2), 20, 110);
  text("pitch: " + nfc(pitch, 2), 20, 130);
  text("roll: " + nfc(roll, 2), 20, 150);
  text("Y vel: " + nfc(vy, 2), 20, 170);
  text("Z vel: " + nfc(vz, 2), 20, 190);
  text("dps yaw: " + nfc(yawps, 2), 20, 210);
  text("dps pitch: " + nfc(pitchps, 2), 20, 230);
  text("dps roll: " + nfc(rollps, 2), 20, 250);
  text("X accel: " + nfc(ax, 2), 20, 270);
  text("Y accel: " + nfc(ay, 2), 20, 290);
  text("Z accel: " + nfc(az, 2), 20, 310);
  text("gyro has a lot of drift, press button to reset", width/2 - (115), height - 20);
  text("no vx is calculated, sorry", width/2 - (50), height - 40);
  text("vy and vz are sensor relative and not adjusted for rotation", width/2 - (160), height - 60);
  hint(ENABLE_DEPTH_TEST);
  
  
  // Move the origin (0,0,0) to the center of the screen
  translate(width/2, height/2, 0); 
  
  // Apply the physical translation offsets calculated by your matrix
  // You can multiply these by a scaling factor if the movements are too small to see
  translate(xOffset * 200, yOffset * 200, -zOffset * 200);
  rotateY(radians(yaw));
  rotateZ(radians(pitch));
  rotateX(radians(roll));
  
  // Draw a representation of your sensor board
  fill(255, 255, 255); // Sleek blue color
  stroke(255);       // White edges
  box(200, 10, 60); // Width, height, depth of the virtual object
}

void serialEvent(Serial myPort) {
  String inputString = myPort.readStringUntil('\n');
  
  if (inputString != null) {
    inputString = trim(inputString);
    String[] data = split(inputString, ',');

    if (data.length >= 6) {
      xOffset = float(data[0]);
      yOffset = float(data[1]);
      zOffset = float(data[2]);
      yaw = float(data[3]);
      pitch = float(data[4]);
      roll = float(data[5]);
      vy = float(data[6]);
      vz = float(data[7]);
      yawps = float(data[8]);
      pitchps = float(data[9]);
      rollps = float(data[10]);
      ax = float(data[11]);
      ay = float(data[12]);
      az = float(data[13]);
    }
  }
}
