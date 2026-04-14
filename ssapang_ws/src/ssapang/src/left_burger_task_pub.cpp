#include <ros/ros.h>
#include <random>
#include <ssapang/Task.h>
#include <ssapang/TaskList.h>
#include <time.h>
#include <unistd.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>

using namespace std;

class Task
{
public:
    Task(ros::NodeHandle *nh){
        taskPub = nh->advertise<ssapang::TaskList>("/burger/left_task_list", 1);

        ros::Rate loop_rate(10);
        sleep(3);
        for(int i = 0; i < 20; i++){
            task.product = sectTaskList[rand()%9][rand()%4];
            task.destination = destination[rand()%4];
            taskList.list.push_back(task);
        }

        taskPub.publish(taskList);
    }

private:
    ros::Publisher taskPub;
    std::string startNode, endNode;
    ssapang::Task task;
    ssapang::TaskList taskList;
    std::string  sectTaskList[9][4] = {
        {"BP0101", "BP0102", "BP0201", "BP0202"},
        {"BP0103","BP0104", "BP0203","BP0204"},
        {"BP0105","BP0106", "BP0205","BP0206"},
        {"BP0301", "BP0302", "BP0401", "BP0402"},
        {"BP0303","BP0304", "BP0403","BP0404"},
        {"BP0305","BP0306", "BP0405","BP0406"},
        {"BP0501", "BP0502", "BP0601", "BP0602"},
        {"BP0503","BP0504", "BP0603","BP0604"},
        {"BP0505","BP0506", "BP0605","BP0606"}
    };

    std::string destination[4] = {
        "BO0102","BO0104","BO0106","BO0107",
    };
};


int main(int argc, char **argv)
{
    ros::init(argc, argv, "left_burger_task_pub");
    ros::NodeHandle nh;
    Task burger = Task(&nh);
    ros::spin();

    return 0;
}