#!/usr/bin/env python3
import rospy
from push_robot_node import push_robot_node
from ssapang.msg import str
from std_msgs.msg import Float64
import time

def pub(num, row, col, push):
    pushPub[num][1].publish(row)
    pushPub[num][2].publish(row)
    pushPub[num][3].publish(col)
    rate.sleep()
    time.sleep(2)
    pushPub[num][0].publish(push)
    rate.sleep()
    time.sleep(3)
    push.data = 0.03 if push.data > 0 else -0.03
    pushPub[num][0].publish(push)
    rate.sleep()
    time.sleep(0.5)
    push.data = 0.0
    pushPub[num][0].publish(push)
    rate.sleep()

def pushing(msg):
    global data, row, col, push
    burger_node = msg.data
    try:
        push_code = push_robot_node.get(burger_node)
        parameter = push_code.split('-')
    except:
        print('error')
    print(burger_node, parameter)
    
    
    power = 0.12 if burger_node in data else 0.2
    row.data = 0.3 if burger_node in data else -0.3
    col.data = 0.5 if parameter[1] == '1' else -0.5
    push.data = -power if parameter[2] == 'l' else power
    pub(int(parameter[0]), row, col, push)
    data.add(burger_node)

if __name__ == '__main__':

    rospy.init_node('pushing')
    sub=rospy.Subscriber('/picking', str, pushing)
    rate = rospy.Rate(30) 
    data = set()
    row = Float64()
    col = Float64()
    push = Float64()
    row.data = 0
    col.data = 0
    push.data = 0

    n = 18
    pushPub = [[None for _ in range(4)] for _ in range(n+1)]
    for i in range(1,n+1):
        topic1 = '/push_robot{num}/push_joint2_position_controller/command'   ## push
        topic2 = '/push_robot{num}/push_joint1_position_controller/command'  ## row
        topic3 = '/push_robot{num}/virtual_joint2_position_controller/command' ## row
        topic4 = '/push_robot{num}/virtual_joint1_position_controller/command' ## col
        pushPub[i][0] = rospy.Publisher(topic1.format(num=i), Float64, queue_size=1)
        pushPub[i][1] = rospy.Publisher(topic2.format(num=i), Float64, queue_size=1)
        pushPub[i][2] = rospy.Publisher(topic3.format(num=i), Float64, queue_size=1)
        pushPub[i][3] = rospy.Publisher(topic4.format(num=i), Float64, queue_size=1)
        # pub(i,row, col, push)


    rospy.spin()