#include "DT_Plot.h"
#include "gnuplot-iostream.h"

std::vector<std::vector<std::string>> dt_plot::readCSV(const std::string& filename) {
    std::vector<std::vector<std::string>> data;
    std::ifstream file(filename);
    std::string line;

    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return data;
    }

    while (std::getline(file, line)) {
        std::vector<std::string> row;
        std::stringstream linestream(line);
        std::string cell;
        while (std::getline(linestream, cell, ',')) {
            row.push_back(cell);
        }
        data.push_back(row);
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
	std::string plotPath = "E:\\ML\\Elevator Git\\Effective-Elevator-Energy-Calculation-for-SejongAI-Center\\Resources\\DTServer\\Log\\Plot\\test.csv";
	std::string cmd = "\\" + plotPath + "\\";
	Gnuplot gp("\"C:\\Program Files\\gnuplot\\bin\\gnuplot.exe\"");

	const std::vector<std::vector<std::string>> data = readCSV(plotPath);
    const auto ed_values = extractED(data);

    gp << "set title 'ED Values Plot'\n";
    gp << "set xlabel 'Index'\n";
    gp << "set ylabel 'ED Value'\n";
    gp << "set yrange [0:*]\n";  // Y축 범위 설정
    gp << "set xtics 1\n";
    gp << "set grid\n";

    for (const auto& building_pair : ed_values) 
    {
        for (const auto& elevator_pair : building_pair.second) 
        {
            std::string elevator_name = elevator_pair.first;
            const auto& ed_values = elevator_pair.second;

            // X축 값을 생성 (0, 1, 2, ...)
            std::vector<int> x_values(ed_values.size());
            for (size_t i = 0; i < x_values.size(); i++) 
            {
                x_values[i] = i;
            }

            std::string color = colors[color_idx % colors.size()];
            color_idx++;

            gp << "plot '-' with linespoints pointtype 7 pointsize 1.5 linecolor rgb '" << color << "' title '" << elevator_name << "'\n";
            gp.send1d(boost::make_tuple(x_values, ed_values));

            std::cin.get(); // pause (enter
        }
    }
}