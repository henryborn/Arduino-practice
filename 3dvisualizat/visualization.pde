import processing.serial.*; //imports every sublibrary from processing.serial, allows it to access the stuff I am printing to Serial in the other file which always runs on the arduino whether it is open or not

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
  size(800, 600, P3D); // This is the size that it initially creates at, was good for visualizing everything but you can fullscreen and in the final product this has no meaning
  
  // this was to chech which usb port my arduino is, it ended up being 0 but always check it
  printArray(Serial.list());
                                                             
  String portName = Serial.list()[0]; //So it knows what the arduino is because unlike the arduino IDE this does not automatically find and connect to it
  myPort = new Serial(this, portName, 115200); //brings the arduino into this file and makes sure the baud rate stays consistant across files
  myPort.bufferUntil('\n'); // this basically means loop after every new line character which is why all the data is in one continuous string in the other file
}

void draw() {
  background(50); // background, higher number means closer to white lower number means closer to black
  lights();       // Turn on default 3D ambient lighting apparently
  
  hint(DISABLE_DEPTH_TEST); //this stops the "3d camera frame" in order for the text to be still on the picture I can't be moving it in the same dimensions I am moving the breadboard so I need it to stop moving everything when writing the stillframe text as the code can't tell the difference
  fill(255); //grayscale, 255 is white 0 is black
  textSize(13); //13 pixels tall
  text("Data: ", 20, 30); //all of this is basically just a print statement to specific points on the screen
  text("X pos: " + nfc(xOffset, 2), 20, 50); //nfc trunkates the value to however many listed decimal points, in this case 2
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
  text("gyro has a lot of drift, press button to reset", width/2 - (120), height - 20); //makes sure it is centered if you go into fullscreen, the offsets were plug and chug, do not rely on them to be consistant based on a certain number of characters as some characters are smaller than others
  text("no vx is calculated, sorry", width/2 - (70), height - 40);
  text("vy and vz are sensor relative and not adjusted for rotation", width/2 - (160), height - 60);
  hint(ENABLE_DEPTH_TEST); //turn back on 3D moving, it doesn't move the text because the breadboard is actually redrawn every frame at the set offsets and the text has already been drawn for this frame
  
  
  // Move the origin (0,0,0) to the center of the screen, where the breadboard starts at
  translate(width/2, height/2, 0); 
  
  translate(xOffset * 200, yOffset * 200, -zOffset * 200); //this moves it from the origin to whatever the values are times and arbitrary number since the .1 or .2 (20 cm) movements translate to like less than a pixel and it would just look like it spins with no moving
  rotateY(radians(yaw)); //this was guess and check, what I though the axes looked like in real life are not what they ended up actually being
  rotateZ(radians(pitch));
  rotateX(radians(roll));
  
  fill(255, 255, 255); // creates the breadboard as white
  stroke(255);       // with white edges, though this could probably be removed honestly. I'm done writing code and testing though. "If it works don't fix it." If this were a professional thing I would remove redundancy like this though.
  box(200, 10, 60); // Width, height, depth, guess and check to make it seem like the approximate dimensions of the physical board sitting enxt to me
}

void serialEvent(Serial myPort) { //grab the data from serial
  String inputString = myPort.readStringUntil('\n'); //reads until a new line is created. After new line character it resets the data array back to item 0
  
  if (inputString != null) { //if theres data
    inputString = trim(inputString); //only get the actual items
    String[] data = split(inputString, ','); //create an array of the data that is separated by every comma, so if I forgot a comma both datasets would be counted as one and az would probably print either the new line character or null depending on if this actually stops at or after the new line character

    if (data.length >= 11) { //allows for data to be collected even if up to two items are missing as if two items are missing then it's going to be something unimportant, everything important come in like 3-5 different variables. This comment probably doesn't make sense I'm writing this at 2 AM 
      xOffset = float(data[0]); //defines each variable as that item in the data array translated into a float so if it was an int it would just throw 0s after a decimal point then store it. If it was a char then it would throw an error
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
