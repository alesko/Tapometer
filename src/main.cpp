/*********************************************************************
 *
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2010, Alexander Skoglund, Karolinska Institute
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of the Karolinska Institute nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************/

#include <string.h>
#include "labjack_u6tapometer.h"

TapoMeter *tap_counter;  // Global variable 


using namespace std;

void HandleKeyboard(unsigned char key,int x, int y)
{
  switch (key) {
  case '<':                           // Decrease multiplier
  case ',':
    tap_counter->decrease_k_();
    break;
  case '>':                           // Increase multiplier
  case '.':
    tap_counter->increase_k_();
    break;
  case '-':                           // Decrease threshold 
  case '_':
    tap_counter->decrease_c_();
    break;
  case '+':                           // Increase threshold 
  case '=':
    tap_counter->increase_c_();
    break;    
  case 27:                            // Quit
  case 'Q':
  case 'q':
    {
      tap_counter->close_LJ_device();
      usleep(1000);
      tap_counter->set_is_running(false);      
      break;
    }
  }
}


void HandleIdle(void)
{
   static double tstart = -1;
   double tstop;

   double fps = (double)tap_counter->get_fps();
   if (tstart < 0)
      tstart = tap_counter->GetRunTime();
   tstop = tap_counter->GetRunTime();
   if (tstop - tstart > 1.0/fps) {
      glutPostRedisplay();
      tstart = tstop;
   }
}

void HandleReshape(int w,int h)
{
  tap_counter->set_screen_width(w);
  tap_counter->set_screen_height(h);
}

void IsRunning(void)
{
  
  if(tap_counter->get_is_running())
    tap_counter->SampleData();
  else
    exit(0);

}



int main(int argc, char* argv[])
{
  float  k = 1.3;
  float  c = 0.03;

  for(int i = 1; i < argc; i++)
    {
      if( 0 == strcmp(argv[i], "-k"))
	{
	  if (argv[i+1] != NULL )
	    {
	      std::cout << "k:" << atof(argv[i+1]) << std::endl;
	      k = atof(argv[i+1]);
	    }
	}
      if( 0 == strcmp(argv[i], "-c"))
	{
	  if (argv[i+1] != NULL )
	    {
	      std::cout << "c:" << atof(argv[i+1]) << std::endl;
	      c = atof(argv[i+1]);
	    }
	}

    }
  // Initialize the LabJack device
      tap_counter = new TapoMeter(640,480,k,c);
  tap_counter->set_is_running(true);
  tap_counter->set_fps(200.0);

/* You must call glutInit before any other OpenGL/GLUT calls */
  glutInit(&argc,argv); 
  //glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB); // | GLUT_DEPTH);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB); // | GLUT_DEPTH);
  glutCreateWindow("Tapometer");
  //glutInitWindowSize(500,500);
  glutReshapeWindow(tap_counter->get_screen_width(),tap_counter->get_screen_height());
  glutInitWindowPosition(0,0); 
  glutReshapeFunc(HandleReshape);
  glutKeyboardFunc(HandleKeyboard);
  glutIdleFunc(HandleIdle);

  //glutDisplayFunc(display);
  glutDisplayFunc(IsRunning);
  //init();
  glutMainLoop();

  // Set up GLUT environment
	/* glutInit(&argc,argv);
  
  glutReshapeWindow(tap_counter->get_screen_width(),tap_counter->get_screen_height());
  glutInitWindowPosition(0,0);
  glutReshapeFunc(HandleReshape);
  glutKeyboardFunc(HandleKeyboard);
  //glutIdleFunc(HandleIdle);
  glutDisplayFunc(IsRunning);
  glutMainLoop();*/

  return(0);

}

