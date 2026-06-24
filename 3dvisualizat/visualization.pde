import processing.serial.*;

Serial myPort;
float xOffset = 0;
float yOffset = 0;
float zOffset = 0;
float yaw = 0;
float pitch = 0;
float roll = 0;

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
    }
  }
}
