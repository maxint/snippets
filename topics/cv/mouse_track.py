#!/usr/bin/env python

'''''
kalman_mousetracker.py - OpenCV mouse-tracking demo using 2D Kalman filter

Adapted from

    http://www.morethantechnical.com/2011/06/17/simple-kalman-filter-for-tracking-using-opencv-2-2-w-code/

Copyright (C) 2014 Simon D. Levy

This code is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.
This code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this code. If not, see <http://www.gnu.org/licenses/>.
'''

# This delay will affect the Kalman update rate
DELAY_MSEC = 20

# Arbitrary display params
WINDOW_NAME = 'Kalman Mousetracker [ESC to quit]'
WINDOW_SIZE = 500

import cv2
import cv2.cv as cv
import numpy as np
import sys


class Kalman2D(object):
    """
    A class for 2D Kalman filtering
    """

    def __init__(self, process_noise=1e-4, measure_noise=1e-1, error_post=0.1):
        """
        Constructs a new Kalman2D object.
        For explanation of the error covariances see
        http://en.wikipedia.org/wiki/Kalman_filter
        """
        self.kalman = cv.CreateKalman(6, 2, 0)
        self.kalman_measurement = cv.CreateMat(2, 1, cv2.CV_32FC1)

        # Transition matrix (F)
        cv.SetIdentity(self.kalman.transition_matrix)
        # Constant velocity model:
        #   [ 1 1 ]
        #   [ 0 1 ]
        self.kalman.transition_matrix[0, 2] = 1
        self.kalman.transition_matrix[1, 3] = 1
        # Constant acceleration model:
        #   [ 1 1 0 ]
        #   [ 0 1 1 ]
        #   [ 0 0 1 ]
        self.kalman.transition_matrix[2, 4] = 1
        self.kalman.transition_matrix[3, 5] = 1

        # Measurement matrix (H)
        cv.SetIdentity(self.kalman.measurement_matrix)

        # Q & R
        cv.SetIdentity(self.kalman.process_noise_cov, cv.RealScalar(process_noise))
        cv.SetIdentity(self.kalman.measurement_noise_cov, cv.RealScalar(measure_noise))

        # P(k)
        cv.SetIdentity(self.kalman.error_cov_post, cv.RealScalar(error_post))

        self.predicted = None
        self.corrected = None

    def update(self, x, y):
        """
        Updates the filter with a new X,Y measurement
        """

        self.kalman_measurement[0, 0] = x
        self.kalman_measurement[1, 0] = y

        if self.corrected is None:
            self.kalman.state_post[0, 0] = x
            self.kalman.state_post[1, 0] = y

        self.predicted = cv.KalmanPredict(self.kalman)
        self.corrected = cv.KalmanCorrect(self.kalman, self.kalman_measurement)

        return self.corrected[0,0], self.corrected[1,0]


class MouseInfo(object):
    """
    A class to store X,Y points
    """

    def __init__(self):

        self.x, self.y = -1, -1

    def __str__(self):

        return '%4d %4d' % (self.x, self.y)


def mouse_callback(event, x, y, flags, mouse_info):
    """
    Callback to update a MouseInfo object with new X,Y coordinates
    """

    mouse_info.x = x
    mouse_info.y = y


def draw_cross(img, center, r, g, b):
    """
    Draws a cross a the specified X,Y coordinates with color RGB
    """

    d = 5
    t = 2

    color = (r, g, b)

    ctrx = center[0]
    ctry = center[1]

    cv2.line(img, (ctrx - d, ctry - d), (ctrx + d, ctry + d), color, t, cv2.CV_AA)
    cv2.line(img, (ctrx + d, ctry - d), (ctrx - d, ctry + d), color, t, cv2.CV_AA)


def draw_lines(img, points, r, g, b):
    cv2.polylines(img, [np.int32(points)], isClosed=False, color=(r, g, b))


def empty_image():
    return np.zeros((500,500,3), np.uint8)


def main():
    # Create a new image in a named window
    img = empty_image()
    cv2.namedWindow(WINDOW_NAME)

    # Create an X,Y mouse info object and set the window's mouse callback to modify it
    mouse_info = MouseInfo()
    cv2.setMouseCallback(WINDOW_NAME, mouse_callback, mouse_info)

    # Loop until mouse inside window
    while True:

        if mouse_info.x > 0 and mouse_info.y > 0:
            break

        cv2.imshow(WINDOW_NAME, img)
        if cv2.waitKey(1) == 27:
            sys.exit(0)


    # These will get the trajectories for mouse location and Kalman estiamte
    measured_points = []
    kalman_points = []

    # Create a new Kalman2D filter and initialize it with starting mouse location
    kalman2d = Kalman2D()

    # Loop till user hits escape
    while True:
        # Serve up a fresh image
        img = empty_image()

        # Grab current mouse position and add it to the trajectory
        measured = (mouse_info.x, mouse_info.y)
        measured_points.append(measured)

        # Update the Kalman filter with the mouse point
        new_pos = kalman2d.update(mouse_info.x, mouse_info.y)

        # Get the current Kalman estimate and add it to the trajectory
        estimated = [int (c) for c in new_pos]
        kalman_points.append(estimated)

        # Display the trajectories and current points
        draw_lines(img, kalman_points, 0, 255, 0)
        draw_cross(img, estimated, 255, 255, 255)
        draw_lines(img, measured_points, 255, 255, 0)
        draw_cross(img, measured, 0, 0, 255)

        # Delay for specified interval, quitting on ESC
        cv2.imshow(WINDOW_NAME, img)
        if cv2.waitKey(DELAY_MSEC) == 27:
            break

if __name__ == '__main__':
    main()
