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

std::map<std::string, std::unordered_map<std::string, std::vector<double>>> dt_plot::extractED(const std::vector<std::vector<std::string>>& data) {
    std::map<std::string, std::unordered_map<std::string, std::vector<double>>> processedData;

    if (data.empty()) return processedData;

    for (size_t i = 1; i < data.size(); ++i) {
        const auto& row = data[i];
        std::string building = row[COL_BLDG];
        std::string elevator = row[COL_EV];
        double ed_value = std::stod(row[COL_ED]);

        processedData[building][elevator].push_back(ed_value);
    }

    return processedData;
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

    /*
    * std::map<int, std::string> colMap;
    int col = 0;
    int OpTimeColNum, ERDColNum, ESDColNum, EDColNum;
    while (std::getline(file, line)) {
        if (isFirstLine) {
            isFirstLine = false;

			std::istringstream ss(line);
			std::string token;
			std::vector<std::string> tokens;

            while (std::getline(ss, token, ',')) {
                colMap[col] = token;
                col++;
			}

            for (const auto& pair : colMap) {
                if (pair.second == "OpTime") {
					OpTimeColNum = pair.first;
				}
                else if (pair.second == "ERD") {
					ERDColNum = pair.first;
				}
                else if (pair.second == "ESD") {
					ESDColNum = pair.first;
				}
                else if (pair.second == "ED") {
					EDColNum = pair.first;
				}
			}

            continue; // Skip header line
        }

        std::istringstream ss(line);
        std::string token;
        std::vector<std::string> tokens;

        while (std::getline(ss, token, ',')) {
            tokens.push_back(token);
        }

        if (tokens.size() >= 16) { // Ensure there are enough columns
            DataPoint dp;
            dp.OpTime = std::stod(tokens[OpTimeColNum]);
            dp.ERD = std::stod(tokens[ERDColNum]);
            dp.ESD = std::stod(tokens[ESDColNum]);
            dp.ED = std::stod(tokens[EDColNum]);
            dataPoints.push_back(dp);
        }
    }

    file.close();

    // Prepare data for plotting
    std::vector<std::pair<double, double>> ERD_data;
    std::vector<std::pair<double, double>> ESD_data;
    std::vector<std::pair<double, double>> ED_data;

    for (const auto& dp : dataPoints) {
        ERD_data.push_back(std::make_pair(dp.OpTime, dp.ERD));
        ESD_data.push_back(std::make_pair(dp.OpTime, dp.ESD));
        ED_data.push_back(std::make_pair(dp.OpTime, dp.ED));
    }

    // Plot the data
    gp << "set title 'SejongAI EV1'\n";
    gp << "set xlabel 'OpTime'\n";
    gp << "set ylabel 'Value'\n";
    gp << "set ytics 10000\n";
    gp << "set grid\n";
    gp << "plot '-' with lines title 'ERD', '-' with lines title 'ESD', '-' with lines title 'ED' lw 3\n";
    gp.send1d(ERD_data);
    gp.send1d(ESD_data);
    gp.send1d(ED_data);
    */
    
    const auto Data = readCSV(plotPath);

    gp << "set title 'Elevator Operation Time vs Distance'\n";
    gp << "set xlabel 'Operation Time (OpTime)'\n";
    gp << "set ylabel 'Elevator Distance (ED)'\n";
    gp << "set yrange [0:]\n";
    gp << "set grid\n";
    gp << "set style line 1 lc rgb 'red' pt 7 ps 1\n";   // Red for EV1
    gp << "set style line 2 lc rgb 'green' pt 7 ps 1\n"; // Green for EV2
    gp << "set style line 3 lc rgb 'blue' pt 7 ps 1\n";  // Blue for EV3

    gp << "plot '-' with linespoints linestyle 1 title 'EV1', "
        "'-' with linespoints linestyle 2 title 'EV2', "
        "'-' with linespoints linestyle 3 title 'EV3'\n";

    for (const auto& EachElevator : Data)
    {
        for (const auto& point : EachElevator.second)
        {
            gp << point.OpTime << " " << point.ED << "\n";
        }
        gp << "e\n";
    }

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
    gp << "replot\n";
    gp.flush();

    return;
}