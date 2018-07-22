// Oled screen panel

// Screen dimensions (may vary a little)
screenX = 28.5;
screenY = 27.5;

// Dimensions between holes axis
xInterHole = 23.7;
yInterHole = 24.4;

// Dimensions of the window in the panel
xWindow = 27;
yWindow = 15;
yWindowoffset = 1.5;

// Some offset
xHoleCenter = 0.1;
yHoleCenter = 0;

//windowPanel(45, 45, 2, 6);

module windowPanel(x, y, z, yOffset) {
  xPanel = x;
  yPanel = y;
  zPanel = z;
  
  difference() {
    oledPanel(xPanel, yPanel, zPanel, yOffset);
    translate([(xPanel - xWindow)/2, (yPanel - yWindow)/2 + yWindowoffset + yOffset, -zPanel/2]) {
      // opening
      cube([xWindow, yWindow, 2 * zPanel]);  
    }
  }
}


module oledPanel(xPanel, yPanel, zPanel, yOffset) {   
  xMargin = xPanel - xInterHole;
  yMargin = yPanel - yInterHole;
  difference() {
    union() {
      cube([xPanel, yPanel, zPanel]);  
      pillar(xHoleCenter, yHoleCenter + yOffset, xMargin, yMargin, zPanel);
      pillar(xHoleCenter + xInterHole, yHoleCenter + yOffset, xMargin, yMargin, zPanel);
      pillar(xHoleCenter, yHoleCenter + yInterHole + yOffset, xMargin, yMargin, zPanel);
      pillar(xHoleCenter + xInterHole, yHoleCenter + yInterHole + yOffset, xMargin, yMargin, zPanel);
    }
    // cut the base of pillars
    translate([(xMargin - yInterHole)/2,  yMargin/2+1 + yOffset, zPanel]) {
      cube([xInterHole *2, yInterHole - 2, 3]);
    }
  }
}

// To fix the Oled
module pillar(x, y, xMargin, yMargin, zPanel) {
  echo (x, y, xMargin, yMargin, zPanel);
  translate([x + xMargin/2, y + yMargin/2, zPanel]) {
    cylinder(d=1.6, h=5, $fn=50);
    cylinder(d=4, h=1.5, $fn=50);
  }
    
}

// Screen model to make simulations
module screen() {
  screenY2 = 19;
  z1 = 1.2;
  z2 = 1.5;
  sBottomOffset = 3.7;

  xMargin = screenX - xInterHole;
  yMargin = screenY - yInterHole;
  
  translate([0, sBottomOffset, 0]) {
    cube([screenX, screenY2, z2]);
  }
  translate([0, 0, z2]) {
    difference() {
      cube([screenX, screenY, z1]);
      hole(xHoleCenter, yHoleCenter, xMargin, yMargin, z1);
      hole(xHoleCenter + xInterHole, yHoleCenter, xMargin, yMargin, z1);
      hole(xHoleCenter, yHoleCenter + yInterHole, xMargin, yMargin, z1);
      hole(xHoleCenter + xInterHole, yHoleCenter + yInterHole, xMargin, yMargin, z1);
      // pin placeholders
      translate([(screenX - 11)/2, screenY-3, z1 - 0.1]) {
        cube([11, 2.6, 1]);
      }
    }
  }
}

module hole(x, y, xMargin, yMargin, z) {
  translate([x + xMargin/2, y + yMargin/2, -0.1]) {
    cylinder(d=1.9, h = z+0.2, $fn=20);
  }
  
}