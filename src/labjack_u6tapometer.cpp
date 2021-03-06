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


#include "labjack_u6tapometer.h"

using namespace std;

TapoMeter::TapoMeter(int w, int h, float k, float c)
{

  tstart_ = GetRunTime();

  is_running_ = true;
  screen_width_  = w;
  screen_height_ = h;
  flag_ = 0;

  counts_=0;
  loops_ = 0;
 

  //Open first found U6 over USB
  local_ID_ = -1;
  if( (h_device_ = openUSBConnection(local_ID_)) == NULL)
    {
      printf("ERROR: Unable to open USB connection\n");
      exit(0);
    }
  
  //Get calibration information from U6
  if(getCalibrationInfo(h_device_, &cali_info_) < 0)
    {
      printf("ERROR: Unable to get calibration\n");
      exit(0);
    }

  // 
  get_power_state();
  
  // Open data file for printing
  time_t t = time(0);
  struct tm* lt = localtime(&t);
  char time_str[256];
  sprintf(time_str, "data/taplog_%04d%02d%02d_%02d%02d%02d.log",
          lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday,
          lt->tm_hour, lt->tm_min, lt->tm_sec);
 
  data_file_.open(time_str);
  //data_file_ << "data=[";

  // Init thresholds
  k_ = k;
  c_ = c;
  /*k_ = 1.3; //
    c_ = 0.03; //0.008;*/
  dt_c_ = 0.00025;
  dt_k_ = 0.1;
  K_MAX = 10;
  C_MAX = 1.0;
  K_MIN = 1;
  C_MIN = 0.0;
  if( k_ > K_MAX){
    std::cout << "ERROR: k exceeds max value." << std::endl; 
    exit(0);
  }
  if( c_ > C_MAX)
    {
      std::cout << "ERROR: c exceeds max value." << std::endl; 
      exit(0);
    }
  if( k_ < K_MIN )
    {
      std::cout << "ERROR: k is below the min limit." << std::endl; 
      exit(0);
    }
  if( c_ < C_MIN)
    {
      std::cout << "ERROR: c is below the min limit." << std::endl; 
      exit(0);
    }

}

TapoMeter::~TapoMeter(void)
{
  
  printf("TapoMeter object destroyed!");
  
}

double TapoMeter::GetRunTime(void)
{
   double sec = 0;
   struct timeval tp;

   gettimeofday(&tp,NULL);
   sec = tp.tv_sec + tp.tv_usec / 1000000.0;

   return(sec);
}

void TapoMeter::SampleData(void)
{

  double time_now;

  time_now = GetRunTime();
  
  //Read the differtial voltage from AIN0-1
  //printf("\nCalling eAIN to read voltage from AIN0\n");
  double dblVoltage;
  if((error_ = eAIN(h_device_, &cali_info_, 0, 1, &dblVoltage, 0, 0, 0, 0, 0, 0)) != 0)
    {
      printf("ERROR: Unable to aquire data \n");
      exit(0);
    }       
  data_.push_back(dblVoltage);

  //Read state of FIO0
  //printf("\nCalling eDI to read the state of FIO3\n");
  //long lng_state;
  //if((error_ = eDI(h_device_, 0, &lng_state)) != 0)
    //  printf("\ERROR: When reading FIO0\n");
  
  //printf("FIO3 state = %ld\n", lngState);
  
  //Set FIO2 to output-high
  //printf("\nCalling eDO to set FIO1 to output-high\n");
  //if((error_ = eDO(h_device_, 1, lng_state)) != 0)
    //  printf("ERROR: When writing to FIO1, state = %ld\n", lng_state);
  //printf("FIO1 state = %ld\n", lng_state);


  //printf("%f\n",dblVoltage);
  /*if ( loops_ == 0 )
    {
      //data_file_ << (time_now-tstart_) << " " <<  dblVoltage << " " << lng_state << ";";
      data_file_ << (time_now-tstart_) << " " <<  dblVoltage;  <<  ";";
    }
  else
    {
      //data_file_ << std::endl<< (time_now-tstart_) << " " <<  dblVoltage << " " << lng_state << ";";
      data_file_ << std::endl<< (time_now-tstart_) << " " <<  dblVoltage << ";";
    }
  */
  data_file_ << (time_now-tstart_) << "\t" <<  dblVoltage << std::endl;

  /*if ( loops_ == 0 )
    cout << GetRunTime()-tstart_ << " " <<  dblVoltage << ";";
  else
    cout << std::endl<< setprecision(12) << GetRunTime()- tstart_ << " " <<  dblVoltage << ";";
  */
  
  if ( loops_ > 0 )
    {
      // Start
      if ( (data_[loops_-1] <= c_)  && (flag_ == 0) )
	{
	  if( data_[loops_] > (k_*c_))    
	    flag_ = 1;
	}
      // Stop
      if( (data_[loops_-1] < (k_*c_)) && (flag_ ==1))
	{
	  if(data_[loops_] < c_)
	    {
	      flag_ = 0;
	      counts_++;
	      printf("Taps = %d\n", counts_);
	    }
	}
    }    
  loops_++;
  // Draw OpenGL stuff 
  DoRedraw(flag_,data_,c_,c_*k_);


  
}


int TapoMeter::DrawCircle(double pos_x, double pos_y, double radius, double color[3]){
  int i;
  double degInRad;

  glBegin(GL_TRIANGLE_FAN);
  glColor3f(color[0], color[1], color[2]);
  glVertex2f(pos_x,pos_y); 
  for(i = 0;i<361;i++){
    degInRad = i*M_PI/180.0;
    glVertex2f(pos_x+cos(degInRad)*radius,pos_y+sin(degInRad)*radius);
  }
  glEnd();
  return 1;
}

int TapoMeter::DrawPlot(std::vector<double> data,double color[3], double t1, double t2){//,color,maximum)
  //screenWidth=1280; screenHeight=960;
  //maximum = 
  int len = data_.size();
  int i = 0;
  int s = 0;
  int x=0;
  int gain=5000;

  glLineWidth(1);
  
  glBegin(GL_LINES);
  glColor3f(1.0, 1.0, 1.0);
  glVertex2f(0,screen_height_-gain*t1);
  glVertex2f(screen_width_,screen_height_-gain*t1);
  glEnd();

  glBegin(GL_LINES);
  glColor3f(0.0, 1.0, 0.0);
  glVertex2f(0,screen_height_-gain*t2);
  glVertex2f(screen_width_,screen_height_-gain*t2);
  glEnd();

  if( len > screen_width_ )
      x = len - screen_width_;
  glBegin(GL_LINE_STRIP);
  glColor3f(color[0], color[1], color[2]);
  for(i=0;i < len; i++){
    glVertex2f(i,screen_height_-gain*data_[x]);
    x++;
  }
  glEnd();

  return 1;
}

// Called by GLUT when the screen needs to be redrawn 
void TapoMeter::DoRedraw(int flag, std::vector<double> d,double t1, double t2)
{
  // Setup the display 
  glViewport(0,0,(GLsizei) screen_width_,(GLsizei) screen_height_); 
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0, (GLdouble)screen_width_, (GLdouble) screen_height_, 0.0, 0.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glClearColor(0.0,0.0,0.0,0.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  double white[3]={1.0, 1.0, 1.0};
  double red [3]={1.0, 0.0, 0.0};
  double green[3]={0.0, 1.0, 0.0};
  double blue[3]={0.0, 0.0, 1.0};

  if( flag == 0)
    DrawCircle(screen_width_/2,screen_height_/2, screen_height_/4, red);
  if( flag == 1)
    DrawCircle(screen_width_/2,screen_height_/2, screen_height_/4, green);
  if( GRAPH == 1)
    DrawPlot(d,red, t1,t2);
  glFlush();
  glutSwapBuffers();

}

void TapoMeter::close_LJ_device(void)
{
  printf("Closing LabJack device!\n");
  closeUSBConnection(h_device_);
  //data_file_ << "];";
  //data_file_ << "];" << std::endl;
  data_file_ << "counts=" << counts_ << ";";
  data_file_.close();
  printf("LabJack device closed!\n");

}

void TapoMeter::increase_k_(void){
  if ( k_ < K_MAX )
    k_ = k_+  dt_k_;
  printf("Threshold(c) = %f, Multiplier(k) = %f \n",c_,k_);

}

void TapoMeter::increase_c_(void){
  if ( c_ < K_MAX )
    c_ = c_ + dt_c_;
  printf("Threshold(c) = %f, Multiplier(k) = %f \n",c_,k_);
}

void TapoMeter::decrease_k_(void){
  if ( k_ > 1 )
    k_ = k_ - dt_k_;
  printf("Threshold(c) = %f, Multiplier(k) = %f \n",c_,k_);
}

void TapoMeter::decrease_c_(void)
{
  if ( c_ > 0 )
    c_ = c_ - dt_c_;
  printf("Threshold(c) = %f, Multiplier(k) = %f \n",c_,k_);
}

bool TapoMeter::get_power_state(void)
{
  //Read the differential voltage from AIN4
  //printf("\nCalling eAIN to read voltage from AIN0\n");
  double dblVoltage;
  if((error_ = eAIN(h_device_, &cali_info_, 4, 5, &dblVoltage, 0, 0, 0, 0, 0, 0)) != 0)
    {
      printf("ERROR: Unable to aquire data \n");
      exit(0);
    }
  else
    {
      //printf("AIN4-5: %lf \n",dblVoltage);
      if( dblVoltage < 3.5 )
	{
	  printf("ERROR: Power is turned of!\nTurn power on and restart program.\n");
	  exit(0); 
	}
    }

}

bool TapoMeter::set_is_running(bool state){
  is_running_ = state;
  return(state);
}

bool TapoMeter::get_is_running(void){
  bool state = is_running_;
  return(state);
}

int TapoMeter::get_screen_width(void){
  return(screen_width_);
}

int TapoMeter::get_screen_height(void){
  return(screen_height_);
}

int TapoMeter::set_screen_width(int w){
  screen_width_ = w;
  return(screen_width_);
}

int TapoMeter::set_screen_height(int h){
  screen_height_ = h;
  return(screen_height_);
}

int TapoMeter::set_fps(int fps){
  fps_ = fps;
  return(fps_);
}

int TapoMeter::get_fps(void){
  return(fps_);
}

double TapoMeter::get_tstart(void){
  return(tstart_);
}
