include <oledPanel.scad>;


innerX = 41;
innerY = 41;
innerZ = 41;
wall = 2;

outerX = innerX + 2 * wall;
outerY = innerY + 2 * wall;
outerZ = innerZ + 2 * wall;

boardZ = 2.25;

leftHookY = 10;
leftHookZ = 0.7;
leftHookX = 1.5;

rightHookY = 5;
rightHookZ = 0.7;
rightHookX = 1.1;

indentX = wall;
indentY = 8;
indentZ = wall;

rail = 2;
corner = 4;


cover();
// iotinator();
//closed();

module closed() {
  iotinator();
  translate([wall + coverTolerance/2, wall + coverTolerance/2, outerZ - wall]) {
    cover();
  }
}


module iotinator() {
  windowPanel(outerX, outerY,  wall, 4);  
  difference() {
    // outer cube
    cube([outerX, outerY, outerZ]);
    translate([wall, wall, -wall]) {
      // inner cube
      cube([innerX, innerY, innerZ + 4 * wall]);
    }
    
    // Small opening for the cover
    translate([innerX + wall -1 , (outerY - indentY)/2, outerZ-indentZ]) {
      cube([indentX + 2, indentY, indentZ+1]);
    }      
  }
  
  // bottom left rail
  translate([wall, wall + corner, 0])
    cube([rail, rail, innerZ-5]);
  // top left rail
  translate([wall, wall + corner + rail + boardZ, 0])
    cube([rail, rail, innerZ-5]);
  // bottom right rail
  translate([innerX + wall - rail , wall + corner, 0])
    cube([rail, rail, innerZ-5]);
  // top right rail
  translate([innerX + wall - rail , wall + corner + rail + boardZ, 0])
    cube([rail, rail, innerZ-5]);
  
  // bottom left corner
  translate([wall, wall, wall])
    cube([corner, corner, innerZ]);
  // top left corner
  translate([wall , innerY + wall - corner , wall])
    cube([corner, corner, innerZ]);
    
  // bottom right corner
  
  translate([innerX + wall - corner, wall, wall])
    cube([corner, corner, innerZ]);
  // top right corner
  translate([innerX + wall - corner, innerY + wall - corner, wall])
    cube([corner, corner, innerZ]);
  
  // left cover Hook
  translate([wall, (outerY - leftHookY)/2, outerZ-leftHookZ]) {
    cube([leftHookX, leftHookY, leftHookZ]);
  }
  
  color("red") {
    // bottom right cover Hook
    translate([outerX - rightHookX - wall, (outerY - rightHookY)/4, outerZ-rightHookZ ]) {
      cube([rightHookX, rightHookY, rightHookZ]);
    }
    // top right cover Hook
    translate([outerX - rightHookX - wall, (outerY - rightHookY) - (outerY - rightHookY)/4, outerZ-rightHookZ ]) {
      cube([rightHookX, rightHookY, rightHookZ]);
    }   
  }
}

coverTolerance = 0.2;
coverX = innerX - coverTolerance;
coverY = innerY - coverTolerance;

module cover() {
    union() {
      difference() {
        cube([coverX, coverY, wall - 0.1]);
        translate([0, (coverY - leftHookY - 0.2)/2, wall - leftHookZ - 0.2])
          cube([leftHookX +0.1, leftHookY + 0.2, rightHookZ + 2 ]);
        // bottom right cover Hook
        translate([coverX - rightHookX, (coverY - rightHookY - 0.2)/4 - (wall + coverTolerance)/2,  wall - rightHookZ - 0.2]) {
          cube([rightHookX, rightHookY+0.2, rightHookZ + 2]);
        }
        // top right cover Hook
        translate([coverX - rightHookX,  
                   (coverY - rightHookY -0.2) - (coverY - rightHookY - 0.2)/4  + (wall + coverTolerance)/2,
                    wall - rightHookZ - 0.2 ]) {
          cube([rightHookX, rightHookY+0.2, rightHookZ + 2]);
        }  

      }
      translate([coverX , (coverY - indentY + 0.2)/2, 0.2]) {
        cube([indentX + 0.2, indentY - 0.2, wall - 0.2]);
      }
    }
  
  
}