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
![IFTT_POST](/ProjectThing/Images/IFTT_POST.JPG)
2. Twitter to Adafruit IO. Whenever there was a tweet which used geo loaction and was within the selected area. This event was triggered. This stored the twitter info in Adafruit IO.
![IFTT_Get_vote](/ProjectThing/Images/IFTT_Get_vote.JPG)

# Adafruit IO
Adafruit IO was used to monitor and store any tweets which was triggered on the IoT platform. As the device may not always be connected to the internet there is a sure way to ensure that all data is stored and later can be recieved by the ESP-32 Module.

On Adafruit IO a feed was created which allowed the date to be stored in real time.
![Adafruit](/ProjectThing/Images/Adafruit.JPG)
The feed is public and be accessed by anyone with the url
https://io.adafruit.com/likiths/feeds/esp-32

# ESP-32 Local Server
The local hosted server run a webpage which provided the following Functions
1. To connect to new network
2. To check the status of the network
3. Start an election by sending a tweet out
4. See the results of the election

# Results
![Result_1](/ProjectThing/Images/Result_1.JPG)

Here is an example of the serial monitor on real time data.
![monitor](/ProjectThing/Images/monitor.JPG)

# Limitations
1. The tweet are not filtered so anyone who tweets with the geo location within the IFTTT area will be stored and displayed as a vote.
2. When starting a new election the past votes are not deleted
3. The use of MQTT would have made the system more efficient and have lesser overhead. 
