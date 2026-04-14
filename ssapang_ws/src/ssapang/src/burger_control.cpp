 
#include <ros/ros.h>
#include <geometry_msgs/Twist.h>
#include <tf/transform_datatypes.h>
#include <turtlesim/Pose.h>
#include <nav_msgs/Odometry.h>
#include <iostream>
#include <ssapang/Locations.h>
#include <ssapang/Coordinate.h>
#include <ssapang/Move.h>
#include <ssapang/str.h>
#include <ssapang/RobotWait.h>
#include <ssapang/RobotPos.h>
#include <ssapang/RobotStatus.h>
#include <ssapang/Task.h>
#include <ssapang/End.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <queue>
#include <unistd.h>
#include <ssapang/Station.h>
#include<std_msgs/Float64.h>

const double PI = std::acos(-1);

class RobotControl
{
public:
    std::string robotName;
    ssapang::Coordinate nextPos;
    RobotControl(int argc, char **argv, ros::NodeHandle *nh){
        cmdPub = nh->advertise<geometry_msgs::Twist>("cmd_vel", 10);
        movePub = nh->advertise<ssapang::Move>("move", 10);
        posPub = nh->advertise<ssapang::RobotPos>("pos", 10);
        statusPub = nh->advertise<ssapang::RobotStatus>("status", 10);
        checkGoPub = nh->advertise<ssapang::str>("checkGo", 1);
        goPub = nh->advertise<ssapang::str>("Go", 1);
        dumpPub = nh->advertise<std_msgs::Float64>("dump_joint_position_controller/command",1);
        pickPub = nh->advertise<ssapang::str>("/picking",1);


        odomSub = nh->subscribe("odom", 10, &RobotControl::odomCallback, this);
        pathSub = nh->subscribe("path", 10, &RobotControl::pathCallback, this);
        waitSub = nh->subscribe("wait", 10, &RobotControl::waitCallback, this);
        shelfSub = nh->subscribe("shelf", 10, &RobotControl::shelfCallback, this);
        taskSub = nh->subscribe("task", 10, &RobotControl::taskCallback, this);

        endSrv = nh->serviceClient<ssapang::End>("end");
        stationCli = nh->serviceClient<ssapang::Station>(argv[1][0] == 'l' ? "/burger/left_station" : "/burger/right_station");

        stop.angular.z = 0.0;
        stop.linear.x = 0.0;

        linearSpeed = 0.22;
        angularSpeed = 1.0;
        idx = -1;
        status.status = 0;
        battery = 100.0;
        nextPos.QR = argv[2];
        robotName = argv[1];
        GO.data = "GO";
        wait = 1;

        try
        {
            sleep(20);
            while (ros::ok())
            {
                if(status.status == 4){
                    sleep(1);
                    battery += 50;
                    // std:: cout << robotName << " battery : " << battery << "\n";
                    if(battery < 100) continue;
                    battery = 100;
                    status.status = -2;
                    statePush();
                    status.status = 0;
                }
                // 판단
                ros::spinOnce();
                rate.sleep();
                if(idx < 0) continue;
                if(wait) {
                    checkGoPub.publish(NEXT); // 다음위치
                    continue;;
                }               
                
                nextIdx();
                if(nextPos.y == 100) {
                    std::cout << robotName << " 잔여 전력량 : " << battery << "\n";
                    path.clear();
                    idx = -1;
                    cmdPub.publish(stop);
                    rate.sleep();
                    sleep(3);
                    switch (status.status)
                    {
                    case 3:
                        std::cout << robotName << " 충전소 도착\n";
                        statePush();
                        break;
                    case 1:
                        nextPos.deg = ( (NOW.data[NOW.data.length()-1] -'0') & 1 ) ? 0 : PI;
                        turn();
                        cmdPub.publish(stop);
                        pickPub.publish(NOW);
                        rate.sleep();

                        sleep(5);
                        std::cout << robotName << " 픽업 완료 후 경로 생성\n";
                        makePath(task.destination);
                        break;
                    case 2:
                        dump.data = 1.0;
                        dumpPub.publish(dump);
                        rate.sleep();
                        sleep(2);
                        dump.data = 0.0;
                        dumpPub.publish(dump);
                        std::cout << robotName << " 일처리 완료 후 일하러갈지 충전소 갈지 판단\n";
                        ssapang::End end;
                        end.request.name = robotName;
                        if(battery > 30 && endSrv.call(end)){  
                            std::cout << "일 재할당\n";
                            task = end.response.task;
                            status.status = 0;
                            makePath(task.product);
                        }
                        else{
                            std::cout << "충전소\n";
                            ssapang::Station srv;
                            srv.request.nowNode = nextPos.QR;
                            srv.request.num = stoi(robotName.substr(7,robotName.length() - 7));
                            std::cout << "num : " <<srv.request.num  <<std::endl;
                            std::string destination;
                            if(stationCli.call(srv)){  
                                destination = srv.response.stationNode;        
                            }
                            else{
                                ROS_ERROR("robot come back fail");
                            }
                            std::cout << "destination : " << destination << std::endl;
                            makePath(destination);
                        }
                        break;
                    }


                }else{
                    turn();
                    go();
                    goPub.publish(NOW);
                    rate.sleep();
                    // if(NOW.data[0]=='L' || NOW.data[0]=='R' || NOW.data[1] == 'O'){
                    //     go();
                    //     goPub.publish(NOW);
                    //     rate.sleep();
                    // }else{
                    //     goPub.publish(NOW);
                    //     rate.sleep();
                    //     go();
                    // }
                    cmdPub.publish(stop);
                    rate.sleep();
                    turn();
                    if(idx < path.size()){
                        ssapang::RobotPos pos;
                        pos.fromNode = nextPos.QR;
                        NOW.data = path[idx].QR;
                        battery -= 2;
                        pos.battery = battery;
                        pos.idx = ++idx;
                        if(NOW.data != path[idx].QR){
                            pos.toNode = path[idx].QR;
                            NEXT.data = path[idx].QR;
                            posPub.publish(pos);

                        }
                    }
                    // std::cout << robotName << " - "<< idx << '/' << path.size() << "\n";
                }
                wait = 1;
                cmdPub.publish(stop);
                rate.sleep();
            }
        }
        
        catch (const std::exception &e)
        {
            std::cerr << "Exception occurred: " << e.what() << std::endl;
        }
        catch (...)
        {
            std::cerr << "Unknown exception occurred." << std::endl;
        }
        cmdPub.publish(stop);
    }

private:
    ros::Publisher cmdPub, movePub, posPub, statusPub, checkGoPub, goPub, dumpPub, pickPub;
    ros::Subscriber odomSub, pathSub, waitSub, shelfSub, taskSub;
    ros::ServiceClient endSrv, stationCli;
    ros::Rate rate = 30;

    geometry_msgs::Twist moveCmd;
    geometry_msgs::Twist stop;
    turtlesim::Pose nowPosition;  
    std::string shelfNode;
    ssapang::Task task;
    std::vector<ssapang::Coordinate> path;
    ssapang::str NOW, NEXT, GO;
    std_msgs::Float64 dump;
    int idx;
    double d, linearSpeed, angularSpeed;
    double lastDeg;
    int wait;
    ssapang::RobotStatus status;
    double battery;

    void odomCallback(const nav_msgs::Odometry::ConstPtr &msg)
    {
        turtlesim::Pose pose; 
        pose.x = msg->pose.pose.position.x;
        pose.y = msg->pose.pose.position.y;
        tf::Quaternion quat;
        tf::quaternionMsgToTF(msg->pose.pose.orientation, quat);
        double roll, pitch, yaw;
        tf::Matrix3x3(quat).getRPY(roll, pitch, yaw);
        pose.theta = yaw;
        nowPosition = pose;
    }

    void shelfCallback(const ssapang::str::ConstPtr &msg)
    {
        shelfNode = msg->data;
        makePath(shelfNode);
    }
    void taskCallback(const ssapang::Task::ConstPtr &msg)
    {
        task = *msg;
        makePath(task.product);
        std::cout << robotName << " => 운반물 픽업 위치 :" <<  task.product << ", 운반 장소 : " << task.destination << "\n";
    }

    void pathCallback(const ssapang::Locations::ConstPtr &msg)
    {
        path = msg->location;
        // std::cout << robotName << " - " << path.size() << "\n";
        idx = 1;
        NEXT.data = path[1].QR;
        NOW.data = path[0].QR;
        ssapang::RobotPos pos;
        pos.fromNode = path[0].QR;
        pos.toNode = path[1].QR;
        pos.battery = battery;
        pos.idx = idx;
        posPub.publish(pos);
        rate.sleep();
    }
    void waitCallback(const ssapang::RobotWait::ConstPtr &msg)
    {
        wait = msg->wait;
    }

    void makePath(std::string nodeName){
        ssapang::Move move;
        move.startNode = nextPos.QR;
        move.endNode = nodeName;
        // move.types = "burger";
        movePub.publish(move);
        rate.sleep();
        statePush();
    }

    void statePush(){
        status.status++;
        statusPub.publish(status);
        rate.sleep();
    }

    void nextIdx()
    {
        nextPos = path[idx];
        nextPos.deg = path[idx].deg * PI / 180.0;
    }

    void turn()
    {
        try{
            moveCmd.linear.x = 0.00;
            float speed = 0.0;
            double deg=0.0;
            while (1){
                ros::spinOnce();
                if (std::abs(nowPosition.theta - nextPos.deg) <= 0.02)
                    return;
                deg = std::abs(nowPosition.theta - nextPos.deg);
                speed = std::max(2*std::min(deg, 1.0), 0.1);
                
                
                if (nextPos.deg >= 0){
                    if (nowPosition.theta <= nextPos.deg and nowPosition.theta >= nextPos.deg - PI)
                        moveCmd.angular.z = speed;
                    else
                        moveCmd.angular.z = -speed;
                }
                else{
                    if (nowPosition.theta <= nextPos.deg + PI and nowPosition.theta > nextPos.deg)
                        moveCmd.angular.z = -speed;
                    else
                        moveCmd.angular.z = speed;
                }
                cmdPub.publish(moveCmd);
                rate.sleep();
            }
        }catch(...){
            std::cout << robotName <<  " - turn error" << std::endl;
        }
        cmdPub.publish(stop);
        rate.sleep();
    }

     
    void go()
    {
        try
        {
            double dX, dY, pathAng, distance;
            moveCmd.linear.x = linearSpeed;
            while(1){
                ros::spinOnce();
                dX = nextPos.x - nowPosition.x;
                dY = nextPos.y - nowPosition.y;
                distance = std::sqrt(std::pow(dY,2) + std::pow(dX,2));
                if(distance <= 0.02) return;

                pathAng = std::atan2(dY, dX);
                moveCmd.linear.x = std::max(std::min(distance,0.15), 0.1);
                
                if(pathAng >= 0){
                    if(nowPosition.theta <= pathAng && nowPosition.theta >= pathAng - PI)
                        moveCmd.angular.z = 0.2;
                    else
                        moveCmd.angular.z = -0.2;
                }
                else{
                    if(nowPosition.theta <= pathAng + PI && nowPosition.theta > pathAng)
                        moveCmd.angular.z = -0.2;
                    else
                        moveCmd.angular.z = 0.2;
                }
                cmdPub.publish(moveCmd);
                rate.sleep();
            }
        }
        catch(...)
        {
            std::cout << robotName <<  " - go error" << std::endl;
        }
        cmdPub.publish(stop);
        rate.sleep();
    }
};

int main(int argc, char **argv)
{
    ros::init(argc, argv, "burger_control");
    ros::NodeHandle nh;
    // ros::AsyncSpinner spinner(0);
    // spinner.start();
    RobotControl RobotControl(argc, argv, &nh);
    // ros::waitForShutdown();

    return 0;
}