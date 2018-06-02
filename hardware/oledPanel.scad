// Oled screen panel

// Dimensions between holes axis
xInterHole = 24.2;
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
  
  xMargin = xPanel - xInterHole;
  yMargin = yPanel - yInterHole;
  
  difference() {
    oledPanel(xPanel, yPanel, zPanel, xMargin, yMargin, yOffset);
    translate([(xPanel - xWindow)/2, (yPanel - yWindow)/2 + yWindowoffset + yOffset, -zPanel/2]) {
      // opening
      cube([xWindow, yWindow, 2 * zPanel]);  
    }
  }
}


module oledPanel(xPanel, yPanel, zPanel, xMargin, yMargin, yOffset) {
echo (zPanel);
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
    cylinder(d=1.6, h=8, $fn=50);
    cylinder(d=4, h=2, $fn=50);
    
  }
    
}