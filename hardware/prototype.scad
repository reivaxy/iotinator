include <oledPanel.scad>;


//windowPanel();

innerX = 41;
innerY = 41;
innerZ = 41;
wall=2;

outerX = innerX + 2 * wall;
outerY = innerY + 2 * wall;
outerZ = innerZ + 2 * wall;

boardZ = 2.1;

leftHookY = 10;
leftHookZ = 0.7;
leftHookX = 1.5;

rightHookY = 5;
rightHookZ = 0.7;
rightHookX = 1.1;

indentX = wall;
indentY = 8;
indentZ = wall;

iotinator();

module iotinator() {

  windowPanel(outerX, outerY,  wall, 5);  
  difference() {
    cube([outerX, outerY, outerZ]);
    translate([wall, wall, -wall])
      cube([innerX, innerY, innerZ + 4 * wall]);
    translate([innerX + wall -1 , (outerY - indentY)/2, outerZ-indentZ]) {
      cube([indentX + 2, indentY, indentZ]);
    }      
  }
  
  // rails
  translate([wall - 0.1, 5, 0])
    cube([2, 2, innerZ-5]);
  translate([wall - 0.1, 5 + 2 + boardZ, 0])
    cube([2, 2, innerZ-5]);
  translate([innerX + 0.1, 5, 0])
    cube([2, 2, innerZ-5]);
  translate([innerX + 0.1, 5 + 2 + boardZ, 0])
    cube([2, 2, innerZ-5]);
  
  // corners bottom left
  translate([wall - 0.1, wall - 0.1, wall - 0.1])
    cube([4, 4, innerZ]);
  // corners top left
  translate([wall - 0.1, innerY-wall + 0.1, wall - 0.1])
    cube([4, 4, innerZ]);
  // corners bottom right
  translate([innerX - wall + 0.1, wall - 0.1, wall - 0.1])
    cube([4, 4, innerZ]);
  // corners top right
  translate([innerX - wall + 0.1, innerY - wall + 0.1, wall - 0.1])
    cube([4, 4, innerZ]);
  
  // left cover Hook
  translate([wall, (outerY - leftHookY)/2, outerZ-leftHookZ]) {
    cube([leftHookX, leftHookY, leftHookZ]);
  }
  
  // bottom right cover Hook
  translate([outerX - rightHookX - wall, (outerY - rightHookY)/4, outerZ-rightHookZ ]) {
    cube([rightHookX, rightHookY, rightHookZ]);
  }
  // top right cover Hook
  translate([outerX - rightHookX - wall, (outerY - rightHookY) - (outerY - rightHookY)/4, outerZ-rightHookZ ]) {
    cube([rightHookX, rightHookY, rightHookZ]);
  }  
  
       
}