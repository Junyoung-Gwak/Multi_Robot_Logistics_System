#include <ros/ros.h>
#include <random>
#include <ssapang/Task.h>
#include <ssapang/TaskList.h>
#include <ssapang/str.h>
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
        taskPub = nh->advertise<ssapang::TaskList>( "/burger/right_task_list", 1);
        makePathPub = nh->subscribe<ssapang::str>("/right_make_task", 10, &Task::callback, this);
    }

private:
    ros::Publisher taskPub;
    ros::Subscriber makePathPub;
    std::string startNode, endNode;
    ssapang::Task task;
    ssapang::TaskList taskList;
    std::string sectTaskList[9][4] = {
        {"BP0107", "BP0108", "BP0207", "BP0208"},
        {"BP0109","BP0110","BP0209","BP0210"},
        {"BP0111","BP0112","BP0211","BP0212"},
        {"BP0307", "BP0308", "BP0407", "BP0408"},
        {"BP0309","BP0310","BP0409","BP0410"},
        {"BP0311","BP0312","BP0411","BP0412"},
        {"BP0507", "BP0508", "BP0607","BP0608"},
        {"BP0509","BP0510","BP0609","BP0610"},
        {"BP0511","BP0512","BP0611","BP0612"}
    };

    std::string destination[4] = {
        "BO0109","BO0111","BO0113","BO0115",
    };

    void callback(const ssapang::str::ConstPtr &msg){
        for(int i = 0; i < 20; i++){
            task.product = sectTaskList[rand()%9][rand()%4];
            task.destination = destination[rand()%4];
            taskList.list.push_back(task);
        }

        taskPub.publish(taskList);
    }
};


int main(int argc, char **argv)
{
    ros::init(argc, argv, "right_burger_task_pub");
    ros::NodeHandle nh;
    Task burger = Task(&nh);
    ros::spin();

    return 0;
}