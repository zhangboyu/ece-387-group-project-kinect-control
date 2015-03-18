import java.util.Map;
import java.util.Iterator;
import processing.serial.*;
import SimpleOpenNI.*;
import fingertracker.*;


SimpleOpenNI context;
Serial myPort;
FingerTracker fingers;

int counter;
int threshold = 625;
int handVecListSize = 20;
Map<Integer,ArrayList<PVector>>  handPathList = new HashMap<Integer,ArrayList<PVector>>();
color[]       userClr = new color[]{ color(255,0,0),
                                     color(0,255,0),
                                     color(0,0,255),
                                     color(255,255,0),
                                     color(255,0,255),
                                     color(0,255,255)
                                   };
void setup()
{
//  frameRate(200);
  
  size(640,480);

  context = new SimpleOpenNI(this);
  if(context.isInit() == false)
  {
     println("Can't init SimpleOpenNI, maybe the camera is not connected!"); 
     exit();
     return;  
  }   

  // enable depthMap generation 
  context.enableDepth();
  
  // disable mirror
  context.setMirror(true);

  // enable hands + gesture generation
  //context.enableGesture();
  context.enableHand();
  context.startGesture(SimpleOpenNI.GESTURE_WAVE);
  
  // set how smooth the hand capturing should be
  context.setSmoothingHand(.25);
  fingers = new FingerTracker(this, 640, 480);
  fingers.setMeltFactor(100);
  
  String portName = Serial.list()[5];
  myPort = new Serial(this, portName, 115200);
 }

void draw()
{
  // update the cam
  context.update();

  image(context.depthImage(),0,0);
  
  //this block of code implemented the finger contour
  fingers.setThreshold(threshold);
  int[] depthMap = context.depthMap();
  fingers.update(depthMap);
  stroke(0,255,0);
  for (int k = 0; k < fingers.getNumContours(); k++) {
    fingers.drawContour(k);
  }
  noStroke();
  fill(255,0,0);
  for (int i = 0; i < fingers.getNumFingers(); i++) {
    PVector position = fingers.getFinger(i);
    ellipse(position.x - 5, position.y -5, 10, 10);
  }
  
  // show the threshold on the screen
  fill(255,0,0);
  text(threshold, 10, 20);
  int fingerNum = fingers.getNumFingers();
  //println(fingerNum);
  //end
    
  // draw the tracked hands
  if(handPathList.size() > 0)  
  {    
    Iterator itr = handPathList.entrySet().iterator();     
    while(itr.hasNext())
    {
      Map.Entry mapEntry = (Map.Entry)itr.next(); 
      int handId =  (Integer)mapEntry.getKey();
      ArrayList<PVector> vecList = (ArrayList<PVector>)mapEntry.getValue();
      PVector p;
      PVector p2d = new PVector();
      
        stroke(userClr[ (handId - 1) % userClr.length ]);
        noFill(); 
        strokeWeight(1);        
        Iterator itrVec = vecList.iterator(); 
        beginShape();
          while( itrVec.hasNext() ) 
          { 
            p = (PVector) itrVec.next(); 
            
            context.convertRealWorldToProjective(p,p2d);
            vertex(p2d.x,p2d.y);
          }
        endShape();   
  
        stroke(userClr[ (handId - 1) % userClr.length ]);
        strokeWeight(4);
        p = vecList.get(0);
        context.convertRealWorldToProjective(p,p2d);
        point(p2d.x,p2d.y);
        if(p2d.x >= 140 && p2d.x <= 500){
          int screenX = (int)map(p2d.x,140,500,0,240);
          int screenY = (int)map(p2d.y,0,480,0,320);
          int screenY1 = screenY >> 8;
          int screenY2 = screenY & 0xFF;
          myPort.write('P');
          myPort.write('O');
          myPort.write('S');
          if(fingerNum >= 3){
            myPort.write(1);
            println("port write 1");
          }
          else{
            myPort.write(0);
            println("port write 0");
          }
          myPort.write(screenX);
          myPort.write(screenY1);
          myPort.write(screenY2);
  //        print(p2d.x);
  //        println("\t" + screenX);
  //        print(p2d.y);
  //        println("\t" + screenY1 + "\t" + screenY2);
          println(screenX + "\t" + screenY + "\t" + counter++);
        }
    }        
  }
}


// -----------------------------------------------------------------
// hand events

void onNewHand(SimpleOpenNI curContext,int handId,PVector pos)
{
  println("onNewHand - handId: " + handId + ", pos: " + pos);
 
  ArrayList<PVector> vecList = new ArrayList<PVector>();
  vecList.add(pos);
  
  handPathList.put(handId,vecList);
}

void onTrackedHand(SimpleOpenNI curContext,int handId,PVector pos)
{
  //println("onTrackedHand - handId: " + handId + ", pos: " + pos );
  
  ArrayList<PVector> vecList = handPathList.get(handId);
  if(vecList != null)
  {
    vecList.add(0,pos);
    if(vecList.size() >= handVecListSize)
      // remove the last point 
      vecList.remove(vecList.size()-1); 
  }  
}

void onLostHand(SimpleOpenNI curContext,int handId)
{
  println("onLostHand - handId: " + handId);
  handPathList.remove(handId);
}

// -----------------------------------------------------------------
// gesture events

void onCompletedGesture(SimpleOpenNI curContext,int gestureType, PVector pos)
{
  println("onCompletedGesture - gestureType: " + gestureType + ", pos: " + pos);
  
  int handId = context.startTrackingHand(pos);
  println("hand stracked: " + handId);
}

// -----------------------------------------------------------------
// Keyboard event
void keyPressed()
{

  switch(key)
  {
  case ' ':
    context.setMirror(!context.mirror());
    break;
  case '1':
    context.setMirror(true);
    break;
  case '2':
    context.setMirror(false);
    break;
  }
}
