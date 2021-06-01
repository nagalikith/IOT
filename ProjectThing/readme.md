# COM3505 Lab 2 Peer-to-Peer Voting System

The challenge with this project is to provide mechanisms to prevent vote stuffing, interception and ensure privacy of votes.
Twitter was used to store and access votes. IFTTT platform was used to make events to post to twitter and get any votes from a perticular place.
ESP-32 runs a local web server which can be connect to and the results of the election can be accessed from there.

Overview of how the System works
![Image 1](/TouchyThing/Images/Activity_diagram.png)

#Functions
1. Connect to ESP-32 local server to send a tweet to post to twitter and start election
2. Get ESP-32 device location
3. Get a List of votes or tweets in that locality

#IFTTT 
