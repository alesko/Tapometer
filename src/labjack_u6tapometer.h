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

#ifndef LABJACK_u6TAPOMETER_H_
#define LABJACK_u6TAPOMETER_H_

#define GRAPH 0

#include "u6.h"
#include <unistd.h>
#include <iostream>
#include <vector>
#include <string>
#include <time.h>
#include <stdio.h>
#include <fstream>
#include <math.h>
#include <sys/time.h>
#include <iomanip>

/* OpenGL includes */
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h> //#include <glut/glut.h> 
#endif



class TapoMeter{
 protected:
  static TapoMeter *instance_; // To make callback functions work

 public:

  TapoMeter(int w, int h, float k, float c);
  ~TapoMeter();

  int Init(void);
  void close_LJ_device(void);
  void SampleData(void);

  //double GetRunTime(void);
  int  DrawCircle(double pos_x, double pos_y, double radius, double color[3]);
  int  DrawPlot(std::vector<double> data,double color[3], double t1, double t2);
  void DoRedraw(int flag, std::vector<double> d,double t1, double t2);

  double GetRunTime(void);
  double get_tstart(void);

  bool get_power_state(void);
  bool set_is_running(bool state);
  bool get_is_running(void);
  int get_screen_width(void);
  int get_screen_height(void);
  int set_screen_width(int w);
  int set_screen_height(int h);
  int set_fps(int fps);
  int get_fps(void);

  void increase_k_(void);
  void increase_c_(void);
  void decrease_k_(void);
  void decrease_c_(void);
  
 private:

  int screen_width_;       // Screen dimensions       
  int screen_height_;      
  bool is_running_;
  int fps_;
  double tstart_;

  // Screen handling
      
  // Data storange
  std::ofstream data_file_;
  std::vector<double> data_;

  // Counters
  long loops_;
  int flag_;
  int counts_;

  // Tap detection algorithm paramters
  float k_; // Multiplier
  float c_; // Threshold
  float dt_c_;
  float dt_k_;
  float K_MAX;
  float C_MAX;
  float K_MIN;
  float C_MIN;

  // LabJack 
  HANDLE            h_device_;
  u6CalibrationInfo cali_info_;
  int local_ID_;
  long error_;

};


#endif
