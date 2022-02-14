// 
// Frame to hold display (foam at back of disply to hold in place).
//

glass_z = 2.1;
polariser_z = 0.2;

glass_y = 24.2;
polariser_y = 18;

frame_y = 32.8;
frame_z = (4.0);

module oled()
{
     // Glass
     cube([62.0, glass_y, glass_z], center=true);

     // Polariser
     translate([0, glass_y/2-polariser_y/2-1.0, glass_z/2+polariser_z/2])
     {
	  cube([60, polariser_y, polariser_z], center=true);
     }

     // Flex cable (thickened to cut through frame entirely)
     translate([0, -(glass_y/2)-10+0.1, 0])
     {
	  cube([13, 20, 20], center=true);
     }
}

// 8 from bottom
// centred

module frame()
{
     difference()
     {
	  cube([70, frame_y, frame_z], center=true);

      // recess for display
	  translate([0, -(frame_y/2)+(glass_y/2)+3, -frame_z/2+glass_z/2-0.01])
	  {
	       rotate([0, 180, 0])
	       {
		    oled();
	       }
	  }
      
      // Hole for foam if needed
      cube([20,10, 20], center= true);
      
     }
}

frame();
