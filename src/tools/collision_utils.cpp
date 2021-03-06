/*
# Software License Agreement (BSD License)
#
# Copyright (c) 2012, University of California, Berkeley
# All rights reserved.
# Authors: Cameron Lee (cameronlee@berkeley.edu) and Dmitry Berenson (
berenson@eecs.berkeley.edu)
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above
#    copyright notice, this list of conditions and the following
#    disclaimer in the documentation and/or other materials provided
#    with the distribution.
#  * Neither the name of University of California, Berkeley nor the names
of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
*/

// See collision_utils.h for additional documentation.

#include "lightning/collision_utils.h"

std::vector<double> getPointBetween(const std::vector<double> &p1,
                                    const std::vector<double> &p2,
                                    double dist) {
  std::vector<double> new_point(p1.size());
  double fraction = dist / getLineDistance(p2, p1);
  for (unsigned int i = 0; i < p1.size(); i++) {
    new_point[i] = p1[i] +
                   fraction * getDirectionMultiplier(p1[i], p2[i]) *
                       getAngleBetween(p1[i], p2[i]);
  }
  return new_point;
}

std::vector<std::vector<double> > rediscretizePath(
    const std::vector<std::vector<double> > &path, double step_size) {
  std::vector<std::vector<double> > new_path;
  if (path.size() > 0) {
    double current_dist = 0;
    double move_dist = 0;
    bool on_initial_line = true;
    new_path.push_back(path[0]);
    unsigned int next_point_index = 1;
    while (next_point_index < path.size()) {
      on_initial_line = true;
      current_dist = getLineDistance(new_path[new_path.size() - 1],
                                     path[next_point_index]);
      while (current_dist < step_size) {
        if (next_point_index == path.size() - 1) {
          new_path.push_back(path[path.size() - 1]);
          return new_path;
        }
        current_dist +=
            getLineDistance(path[next_point_index], path[next_point_index + 1]);
        next_point_index++;
        on_initial_line = false;
      }
      if (!on_initial_line) {
        move_dist = step_size -
                    (current_dist - (getLineDistance(path[next_point_index - 1],
                                                     path[next_point_index])));
        new_path.push_back(getPointBetween(path[next_point_index - 1],
                                           path[next_point_index], move_dist));
      } else {
        new_path.push_back(getPointBetween(new_path[new_path.size() - 1],
                                           path[next_point_index], step_size));
      }
    }
    ROS_INFO("Collision utils: Got a new path of %i points",
             (unsigned int)new_path.size());
  } else {
    ROS_INFO("Collision utils: Rediscretize path got a path with no points");
  }
  return new_path;
}

double getAngleBetween(double a, double b) {
  double temp = 0;
  double twopi = atan(1) * 8;
  double x = (a < 0) ? (a - twopi * int((a / twopi) - 1)) : (fmod(a, twopi));
  double y = (b < 0) ? (b - twopi * int((b / twopi) - 1)) : (fmod(b, twopi));
  if (x > y) {
    temp = x;
    x = y;
    y = temp;
  }
  return (y - x < twopi / 2.0) ? (y - x) : (x + (twopi - y));
}

double getLineDistance(const std::vector<double> &p1,
                       const std::vector<double> &p2) {
  double sqDist = 0;
  for (unsigned int i = 0; i < p1.size(); i++) {
    sqDist += pow(getAngleBetween(p1[i], p2[i]), 2);
  }
  return sqrt(sqDist);
}

int getDirectionMultiplier(double a, double b) {
  double twopi = atan(1) * 8;
  double x = (a < 0) ? (a - twopi * int((a / twopi) - 1)) : (fmod(a, twopi));
  double y = (b < 0) ? (b - twopi * int((b / twopi) - 1)) : (fmod(b, twopi));
  if (x <= y) {
    return (0 <= y - x && y - x < twopi / 2) ? 1 : -1;
  } else {
    return (0 <= x - y && x - y < twopi / 2) ? -1 : 1;
  }
}

std::vector<std::vector<double> > interpolate(const std::vector<double> &p1,
                                              const std::vector<double> &p2,
                                              double step_size) {
  std::vector<std::vector<double> > interpolation;
  double dist = getLineDistance(p1, p2);
  if (dist > step_size) {
    double fraction = step_size / dist;
    std::vector<double> diffs(p1.size());
    std::vector<double> point(p1.size());
    for (unsigned int i = 0; i < p1.size(); i++) {
      diffs[i] = getAngleBetween(p1[i], p2[i]);
    }
    for (unsigned int i = 1; fraction * i < 1.0; i++) {
      for (unsigned int j = 0; j < point.size(); j++) {
        point[j] =
            p1[j] +
            fraction * i * getDirectionMultiplier(p1[j], p2[j]) * diffs[j];
      }
      interpolation.push_back(point);
    }
  }
  return interpolation;
}
