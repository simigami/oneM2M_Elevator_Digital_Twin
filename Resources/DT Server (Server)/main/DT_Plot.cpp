#include "DT_Plot.h"
#include "gnuplot-iostream.h"

struct DataPoint {
    double OpTime;
    double ERD;
    double ESD;
    double ED;
};

std::map<std::string, std::vector<DataPoint>> readCSV(const std::string& filename) {
    std::ifstream file(filename);
    std::map<std::string, std::vector<DataPoint>> data;
    std::string line, buildingName, elevatorName, command, labels, temp;
    double datetime, year, month, day, hour, minute, second, optime, currentFloor, velocity, altimeter, distance, erd, esd, ed;

    // Skip the header line
    std::getline(file, line);

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::getline(ss, buildingName, ',');
        std::getline(ss, elevatorName, ',');
        ss >> datetime; ss.ignore();
        ss >> year; ss.ignore();
        ss >> month; ss.ignore();
        ss >> day; ss.ignore();
        ss >> hour; ss.ignore();
        ss >> minute; ss.ignore();
        ss >> second; ss.ignore();
        ss >> optime; ss.ignore();
        std::getline(ss, command, ',');
        ss >> currentFloor; ss.ignore();
        ss >> velocity; ss.ignore();
        ss >> altimeter; ss.ignore();
        ss >> distance; ss.ignore();
        ss >> erd; ss.ignore();
        ss >> esd; ss.ignore();
        ss >> ed; ss.ignore();
        std::getline(ss, labels, ',');

        data[elevatorName].push_back({ optime, erd, esd, ed });
    }
    return data;
}

void dt_plot::run()
{
	//std::string plotPath = "E:\\ML\\Elevator Git\\Effective-Elevator-Energy-Calculation-for-SejongAI-Center\\Resources\\DTServer\\Log\\Plot\\test2.csv";
	std::string plotPath = fs.chooseLog();

    std::string temp = plotPath.substr(0, plotPath.find_last_of("\\"));
    std::string plotDir = temp.substr(0, temp.find_last_of("\\")) + "\\Plot";

	Gnuplot gp("\"C:\\Program Files\\gnuplot\\bin\\gnuplot.exe\"");
    std::vector<DataPoint> dataPoints;

    std::ifstream file(plotPath);
    std::string line;
    bool isFirstLine = true;

    if (!file.is_open()) {
		std::cerr << "Error opening file: " << plotPath << std::endl;
		return;
	}
    
    const auto Data = readCSV(plotPath);
    std::vector<std::string> colors = { "red", "green", "blue" };
    std::string line_style = "'-' with linespoints linestyle ";
    std::string line_color = "set style line";
    const int max_ev = Data.size();

    gp << "set title 'Elevator Operation Time vs Distance'\n";
    gp << "set xlabel 'Operation Time (OpTime)'\n";
    gp << "set ylabel 'Elevator Distance (ED)'\n";
    gp << "set yrange [0:]\n";
    gp << "set grid\n";
    
    //gp << "set style line 1 lc rgb 'red' pt 7 ps 1\n";   // Red for EV1
    //gp << "set style line 2 lc rgb 'green' pt 7 ps 1\n"; // Green for EV2
    //gp << "set style line 3 lc rgb 'blue' pt 7 ps 1\n";  // Blue for EV3

    int i = 1;
    for (auto iter : Data)
    {
        gp << (line_color + " " + std::to_string(i) + " lc rgb '" + colors[i-1] + "' pt 7ps 1\n");
    }

    std::vector<std::string> cmds;
    i = 1;
    for (auto iter : Data)
    {
        if (i == 1)
        {
            gp << "plot " << line_style << i << " title '" << iter.first << "',";
        }
        else if (i != Data.size())
        {
            gp << line_style << i << " title '" << iter.first << "',";
        }
        else
        {
            gp << line_style << i << " title '" << iter.first << "'\n";
        }
        i++;
    }

    for (const auto& EachElevator : Data)
    {
        for (const auto& point : EachElevator.second)
        {
            gp << point.OpTime << " " << point.ED << "\n";
        }
        gp << "e\n";
        continue;
    }

    // Show Example Plot
    gp << "replot\n";
    gp.flush();

    // SAVE PLOT AS SVG
    std::string svgName = "";
    std::cout << "Please Enter svg File Name, Default Will be timestamp : ";
    std::cin >> svgName;

    if (svgName.empty()) {
        // Set File Name as Timestamp
        auto now = std::chrono::system_clock::now();
        auto now_time_t = std::chrono::system_clock::to_time_t(now);

        struct tm now_tm;
        localtime_s(&now_tm, &now_time_t);

        std::stringstream ss;
        ss << std::put_time(&now_tm, "%Y-%m-%d_%H-%M-%S");
        svgName = ss.str();
	}

    std::string svgPath = plotDir + "\\" + svgName + ".svg";
    gp << "set terminal svg\n";
    gp << "set output '" << svgPath << "'\n";

    return;
}