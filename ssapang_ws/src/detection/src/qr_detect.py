#!/usr/bin/env python3
#-*- coding:utf-8 -*-

import rospy
import cv2
import numpy as np

from cv_bridge import CvBridgeError
from pyzbar.pyzbar import decode

from sensor_msgs.msg import CompressedImage

class IMGParser:
    def __init__(self, pkg_name = 'ssafy_3'):

        self.img_bgrD = None
        self.img_hsvR = None

        self.image_subL = rospy.Subscriber("/camera/rgb/image_raw/compressed", CompressedImage, self.callbackD)
        # self.image_subR = rospy.Subscriber("/image_jpeg/compressed_R", CompressedImage, self.callbackU)

        rate = rospy.Rate(5)

        while not rospy.is_shutdown():
            if self.img_bgrD is not None:
                self.detectQR()

                rate.sleep()

    # def detectLane(self, camera, img_hsv):
    #     lower_wlane = np.array([0,20,180])
    #     upper_wlane = np.array([45,70,255])

    #     lower_ylane = np.array([15,100,100])
    #     upper_ylane = np.array([30,255,255])
        
    #     img_wlane = cv2.inRange(img_hsv, lower_wlane, upper_wlane)
    #     img_ylane = cv2.inRange(img_hsv, lower_ylane, upper_ylane)
        
    #     point_w = img_wlane[80,480]
    #     print(point_w)
    #     point_y = img_ylane[80,480]
    #     print(point_y)
    #     if point_w or point_y:
    #         print(camera)
    #     print("\n\n")

    #     cv2.line(img_hsv, (80,480),(80,480),(0,0,255),5)
    #     cv2.imshow(camera, img_hsv)
    #     cv2.waitKey(1)

    def detectQR(self):
        self.img_bgrD = cv2.resize(self.img_bgrD, (0, 0), fx=0.3, fy=0.3)
        decoded = decode(self.img_bgrD)
        print(decoded)


    def callbackD(self, msg):
        try:
            np_arr = np.fromstring(msg.data, np.uint8)
            self.img_bgrD = cv2.imdecode(np_arr, cv2.IMREAD_COLOR)
            # self.img_hsvL = cv2.cvtColor(img_bgrL, cv2.COLOR_BGR2HSV)
        except CvBridgeError as e:
            print(e)
        
        #self.detectLane("left", img_hsvL)


    # def callbackR(self, msg):
    #     try:
    #         np_arr = np.fromstring(msg.data, np.uint8)
    #         img_bgrR = cv2.imdecode(np_arr, cv2.IMREAD_COLOR)
    #         self.img_hsvR = cv2.cvtColor(img_bgrR, cv2.COLOR_BGR2HSV)
    #     except CvBridgeError as e:
    #         print(e)
        
        #self.detectLane("right", img_hsvR)

if __name__ == '__main__':

    rospy.init_node('lane_fitting', anonymous=True)

    image_parser = IMGParser()

    rospy.spin()