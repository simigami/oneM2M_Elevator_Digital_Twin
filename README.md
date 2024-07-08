[![img](https://encrypted-tbn0.gstatic.com/images?q=tbn:ANd9GcSYZQNAE3to-qjsjDn49LWVRkb-3tjRhV7Eww&s)](https://www.onem2m.org/)

# Elevator Digital Twin Framework
This project is a digital twin project that allows individuals to easily measure elevator energy consumption<br/>
and analyze various elevator algorithms to optimize and visualize energy based on trip history.

## How to Install

1. install by ZIP or git clone<br/>
2. Available folders exist inside the Resources folder.
3. Install each folder inside the Resources folder according to its role.
4. Set up the ini file for each elevator client. 
5. run oneM2M CSE Server.
6. Run the DT Server.
7. Mount Sensor(Client) Folder on each elevator or building server and run send.py. 

## Client
The client can send data to the DT server in one of two ways
1. each elevator connects _**directly to DT**_ to send data<br/>
2. a server in a building aggregates data from elevators and sends it to _**DT one-to-one**_.<br/><br/>

If the elevators themselves are part of a smart system and don't require sensors to produce data, you only need to configure the ini file and run send.py.

You can create the elevator's data in the form of txt and specify the absolute path to this file in the rts data path option in each client's config.ini.

## Client - Config.ini
The config.ini is a file that stores information for sending to the digital twin from each elevator or a building that collects data from elevators. 

The client is responsible for performing an Init process with the DT server at runtime to create the correct instance object on the DT server. 

At this time, the client creates building and elevator information based on the information in the config.ini file.

Without the Init process, elevator objects are not created or used on the DT server and oneM2M CSE.

## Client - rts.txt
The elevator data must be stored in the form of txt, and the txt file must be written according to certain rules.<br/><br/>

For RTS, the moment it is written is considered the time of the event, so there is no timestamp information in the txt file.

send.py automatically adds time information when new information is written and sends it to the DT server.

If you collect data in a different way, you will need to write your own code to convert that data to fit into the txt file. 

The txt file is formatted as follows

IN or OUT: The first word on each line. 
- IN is used to hold a list of buttons pressed inside the elevator.
- OUT is used to list elevator calls from outside the elevator.

NAME: Indicates which elevator the call occurred in. Use only in the case of IN.

[int1, int2, int3...]: Records the floor number of the button pressed inside the elevator. The array must be sorted in ascending order, with '-' for basement floors. Use only in the case of IN.

- EX) 3rd basement floor = -3
- EX) 12th floor = 12

[[int1, bool1], [int2, bool2]...] : Records the floor and direction called from outside the elevator.<br/> The array should be sorted in ascending order by the number of floors, with True coming first if the same floor was called in different directions.<br>True means the direction is up, and False means the direction is down.

- EX) Calling down from the 2nd floor = [-2, False]
- EX) Calling up from the 4th floor = [4, True]

In the end, line 1 of each txt file should look like one of the following
- IN NAME [int1, int2, int3...]
- OUT [[int1, bool1], [int2, bool2] ...]