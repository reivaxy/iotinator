// Oled screen panel

// Dimensions between holes axis
xInterHole = 24.2;
yInterHole = 24.4;

// Margin around the whole stuff
xMargin = 10;
yMargin = 10;

// Overall panel dimensions
xPanel = xInterHole + xMargin;
yPanel = yInterHole + yMargin;
zPanel = 2;

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
    translate([(xPanel - xWindow)/2, (yPanel - yWindow)/2 + 1.5, -zPanel/2]) {
      cube([xWindow, yWindow, zWindow]);  
    }
  }
}


module oledPanel() {
  cube([xPanel, yPanel, zPanel]);  
  pillar(xHoleCenter, yHoleCenter);
  pillar(xHoleCenter + xInterHole, yHoleCenter);
  pillar(xHoleCenter, yHoleCenter + yInterHole);
  pillar(xHoleCenter + xInterHole, yHoleCenter + yInterHole);
  
}

// To fix the Oled
module pillar(x, y) {
  translate([x + xMargin/2, y + yMargin/2, zPanel]) {
    cylinder(d=1.9, h=5, $fn=50);
    cylinder(d=4, h=2, $fn=50);
    
  }
  
  
}