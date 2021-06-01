# COM3505 Lab 2 Peer-to-Peer Voting System

The challenge with this project is to provide mechanisms to prevent vote stuffing, interception and ensure privacy of votes.
Twitter was used to store and access votes. IFTTT platform was used to make events to post to twitter and get any votes from a perticular place.
ESP-32 runs a local web server which can be connect to and the results of the election can be accessed from there.

Overview of how the System works
![Activity_diagram](/ProjectThing/Images/Activity_diagram.png)

# Functions
1. Connect to ESP-32 local server to send a tweet to post to twitter and start election
2. Get ESP-32 device location
3. Get a List of votes or tweets in that locality

# IFTTT 
IFTTT stands for “if this, then that.” https://ifttt.com/ was used to create two Applets which acted as triggers whenever a webhook was called or tweet was posted at a certain location.

1. Webhook to Twitter. This Applet was used to POST to twitter to notify when a election was started
![IFTT_POST](/ProjectThing/Images/IFTT_POST.jpg)
2. Twitter to Adafruit IO. Whenever there was a tweet which used geo loaction and was within the selected area. This event was triggered. This stored the twitter info in Adafruit IO.
![IFTT_Get_vote](/ProjectThing/Images/IFTT_Get_vote.jpg)
