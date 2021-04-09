# Monitoring-Module
GOALS: 
Monitoring of a program behavior at runtime.
Include an interface for importing a state-space file (*.statespace) into the module.
Produces an output at console (+logging) by observing input signal data.

CONSIDERATIONS:
Input signal data feeds to the module using a given port.
Initiate and identify present state (initial state id="1_0").
Determine next state using input signal data.

Input Format
[event_name(messageserver_title)]

Output Format
[system_time(computer_clock)] [executionTime] [present_state(state_id)] [input] [next_state(state_id)] {[variable_name: value], [variable_name:value], …}
