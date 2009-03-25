import sys
__author__="Meseret R. Gebre"
__date__  ="Mar 25, 2009"
__version__="0.1"

"""
This is the muse code generator module.
Here we create the project and actually create agents/events/states on command.
This generator will save lots of time by automating repetative work.
This generator is really flexiable and can easily be enhanced. There are no real
checks and this generator is not 100% robust, if you try to break it, it will
break without a fight. The idea here is to learn how to use it and use it
correctly. Feel free to extend modify this any way you see fit.
TAKE THIS CODE AS-IS.
Please leave everything above this line as is. If you modify or add anything
add what you modified or extended belowe.
"""

from sys import argv
import os
import templates

possible_action_command = ['create']
possible_commands = ['project','agent','state','event']

def usage():
    return """Welcome to the muse code generator help menu.

    --help - brings you here!

    CREATE PROJECT USAGE - should be called FIRST:
    python muse.py create project <project_name_here>

    CREATE AGENT USAGE:
    python muse.py create agent <agent_name_here>

    CREATE STATE USAGE:
    python muse.py create state <state_name_here>

    CREATE EVENT USAGE:
    python muse.py create event <event_name_here>

    NOTES aka READ FIRST:
    You should call the create project command first before creating 
    any classes. As a matter of fact you change directory into your 
    project to generate any class via the muse code generator.
    """
 
def create_project(project_name, parent_directory=os.getcwd()):
    """ This method is a helper to generate the directory structure for a typical muse simulation project.
        Please do not modify this in any way. Once this is called it should not be called again.

        @param project_name, this is a string that contains the desired project name to create
        @param parent_directory, by default it's the current working directory. where the project will reside.
    """
    print "Creating Project .."+project_name
    os.makedirs(parent_directory+os.sep+project_name+os.sep+"agents"+os.sep+"src")
    print "Created .."+parent_directory+os.sep+project_name+os.sep+"agents"+os.sep+"src"
    os.makedirs(parent_directory+os.sep+project_name+os.sep+"agents"+os.sep+"include")
    print "Created .."+parent_directory+os.sep+project_name+os.sep+"agents"+os.sep+"include"
    os.makedirs(parent_directory+os.sep+project_name+os.sep+"events"+os.sep+"src")
    print "Created .."+parent_directory+os.sep+project_name+os.sep+"events"+os.sep+"src"
    os.makedirs(parent_directory+os.sep+project_name+os.sep+"events"+os.sep+"include")
    print "Created .."+parent_directory+os.sep+project_name+os.sep+"events"+os.sep+"include"
    os.makedirs(parent_directory+os.sep+project_name+os.sep+"states"+os.sep+"src")
    print "Created .."+parent_directory+os.sep+project_name+os.sep+"states"+os.sep+"src"
    os.makedirs(parent_directory+os.sep+project_name+os.sep+"states"+os.sep+"include")
    print "Created .."+parent_directory+os.sep+project_name+os.sep+"states"+os.sep+"include"
    #this is create for agents oSimStream temp files. Please do not delete
    os.makedirs(parent_directory+os.sep+project_name+os.sep+"temp")
    print "Created .."+parent_directory+os.sep+project_name+os.sep+"temp"

def create_agent(agent_names=[]):
    """ This method will generate agents headers and cpp that are subclasses of
    muse::Agent class.

    @param agent_names, a list of agent names to create header and cpp files for.
    """
    print "Creating agents: "
    for agent in agent_names:
        print "...["+agent+".h | "+agent+".cpp]"

def create_state(state_names=[]):
    """ This method will generate states headers and cpp that are subclasses of
    muse::State class.

    @param state_names, a list of state names to create header and cpp files for.
    """
    print "Creating states: "
    for state in states_names:
        print "...["+state+".h | "+state+".cpp]"

def create_event(event_names=[]):
    """ This method will generate events headers and cpp that are subclasses of
    muse::Event class.

    @param event_names, a list of event names to create header and cpp files for.
    """
    print "Creating events: "
    for event in event_names:
        print "...["+event+".h | "+event+".cpp]"



def action_create(command, command_arg=[]):
    """This method handles the action 'create'

    @param command, the command that that create should execute.
    @param command_arg, the argument for the command
    """

    #basic checking here, lets make sure command is valid
    if possible_commands.count(command) == 0:
        print "command: ["+command+"] is not a valid command. use --help for usage.";
        sys.exit(1);

    if command == 'project':
        create_project(command_arg[0])
    elif command == 'agent':
        create_agent(command_arg)
    elif command == 'state':
        create_state(command_arg)
    elif command == 'event':
        create_event(command_arg)


def execute_action(action_command, command, command_arg=[]):
    """ This method figure out what action to execute based on argv input. Also does light error checking.

    @param action_command, the action you wish to execute, like 'create'
    @param command, the command to call with the given action, like 'project'
    @param command_arg, the argument list to pass in for the command, like ['project name']
    """

    #basic checking here, lets make sure command is valid
    if possible_action_command.count(action_command) == 0:
        print "action command: ["+action_command+"] is not a valid action command. use --help for usage.";
        sys.exit(2);

    if action_command == 'create':
        action_create(command, command_arg)

#make sure there are not asking for help
if len(argv) >= 2 and argv[1] == '--help':
    print usage();
    sys.exit(0);

#check if we have enough args
if len(argv) >= 4:
    #we need to get the arguments and execute the action
    action_command = argv[1]
    command        = argv[2]
    command_arg    = argv[3:]
    execute_action(action_command, command, command_arg)
else:
    print "Error: not enough args. use --help for usage.";


