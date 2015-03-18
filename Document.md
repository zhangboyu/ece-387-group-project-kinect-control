# Document #
(This page holds everything about this project except the code.)

(All the resources I mentioned can be found in the "reference" page.)

(The code is in the "Source"->"Browse" tab, but you need to switch the repository to "wiki".)

(Processing code name: "draw\_board.pde")

(Arduino Mega code: "final\_project\_kinect\_tetris.ino")

(Arduino Uno code: "final\_project\_mp3.ino")
## This wiki page is organized into following sections. ##
## 1.General information about this project. ##
## 2.Kinect and Processing. ##
## 3.Arduino Mega 2560 and TFT touch screen. ##
## 4.Arduino Uno and MP3 shield. ##
## 5.Demo. ##
## 6.Possible improvement in future. ##
## 7.Material list and estimate cost. ##
**---------------------------------------------------------------------------------------------------------------------**
## 1.General information about this project. ##

We build a drawing board and a kinect controlled MP3 player in this project by using Kinect, Processing, OpenNI, NITE, Arduino Mega 2560, TFT touch screen, Arduino Uno, and MP3 shield. Generally speaking, we used Kinect to capture the depth image of the surrounding real world. The Processing is used to handle the data sent from Kinect. The middleware OpenNI and NITE are used to do the data processing. After the raw data has been processed by Processing, Processing send some specific data to Arduino Mega 2560. Arduino Mega 2560 uses this data either to control the drawing board or the Arduino Uno by serial port. The Arduino Uno then control the MP3 shield.

## 2.Kinect and Processing. ##

![http://www.cngulu.com/wp-content/uploads/2013/11/kinect.jpg](http://www.cngulu.com/wp-content/uploads/2013/11/kinect.jpg)

Kinect is actually a advanced sensor. It provides tons of data with includes the RGB information and the depth information of every point that Kinect can "see." This information is extracted by OpenNI, a middleware used to interface with Kinect. However, the data received by OpenNI is raw data which cannot be used directly in our project. We need something to manipulate this raw data so that we can track hands and skeleton. Although it is theoretically possible for us to write our own algorithm to process this raw data, it definitely beyond our current stage and will take very long time to develop it. Fortunately, there are some middlewares have already done this job, and one of them is NITE. NITE is another middleware besides OpenNI. It takes the raw data from OpenNI and do the processing job, which figures out hands, gestures, and skeleton. Then NITE feed this processed information back to OpenNI, and we interface with OpenNI to get such information. In this way, we can easily implement fancy functions like hands tracking, finger detecting, and skeleton tracking. The image below shows the relationships between Kinect, OpenNI, NITE, and application.


![http://yannickloriot.com/wp-content/uploads/2011/03/OpenNi-Architecture.png](http://yannickloriot.com/wp-content/uploads/2011/03/OpenNi-Architecture.png)

OpenNI used to be open source software and NITE was not open source but can be used freely. However, just few weeks ago, Apple brought the company behind OpenNI and the official website of OpenNI cannot be visited any more. Fortunately, in Processing there is a library called "SimpleOpenNI" which wraps OpenNI and NITE and provides almost all the functionalities of the original OpenNI and NITE. So, we used Processing in our project. Processing is a programing language that enables us to prototype and create GUI in a very easy way. Processing can create a lot of fancy applications on it own, but in our project we only used it (and its libraries) to communicate with Kinect and send data to Arduino Mega 2560.

The Processing program of our project is based on one of the examples came with SimpleOpenNI library and another example came with "fingertracker" library. In the remaining of this paragraph, I'll briefly go through mechanism behind the parts that we used of these libraries and also the code we used to talk with Arduino Mega 2560.

1). We used an example from SimpleOpenNI library and modified it to track hands. In this part of code, we created a object called "context" which is the core of all the functionalities about the gestures recognition and hands tracking. There is a type of function called "callback function" which works very similar to the interrupt in the hardware area. This type of function is triggered and the code within it is executed when a certain type of event is detected. The functions in our code  such as "onNewHand," "onTrackedHand," "onLostHand," and "onCompletedGesture" belong to this type of function. For example, when hand is recognized by Kinect, the function "onNewHand" is triggered and the code in this block is executed. We maintained a HashMap which uses Integer as "Key" and an ArrayList of PVector as its "Value" to store the information about the hand ID and hand position. Once a hand is detected, a new "Key" and a new "Value" are put into this HashMap. During the tracking period, the position of hand is consistently been put into the ArrayList of PVector. But when the length of the ArrayList grows to 20, it will remove the oldest PVector to store a new one. By maintain a ArrayList of certain length, we can remember and show the trace of hands. When a new hand is detected, the program will create a new "Key" and a new "Value" to store the information about the new hand. Thus we can distinguish different hand and its position.

2). We used an example from "fingertracker" library to detected whether the hand is open or not. The mechanism behind this library is that it first figures out the contour of your hands (actually any point that has the same predefined distance with Kinect will be detected and the contour is formed by linking the outest points together), and second points out the endpoint of your fingers by detecting the small "U" shape of the contour (the small "U" shape in the contour is like the shape of our fingers). One issue related to distance detecting about Kinect is that the precision of Kinect's distance detecting is not linear, which means the closer between the object and Kinect, the more precise distance results you will get. Thus it was suggested in that example that the contour we want to recognized should not be too far from Kinect to get a accurate result. There are two parameters in the code which have been set to a optimized value to enhance user experience. The method that returns the number of fingers detected was used to determine whether the hand is open or not. However, the detection of the endpoints of fingers is not very reliable. Thus, although the methods in this library can return how many fingers are detected and the position of those fingers, these data is not very easy to use.

![http://farm9.staticflickr.com/8461/7967853830_b998647136.jpg](http://farm9.staticflickr.com/8461/7967853830_b998647136.jpg)

3). We used the "Serial.write" function in Processing and the "Serial.read" function in Arduino to let them talk. Since we need to send the x and y coordinate of hands, and whether fingers are detected to Arduino, we need to let the Arduino know which byte it reads belongs to which part of information. We created our own protocol to let Processing talk to Arduino. At the beginning, we attached a character 'P' at the beginning of real information. However, sometimes part of the real information has the same binary representation of 'P.' Thus the part of real information will lose and 'P' will be treated as part of real information. To solve this problem, we increased the header of the real information into three characters, 'POS'. By doing this, the chance that the real information will confused with header is very little. So we can get reliable information.

## 3.Arduino Mega 2560 and TFT touch screen. ##

![http://arduino.cc/en/uploads/Main/ArduinoMega.jpg](http://arduino.cc/en/uploads/Main/ArduinoMega.jpg)

![http://www.seeedstudio.com/depot/images/product/2.8Touch%20Shield.jpg](http://www.seeedstudio.com/depot/images/product/2.8Touch%20Shield.jpg)

In this section, I will address some issues including the general idea behind the whole Arduino Mega code, the refresh issue of the screen, the mechanism behind the drawing board, and the slide gestures recognition.

1). The general idea behind the whole Arduino code is borrowed from finite state machine. Every time you see a new picture shown on the screen, it is a state of the finite state machine. For example, the "GENERAL\_WELCOME," "DRAWING\_MODECHOOSE," and so on are different state of the finite state machine. Once certain events are triggered, the variable "arduinoState" is changed, and the program jumps to another state.

2). On the main page where you choose which game you want to enter, there is a little blue dot that is blinking to show where you hand is. At the beginning, the ideal performance we imagine was to use a little dot that is not blinking to show the position of hand. However, during the developing process, we found that if we refresh the entire screen every time the hand's position changed, the frame rate would drop to a very low value and the dot would not show up at all. The solution we used to solve this problem was to divide the entire screen into three or four areas, and just refresh the area into which the last hand's position was fell. In this way, the frame rate was significantly increased and the blue dot appeared, although blinking.

3). The features of this drawing board including a indicator on the upper right corner to show whether the hand is open or not (draw when the hand is open, move the stroke to another place when the hand is close), change the stroke weight and stroke color, and choose the mode of drawing (dot or line). The state of the indicator is changed according to the information sent by Processing (as I mentioned above, this piece of information is the first byte after the "header"). As for the drawing, at the "dot" mode, it actually is superposition of different dots, so you will see the dots are separated with each other when you wave your hand quickly. Also because the actual implementation of this functionality, multiple hand can be detected by the drawing board and draw without affecting each other. At the "line" mode, the program will link the current dot and the last dot together by a straight line to make a little bit smoother picture. The picture can be smoother if we adopt a three or four order approximation, but the problem is the heavy burden brought to the microprocessor and the screen. Since the background color of the drawing board is white, when you move (not draw) the stroke to another position, the last position will be filled with a white circle to eliminate the residual image. The problem with this approach is that sometimes it will erase some parts of the image you want to keep. Due to the hardware limitations, we really cannot do anything to improve it. As for the changing of stroke weight and color, it is just a changing of the radius and color of the circle drawn on the screen.

4). For the slide gesture recognition, we maintained two arrays and set a threshold to determine the slide gestures. The hardest part is not the logic to figure out the slide gesture, but to calibrate the length of those arrays and the value of the threshold. The reason of why it is important to choose a proper value and why it is hard to calibrate are that these two values together determine at which speed and amplitude your gesture can be detected and they can only be calibrated by trial and error. Finally, we found a set of proper value for them.

## 4.Arduino Uno and MP3 shield. ##

![http://arduino.cc/en/uploads/Main/ArduinoUno_r2_front450px.jpg](http://arduino.cc/en/uploads/Main/ArduinoUno_r2_front450px.jpg)

![http://www.billporter.info/wp-content/uploads//2012/01/10628-01b.jpg](http://www.billporter.info/wp-content/uploads//2012/01/10628-01b.jpg)

There is really not much things to say about this shield because the example came with this shield is very powerful and we actually used that example to control the MP3 shield. There are four serial port on Arduino Mega, and you can use one of the other three ports (the default port is used to communicate with Processing) to communicate with Uno. The only thing we think is worth to be talked at here is that when you connect the Arduino Mega with Uno by using the serial port, you need to make sure that they have common ground. If not, something very weird will happen.

## 5. Demo. ##

<a href='http://www.youtube.com/watch?feature=player_embedded&v=qQDzbAa9wEc' target='_blank'><img src='http://img.youtube.com/vi/qQDzbAa9wEc/0.jpg' width='425' height=344 /></a>

## 6.Possible improvement in future. ##

1). Using more powerful screen to make the display more beautiful and steady.

2). Improve the method for detecting the slide gestures.

3). Using more powerful microprocessor to solve the problem about residual image.

4). Show more information about the song that is playing on the screen.

5). Use higher order approximation method to make the lines drawn on the screen smoother.

6). Improve the finger detection so that we can get more reliable results about the fingers. After that, we may use that to build some cool stuff.

## 7. Material list and estimate cost. ##

1). Kinect for Windows--------------------------------------$200.

2). Arduino Mega 2560--------------------------------------$25.

3). Arduino Uno------------------------------------------------$18.

4). SeeedStudio TFT touch screen---------------------$40.

5). SparkFun MP3 shield----------------------------------$40.