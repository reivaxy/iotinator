// Oled screen panel

// Dimensions between holes axis
xInterHole = 24.2;
<<<<<<< HEAD
yInterHole = 24.4;
=======
yInterHole = 24.6;
>>>>>>> c01c7a667572b895cc5d70b2c5c2bff9597b5697

// Margin around the whole stuff
xMargin = 10;
yMargin = 10;

// Overall panel dimensions
xPanel = xInterHole + xMargin;
yPanel = yInterHole + yMargin;
<<<<<<< HEAD
zPanel = 2;
=======
zPanel = 1;
>>>>>>> c01c7a667572b895cc5d70b2c5c2bff9597b5697

// Dimensions of the window in the panel
xWindow = 27;
yWindow = 15;
zWindow = 2 * zPanel;

// Some offset
xHoleCenter = 0.1;
yHoleCenter = 0;

windowPanel();

module windowPanel() {
  difference() {
    oledPanel();
<<<<<<< HEAD
    translate([(xPanel - xWindow)/2, (yPanel - yWindow)/2 + 1.5, -zPanel/2]) {
=======
    translate([(xPanel - xWindow)/2, (yPanel - yWindow)/2, -zPanel/2]) {
>>>>>>> c01c7a667572b895cc5d70b2c5c2bff9597b5697
      cube([xWindow, yWindow, zWindow]);  
    }
  }
}


module oledPanel() {
  cube([xPanel, yPanel, zPanel]);  
<<<<<<< HEAD
  pillar(xHoleCenter, yHoleCenter);
  pillar(xHoleCenter + xInterHole, yHoleCenter);
  pillar(xHoleCenter, yHoleCenter + yInterHole);
  pillar(xHoleCenter + xInterHole, yHoleCenter + yInterHole);
=======
  pilar(xHoleCenter, yHoleCenter);
  pilar(xHoleCenter + xInterHole, yHoleCenter);
  pilar(xHoleCenter, yHoleCenter + yInterHole);
  pilar(xHoleCenter + xInterHole, yHoleCenter + yInterHole);
>>>>>>> c01c7a667572b895cc5d70b2c5c2bff9597b5697
  
}

// To fix the Oled
<<<<<<< HEAD
module pillar(x, y) {
  translate([x + xMargin/2, y + yMargin/2, zPanel]) {
    cylinder(d=1.9, h=5, $fn=50);
    cylinder(d=4, h=2, $fn=50);
=======
module pilar(x, y) {
  translate([x + xMargin/2, y + yMargin/2, zPanel]) {
    cylinder(d=1.9, h=10, $fn=50);
    cylinder(d=4, h=2.5, $fn=50);
>>>>>>> c01c7a667572b895cc5d70b2c5c2bff9597b5697
    
  }
  
  
}