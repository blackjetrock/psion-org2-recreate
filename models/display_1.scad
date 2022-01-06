// 
// Frame to hold display (foam at back of disply to hold in place).
//

glass_z = 2.1;
polariser_z = 0.2;

glass_y = 24.2;
polariser_y = 18;

frame_y = 33.7;
frame_z = (4.7+1.7);

module oled()
{
     // Glass
     cube([62.2, glass_y, glass_z], center=true);

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
	  cube([71, frame_y, frame_z], center=true);

	  translate([0, -(frame_y/2)+(glass_y/2)+7, -frame_z/2+glass_z/2-0.01])
	  {
	       rotate([0, 180, 0])
	       {
		    oled();
	       }
	  }
     }
}

frame();
