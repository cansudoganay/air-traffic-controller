
#ifndef ATCLog
#define ATCLog

#include <vector>
#include <iterator>
#include <iomanip>
#include <sys/time.h>
#include <fstream>

using namespace std;

typedef struct plane{
  int planeID; 
  char status;
  time_t runAwayTime;
  time_t arrivalTime;
}plane;

class SimulationLog{
private:
    struct LogData{
        int planeID;
        char status;
        int requestTime;
        int runwayTime;
        int turnAroundTime;
    }typedef LogData;
    vector<LogData> logItems;
public:
    SimulationLog(){}
    ~SimulationLog(){}
    void addItem(plane p){
        LogData item;
        item.planeID = p.planeID;
        item.status = p.status;
        item.requestTime = p.arrivalTime;
        item.runwayTime = p.runAwayTime;
        item.turnAroundTime = item.runwayTime - item.requestTime;
        logItems.push_back(item);
    }
    void printItems(){
        vector<LogData>::iterator traveller;
        cout << "PlaneID" << "\t" << "Status" << "\t" << "Request time" << "\t";
        cout << "Runaway Time" << "\t" << "Turnaround Time" << endl;
        cout << "-------\t------\t------------\t-------------\t---------------" << endl;
        for(traveller = logItems.begin(); traveller < logItems.end(); traveller++){
            cout << setw(3) << traveller->planeID << "\t";
            cout << setw(3) << traveller->status << "\t";
            cout << setw(6) << traveller->requestTime << "\t\t";
            cout << setw(6) << traveller->runwayTime << "\t\t";
            cout << setw(7) << traveller->turnAroundTime << endl;
        }
    }
    void saveLogFile(){
        ofstream outfile;
        outfile.open("planes.log");
        vector<LogData>::iterator traveller;
        outfile << "PlaneID" << "\t" << "Status" << "\t" << "Request time" << "\t";
        outfile << "Runaway Time" << "\t" << "Turnaround Time" << endl;
        outfile << "-------\t------\t------------\t-------------\t---------------" << endl;
        for(traveller = logItems.begin(); traveller < logItems.end(); traveller++){
            outfile << setw(3) << traveller->planeID << "\t";
            outfile << setw(3) << traveller->status << "\t";
            outfile << setw(6) << traveller->requestTime << "\t\t";
            outfile << setw(6) << traveller->runwayTime << "\t\t";
            outfile << setw(7) << traveller->turnAroundTime << endl;
        }
        outfile.close();
    }
};

#endif