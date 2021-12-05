# Monitoring-Module
The monitor module has developed to monitor program/system behavior at runtime. (for more information please read Project Definition.docx file)
There are executable files for both unix and windows systems that is called MonitoringModule.app and MonitoringModule.exe respectively.
Note that the application and stateSpace folder should be adjacent to each other at same directory.

When run the application, it read program/system state-space file (*.statespace) which was located on stateSpace folder. after then it do these actions
  1- Create stateSpaceDB folder and generate it's own database there. 
  2- Surfe on program/system states according to input messages. 
  3- Create eventLogs directory and put program/system event logs there. 

NOTE: Watch simple example video file.

in order to run monitoring module application in linux system use these actions:
  1- change unix working directory where MonitoringModule.app application is located. 
  2- do ./MonitoringModule.app 
  
in order to run monitoring module application in windows system just need to double click on MonitoringModule.exe application.

